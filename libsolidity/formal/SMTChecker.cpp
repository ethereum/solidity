/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libsolidity/formal/SMTChecker.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/formal/SMTPortfolio.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libdevcore/StringUtils.h>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

SMTChecker::SMTChecker(ErrorReporter& _errorReporter, map<h256, string> const& _smtlib2Responses):
	m_interface(make_unique<smt::SMTPortfolio>(_smtlib2Responses)),
	m_errorReporterReference(_errorReporter),
	m_errorReporter(m_smtErrors),
	m_context(*m_interface)
{
#if defined (HAVE_Z3) || defined (HAVE_CVC4)
	if (!_smtlib2Responses.empty())
		m_errorReporter.warning(
			"SMT-LIB2 query responses were given in the auxiliary input, "
			"but this Solidity binary uses an SMT solver (Z3/CVC4) directly."
			"These responses will be ignored."
			"Consider disabling Z3/CVC4 at compilation time in order to use SMT-LIB2 responses."
		);
#endif
}

void SMTChecker::analyze(SourceUnit const& _source, shared_ptr<Scanner> const& _scanner)
{
	m_scanner = _scanner;
	if (_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		_source.accept(*this);

	solAssert(m_interface->solvers() > 0, "");
	// If this check is true, Z3 and CVC4 are not available
	// and the query answers were not provided, since SMTPortfolio
	// guarantees that SmtLib2Interface is the first solver.
	if (!m_interface->unhandledQueries().empty() && m_interface->solvers() == 1)
	{
		if (!m_noSolverWarning)
		{
			m_noSolverWarning = true;
			m_errorReporterReference.warning(
				SourceLocation(),
				"SMTChecker analysis was not possible since no integrated SMT solver (Z3 or CVC4) was found."
			);
		}
	}
	else
		m_errorReporterReference.append(m_errorReporter.errors());
	m_errorReporter.clear();
}

bool SMTChecker::visit(ContractDefinition const& _contract)
{
	for (auto const& contract: _contract.annotation().linearizedBaseContracts)
		for (auto var: contract->stateVariables())
			if (*contract == _contract || var->isVisibleInDerivedContracts())
				createVariable(*var);
	return true;
}

void SMTChecker::endVisit(ContractDefinition const&)
{
	m_variables.clear();
}

void SMTChecker::endVisit(VariableDeclaration const& _varDecl)
{
	if (_varDecl.isLocalVariable() && _varDecl.type()->isValueType() &&_varDecl.value())
		assignment(_varDecl, *_varDecl.value(), _varDecl.location());
}

bool SMTChecker::visit(ModifierDefinition const&)
{
	return false;
}

bool SMTChecker::visit(FunctionDefinition const& _function)
{
	// Not visited by a function call
	if (m_callStack.empty())
	{
		m_interface->reset();
		m_context.reset();
		m_pathConditions.clear();
		m_callStack.clear();
		pushCallStack({&_function, nullptr});
		m_expressions.clear();
		m_globalContext.clear();
		m_uninterpretedTerms.clear();
		m_overflowTargets.clear();
		resetStateVariables();
		initializeLocalVariables(_function);
		m_loopExecutionHappened = false;
		m_arrayAssignmentHappened = false;
		m_externalFunctionCallHappened = false;
	}
	m_modifierDepthStack.push_back(-1);
	if (_function.isConstructor())
	{
		m_errorReporter.warning(
			_function.location(),
			"Assertion checker does not yet support constructors."
		);
	}
	else
	{
		_function.parameterList().accept(*this);
		if (_function.returnParameterList())
			_function.returnParameterList()->accept(*this);
		visitFunctionOrModifier();
	}
	return false;
}

void SMTChecker::visitFunctionOrModifier()
{
	solAssert(!m_callStack.empty(), "");
	solAssert(!m_modifierDepthStack.empty(), "");

	++m_modifierDepthStack.back();
	FunctionDefinition const& function = dynamic_cast<FunctionDefinition const&>(*m_callStack.back().first);

	if (m_modifierDepthStack.back() == int(function.modifiers().size()))
	{
		if (function.isImplemented())
			function.body().accept(*this);
	}
	else
	{
		solAssert(m_modifierDepthStack.back() < int(function.modifiers().size()), "");
		ASTPointer<ModifierInvocation> const& modifierInvocation = function.modifiers()[m_modifierDepthStack.back()];
		solAssert(modifierInvocation, "");
		modifierInvocation->accept(*this);
		auto const& modifierDef = dynamic_cast<ModifierDefinition const&>(
			*modifierInvocation->name()->annotation().referencedDeclaration
		);
		vector<smt::Expression> modifierArgsExpr;
		if (modifierInvocation->arguments())
			for (auto arg: *modifierInvocation->arguments())
				modifierArgsExpr.push_back(expr(*arg));
		initializeFunctionCallParameters(modifierDef, modifierArgsExpr);
		pushCallStack({&modifierDef, modifierInvocation.get()});
		modifierDef.body().accept(*this);
		popCallStack();
	}

	--m_modifierDepthStack.back();
}

bool SMTChecker::visit(PlaceholderStatement const&)
{
	solAssert(!m_callStack.empty(), "");
	auto lastCall = popCallStack();
	visitFunctionOrModifier();
	pushCallStack(lastCall);
	return true;
}

void SMTChecker::endVisit(FunctionDefinition const&)
{
	m_callStack.pop_back();
	solAssert(m_modifierDepthStack.back() == -1, "");
	m_modifierDepthStack.pop_back();
	// If _function was visited from a function call we don't remove
	// the local variables just yet, since we might need them for
	// future calls.
	// Otherwise we remove any local variables from the context and
	// keep the state variables.
	if (m_callStack.empty())
	{
		checkUnderOverflow();
		solAssert(m_callStack.empty(), "");
	}
}

bool SMTChecker::visit(InlineAssembly const& _inlineAsm)
{
	m_errorReporter.warning(
		_inlineAsm.location(),
		"Assertion checker does not support inline assembly."
	);
	return false;
}

bool SMTChecker::visit(IfStatement const& _node)
{
	_node.condition().accept(*this);

	// We ignore called functions here because they have
	// specific input values.
	if (isRootFunction())
		checkBooleanNotConstant(_node.condition(), "Condition is always $VALUE.");

	auto indicesEndTrue = visitBranch(&_node.trueStatement(), expr(_node.condition()));
	auto touchedVars = touchedVariables(_node.trueStatement());
	decltype(indicesEndTrue) indicesEndFalse;
	if (_node.falseStatement())
	{
		indicesEndFalse = visitBranch(_node.falseStatement(), !expr(_node.condition()));
		touchedVars += touchedVariables(*_node.falseStatement());
	}
	else
		indicesEndFalse = copyVariableIndices();

	mergeVariables(touchedVars, expr(_node.condition()), indicesEndTrue, indicesEndFalse);

	return false;
}

// Here we consider the execution of two branches:
// Branch 1 assumes the loop condition to be true and executes the loop once,
// after resetting touched variables.
// Branch 2 assumes the loop condition to be false and skips the loop after
// visiting the condition (it might contain side-effects, they need to be considered)
// and does not erase knowledge.
// If the loop is a do-while, condition side-effects are lost since the body,
// executed once before the condition, might reassign variables.
// Variables touched by the loop are merged with Branch 2.
bool SMTChecker::visit(WhileStatement const& _node)
{
	auto indicesBeforeLoop = copyVariableIndices();
	auto touchedVars = touchedVariables(_node);
	resetVariables(touchedVars);
	decltype(indicesBeforeLoop) indicesAfterLoop;
	if (_node.isDoWhile())
	{
		indicesAfterLoop = visitBranch(&_node.body());
		// TODO the assertions generated in the body should still be active in the condition
		_node.condition().accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(_node.condition(), "Do-while loop condition is always $VALUE.");
	}
	else
	{
		_node.condition().accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(_node.condition(), "While loop condition is always $VALUE.");

		indicesAfterLoop = visitBranch(&_node.body(), expr(_node.condition()));
	}

	// We reset the execution to before the loop
	// and visit the condition in case it's not a do-while.
	// A do-while's body might have non-precise information
	// in its first run about variables that are touched.
	resetVariableIndices(indicesBeforeLoop);
	if (!_node.isDoWhile())
		_node.condition().accept(*this);

	mergeVariables(touchedVars, expr(_node.condition()), indicesAfterLoop, copyVariableIndices());

	m_loopExecutionHappened = true;
	return false;
}

// Here we consider the execution of two branches similar to WhileStatement.
bool SMTChecker::visit(ForStatement const& _node)
{
	if (_node.initializationExpression())
		_node.initializationExpression()->accept(*this);

	auto indicesBeforeLoop = copyVariableIndices();

	// Do not reset the init expression part.
	auto touchedVars = touchedVariables(_node.body());
	if (_node.condition())
		touchedVars += touchedVariables(*_node.condition());
	if (_node.loopExpression())
		touchedVars += touchedVariables(*_node.loopExpression());

	resetVariables(touchedVars);

	if (_node.condition())
	{
		_node.condition()->accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(*_node.condition(), "For loop condition is always $VALUE.");
	}

	m_interface->push();
	if (_node.condition())
		m_interface->addAssertion(expr(*_node.condition()));
	_node.body().accept(*this);
	if (_node.loopExpression())
		_node.loopExpression()->accept(*this);
	m_interface->pop();

	auto indicesAfterLoop = copyVariableIndices();
	// We reset the execution to before the loop
	// and visit the condition.
	resetVariableIndices(indicesBeforeLoop);
	if (_node.condition())
		_node.condition()->accept(*this);

	auto forCondition = _node.condition() ? expr(*_node.condition()) : smt::Expression(true);
	mergeVariables(touchedVars, forCondition, indicesAfterLoop, copyVariableIndices());

	m_loopExecutionHappened = true;
	return false;
}

void SMTChecker::endVisit(VariableDeclarationStatement const& _varDecl)
{
	if (_varDecl.declarations().size() != 1)
	{
		if (auto init = _varDecl.initialValue())
		{
			auto symbTuple = dynamic_pointer_cast<SymbolicTupleVariable>(m_expressions[init]);
			/// symbTuple == nullptr if it is the return of a non-inlined function call.
			if (symbTuple)
			{
				auto const& components = symbTuple->components();
				auto const& declarations = _varDecl.declarations();
				for (unsigned i = 0; i < declarations.size(); ++i)
				{
					solAssert(components.at(i), "");
					if (declarations.at(i) && knownVariable(*declarations.at(i)))
						assignment(*declarations.at(i), components.at(i)->currentValue(), declarations.at(i)->location());
				}
			}
		}
	}
	else if (knownVariable(*_varDecl.declarations().front()))
	{
		if (_varDecl.initialValue())
			assignment(*_varDecl.declarations().front(), *_varDecl.initialValue(), _varDecl.location());
	}
	else
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet implement such variable declarations."
		);

}

void SMTChecker::endVisit(Assignment const& _assignment)
{
	static set<Token> const compoundOps{
		Token::AssignAdd,
		Token::AssignSub,
		Token::AssignMul,
		Token::AssignDiv,
		Token::AssignMod
	};
	Token op = _assignment.assignmentOperator();
	if (op != Token::Assign && !compoundOps.count(op))
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement this assignment operator."
		);
	else if (!isSupportedType(_assignment.annotation().type->category()))
	{
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement type " + _assignment.annotation().type->toString()
		);
		// Give it a new index anyway to keep the SSA scheme sound.
		if (auto varDecl = identifierToVariable(_assignment.leftHandSide()))
			newValue(*varDecl);
	}
	else
	{
		vector<smt::Expression> rightArguments;
		if (_assignment.rightHandSide().annotation().type->category() == Type::Category::Tuple)
		{
			auto symbTuple = dynamic_pointer_cast<SymbolicTupleVariable>(m_expressions[&_assignment.rightHandSide()]);
			solAssert(symbTuple, "");
			for (auto const& component: symbTuple->components())
			{
				/// Right hand side tuple component cannot be empty.
				solAssert(component, "");
				rightArguments.push_back(component->currentValue());
			}
		}
		else
		{
			auto rightHandSide = compoundOps.count(op) ?
				compoundAssignment(_assignment) :
				expr(_assignment.rightHandSide());
			defineExpr(_assignment, rightHandSide);
			rightArguments.push_back(expr(_assignment));
		}
		assignment(
			_assignment.leftHandSide(),
			rightArguments,
			_assignment.annotation().type,
			_assignment.location()
		);
	}
}

void SMTChecker::endVisit(TupleExpression const& _tuple)
{
	if (_tuple.isInlineArray())
		m_errorReporter.warning(
			_tuple.location(),
			"Assertion checker does not yet implement inline arrays."
		);
	else if (_tuple.annotation().type->category() == Type::Category::Tuple)
	{
		createExpr(_tuple);
		vector<shared_ptr<SymbolicVariable>> components;
		for (auto const& component: _tuple.components())
			if (component)
			{
				if (auto varDecl = identifierToVariable(*component))
					components.push_back(m_variables[varDecl]);
				else
				{
					solAssert(knownExpr(*component), "");
					components.push_back(m_expressions[component.get()]);
				}
			}
			else
				components.push_back(nullptr);
		solAssert(components.size() == _tuple.components().size(), "");
		auto const& symbTuple = dynamic_pointer_cast<SymbolicTupleVariable>(m_expressions[&_tuple]);
		solAssert(symbTuple, "");
		symbTuple->setComponents(move(components));
	}
	else
	{
		/// Parenthesized expressions are also TupleExpression regardless their type.
		auto const& components = _tuple.components();
		solAssert(components.size() == 1, "");
		if (isSupportedType(components.front()->annotation().type->category()))
			defineExpr(_tuple, expr(*components.front()));
	}
}

void SMTChecker::addOverflowTarget(
	OverflowTarget::Type _type,
	TypePointer _intType,
	smt::Expression _value,
	SourceLocation const& _location
)
{
	m_overflowTargets.emplace_back(
		_type,
		std::move(_intType),
		std::move(_value),
		currentPathConditions(),
		_location,
		m_callStack
	);
}

void SMTChecker::checkUnderOverflow()
{
	for (auto& target: m_overflowTargets)
	{
		swap(m_callStack, target.callStack);
		if (target.type != OverflowTarget::Type::Overflow)
			checkUnderflow(target);
		if (target.type != OverflowTarget::Type::Underflow)
			checkOverflow(target);
		swap(m_callStack, target.callStack);
	}
}

void SMTChecker::checkUnderflow(OverflowTarget& _target)
{
	solAssert(_target.type != OverflowTarget::Type::Overflow, "");
	auto intType = dynamic_cast<IntegerType const*>(_target.intType);
	checkCondition(
		_target.path && _target.value < minValue(*intType),
		_target.location,
		"Underflow (resulting value less than " + formatNumberReadable(intType->minValue()) + ")",
		"<result>",
		&_target.value
	);
}

void SMTChecker::checkOverflow(OverflowTarget& _target)
{
	solAssert(_target.type != OverflowTarget::Type::Underflow, "");
	auto intType = dynamic_cast<IntegerType const*>(_target.intType);
	checkCondition(
		_target.path && _target.value > maxValue(*intType),
		_target.location,
		"Overflow (resulting value larger than " + formatNumberReadable(intType->maxValue()) + ")",
		"<result>",
		&_target.value
	);
}

void SMTChecker::endVisit(UnaryOperation const& _op)
{
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;

	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(isBool(_op.annotation().type->category()), "");
		defineExpr(_op, !expr(_op.subExpression()));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{

		solAssert(isInteger(_op.annotation().type->category()), "");
		solAssert(_op.subExpression().annotation().lValueRequested, "");
		if (auto identifier = dynamic_cast<Identifier const*>(&_op.subExpression()))
		{
			auto decl = identifierToVariable(*identifier);
			solAssert(decl, "");
			auto innerValue = currentValue(*decl);
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			assignment(*decl, newValue, _op.location());
		}
		else if (dynamic_cast<IndexAccess const*>(&_op.subExpression()))
		{
			auto innerValue = expr(_op.subExpression());
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			arrayIndexAssignment(_op.subExpression(), newValue);
		}
		else
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement such increments / decrements."
			);
		break;
	}
	case Token::Sub: // -
	{
		defineExpr(_op, 0 - expr(_op.subExpression()));
		if (_op.annotation().type->category() == Type::Category::Integer)
			addOverflowTarget(
				OverflowTarget::Type::All,
				_op.annotation().type,
				expr(_op),
				_op.location()
			);
		break;
	}
	case Token::Delete:
	{
		auto const& subExpr = _op.subExpression();
		if (auto decl = identifierToVariable(subExpr))
		{
			newValue(*decl);
			setZeroValue(*decl);
		}
		else
		{
			solAssert(knownExpr(subExpr), "");
			auto const& symbVar = m_expressions[&subExpr];
			symbVar->increaseIndex();
			setZeroValue(*symbVar);
			if (dynamic_cast<IndexAccess const*>(&_op.subExpression()))
				arrayIndexAssignment(_op.subExpression(), symbVar->currentValue());
			else
				m_errorReporter.warning(
					_op.location(),
					"Assertion checker does not yet implement \"delete\" for this expression."
				);
		}
		break;
	}
	default:
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

bool SMTChecker::visit(UnaryOperation const& _op)
{
	return !shortcutRationalNumber(_op);
}

bool SMTChecker::visit(BinaryOperation const& _op)
{
	if (shortcutRationalNumber(_op))
		return false;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
	{
		booleanOperation(_op);
		return false;
	}
	return true;
}

void SMTChecker::endVisit(BinaryOperation const& _op)
{
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
		return;

	if (TokenTraits::isArithmeticOp(_op.getOperator()))
		arithmeticOperation(_op);
	else if (TokenTraits::isCompareOp(_op.getOperator()))
		compareOperation(_op);
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
}

void SMTChecker::endVisit(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind != FunctionCallKind::Unset, "");
	if (_funCall.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	if (_funCall.annotation().kind == FunctionCallKind::TypeConversion)
	{
		visitTypeConversion(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);

	std::vector<ASTPointer<Expression const>> const args = _funCall.arguments();
	switch (funType.kind())
	{
	case FunctionType::Kind::Assert:
		visitAssert(_funCall);
		break;
	case FunctionType::Kind::Require:
		visitRequire(_funCall);
		break;
	case FunctionType::Kind::GasLeft:
		visitGasLeft(_funCall);
		break;
	case FunctionType::Kind::Internal:
	case FunctionType::Kind::External:
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::BareStaticCall:
	case FunctionType::Kind::Creation:
		internalOrExternalFunctionCall(_funCall);
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		abstractFunctionCall(_funCall);
		break;
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		auto const& memberAccess = dynamic_cast<MemberAccess const&>(_funCall.expression());
		auto const& address = memberAccess.expression();
		auto const& value = args.front();
		solAssert(value, "");

		smt::Expression thisBalance = m_context.balance();
		setSymbolicUnknownValue(thisBalance, TypeProvider::uint256(), *m_interface);
		checkCondition(thisBalance < expr(*value), _funCall.location(), "Insufficient funds", "address(this).balance", &thisBalance);

		m_context.transfer(m_context.thisAddress(), expr(address), expr(*value));
		createExpr(_funCall);
		break;
	}
	default:
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
}

void SMTChecker::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
	checkCondition(!(expr(*args[0])), _funCall.location(), "Assertion violation");
	addPathImpliedExpression(expr(*args[0]));
}

void SMTChecker::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
	if (isRootFunction())
		checkBooleanNotConstant(*args[0], "Condition is always $VALUE.");
	addPathImpliedExpression(expr(*args[0]));
}

void SMTChecker::visitGasLeft(FunctionCall const& _funCall)
{
	string gasLeft = "gasleft()";
	// We increase the variable index since gasleft changes
	// inside a tx.
	defineGlobalVariable(gasLeft, _funCall, true);
	auto const& symbolicVar = m_globalContext.at(gasLeft);
	unsigned index = symbolicVar->index();
	// We set the current value to unknown anyway to add type constraints.
	setUnknownValue(*symbolicVar);
	if (index > 0)
		m_interface->addAssertion(symbolicVar->currentValue() <= symbolicVar->valueAtIndex(index - 1));
}

void SMTChecker::inlineFunctionCall(FunctionCall const& _funCall)
{
	FunctionDefinition const* funDef = inlinedFunctionCallToDefinition(_funCall);
	if (!funDef)
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
	else if (visitedFunction(funDef))
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not support recursive function calls.",
			SecondarySourceLocation().append("Starting from function:", funDef->location())
		);
	else
	{
		vector<smt::Expression> funArgs;
		Expression const* calledExpr = &_funCall.expression();
		auto const& funType = dynamic_cast<FunctionType const*>(calledExpr->annotation().type);
		solAssert(funType, "");

		if (funType->bound())
		{
			auto const& boundFunction = dynamic_cast<MemberAccess const*>(calledExpr);
			solAssert(boundFunction, "");
			funArgs.push_back(expr(boundFunction->expression()));
		}

		for (auto arg: _funCall.arguments())
			funArgs.push_back(expr(*arg));
		initializeFunctionCallParameters(*funDef, funArgs);

		// The reason why we need to pushCallStack here instead of visit(FunctionDefinition)
		// is that there we don't have `_funCall`.
		pushCallStack({funDef, &_funCall});
		funDef->accept(*this);
		// The callstack entry is popped only in endVisit(FunctionDefinition) instead of here
		// as well to avoid code duplication (not all entries are from inlined function calls).

		createExpr(_funCall);
		auto const& returnParams = funDef->returnParameters();
		if (returnParams.size() > 1)
		{
			vector<shared_ptr<SymbolicVariable>> components;
			for (auto param: returnParams)
			{
				solAssert(m_variables[param.get()], "");
				components.push_back(m_variables[param.get()]);
			}
			auto const& symbTuple = dynamic_pointer_cast<SymbolicTupleVariable>(m_expressions[&_funCall]);
			solAssert(symbTuple, "");
			symbTuple->setComponents(move(components));
		}
		else if (returnParams.size() == 1)
			defineExpr(_funCall, currentValue(*returnParams.front()));
	}
}

void SMTChecker::internalOrExternalFunctionCall(FunctionCall const& _funCall)
{
	auto funDef = inlinedFunctionCallToDefinition(_funCall);
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (funDef)
		inlineFunctionCall(_funCall);
	else if (funType.kind() == FunctionType::Kind::Internal)
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	else
	{
		m_externalFunctionCallHappened = true;
		resetStateVariables();
		resetStorageReferences();
	}
}

void SMTChecker::abstractFunctionCall(FunctionCall const& _funCall)
{
	vector<smt::Expression> smtArguments;
	for (auto const& arg: _funCall.arguments())
		smtArguments.push_back(expr(*arg));
	defineExpr(_funCall, (*m_expressions.at(&_funCall.expression()))(smtArguments));
	m_uninterpretedTerms.insert(&_funCall);
	setSymbolicUnknownValue(expr(_funCall), _funCall.annotation().type, *m_interface);
}

void SMTChecker::endVisit(Identifier const& _identifier)
{
	if (_identifier.annotation().lValueRequested)
	{
		// Will be translated as part of the node that requested the lvalue.
	}
	else if (_identifier.annotation().type->category() == Type::Category::Function)
		visitFunctionIdentifier(_identifier);
	else if (isSupportedType(_identifier.annotation().type->category()))
	{
		if (auto decl = identifierToVariable(_identifier))
			defineExpr(_identifier, currentValue(*decl));
		else if (_identifier.name() == "now")
			defineGlobalVariable(_identifier.name(), _identifier);
		else if (_identifier.name() == "this")
		{
			defineExpr(_identifier, m_context.thisAddress());
			m_uninterpretedTerms.insert(&_identifier);
		}
		else
			// TODO: handle MagicVariableDeclaration here
			m_errorReporter.warning(
				_identifier.location(),
				"Assertion checker does not yet support the type of this variable."
			);
	}
}

void SMTChecker::visitTypeConversion(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind == FunctionCallKind::TypeConversion, "");
	solAssert(_funCall.arguments().size() == 1, "");
	auto argument = _funCall.arguments().front();
	unsigned argSize = argument->annotation().type->storageBytes();
	unsigned castSize = _funCall.annotation().type->storageBytes();
	if (argSize == castSize)
		defineExpr(_funCall, expr(*argument));
	else
	{
		createExpr(_funCall);
		setUnknownValue(*m_expressions.at(&_funCall));
		auto const& funCallCategory = _funCall.annotation().type->category();
		// TODO: truncating and bytesX needs a different approach because of right padding.
		if (funCallCategory == Type::Category::Integer || funCallCategory == Type::Category::Address)
		{
			if (argSize < castSize)
				defineExpr(_funCall, expr(*argument));
			else
			{
				auto const& intType = dynamic_cast<IntegerType const&>(*m_expressions.at(&_funCall)->type());
				defineExpr(_funCall, smt::Expression::ite(
					expr(*argument) >= minValue(intType) && expr(*argument) <= maxValue(intType),
					expr(*argument),
					expr(_funCall)
				));
			}
		}

		m_errorReporter.warning(
			_funCall.location(),
			"Type conversion is not yet fully supported and might yield false positives."
		);
	}
}

void SMTChecker::visitFunctionIdentifier(Identifier const& _identifier)
{
	auto const& fType = dynamic_cast<FunctionType const&>(*_identifier.annotation().type);
	if (fType.returnParameterTypes().size() == 1)
	{
		defineGlobalFunction(fType.richIdentifier(), _identifier);
		m_expressions.emplace(&_identifier, m_globalContext.at(fType.richIdentifier()));
	}
}

void SMTChecker::endVisit(Literal const& _literal)
{
	solAssert(_literal.annotation().type, "Expected type for AST node");
	Type const& type = *_literal.annotation().type;
	if (isNumber(type.category()))
		defineExpr(_literal, smt::Expression(type.literalValue(&_literal)));
	else if (isBool(type.category()))
		defineExpr(_literal, smt::Expression(_literal.token() == Token::TrueLiteral ? true : false));
	else
	{
		if (type.category() == Type::Category::StringLiteral)
		{
			auto stringType = TypeProvider::stringMemory();
			auto stringLit = dynamic_cast<StringLiteralType const*>(_literal.annotation().type);
			solAssert(stringLit, "");
			auto result = newSymbolicVariable(*stringType, stringLit->richIdentifier(), *m_interface);
			m_expressions.emplace(&_literal, result.second);
		}
		m_errorReporter.warning(
			_literal.location(),
			"Assertion checker does not yet support the type of this literal (" +
			_literal.annotation().type->toString() +
			")."
		);
	}
}

void SMTChecker::endVisit(Return const& _return)
{
	if (_return.expression() && knownExpr(*_return.expression()))
	{
		auto returnParams = m_callStack.back().first->returnParameters();
		if (returnParams.size() > 1)
		{
			auto tuple = dynamic_cast<TupleExpression const*>(_return.expression());
			solAssert(tuple, "");
			auto const& components = tuple->components();
			solAssert(components.size() == returnParams.size(), "");
			for (unsigned i = 0; i < returnParams.size(); ++i)
				if (components.at(i))
					m_interface->addAssertion(expr(*components.at(i)) == newValue(*returnParams.at(i)));
		}
		else if (returnParams.size() == 1)
			m_interface->addAssertion(expr(*_return.expression()) == newValue(*returnParams.front()));
	}
}

bool SMTChecker::visit(MemberAccess const& _memberAccess)
{
	auto const& accessType = _memberAccess.annotation().type;
	if (accessType->category() == Type::Category::Function)
		return true;

	auto const& exprType = _memberAccess.expression().annotation().type;
	solAssert(exprType, "");
	auto identifier = dynamic_cast<Identifier const*>(&_memberAccess.expression());
	if (exprType->category() == Type::Category::Magic)
	{
		string accessedName;
		if (identifier)
			accessedName = identifier->name();
		else
			m_errorReporter.warning(
				_memberAccess.location(),
				"Assertion checker does not yet support this expression."
			);
		defineGlobalVariable(accessedName + "." + _memberAccess.memberName(), _memberAccess);
		return false;
	}
	else if (exprType->category() == Type::Category::TypeType)
	{
		if (identifier && dynamic_cast<EnumDefinition const*>(identifier->annotation().referencedDeclaration))
		{
			auto enumType = dynamic_cast<EnumType const*>(accessType);
			solAssert(enumType, "");
			defineExpr(_memberAccess, enumType->memberValue(_memberAccess.memberName()));
		}
		return false;
	}
	else if (exprType->category() == Type::Category::Address)
	{
		_memberAccess.expression().accept(*this);
		if (_memberAccess.memberName() == "balance")
		{
			defineExpr(_memberAccess, m_context.balance(expr(_memberAccess.expression())));
			setSymbolicUnknownValue(*m_expressions[&_memberAccess], *m_interface);
			m_uninterpretedTerms.insert(&_memberAccess);
			return false;
		}
	}
	else
		m_errorReporter.warning(
			_memberAccess.location(),
			"Assertion checker does not yet support this expression."
		);

	createExpr(_memberAccess);
	return true;
}

void SMTChecker::endVisit(IndexAccess const& _indexAccess)
{
	shared_ptr<SymbolicVariable> array;
	if (auto const& id = dynamic_cast<Identifier const*>(&_indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");
		array = m_variables[varDecl];
	}
	else if (auto const& innerAccess = dynamic_cast<IndexAccess const*>(&_indexAccess.baseExpression()))
	{
		solAssert(knownExpr(*innerAccess), "");
		array = m_expressions[innerAccess];
	}
	else
	{
		m_errorReporter.warning(
			_indexAccess.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	solAssert(array, "");
	defineExpr(_indexAccess, smt::Expression::select(
		array->currentValue(),
		expr(*_indexAccess.indexExpression())
	));
	setSymbolicUnknownValue(
		expr(_indexAccess),
		_indexAccess.annotation().type,
		*m_interface
	);
	m_uninterpretedTerms.insert(&_indexAccess);
}

void SMTChecker::arrayAssignment()
{
	m_arrayAssignmentHappened = true;
}

void SMTChecker::arrayIndexAssignment(Expression const& _expr, smt::Expression const& _rightHandSide)
{
	auto const& indexAccess = dynamic_cast<IndexAccess const&>(_expr);
	if (auto const& id = dynamic_cast<Identifier const*>(&indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");

		if (varDecl->hasReferenceOrMappingType())
			resetVariables([&](VariableDeclaration const& _var) {
				if (_var == *varDecl)
					return false;
				TypePointer prefix = _var.type();
				TypePointer originalType = typeWithoutPointer(varDecl->type());
				while (
					prefix->category() == Type::Category::Mapping ||
					prefix->category() == Type::Category::Array
				)
				{
					if (*originalType == *typeWithoutPointer(prefix))
						return true;
					if (prefix->category() == Type::Category::Mapping)
					{
						auto mapPrefix = dynamic_cast<MappingType const*>(prefix);
						solAssert(mapPrefix, "");
						prefix = mapPrefix->valueType();
					}
					else
					{
						auto arrayPrefix = dynamic_cast<ArrayType const*>(prefix);
						solAssert(arrayPrefix, "");
						prefix = arrayPrefix->baseType();
					}
				}
				return false;
			});

		smt::Expression store = smt::Expression::store(
			m_variables[varDecl]->currentValue(),
			expr(*indexAccess.indexExpression()),
			_rightHandSide
		);
		m_interface->addAssertion(newValue(*varDecl) == store);
		// Update the SMT select value after the assignment,
		// necessary for sound models.
		defineExpr(indexAccess, smt::Expression::select(
			m_variables[varDecl]->currentValue(),
			expr(*indexAccess.indexExpression())
		));
	}
	else if (dynamic_cast<IndexAccess const*>(&indexAccess.baseExpression()))
		m_errorReporter.warning(
			indexAccess.location(),
			"Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays."
		);
	else
		m_errorReporter.warning(
			_expr.location(),
			"Assertion checker does not yet implement this expression."
		);
}

void SMTChecker::defineGlobalVariable(string const& _name, Expression const& _expr, bool _increaseIndex)
{
	if (!knownGlobalSymbol(_name))
	{
		auto result = newSymbolicVariable(*_expr.annotation().type, _name, *m_interface);
		m_globalContext.emplace(_name, result.second);
		setUnknownValue(*result.second);
		if (result.first)
			m_errorReporter.warning(
				_expr.location(),
				"Assertion checker does not yet support this global variable."
			);
	}
	else if (_increaseIndex)
		m_globalContext.at(_name)->increaseIndex();
	// The default behavior is not to increase the index since
	// most of the global values stay the same throughout a tx.
	if (isSupportedType(_expr.annotation().type->category()))
		defineExpr(_expr, m_globalContext.at(_name)->currentValue());
}

void SMTChecker::defineGlobalFunction(string const& _name, Expression const& _expr)
{
	if (!knownGlobalSymbol(_name))
	{
		auto result = newSymbolicVariable(*_expr.annotation().type, _name, *m_interface);
		m_globalContext.emplace(_name, result.second);
		if (result.first)
			m_errorReporter.warning(
				_expr.location(),
				"Assertion checker does not yet support the type of this function."
			);
	}
}

bool SMTChecker::shortcutRationalNumber(Expression const& _expr)
{
	if (_expr.annotation().type->category() == Type::Category::RationalNumber)
	{
		auto rationalType = dynamic_cast<RationalNumberType const*>(_expr.annotation().type);
		solAssert(rationalType, "");
		if (rationalType->isNegative())
			defineExpr(_expr, smt::Expression(u2s(rationalType->literalValue(nullptr))));
		else
			defineExpr(_expr, smt::Expression(rationalType->literalValue(nullptr)));
		return true;
	}
	return false;
}

void SMTChecker::arithmeticOperation(BinaryOperation const& _op)
{
	auto type = _op.annotation().commonType;
	solAssert(type, "");
	if (type->category() == Type::Category::Integer)
	{
		switch (_op.getOperator())
		{
		case Token::Add:
		case Token::Sub:
		case Token::Mul:
		case Token::Div:
		case Token::Mod:
		{
			defineExpr(_op, arithmeticOperation(
				_op.getOperator(),
				expr(_op.leftExpression()),
				expr(_op.rightExpression()),
				_op.annotation().commonType,
				_op.location()
			));
			break;
		}
		default:
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement this operator."
			);
		}
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator for type " + type->richIdentifier() + "."
		);
}

smt::Expression SMTChecker::arithmeticOperation(
	Token _op,
	smt::Expression const& _left,
	smt::Expression const& _right,
	TypePointer const& _commonType,
	langutil::SourceLocation const& _location
)
{
	static set<Token> validOperators{
		Token::Add,
		Token::Sub,
		Token::Mul,
		Token::Div,
		Token::Mod
	};
	solAssert(validOperators.count(_op), "");
	solAssert(_commonType, "");
	solAssert(_commonType->category() == Type::Category::Integer, "");

	auto const& intType = dynamic_cast<IntegerType const&>(*_commonType);
	smt::Expression value(
		_op == Token::Add ? _left + _right :
		_op == Token::Sub ? _left - _right :
		_op == Token::Div ? division(_left, _right, intType) :
		_op == Token::Mul ? _left * _right :
		/*_op == Token::Mod*/ _left % _right
	);

	if (_op == Token::Div || _op == Token::Mod)
	{
		checkCondition(_right == 0, _location, "Division by zero", "<result>", &_right);
		m_interface->addAssertion(_right != 0);
	}

	addOverflowTarget(
		OverflowTarget::Type::All,
		_commonType,
		value,
		_location
	);

	smt::Expression intValueRange = (0 - minValue(intType)) + maxValue(intType) + 1;
	value = smt::Expression::ite(
		value > maxValue(intType) || value < minValue(intType),
		value % intValueRange,
		value
	);
	if (intType.isSigned())
	{
		value = smt::Expression::ite(
			value > maxValue(intType),
			value - intValueRange,
			value
		);
	}

	return value;
}

void SMTChecker::compareOperation(BinaryOperation const& _op)
{
	solAssert(_op.annotation().commonType, "");
	if (isSupportedType(_op.annotation().commonType->category()))
	{
		smt::Expression left(expr(_op.leftExpression()));
		smt::Expression right(expr(_op.rightExpression()));
		Token op = _op.getOperator();
		shared_ptr<smt::Expression> value;
		if (isNumber(_op.annotation().commonType->category()))
		{
			value = make_shared<smt::Expression>(
				op == Token::Equal ? (left == right) :
				op == Token::NotEqual ? (left != right) :
				op == Token::LessThan ? (left < right) :
				op == Token::LessThanOrEqual ? (left <= right) :
				op == Token::GreaterThan ? (left > right) :
				/*op == Token::GreaterThanOrEqual*/ (left >= right)
			);
		}
		else // Bool
		{
			solUnimplementedAssert(isBool(_op.annotation().commonType->category()), "Operation not yet supported");
			value = make_shared<smt::Expression>(
				op == Token::Equal ? (left == right) :
				/*op == Token::NotEqual*/ (left != right)
			);
		}
		// TODO: check that other values for op are not possible.
		defineExpr(_op, *value);
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for comparisons"
		);
}

void SMTChecker::booleanOperation(BinaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::And || _op.getOperator() == Token::Or, "");
	solAssert(_op.annotation().commonType, "");
	if (_op.annotation().commonType->category() == Type::Category::Bool)
	{
		// @TODO check that both of them are not constant
		_op.leftExpression().accept(*this);
		if (_op.getOperator() == Token::And)
		{
			auto indicesAfterSecond = visitBranch(&_op.rightExpression(), expr(_op.leftExpression()));
			mergeVariables(touchedVariables(_op.rightExpression()), !expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
			defineExpr(_op, expr(_op.leftExpression()) && expr(_op.rightExpression()));
		}
		else
		{
			auto indicesAfterSecond = visitBranch(&_op.rightExpression(), !expr(_op.leftExpression()));
			mergeVariables(touchedVariables(_op.rightExpression()), expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
			defineExpr(_op, expr(_op.leftExpression()) || expr(_op.rightExpression()));
		}
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for boolean operations"
		);
}

smt::Expression SMTChecker::division(smt::Expression _left, smt::Expression _right, IntegerType const& _type)
{
	// Signed division in SMTLIB2 rounds differently for negative division.
	if (_type.isSigned())
		return (smt::Expression::ite(
			_left >= 0,
			smt::Expression::ite(_right >= 0, _left / _right, 0 - (_left / (0 - _right))),
			smt::Expression::ite(_right >= 0, 0 - ((0 - _left) / _right), (0 - _left) / (0 - _right))
		));
	else
		return _left / _right;
}

void SMTChecker::assignment(
	Expression const& _left,
	vector<smt::Expression> const& _right,
	TypePointer const& _type,
	langutil::SourceLocation const& _location
)
{
	if (!isSupportedType(_type->category()))
		m_errorReporter.warning(
			_location,
			"Assertion checker does not yet implement type " + _type->toString()
		);
	else if (auto varDecl = identifierToVariable(_left))
	{
		solAssert(_right.size() == 1, "");
		assignment(*varDecl, _right.front(), _location);
	}
	else if (dynamic_cast<IndexAccess const*>(&_left))
	{
		solAssert(_right.size() == 1, "");
		arrayIndexAssignment(_left, _right.front());
	}
	else if (auto tuple = dynamic_cast<TupleExpression const*>(&_left))
	{
		auto const& components = tuple->components();
		solAssert(_right.size() == components.size(), "");
		for (unsigned i = 0; i < _right.size(); ++i)
			if (auto component = components.at(i))
				assignment(*component, {_right.at(i)}, component->annotation().type, component->location());
	}
	else
		m_errorReporter.warning(
			_location,
			"Assertion checker does not yet implement such assignments."
		);
}

smt::Expression SMTChecker::compoundAssignment(Assignment const& _assignment)
{
	static map<Token, Token> const compoundToArithmetic{
		{Token::AssignAdd, Token::Add},
		{Token::AssignSub, Token::Sub},
		{Token::AssignMul, Token::Mul},
		{Token::AssignDiv, Token::Div},
		{Token::AssignMod, Token::Mod}
	};
	Token op = _assignment.assignmentOperator();
	solAssert(compoundToArithmetic.count(op), "");
	auto decl = identifierToVariable(_assignment.leftHandSide());
	return arithmeticOperation(
		compoundToArithmetic.at(op),
		decl ? currentValue(*decl) : expr(_assignment.leftHandSide()),
		expr(_assignment.rightHandSide()),
		_assignment.annotation().type,
		_assignment.location()
	);
}

void SMTChecker::assignment(VariableDeclaration const& _variable, Expression const& _value, SourceLocation const& _location)
{
	assignment(_variable, expr(_value), _location);
}

void SMTChecker::assignment(VariableDeclaration const& _variable, smt::Expression const& _value, SourceLocation const& _location)
{
	TypePointer type = _variable.type();
	if (type->category() == Type::Category::Integer)
		addOverflowTarget(OverflowTarget::Type::All, type,	_value,	_location);
	else if (type->category() == Type::Category::Address)
		addOverflowTarget(OverflowTarget::Type::All, TypeProvider::uint(160), _value, _location);
	else if (type->category() == Type::Category::Mapping)
		arrayAssignment();
	m_interface->addAssertion(newValue(_variable) == _value);
}

SMTChecker::VariableIndices SMTChecker::visitBranch(ASTNode const* _statement, smt::Expression _condition)
{
	return visitBranch(_statement, &_condition);
}

SMTChecker::VariableIndices SMTChecker::visitBranch(ASTNode const* _statement, smt::Expression const* _condition)
{
	auto indicesBeforeBranch = copyVariableIndices();
	if (_condition)
		pushPathCondition(*_condition);
	_statement->accept(*this);
	if (_condition)
		popPathCondition();
	auto indicesAfterBranch = copyVariableIndices();
	resetVariableIndices(indicesBeforeBranch);
	return indicesAfterBranch;
}

void SMTChecker::checkCondition(
	smt::Expression _condition,
	SourceLocation const& _location,
	string const& _description,
	string const& _additionalValueName,
	smt::Expression const* _additionalValue
)
{
	m_interface->push();
	addPathConjoinedExpression(_condition);

	vector<smt::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	if (m_callStack.size())
	{
		solAssert(m_scanner, "");
		if (_additionalValue)
		{
			expressionsToEvaluate.emplace_back(*_additionalValue);
			expressionNames.push_back(_additionalValueName);
		}
		for (auto const& var: m_variables)
		{
			if (var.first->type()->isValueType())
			{
				expressionsToEvaluate.emplace_back(currentValue(*var.first));
				expressionNames.push_back(var.first->name());
			}
		}
		for (auto const& var: m_globalContext)
		{
			auto const& type = var.second->type();
			if (
				type->isValueType() &&
				smtKind(type->category()) != smt::Kind::Function
			)
			{
				expressionsToEvaluate.emplace_back(var.second->currentValue());
				expressionNames.push_back(var.first);
			}
		}
		for (auto const& uf: m_uninterpretedTerms)
		{
			if (uf->annotation().type->isValueType())
			{
				expressionsToEvaluate.emplace_back(expr(*uf));
				expressionNames.push_back(m_scanner->sourceAt(uf->location()));
			}
		}
	}
	smt::CheckResult result;
	vector<string> values;
	tie(result, values) = checkSatisfiableAndGenerateModel(expressionsToEvaluate);

	string extraComment;
	if (m_loopExecutionHappened)
		extraComment =
			"\nNote that some information is erased after the execution of loops.\n"
			"You can re-introduce information using require().";
	if (m_arrayAssignmentHappened)
		extraComment +=
			"\nNote that array aliasing is not supported,"
			" therefore all mapping information is erased after"
			" a mapping local variable/parameter is assigned.\n"
			"You can re-introduce information using require().";
	if (m_externalFunctionCallHappened)
		extraComment +=
			"\nNote that external function calls are not inlined,"
			" even if the source code of the function is available."
			" This is due to the possibility that the actual called contract"
			" has the same ABI but implements the function differently.";

	SecondarySourceLocation secondaryLocation{};
	secondaryLocation.append(extraComment, SourceLocation{});

	switch (result)
	{
	case smt::CheckResult::SATISFIABLE:
	{
		std::ostringstream message;
		message << _description << " happens here";
		if (m_callStack.size())
		{
			std::ostringstream modelMessage;
			modelMessage << "  for:\n";
			solAssert(values.size() == expressionNames.size(), "");
			map<string, string> sortedModel;
			for (size_t i = 0; i < values.size(); ++i)
				if (expressionsToEvaluate.at(i).name != values.at(i))
					sortedModel[expressionNames.at(i)] = values.at(i);

			for (auto const& eval: sortedModel)
				modelMessage << "  " << eval.first << " = " << eval.second << "\n";
			m_errorReporter.warning(
				_location,
				message.str(),
				SecondarySourceLocation().append(modelMessage.str(), SourceLocation{})
				.append(currentCallStack())
				.append(move(secondaryLocation))
			);
		}
		else
		{
			message << ".";
			m_errorReporter.warning(_location, message.str(), secondaryLocation);
		}
		break;
	}
	case smt::CheckResult::UNSATISFIABLE:
		break;
	case smt::CheckResult::UNKNOWN:
		m_errorReporter.warning(_location, _description + " might happen here.", secondaryLocation);
		break;
	case smt::CheckResult::CONFLICTING:
		m_errorReporter.warning(_location, "At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case smt::CheckResult::ERROR:
		m_errorReporter.warning(_location, "Error trying to invoke SMT solver.");
		break;
	}
	m_interface->pop();
}

void SMTChecker::checkBooleanNotConstant(Expression const& _condition, string const& _description)
{
	// Do not check for const-ness if this is a constant.
	if (dynamic_cast<Literal const*>(&_condition))
		return;

	m_interface->push();
	addPathConjoinedExpression(expr(_condition));
	auto positiveResult = checkSatisfiable();
	m_interface->pop();

	m_interface->push();
	addPathConjoinedExpression(!expr(_condition));
	auto negatedResult = checkSatisfiable();
	m_interface->pop();

	if (positiveResult == smt::CheckResult::ERROR || negatedResult == smt::CheckResult::ERROR)
		m_errorReporter.warning(_condition.location(), "Error trying to invoke SMT solver.");
	else if (positiveResult == smt::CheckResult::CONFLICTING || negatedResult == smt::CheckResult::CONFLICTING)
		m_errorReporter.warning(_condition.location(), "At least two SMT solvers provided conflicting answers. Results might not be sound.");
	else if (positiveResult == smt::CheckResult::SATISFIABLE && negatedResult == smt::CheckResult::SATISFIABLE)
	{
		// everything fine.
	}
	else if (positiveResult == smt::CheckResult::UNKNOWN || negatedResult == smt::CheckResult::UNKNOWN)
	{
		// can't do anything.
	}
	else if (positiveResult == smt::CheckResult::UNSATISFIABLE && negatedResult == smt::CheckResult::UNSATISFIABLE)
		m_errorReporter.warning(_condition.location(), "Condition unreachable.", currentCallStack());
	else
	{
		string value;
		if (positiveResult == smt::CheckResult::SATISFIABLE)
		{
			solAssert(negatedResult == smt::CheckResult::UNSATISFIABLE, "");
			value = "true";
		}
		else
		{
			solAssert(positiveResult == smt::CheckResult::UNSATISFIABLE, "");
			solAssert(negatedResult == smt::CheckResult::SATISFIABLE, "");
			value = "false";
		}
		m_errorReporter.warning(
			_condition.location(),
			boost::algorithm::replace_all_copy(_description, "$VALUE", value),
			currentCallStack()
		);
	}
}

pair<smt::CheckResult, vector<string>>
SMTChecker::checkSatisfiableAndGenerateModel(vector<smt::Expression> const& _expressionsToEvaluate)
{
	smt::CheckResult result;
	vector<string> values;
	try
	{
		tie(result, values) = m_interface->check(_expressionsToEvaluate);
	}
	catch (smt::SolverError const& _e)
	{
		string description("Error querying SMT solver");
		if (_e.comment())
			description += ": " + *_e.comment();
		m_errorReporter.warning(description);
		result = smt::CheckResult::ERROR;
	}

	for (string& value: values)
	{
		try
		{
			// Parse and re-format nicely
			value = formatNumberReadable(bigint(value));
		}
		catch (...) { }
	}

	return make_pair(result, values);
}

smt::CheckResult SMTChecker::checkSatisfiable()
{
	return checkSatisfiableAndGenerateModel({}).first;
}

void SMTChecker::initializeFunctionCallParameters(CallableDeclaration const& _function, vector<smt::Expression> const& _callArgs)
{
	auto const& funParams = _function.parameters();
	solAssert(funParams.size() == _callArgs.size(), "");
	for (unsigned i = 0; i < funParams.size(); ++i)
		if (createVariable(*funParams[i]))
		{
			m_interface->addAssertion(_callArgs[i] == newValue(*funParams[i]));
			if (funParams[i]->annotation().type->category() == Type::Category::Mapping)
				m_arrayAssignmentHappened = true;
		}

	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
		{
			newValue(*variable);
			setZeroValue(*variable);
		}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
			{
				newValue(*retParam);
				setZeroValue(*retParam);
			}
}

void SMTChecker::initializeLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
			setZeroValue(*variable);

	for (auto const& param: _function.parameters())
		if (createVariable(*param))
			setUnknownValue(*param);

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
				setZeroValue(*retParam);
}

void SMTChecker::removeLocalVariables()
{
	for (auto it = m_variables.begin(); it != m_variables.end(); )
	{
		if (it->first->isLocalVariable())
			it = m_variables.erase(it);
		else
			++it;
	}
}

void SMTChecker::resetVariable(VariableDeclaration const& _variable)
{
	newValue(_variable);
	setUnknownValue(_variable);
}

void SMTChecker::resetStateVariables()
{
	resetVariables([&](VariableDeclaration const& _variable) { return _variable.isStateVariable(); });
}

void SMTChecker::resetStorageReferences()
{
	resetVariables([&](VariableDeclaration const& _variable) { return _variable.hasReferenceOrMappingType(); });
}

void SMTChecker::resetVariables(set<VariableDeclaration const*> const& _variables)
{
	for (auto const* decl: _variables)
		resetVariable(*decl);
}

void SMTChecker::resetVariables(function<bool(VariableDeclaration const&)> const& _filter)
{
	for_each(begin(m_variables), end(m_variables), [&](auto _variable)
	{
		if (_filter(*_variable.first))
			this->resetVariable(*_variable.first);
	});
}

TypePointer SMTChecker::typeWithoutPointer(TypePointer const& _type)
{
	if (auto refType = dynamic_cast<ReferenceType const*>(_type))
		return TypeProvider::withLocationIfReference(refType->location(), _type);
	return _type;
}

void SMTChecker::mergeVariables(set<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse)
{
	auto cmp = [] (VariableDeclaration const* var1, VariableDeclaration const* var2) {
		return var1->id() < var2->id();
	};
	set<VariableDeclaration const*, decltype(cmp)> sortedVars(begin(_variables), end(_variables), cmp);
	for (auto const* decl: sortedVars)
	{
		solAssert(_indicesEndTrue.count(decl) && _indicesEndFalse.count(decl), "");
		int trueIndex = _indicesEndTrue.at(decl);
		int falseIndex = _indicesEndFalse.at(decl);
		solAssert(trueIndex != falseIndex, "");
		m_interface->addAssertion(newValue(*decl) == smt::Expression::ite(
			_condition,
			valueAtIndex(*decl, trueIndex),
			valueAtIndex(*decl, falseIndex))
		);
	}
}

bool SMTChecker::createVariable(VariableDeclaration const& _varDecl)
{
	// This might be the case for multiple calls to the same function.
	if (knownVariable(_varDecl))
		return true;
	auto const& type = _varDecl.type();
	solAssert(m_variables.count(&_varDecl) == 0, "");
	auto result = newSymbolicVariable(*type, _varDecl.name() + "_" + to_string(_varDecl.id()), *m_interface);
	m_variables.emplace(&_varDecl, result.second);
	if (result.first)
	{
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet support the type of this variable."
		);
		return false;
	}
	return true;
}

bool SMTChecker::knownVariable(VariableDeclaration const& _decl)
{
	return m_variables.count(&_decl);
}

smt::Expression SMTChecker::currentValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->currentValue();
}

smt::Expression SMTChecker::valueAtIndex(VariableDeclaration const& _decl, int _index)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->valueAtIndex(_index);
}

smt::Expression SMTChecker::newValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->increaseIndex();
}

void SMTChecker::setZeroValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setZeroValue(*m_variables.at(&_decl));
}

void SMTChecker::setZeroValue(SymbolicVariable& _variable)
{
	smt::setSymbolicZeroValue(_variable, *m_interface);
}

void SMTChecker::setUnknownValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setUnknownValue(*m_variables.at(&_decl));
}

void SMTChecker::setUnknownValue(SymbolicVariable& _variable)
{
	smt::setSymbolicUnknownValue(_variable, *m_interface);
}

smt::Expression SMTChecker::expr(Expression const& _e)
{
	if (!knownExpr(_e))
	{
		m_errorReporter.warning(_e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}
	return m_expressions.at(&_e)->currentValue();
}

bool SMTChecker::knownExpr(Expression const& _e) const
{
	return m_expressions.count(&_e);
}

bool SMTChecker::knownGlobalSymbol(string const& _var) const
{
	return m_globalContext.count(_var);
}

void SMTChecker::createExpr(Expression const& _e)
{
	solAssert(_e.annotation().type, "");
	if (knownExpr(_e))
		m_expressions.at(&_e)->increaseIndex();
	else
	{
		auto result = newSymbolicVariable(*_e.annotation().type, "expr_" + to_string(_e.id()), *m_interface);
		m_expressions.emplace(&_e, result.second);
		if (result.first)
			m_errorReporter.warning(
				_e.location(),
				"Assertion checker does not yet implement this type."
			);
	}
}

void SMTChecker::defineExpr(Expression const& _e, smt::Expression _value)
{
	createExpr(_e);
	solAssert(smtKind(_e.annotation().type->category()) != smt::Kind::Function, "Equality operator applied to type that is not fully supported");
	m_interface->addAssertion(expr(_e) == _value);
}

void SMTChecker::popPathCondition()
{
	solAssert(m_pathConditions.size() > 0, "Cannot pop path condition, empty.");
	m_pathConditions.pop_back();
}

void SMTChecker::pushPathCondition(smt::Expression const& _e)
{
	m_pathConditions.push_back(currentPathConditions() && _e);
}

smt::Expression SMTChecker::currentPathConditions()
{
	if (m_pathConditions.empty())
		return smt::Expression(true);
	return m_pathConditions.back();
}

SecondarySourceLocation SMTChecker::currentCallStack()
{
	SecondarySourceLocation callStackLocation;
	solAssert(!m_callStack.empty(), "");
	callStackLocation.append("Callstack: ", SourceLocation());
	for (auto const& call: m_callStack | boost::adaptors::reversed)
		if (call.second)
			callStackLocation.append("", call.second->location());
	// The first function in the tx has no FunctionCall.
	solAssert(m_callStack.front().second == nullptr, "");
	return callStackLocation;
}

pair<CallableDeclaration const*, ASTNode const*> SMTChecker::popCallStack()
{
	solAssert(!m_callStack.empty(), "");
	auto lastCalled = m_callStack.back();
	m_callStack.pop_back();
	return lastCalled;
}

void SMTChecker::pushCallStack(CallStackEntry _entry)
{
	m_callStack.push_back(_entry);
}

void SMTChecker::addPathConjoinedExpression(smt::Expression const& _e)
{
	m_interface->addAssertion(currentPathConditions() && _e);
}

void SMTChecker::addPathImpliedExpression(smt::Expression const& _e)
{
	m_interface->addAssertion(smt::Expression::implies(currentPathConditions(), _e));
}

bool SMTChecker::isRootFunction()
{
	return m_callStack.size() == 1;
}

bool SMTChecker::visitedFunction(FunctionDefinition const* _funDef)
{
	for (auto const& call: m_callStack)
		if (call.first == _funDef)
			return true;
	return false;
}

SMTChecker::VariableIndices SMTChecker::copyVariableIndices()
{
	VariableIndices indices;
	for (auto const& var: m_variables)
		indices.emplace(var.first, var.second->index());
	return indices;
}

void SMTChecker::resetVariableIndices(VariableIndices const& _indices)
{
	for (auto const& var: _indices)
		m_variables.at(var.first)->index() = var.second;
}

FunctionDefinition const* SMTChecker::inlinedFunctionCallToDefinition(FunctionCall const& _funCall)
{
	if (_funCall.annotation().kind != FunctionCallKind::FunctionCall)
		return nullptr;

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (funType.kind() == FunctionType::Kind::External)
	{
		auto memberAccess = dynamic_cast<MemberAccess const*>(&_funCall.expression());
		auto identifier = memberAccess ?
			dynamic_cast<Identifier const*>(&memberAccess->expression()) :
			nullptr;
		if (!(
			identifier &&
			identifier->name() == "this" &&
			identifier->annotation().referencedDeclaration &&
			dynamic_cast<MagicVariableDeclaration const*>(identifier->annotation().referencedDeclaration)
		))
			return nullptr;
	}
	else if (funType.kind() != FunctionType::Kind::Internal)
		return nullptr;

	FunctionDefinition const* funDef = nullptr;
	Expression const* calledExpr = &_funCall.expression();

	if (TupleExpression const* fun = dynamic_cast<TupleExpression const*>(&_funCall.expression()))
	{
		solAssert(fun->components().size() == 1, "");
		calledExpr = fun->components().front().get();
	}

	if (Identifier const* fun = dynamic_cast<Identifier const*>(calledExpr))
		funDef = dynamic_cast<FunctionDefinition const*>(fun->annotation().referencedDeclaration);
	else if (MemberAccess const* fun = dynamic_cast<MemberAccess const*>(calledExpr))
		funDef = dynamic_cast<FunctionDefinition const*>(fun->annotation().referencedDeclaration);

	if (funDef && funDef->isImplemented())
		return funDef;

	return nullptr;
}

set<VariableDeclaration const*> SMTChecker::touchedVariables(ASTNode const& _node)
{
	solAssert(!m_callStack.empty(), "");
	vector<CallableDeclaration const*> callStack;
	for (auto const& call: m_callStack)
		callStack.push_back(call.first);
	return m_variableUsage.touchedVariables(_node, callStack);
}

VariableDeclaration const* SMTChecker::identifierToVariable(Expression const& _expr)
{
	if (auto identifier = dynamic_cast<Identifier const*>(&_expr))
	{
		if (auto decl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
		{
			solAssert(knownVariable(*decl), "");
			return decl;
		}
	}
	return nullptr;
}
