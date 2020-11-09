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
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/formal/SymbolicState.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

#include <boost/range/adaptors.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

SMTEncoder::SMTEncoder(smt::EncodingContext& _context):
	m_errorReporter(m_smtErrors),
	m_context(_context)
{
}

bool SMTEncoder::visit(ContractDefinition const& _contract)
{
	solAssert(m_currentContract, "");

	for (auto const& node: _contract.subNodes())
		if (
			!dynamic_pointer_cast<FunctionDefinition>(node) &&
			!dynamic_pointer_cast<VariableDeclaration>(node)
		)
			node->accept(*this);

	vector<FunctionDefinition const*> resolvedFunctions = _contract.definedFunctions();
	for (auto const& base: _contract.annotation().linearizedBaseContracts)
	{
		// Look for all the constructor invocations bottom up.
		if (auto const& constructor =  base->constructor())
			for (auto const& invocation: constructor->modifiers())
			{
				auto refDecl = invocation->name()->annotation().referencedDeclaration;
				if (auto const& baseContract = dynamic_cast<ContractDefinition const*>(refDecl))
				{
					solAssert(!m_baseConstructorCalls.count(baseContract), "");
					m_baseConstructorCalls[baseContract] = invocation.get();
				}
			}

		// Check for function overrides.
		for (auto const& baseFunction: base->definedFunctions())
		{
			if (baseFunction->isConstructor())
				continue;
			bool overridden = false;
			for (auto const& function: resolvedFunctions)
				if (
					function->name() == baseFunction->name() &&
					function->kind() == baseFunction->kind() &&
					FunctionType(*function).asExternallyCallableFunction(false)->
						hasEqualParameterTypes(*FunctionType(*baseFunction).asExternallyCallableFunction(false))
				)
				{
					overridden = true;
					break;
				}
			if (!overridden)
				resolvedFunctions.push_back(baseFunction);
		}
	}

	// Functions are visited first since they might be used
	// for state variable initialization which is part of
	// the constructor.
	// Constructors are visited as part of the constructor
	// hierarchy inlining.
	for (auto const& function: resolvedFunctions)
		if (!function->isConstructor())
			function->accept(*this);

	// Constructors need to be handled by the engines separately.

	return false;
}

void SMTEncoder::endVisit(ContractDefinition const& _contract)
{
	m_context.resetAllVariables();

	m_baseConstructorCalls.clear();

	solAssert(m_currentContract == &_contract, "");
	m_currentContract = nullptr;

	if (m_callStack.empty())
		m_context.popSolver();
}

void SMTEncoder::endVisit(VariableDeclaration const& _varDecl)
{
	// State variables are handled by the constructor.
	if (_varDecl.isLocalVariable() &&_varDecl.value())
		assignment(_varDecl, *_varDecl.value());
}

bool SMTEncoder::visit(ModifierDefinition const&)
{
	return false;
}

bool SMTEncoder::visit(FunctionDefinition const& _function)
{
	m_modifierDepthStack.push_back(-1);

	if (_function.isConstructor())
		inlineConstructorHierarchy(dynamic_cast<ContractDefinition const&>(*_function.scope()));

	initializeLocalVariables(_function);

	_function.parameterList().accept(*this);
	if (_function.returnParameterList())
		_function.returnParameterList()->accept(*this);

	visitFunctionOrModifier();

	return false;
}

void SMTEncoder::visitFunctionOrModifier()
{
	solAssert(!m_callStack.empty(), "");
	solAssert(!m_modifierDepthStack.empty(), "");

	++m_modifierDepthStack.back();
	FunctionDefinition const& function = dynamic_cast<FunctionDefinition const&>(*m_callStack.back().first);

	if (m_modifierDepthStack.back() == static_cast<int>(function.modifiers().size()))
	{
		if (function.isImplemented())
			function.body().accept(*this);
	}
	else
	{
		solAssert(m_modifierDepthStack.back() < static_cast<int>(function.modifiers().size()), "");
		ASTPointer<ModifierInvocation> const& modifierInvocation =
			function.modifiers()[static_cast<size_t>(m_modifierDepthStack.back())];
		solAssert(modifierInvocation, "");
		auto refDecl = modifierInvocation->name()->annotation().referencedDeclaration;
		if (dynamic_cast<ContractDefinition const*>(refDecl))
			visitFunctionOrModifier();
		else if (auto modifierDef = dynamic_cast<ModifierDefinition const*>(refDecl))
			inlineModifierInvocation(modifierInvocation.get(), modifierDef);
		else
			solAssert(false, "");
	}

	--m_modifierDepthStack.back();
}

void SMTEncoder::inlineModifierInvocation(ModifierInvocation const* _invocation, CallableDeclaration const* _definition)
{
	solAssert(_invocation, "");
	_invocation->accept(*this);

	vector<smtutil::Expression> args;
	if (auto const* arguments = _invocation->arguments())
	{
		auto const& modifierParams = _definition->parameters();
		solAssert(modifierParams.size() == arguments->size(), "");
		for (unsigned i = 0; i < arguments->size(); ++i)
			args.push_back(expr(*arguments->at(i), modifierParams.at(i)->type()));
	}

	initializeFunctionCallParameters(*_definition, args);

	pushCallStack({_definition, _invocation});
	if (auto modifier = dynamic_cast<ModifierDefinition const*>(_definition))
	{
		if (modifier->isImplemented())
			modifier->body().accept(*this);
		popCallStack();
	}
	else if (auto function = dynamic_cast<FunctionDefinition const*>(_definition))
	{
		if (function->isImplemented())
			function->accept(*this);
		// Functions are popped from the callstack in endVisit(FunctionDefinition)
	}
}

void SMTEncoder::inlineConstructorHierarchy(ContractDefinition const& _contract)
{
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	auto it = find(begin(hierarchy), end(hierarchy), &_contract);
	solAssert(it != end(hierarchy), "");

	auto nextBase = it + 1;
	// Initialize the base contracts here as long as their constructors are implicit,
	// stop when the first explicit constructor is found.
	while (nextBase != end(hierarchy))
	{
		if (auto baseConstructor = (*nextBase)->constructor())
		{
			createLocalVariables(*baseConstructor);
			// If any subcontract explicitly called baseConstructor, use those arguments.
			if (m_baseConstructorCalls.count(*nextBase))
				inlineModifierInvocation(m_baseConstructorCalls.at(*nextBase), baseConstructor);
			else if (baseConstructor->isImplemented())
			{
				// The first constructor found is handled like a function
				// and its pushed into the callstack there.
				// This if avoids duplication in the callstack.
				if (!m_callStack.empty())
					pushCallStack({baseConstructor, nullptr});
				baseConstructor->accept(*this);
				// popped by endVisit(FunctionDefinition)
			}
			break;
		}
		else
		{
			initializeStateVariables(**nextBase);
			++nextBase;
		}
	}

	initializeStateVariables(_contract);
}

bool SMTEncoder::visit(PlaceholderStatement const&)
{
	solAssert(!m_callStack.empty(), "");
	auto lastCall = popCallStack();
	visitFunctionOrModifier();
	pushCallStack(lastCall);
	return true;
}

void SMTEncoder::endVisit(FunctionDefinition const&)
{
	popCallStack();
	solAssert(m_modifierDepthStack.back() == -1, "");
	m_modifierDepthStack.pop_back();
	if (m_callStack.empty())
		m_context.popSolver();
}

bool SMTEncoder::visit(InlineAssembly const& _inlineAsm)
{
	m_errorReporter.warning(
		7737_error,
		_inlineAsm.location(),
		"Assertion checker does not support inline assembly."
	);
	return false;
}

bool SMTEncoder::visit(TryCatchClause const& _clause)
{
	if (auto params = _clause.parameters())
		for (auto const& var: params->parameters())
			createVariable(*var);

	m_errorReporter.warning(
		7645_error,
		_clause.location(),
		"Assertion checker does not support try/catch clauses."
	);
	return false;
}

bool SMTEncoder::visit(IfStatement const& _node)
{
	_node.condition().accept(*this);

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

void SMTEncoder::endVisit(VariableDeclarationStatement const& _varDecl)
{
	if (_varDecl.declarations().size() != 1)
	{
		if (auto init = _varDecl.initialValue())
		{
			auto symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(*init));
			solAssert(symbTuple, "");
			auto const& symbComponents = symbTuple->components();

			auto tupleType = dynamic_cast<TupleType const*>(init->annotation().type);
			solAssert(tupleType, "");
			solAssert(tupleType->components().size() == symbTuple->components().size(), "");
			auto const& components = tupleType->components();

			auto const& declarations = _varDecl.declarations();
			solAssert(symbComponents.size() == declarations.size(), "");
			for (unsigned i = 0; i < declarations.size(); ++i)
				if (
					components.at(i) &&
					declarations.at(i) &&
					m_context.knownVariable(*declarations.at(i))
				)
					assignment(*declarations.at(i), symbTuple->component(i, components.at(i), declarations.at(i)->type()));
		}
	}
	else if (m_context.knownVariable(*_varDecl.declarations().front()))
	{
		if (_varDecl.initialValue())
			assignment(*_varDecl.declarations().front(), *_varDecl.initialValue());
	}
	else
		m_errorReporter.warning(
			7186_error,
			_varDecl.location(),
			"Assertion checker does not yet implement such variable declarations."
		);

}

bool SMTEncoder::visit(Assignment const& _assignment)
{
	auto const& left = _assignment.leftHandSide();
	auto const& right = _assignment.rightHandSide();

	if (auto const* memberAccess = isEmptyPush(left))
	{
		right.accept(*this);
		left.accept(*this);

		auto const& memberExpr = memberAccess->expression();
		auto& symbArray = dynamic_cast<smt::SymbolicArrayVariable&>(*m_context.expression(memberExpr));
		smtutil::Expression oldElements = symbArray.elements();
		smtutil::Expression length = symbArray.length();
		symbArray.increaseIndex();
		m_context.addAssertion(symbArray.elements() == smtutil::Expression::store(
			oldElements,
			length - 1,
			expr(right)
		));
		m_context.addAssertion(symbArray.length() == length);

		arrayPushPopAssign(memberExpr, symbArray.currentValue());
		defineExpr(_assignment, expr(left));
		return false;
	}

	return true;
}

void SMTEncoder::endVisit(Assignment const& _assignment)
{
	createExpr(_assignment);

	Token op = _assignment.assignmentOperator();
	solAssert(TokenTraits::isAssignmentOp(op), "");

	if (isEmptyPush(_assignment.leftHandSide()))
		return;

	if (!smt::isSupportedType(*_assignment.annotation().type))
	{
		// Give it a new index anyway to keep the SSA scheme sound.

		Expression const* base = &_assignment.leftHandSide();
		if (auto const* indexAccess = dynamic_cast<IndexAccess const*>(base))
			base = leftmostBase(*indexAccess);

		if (auto varDecl = identifierToVariable(*base))
			m_context.newValue(*varDecl);
	}
	else
	{
		if (dynamic_cast<TupleType const*>(_assignment.rightHandSide().annotation().type))
			tupleAssignment(_assignment.leftHandSide(), _assignment.rightHandSide());
		else
		{
			auto const& type = _assignment.annotation().type;
			auto rightHandSide = op == Token::Assign ?
				expr(_assignment.rightHandSide(), type) :
				compoundAssignment(_assignment);
			defineExpr(_assignment, rightHandSide);
			assignment(
				_assignment.leftHandSide(),
				expr(_assignment, type),
				type
			);
		}
	}
}

void SMTEncoder::endVisit(TupleExpression const& _tuple)
{
	createExpr(_tuple);

	if (_tuple.isInlineArray())
	{
		// Add constraints for the length and values as it is known.
		auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_tuple));
		solAssert(symbArray, "");

		addArrayLiteralAssertions(*symbArray, applyMap(_tuple.components(), [&](auto const& c) { return expr(*c); }));
	}
	else if (_tuple.components().size() == 1)
		defineExpr(_tuple, expr(*_tuple.components().front()));
	else
	{
		solAssert(_tuple.annotation().type->category() == Type::Category::Tuple, "");
		auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_tuple));
		solAssert(symbTuple, "");
		auto const& symbComponents = symbTuple->components();
		auto const* tuple = dynamic_cast<TupleExpression const*>(innermostTuple(_tuple));
		solAssert(tuple, "");
		auto const& tupleComponents = tuple->components();
		solAssert(symbComponents.size() == tupleComponents.size(), "");
		for (unsigned i = 0; i < symbComponents.size(); ++i)
		{
			auto tComponent = tupleComponents.at(i);
			if (tComponent)
			{
				if (auto varDecl = identifierToVariable(*tComponent))
					m_context.addAssertion(symbTuple->component(i) == currentValue(*varDecl));
				else
				{
					if (!m_context.knownExpression(*tComponent))
						createExpr(*tComponent);
					m_context.addAssertion(symbTuple->component(i) == expr(*tComponent));
				}
			}
		}
	}
}

void SMTEncoder::endVisit(UnaryOperation const& _op)
{
	/// We need to shortcut here due to potentially unknown
	/// rational number sizes.
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;

	if (TokenTraits::isBitOp(_op.getOperator()))
		return bitwiseNotOperation(_op);

	createExpr(_op);

	auto const* subExpr = innermostTuple(_op.subExpression());
	auto type = _op.annotation().type;
	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(smt::isBool(*type), "");
		defineExpr(_op, !expr(*subExpr));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{
		solAssert(smt::isInteger(*type) || smt::isFixedPoint(*type), "");
		solAssert(subExpr->annotation().willBeWrittenTo, "");
		if (auto identifier = dynamic_cast<Identifier const*>(subExpr))
		{
			auto decl = identifierToVariable(*identifier);
			solAssert(decl, "");
			auto innerValue = currentValue(*decl);
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			assignment(*decl, newValue);
		}
		else if (
			dynamic_cast<IndexAccess const*>(subExpr) ||
			dynamic_cast<MemberAccess const*>(subExpr)
		)
		{
			auto innerValue = expr(*subExpr);
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			indexOrMemberAssignment(*subExpr, newValue);
		}
		else
			m_errorReporter.warning(
				1950_error,
				_op.location(),
				"Assertion checker does not yet implement such increments / decrements."
			);

		break;
	}
	case Token::Sub: // -
	{
		defineExpr(_op, 0 - expr(*subExpr));
		break;
	}
	case Token::Delete:
	{
		if (auto decl = identifierToVariable(*subExpr))
		{
			m_context.newValue(*decl);
			m_context.setZeroValue(*decl);
		}
		else
		{
			solAssert(m_context.knownExpression(*subExpr), "");
			auto const& symbVar = m_context.expression(*subExpr);
			symbVar->increaseIndex();
			m_context.setZeroValue(*symbVar);
			if (
				dynamic_cast<IndexAccess const*>(subExpr) ||
				dynamic_cast<MemberAccess const*>(subExpr)
			)
				indexOrMemberAssignment(*subExpr, symbVar->currentValue());
			// Empty push added a zero value anyway, so no need to delete extra.
			else if (!isEmptyPush(*subExpr))
				solAssert(false, "");
		}
		break;
	}
	default:
		m_errorReporter.warning(
			3682_error,
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

bool SMTEncoder::visit(UnaryOperation const& _op)
{
	return !shortcutRationalNumber(_op);
}

bool SMTEncoder::visit(BinaryOperation const& _op)
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

void SMTEncoder::endVisit(BinaryOperation const& _op)
{
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
		return;

	createExpr(_op);

	if (TokenTraits::isArithmeticOp(_op.getOperator()))
		arithmeticOperation(_op);
	else if (TokenTraits::isCompareOp(_op.getOperator()))
		compareOperation(_op);
	else if (TokenTraits::isBitOp(_op.getOperator()) || TokenTraits::isShiftOp(_op.getOperator()))
		bitwiseOperation(_op);
	else
		m_errorReporter.warning(
			3876_error,
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
}

bool SMTEncoder::visit(Conditional const& _op)
{
	_op.condition().accept(*this);

	auto indicesEndTrue = visitBranch(&_op.trueExpression(), expr(_op.condition()));
	auto touchedVars = touchedVariables(_op.trueExpression());

	auto indicesEndFalse = visitBranch(&_op.falseExpression(), !expr(_op.condition()));
	touchedVars += touchedVariables(_op.falseExpression());

	mergeVariables(touchedVars, expr(_op.condition()), indicesEndTrue, indicesEndFalse);

	defineExpr(_op, smtutil::Expression::ite(
		expr(_op.condition()),
		expr(_op.trueExpression(), _op.annotation().type),
		expr(_op.falseExpression(), _op.annotation().type)
	));

	return false;
}

void SMTEncoder::endVisit(FunctionCall const& _funCall)
{
	auto functionCallKind = *_funCall.annotation().kind;

	createExpr(_funCall);
	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		m_errorReporter.warning(
			4639_error,
			_funCall.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	if (functionCallKind == FunctionCallKind::TypeConversion)
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
	case FunctionType::Kind::Revert:
		// Revert is a special case of require and equals to `require(false)`
		addPathImpliedExpression(smtutil::Expression(false));
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
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
		visitCryptoFunction(_funCall);
		break;
	case FunctionType::Kind::BlockHash:
		defineExpr(_funCall, m_context.state().blockhash(expr(*_funCall.arguments().at(0))));
		break;
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		visitAddMulMod(_funCall);
		break;
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		auto const& memberAccess = dynamic_cast<MemberAccess const&>(_funCall.expression());
		auto const& address = memberAccess.expression();
		auto const& value = args.front();
		solAssert(value, "");

		smtutil::Expression thisBalance = m_context.state().balance();
		setSymbolicUnknownValue(thisBalance, TypeProvider::uint256(), m_context);

		m_context.state().transfer(m_context.state().thisAddress(), expr(address), expr(*value));
		break;
	}
	case FunctionType::Kind::ArrayPush:
		arrayPush(_funCall);
		break;
	case FunctionType::Kind::ArrayPop:
		arrayPop(_funCall);
		break;
	case FunctionType::Kind::Log0:
	case FunctionType::Kind::Log1:
	case FunctionType::Kind::Log2:
	case FunctionType::Kind::Log3:
	case FunctionType::Kind::Log4:
	case FunctionType::Kind::Event:
		// These can be safely ignored.
		break;
	case FunctionType::Kind::ObjectCreation:
		visitObjectCreation(_funCall);
		return;
	default:
		m_errorReporter.warning(
			4588_error,
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
}

bool SMTEncoder::visit(ModifierInvocation const& _node)
{
	if (auto const* args = _node.arguments())
		for (auto const& arg: *args)
			if (arg)
				arg->accept(*this);
	return false;
}

void SMTEncoder::initContract(ContractDefinition const& _contract)
{
	solAssert(m_currentContract == nullptr, "");
	m_currentContract = &_contract;

	m_context.reset();
	m_context.pushSolver();
	createStateVariables(_contract);
	clearIndices(m_currentContract, nullptr);
}

void SMTEncoder::initFunction(FunctionDefinition const& _function)
{
	solAssert(m_callStack.empty(), "");
	solAssert(m_currentContract, "");
	m_context.pushSolver();
	m_pathConditions.clear();
	pushCallStack({&_function, nullptr});
	m_uninterpretedTerms.clear();
	createStateVariables(*m_currentContract);
	createLocalVariables(_function);
	m_arrayAssignmentHappened = false;
	clearIndices(m_currentContract, &_function);
}

void SMTEncoder::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
}

void SMTEncoder::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addPathImpliedExpression(expr(*args.front()));
}

void SMTEncoder::visitCryptoFunction(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	auto arg0 = expr(*_funCall.arguments().at(0));
	optional<smtutil::Expression> result;
	if (kind == FunctionType::Kind::KECCAK256)
		result = smtutil::Expression::select(m_context.state().cryptoFunction("keccak256"), arg0);
	else if (kind == FunctionType::Kind::SHA256)
		result = smtutil::Expression::select(m_context.state().cryptoFunction("sha256"), arg0);
	else if (kind == FunctionType::Kind::RIPEMD160)
		result = smtutil::Expression::select(m_context.state().cryptoFunction("ripemd160"), arg0);
	else if (kind == FunctionType::Kind::ECRecover)
	{
		auto e = m_context.state().cryptoFunction("ecrecover");
		auto arg0 = expr(*_funCall.arguments().at(0));
		auto arg1 = expr(*_funCall.arguments().at(1));
		auto arg2 = expr(*_funCall.arguments().at(2));
		auto arg3 = expr(*_funCall.arguments().at(3));
		auto inputSort = dynamic_cast<smtutil::ArraySort&>(*e.sort).domain;
		auto ecrecoverInput = smtutil::Expression::tuple_constructor(
			smtutil::Expression(make_shared<smtutil::SortSort>(inputSort), ""),
			{arg0, arg1, arg2, arg3}
		);
		result = smtutil::Expression::select(e, ecrecoverInput);
	}
	else
		solAssert(false, "");

	defineExpr(_funCall, *result);
}

void SMTEncoder::visitGasLeft(FunctionCall const& _funCall)
{
	string gasLeft = "gasleft()";
	// We increase the variable index since gasleft changes
	// inside a tx.
	defineGlobalVariable(gasLeft, _funCall, true);
	auto const& symbolicVar = m_context.globalSymbol(gasLeft);
	unsigned index = symbolicVar->index();
	// We set the current value to unknown anyway to add type constraints.
	m_context.setUnknownValue(*symbolicVar);
	if (index > 0)
		m_context.addAssertion(symbolicVar->currentValue() <= symbolicVar->valueAtIndex(index - 1));
}

void SMTEncoder::visitAddMulMod(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::AddMod || kind == FunctionType::Kind::MulMod, "");
	auto const& args = _funCall.arguments();
	solAssert(args.at(0) && args.at(1) && args.at(2), "");
	auto x = expr(*args.at(0));
	auto y = expr(*args.at(1));
	auto k = expr(*args.at(2));
	auto const& intType = dynamic_cast<IntegerType const&>(*_funCall.annotation().type);

	if (kind == FunctionType::Kind::AddMod)
		defineExpr(_funCall, divModWithSlacks(x + y, k, intType).second);
	else
		defineExpr(_funCall, divModWithSlacks(x * y, k, intType).second);
}

void SMTEncoder::visitObjectCreation(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	auto argType = args.front()->annotation().type->category();
	solAssert(argType == Type::Category::Integer || argType == Type::Category::RationalNumber, "");

	smtutil::Expression arraySize = expr(*args.front());
	setSymbolicUnknownValue(arraySize, TypeProvider::uint256(), m_context);

	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_funCall));
	solAssert(symbArray, "");
	smt::setSymbolicZeroValue(*symbArray, m_context);
	auto zeroElements = symbArray->elements();
	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->length() == arraySize);
	m_context.addAssertion(symbArray->elements() == zeroElements);
}

void SMTEncoder::endVisit(Identifier const& _identifier)
{
	if (auto decl = identifierToVariable(_identifier))
		defineExpr(_identifier, currentValue(*decl));
	else if (_identifier.annotation().type->category() == Type::Category::Function)
		visitFunctionIdentifier(_identifier);
	else if (_identifier.name() == "now")
		defineGlobalVariable(_identifier.name(), _identifier);
	else if (_identifier.name() == "this")
	{
		defineExpr(_identifier, m_context.state().thisAddress());
		m_uninterpretedTerms.insert(&_identifier);
	}
	// Ignore the builtin abi, it is handled in FunctionCall.
	// TODO: ignore MagicType in general (abi, block, msg, tx, type)
	else if (auto magicType = dynamic_cast<MagicType const*>(_identifier.annotation().type); magicType && magicType->kind() == MagicType::Kind::ABI)
	{
		solAssert(_identifier.name() == "abi", "");
		return;
	}
	else
		createExpr(_identifier);
}

void SMTEncoder::endVisit(ElementaryTypeNameExpression const& _typeName)
{
	auto const& typeType = dynamic_cast<TypeType const&>(*_typeName.annotation().type);
	auto result = smt::newSymbolicVariable(
		*TypeProvider::uint256(),
		typeType.actualType()->toString(false),
		m_context
	);
	solAssert(!result.first && result.second, "");
	m_context.createExpression(_typeName, result.second);
}

void SMTEncoder::visitTypeConversion(FunctionCall const& _funCall)
{
	solAssert(*_funCall.annotation().kind == FunctionCallKind::TypeConversion, "");
	solAssert(_funCall.arguments().size() == 1, "");

	auto argument = _funCall.arguments().front();
	auto const& argType = argument->annotation().type;

	unsigned argSize = argument->annotation().type->storageBytes();
	unsigned castSize = _funCall.annotation().type->storageBytes();

	auto const& funCallType = _funCall.annotation().type;

	// TODO Simplify this whole thing for 0.8.0 where weird casts are disallowed.

	auto symbArg = expr(*argument, funCallType);
	bool castIsSigned = smt::isNumber(*funCallType) && smt::isSigned(funCallType);
	bool argIsSigned = smt::isNumber(*argType) && smt::isSigned(argType);
	optional<smtutil::Expression> symbMin;
	optional<smtutil::Expression> symbMax;
	if (smt::isNumber(*funCallType))
	{
		symbMin = smt::minValue(funCallType);
		symbMax = smt::maxValue(funCallType);
	}
	if (argSize == castSize)
	{
		// If sizes are the same, it's possible that the signs are different.
		if (smt::isNumber(*funCallType) && smt::isNumber(*argType))
		{
			// castIsSigned && !argIsSigned => might overflow if arg > castType.max
			// !castIsSigned && argIsSigned => might underflow if arg < castType.min
			// !castIsSigned && !argIsSigned => ok
			// castIsSigned && argIsSigned => ok

			if (castIsSigned && !argIsSigned)
			{
				auto wrap = smtutil::Expression::ite(
					symbArg > *symbMax,
					symbArg - (*symbMax - *symbMin + 1),
					symbArg
				);
				defineExpr(_funCall, wrap);
			}
			else if (!castIsSigned && argIsSigned)
			{
				auto wrap = smtutil::Expression::ite(
					symbArg < *symbMin,
					symbArg + (*symbMax + 1),
					symbArg
				);
				defineExpr(_funCall, wrap);
			}
			else
				defineExpr(_funCall, symbArg);
		}
		else
			defineExpr(_funCall, symbArg);
	}
	else if (castSize > argSize)
	{
		solAssert(smt::isNumber(*funCallType), "");
		// RationalNumbers have size 32.
		solAssert(argType->category() != Type::Category::RationalNumber, "");

		// castIsSigned && !argIsSigned => ok
		// castIsSigned && argIsSigned => ok
		// !castIsSigned && !argIsSigned => ok except for FixedBytesType, need to adjust padding
		// !castIsSigned && argIsSigned => might underflow if arg < castType.min

		if (!castIsSigned && argIsSigned)
		{
			auto wrap = smtutil::Expression::ite(
				symbArg < *symbMin,
				symbArg + (*symbMax + 1),
				symbArg
			);
			defineExpr(_funCall, wrap);
		}
		else if (!castIsSigned && !argIsSigned)
		{
			if (auto const* fixedCast = dynamic_cast<FixedBytesType const*>(funCallType))
			{
				auto const* fixedArg = dynamic_cast<FixedBytesType const*>(argType);
				solAssert(fixedArg, "");
				auto diff = fixedCast->numBytes() - fixedArg->numBytes();
				solAssert(diff > 0, "");
				auto bvSize = fixedCast->numBytes() * 8;
				defineExpr(
					_funCall,
					smtutil::Expression::bv2int(smtutil::Expression::int2bv(symbArg, bvSize) << smtutil::Expression::int2bv(diff * 8, bvSize))
				);
			}
			else
				defineExpr(_funCall, symbArg);
		}
		else
			defineExpr(_funCall, symbArg);
	}
	else // castSize < argSize
	{
		solAssert(smt::isNumber(*funCallType), "");

		auto const* fixedCast = dynamic_cast<FixedBytesType const*>(funCallType);
		auto const* fixedArg = dynamic_cast<FixedBytesType const*>(argType);
		if (fixedCast && fixedArg)
		{
			createExpr(_funCall);
			auto diff = argSize - castSize;
			solAssert(fixedArg->numBytes() - fixedCast->numBytes() == diff, "");

			auto argValueBV = smtutil::Expression::int2bv(symbArg, argSize * 8);
			auto shr = smtutil::Expression::int2bv(diff * 8, argSize * 8);
			solAssert(!castIsSigned, "");
			defineExpr(_funCall, smtutil::Expression::bv2int(argValueBV >> shr));
		}
		else
		{
			auto argValueBV = smtutil::Expression::int2bv(symbArg, castSize * 8);
			defineExpr(_funCall, smtutil::Expression::bv2int(argValueBV, castIsSigned));
		}
	}
}

void SMTEncoder::visitFunctionIdentifier(Identifier const& _identifier)
{
	auto const& fType = dynamic_cast<FunctionType const&>(*_identifier.annotation().type);
	if (fType.returnParameterTypes().size() == 1)
	{
		defineGlobalVariable(fType.identifier(), _identifier);
		m_context.createExpression(_identifier, m_context.globalSymbol(fType.identifier()));
	}
}

void SMTEncoder::endVisit(Literal const& _literal)
{
	solAssert(_literal.annotation().type, "Expected type for AST node");
	Type const& type = *_literal.annotation().type;
	if (smt::isNumber(type))
		defineExpr(_literal, smtutil::Expression(type.literalValue(&_literal)));
	else if (smt::isBool(type))
		defineExpr(_literal, smtutil::Expression(_literal.token() == Token::TrueLiteral ? true : false));
	else if (smt::isStringLiteral(type))
	{
		createExpr(_literal);

		// Add constraints for the length and values as it is known.
		auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_literal));
		solAssert(symbArray, "");

		addArrayLiteralAssertions(
			*symbArray,
			applyMap(_literal.value(), [&](auto const& c) { return smtutil::Expression{size_t(c)}; })
		);
	}
	else
	{
		m_errorReporter.warning(
			7885_error,
			_literal.location(),
			"Assertion checker does not yet support the type of this literal (" +
			_literal.annotation().type->toString() +
			")."
		);
	}
}

void SMTEncoder::addArrayLiteralAssertions(
	smt::SymbolicArrayVariable& _symArray,
	vector<smtutil::Expression> const& _elementValues
)
{
	m_context.addAssertion(_symArray.length() == _elementValues.size());
	for (size_t i = 0; i < _elementValues.size(); i++)
		m_context.addAssertion(smtutil::Expression::select(_symArray.elements(), i) == _elementValues[i]);
}

void SMTEncoder::endVisit(Return const& _return)
{
	if (_return.expression() && m_context.knownExpression(*_return.expression()))
	{
		auto returnParams = m_callStack.back().first->returnParameters();
		if (returnParams.size() > 1)
		{
			auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(*_return.expression()));
			solAssert(symbTuple, "");
			solAssert(symbTuple->components().size() == returnParams.size(), "");

			auto const* tupleType = dynamic_cast<TupleType const*>(_return.expression()->annotation().type);
			solAssert(tupleType, "");
			auto const& types = tupleType->components();
			solAssert(types.size() == returnParams.size(), "");

			for (unsigned i = 0; i < returnParams.size(); ++i)
				m_context.addAssertion(symbTuple->component(i, types.at(i), returnParams.at(i)->type()) == m_context.newValue(*returnParams.at(i)));
		}
		else if (returnParams.size() == 1)
			m_context.addAssertion(expr(*_return.expression(), returnParams.front()->type()) == m_context.newValue(*returnParams.front()));
	}
}

bool SMTEncoder::visit(MemberAccess const& _memberAccess)
{
	auto const& accessType = _memberAccess.annotation().type;
	if (accessType->category() == Type::Category::Function)
		return true;

	createExpr(_memberAccess);

	auto const& exprType = _memberAccess.expression().annotation().type;
	solAssert(exprType, "");
	if (exprType->category() == Type::Category::Magic)
	{
		if (auto const* identifier = dynamic_cast<Identifier const*>(&_memberAccess.expression()))
		{
			auto const& name = identifier->name();
			solAssert(name == "block" || name == "msg" || name == "tx", "");
			defineExpr(_memberAccess, m_context.state().txMember(name + "." + _memberAccess.memberName()));
		}
		else if (auto magicType = dynamic_cast<MagicType const*>(exprType); magicType->kind() == MagicType::Kind::MetaType)
		{
			auto const& memberName = _memberAccess.memberName();
			if (memberName == "min" || memberName == "max")
			{
				IntegerType const& integerType = dynamic_cast<IntegerType const&>(*magicType->typeArgument());
				defineExpr(_memberAccess, memberName == "min" ? integerType.minValue() : integerType.maxValue());
			}
			else if (memberName == "interfaceId")
			{
				ContractDefinition const& contract = dynamic_cast<ContractType const&>(*magicType->typeArgument()).contractDefinition();
				defineExpr(_memberAccess, contract.interfaceId());
			}
			else
				// NOTE: supporting name, creationCode, runtimeCode would be easy enough, but the bytes/string they return are not
				//       at all useable in the SMT checker currently
				m_errorReporter.warning(
					7507_error,
					_memberAccess.location(),
					"Assertion checker does not yet support this expression."
				);
		}
		else
			m_errorReporter.warning(
				9551_error,
				_memberAccess.location(),
				"Assertion checker does not yet support this expression."
			);
		return false;
	}
	else if (smt::isNonRecursiveStruct(*exprType))
	{
		_memberAccess.expression().accept(*this);
		auto const& symbStruct = dynamic_pointer_cast<smt::SymbolicStructVariable>(m_context.expression(_memberAccess.expression()));
		defineExpr(_memberAccess, symbStruct->member(_memberAccess.memberName()));
		return false;
	}
	else if (exprType->category() == Type::Category::TypeType)
	{
		auto const* decl = expressionToDeclaration(_memberAccess.expression());
		if (dynamic_cast<EnumDefinition const*>(decl))
		{
			auto enumType = dynamic_cast<EnumType const*>(accessType);
			solAssert(enumType, "");
			defineExpr(_memberAccess, enumType->memberValue(_memberAccess.memberName()));

			return false;
		}
		else if (dynamic_cast<ContractDefinition const*>(decl))
		{
			if (auto const* var = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
			{
				defineExpr(_memberAccess, currentValue(*var));
				return false;
			}
		}
	}
	else if (exprType->category() == Type::Category::Address)
	{
		_memberAccess.expression().accept(*this);
		if (_memberAccess.memberName() == "balance")
		{
			defineExpr(_memberAccess, m_context.state().balance(expr(_memberAccess.expression())));
			setSymbolicUnknownValue(*m_context.expression(_memberAccess), m_context);
			m_uninterpretedTerms.insert(&_memberAccess);
			return false;
		}
	}
	else if (exprType->category() == Type::Category::Array)
	{
		_memberAccess.expression().accept(*this);
		if (_memberAccess.memberName() == "length")
		{
			auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(_memberAccess.expression()));
			solAssert(symbArray, "");
			defineExpr(_memberAccess, symbArray->length());
			m_uninterpretedTerms.insert(&_memberAccess);
			setSymbolicUnknownValue(
				expr(_memberAccess),
				_memberAccess.annotation().type,
				m_context
			);
		}
		return false;
	}
	else if (
		auto const* functionType = dynamic_cast<FunctionType const*>(exprType);
		functionType &&
		_memberAccess.memberName() == "selector" &&
		functionType->hasDeclaration()
	)
	{
		defineExpr(_memberAccess, functionType->externalIdentifier());
		return false;
	}
	else
		m_errorReporter.warning(
			7650_error,
			_memberAccess.location(),
			"Assertion checker does not yet support this expression."
		);

	return true;
}

void SMTEncoder::endVisit(IndexAccess const& _indexAccess)
{
	createExpr(_indexAccess);

	if (_indexAccess.annotation().type->category() == Type::Category::TypeType)
		return;
	if (auto const* type = dynamic_cast<FixedBytesType const*>(_indexAccess.baseExpression().annotation().type))
	{
		smtutil::Expression base = expr(_indexAccess.baseExpression());

		if (type->numBytes() == 1)
			defineExpr(_indexAccess, base);
		else
		{
			auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_indexAccess.baseExpression().annotation().type);
			solAssert(!isSigned, "");
			solAssert(bvSize >= 16, "");
			solAssert(bvSize % 8 == 0, "");

			smtutil::Expression idx = expr(*_indexAccess.indexExpression());

			auto bvBase = smtutil::Expression::int2bv(base, bvSize);
			auto bvShl = smtutil::Expression::int2bv(idx * 8, bvSize);
			auto bvShr = smtutil::Expression::int2bv(bvSize - 8, bvSize);
			auto result = (bvBase << bvShl) >> bvShr;

			auto anyValue = expr(_indexAccess);
			m_context.expression(_indexAccess)->increaseIndex();
			unsigned numBytes = bvSize / 8;
			auto withBound = smtutil::Expression::ite(
				idx < numBytes,
				smtutil::Expression::bv2int(result, false),
				anyValue
			);
			defineExpr(_indexAccess, withBound);
		}
		return;
	}

	shared_ptr<smt::SymbolicVariable> array;
	if (auto const* id = dynamic_cast<Identifier const*>(&_indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");
		array = m_context.variable(*varDecl);
	}
	else
	{
		solAssert(m_context.knownExpression(_indexAccess.baseExpression()), "");
		array = m_context.expression(_indexAccess.baseExpression());
	}

	auto arrayVar = dynamic_pointer_cast<smt::SymbolicArrayVariable>(array);
	solAssert(arrayVar, "");
	TypePointer baseType = _indexAccess.baseExpression().annotation().type;
	defineExpr(_indexAccess, smtutil::Expression::select(
		arrayVar->elements(),
		expr(*_indexAccess.indexExpression(), keyType(baseType))
	));
	setSymbolicUnknownValue(
		expr(_indexAccess),
		_indexAccess.annotation().type,
		m_context
	);
	m_uninterpretedTerms.insert(&_indexAccess);
}

void SMTEncoder::endVisit(IndexRangeAccess const& _indexRangeAccess)
{
	createExpr(_indexRangeAccess);
	/// The actual slice is created by CHC which also assigns the length.
}

void SMTEncoder::arrayAssignment()
{
	m_arrayAssignmentHappened = true;
}

void SMTEncoder::indexOrMemberAssignment(Expression const& _expr, smtutil::Expression const& _rightHandSide)
{
	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_expr))
	{
		if (dynamic_cast<ContractDefinition const*>(expressionToDeclaration(memberAccess->expression())))
		{
			if (auto const* var = dynamic_cast<VariableDeclaration const*>(memberAccess->annotation().referencedDeclaration))
			{
				if (var->hasReferenceOrMappingType())
					resetReferences(*var);

				m_context.addAssertion(m_context.newValue(*var) == _rightHandSide);
				m_context.expression(_expr)->increaseIndex();
				defineExpr(_expr, currentValue(*var));
				return;
			}
		}
	}

	auto toStore = _rightHandSide;
	auto const* lastExpr = &_expr;
	while (true)
	{
		if (auto const* indexAccess = dynamic_cast<IndexAccess const*>(lastExpr))
		{
			auto const& base = indexAccess->baseExpression();
			if (dynamic_cast<Identifier const*>(&base))
				base.accept(*this);

			TypePointer baseType = base.annotation().type;
			auto indexExpr = expr(*indexAccess->indexExpression(), keyType(baseType));
			auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(base));
			solAssert(symbArray, "");
			toStore = smtutil::Expression::tuple_constructor(
				smtutil::Expression(make_shared<smtutil::SortSort>(smt::smtSort(*baseType)), baseType->toString(true)),
				{smtutil::Expression::store(symbArray->elements(), indexExpr, toStore), symbArray->length()}
			);
			m_context.expression(*indexAccess)->increaseIndex();
			defineExpr(*indexAccess, smtutil::Expression::select(
				symbArray->elements(),
				indexExpr
			));
			lastExpr = &indexAccess->baseExpression();
		}
		else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(lastExpr))
		{
			auto const& base = memberAccess->expression();
			if (dynamic_cast<Identifier const*>(&base))
				base.accept(*this);

			if (
				auto const* structType = dynamic_cast<StructType const*>(base.annotation().type);
				structType && structType->recursive()
			)
			{
				m_errorReporter.warning(
					4375_error,
					memberAccess->location(),
					"Assertion checker does not support recursive structs."
				);
				return;
			}

			auto symbStruct = dynamic_pointer_cast<smt::SymbolicStructVariable>(m_context.expression(base));
			solAssert(symbStruct, "");
			symbStruct->assignMember(memberAccess->memberName(), toStore);
			toStore = symbStruct->currentValue();
			defineExpr(*memberAccess, symbStruct->member(memberAccess->memberName()));
			lastExpr = &memberAccess->expression();
		}
		else if (auto const& id = dynamic_cast<Identifier const*>(lastExpr))
		{
			auto varDecl = identifierToVariable(*id);
			solAssert(varDecl, "");

			if (varDecl->hasReferenceOrMappingType())
				resetReferences(*varDecl);

			m_context.addAssertion(m_context.newValue(*varDecl) == toStore);
			m_context.expression(*id)->increaseIndex();
			defineExpr(*id, currentValue(*varDecl));
			break;
		}
		else
		{
			auto type = lastExpr->annotation().type;
			if (
				dynamic_cast<ReferenceType const*>(type) ||
				dynamic_cast<MappingType const*>(type)
			)
				resetReferences(type);

			m_context.expression(*lastExpr)->increaseIndex();
			m_context.addAssertion(expr(*lastExpr) == toStore);
			break;
		}
	}
}

void SMTEncoder::arrayPush(FunctionCall const& _funCall)
{
	auto memberAccess = dynamic_cast<MemberAccess const*>(&_funCall.expression());
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");
	auto oldLength = symbArray->length();
	m_context.addAssertion(oldLength >= 0);
	// Real world assumption: the array length is assumed to not overflow.
	// This assertion guarantees that both the current and updated lengths have the above property.
	m_context.addAssertion(oldLength + 1 < (smt::maxValue(*TypeProvider::uint256()) - 1));

	auto const& arguments = _funCall.arguments();
	smtutil::Expression element = arguments.empty() ?
		smt::zeroValue(_funCall.annotation().type) :
		expr(*arguments.front());
	smtutil::Expression store = smtutil::Expression::store(
		symbArray->elements(),
		oldLength,
		element
	);
	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->elements() == store);
	m_context.addAssertion(symbArray->length() == oldLength + 1);

	if (arguments.empty())
		defineExpr(_funCall, smtutil::Expression::select(symbArray->elements(), oldLength));

	arrayPushPopAssign(memberAccess->expression(), symbArray->currentValue());
}

void SMTEncoder::arrayPop(FunctionCall const& _funCall)
{
	auto memberAccess = dynamic_cast<MemberAccess const*>(&_funCall.expression());
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");

	makeArrayPopVerificationTarget(_funCall);

	auto oldElements = symbArray->elements();
	auto oldLength = symbArray->length();

	symbArray->increaseIndex();
	m_context.addAssertion(symbArray->elements() == oldElements);
	auto newLength = smtutil::Expression::ite(
		oldLength > 0,
		oldLength - 1,
		0
	);
	m_context.addAssertion(symbArray->length() == newLength);

	arrayPushPopAssign(memberAccess->expression(), symbArray->currentValue());
}

void SMTEncoder::arrayPushPopAssign(Expression const& _expr, smtutil::Expression const& _array)
{
	Expression const* expr = innermostTuple(_expr);

	if (auto const* id = dynamic_cast<Identifier const*>(expr))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");
		if (varDecl->hasReferenceOrMappingType())
			resetReferences(*varDecl);
		m_context.addAssertion(m_context.newValue(*varDecl) == _array);
		m_context.expression(*id)->increaseIndex();
		defineExpr(*id,currentValue(*varDecl));
	}
	else if (
		dynamic_cast<IndexAccess const*>(expr) ||
		dynamic_cast<MemberAccess const*>(expr)
	)
		indexOrMemberAssignment(_expr, _array);
	else if (auto const* funCall = dynamic_cast<FunctionCall const*>(expr))
	{
		FunctionType const& funType = dynamic_cast<FunctionType const&>(*funCall->expression().annotation().type);
		if (funType.kind() == FunctionType::Kind::ArrayPush)
		{
			auto memberAccess = dynamic_cast<MemberAccess const*>(&funCall->expression());
			solAssert(memberAccess, "");
			auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
			solAssert(symbArray, "");

			auto oldLength = symbArray->length();
			auto store = smtutil::Expression::store(
				symbArray->elements(),
				symbArray->length() - 1,
				_array
			);
			symbArray->increaseIndex();
			m_context.addAssertion(symbArray->elements() == store);
			m_context.addAssertion(symbArray->length() == oldLength);
			arrayPushPopAssign(memberAccess->expression(), symbArray->currentValue());
		}
	}
	else
		solAssert(false, "");
}

void SMTEncoder::defineGlobalVariable(string const& _name, Expression const& _expr, bool _increaseIndex)
{
	if (!m_context.knownGlobalSymbol(_name))
	{
		bool abstract = m_context.createGlobalSymbol(_name, _expr);
		if (abstract)
			m_errorReporter.warning(
				1695_error,
				_expr.location(),
				"Assertion checker does not yet support this global variable."
			);
	}
	else if (_increaseIndex)
		m_context.globalSymbol(_name)->increaseIndex();
	// The default behavior is not to increase the index since
	// most of the global values stay the same throughout a tx.
	if (smt::isSupportedType(*_expr.annotation().type))
		defineExpr(_expr, m_context.globalSymbol(_name)->currentValue());
}

bool SMTEncoder::shortcutRationalNumber(Expression const& _expr)
{
	if (_expr.annotation().type->category() == Type::Category::RationalNumber)
	{
		auto rationalType = dynamic_cast<RationalNumberType const*>(_expr.annotation().type);
		solAssert(rationalType, "");
		if (rationalType->isNegative())
			defineExpr(_expr, smtutil::Expression(u2s(rationalType->literalValue(nullptr))));
		else
			defineExpr(_expr, smtutil::Expression(rationalType->literalValue(nullptr)));
		return true;
	}
	return false;
}

void SMTEncoder::arithmeticOperation(BinaryOperation const& _op)
{
	auto type = _op.annotation().commonType;
	solAssert(type, "");
	if (type->category() == Type::Category::Integer || type->category() == Type::Category::FixedPoint)
	{
		switch (_op.getOperator())
		{
		case Token::Add:
		case Token::Sub:
		case Token::Mul:
		case Token::Div:
		case Token::Mod:
		{
			auto values = arithmeticOperation(
				_op.getOperator(),
				expr(_op.leftExpression()),
				expr(_op.rightExpression()),
				_op.annotation().commonType,
				_op
			);
			defineExpr(_op, values.first);
			break;
		}
		default:
			m_errorReporter.warning(
				5188_error,
				_op.location(),
				"Assertion checker does not yet implement this operator."
			);
		}
	}
	else
		m_errorReporter.warning(
			9011_error,
			_op.location(),
			"Assertion checker does not yet implement this operator for type " + type->richIdentifier() + "."
		);
}

pair<smtutil::Expression, smtutil::Expression> SMTEncoder::arithmeticOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	TypePointer const& _commonType,
	Expression const& _operation
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
	solAssert(
		_commonType->category() == Type::Category::Integer || _commonType->category() == Type::Category::FixedPoint,
		""
	);

	IntegerType const* intType = nullptr;
	if (auto type = dynamic_cast<IntegerType const*>(_commonType))
		intType = type;
	else
		intType = TypeProvider::uint256();

	auto valueUnbounded = [&]() -> smtutil::Expression {
		switch (_op)
		{
		case Token::Add: return _left + _right;
		case Token::Sub: return _left - _right;
		case Token::Mul: return _left * _right;
		case Token::Div: return divModWithSlacks(_left, _right, *intType).first;
		case Token::Mod: return divModWithSlacks(_left, _right, *intType).second;
		default: solAssert(false, "");
		}
	}();

	if (_op == Token::Div || _op == Token::Mod)
	{
		// mod and unsigned division never underflow/overflow
		if (_op == Token::Mod || !intType->isSigned())
			return {valueUnbounded, valueUnbounded};

		// The only case where division overflows is
		// - type is signed
		// - LHS is type.min
		// - RHS is -1
		// the result is then -(type.min), which wraps back to type.min
		smtutil::Expression maxLeft = _left == smt::minValue(*intType);
		smtutil::Expression minusOneRight = _right == numeric_limits<size_t >::max();
		smtutil::Expression wrap = smtutil::Expression::ite(maxLeft && minusOneRight, smt::minValue(*intType), valueUnbounded);
		return {wrap, valueUnbounded};
	}

	auto symbMin = smt::minValue(*intType);
	auto symbMax = smt::maxValue(*intType);

	smtutil::Expression intValueRange = (0 - symbMin) + symbMax + 1;
	string suffix = to_string(_operation.id()) + "_" + to_string(m_context.newUniqueId());
	smt::SymbolicIntVariable k(intType, intType, "k_" + suffix, m_context);
	smt::SymbolicIntVariable m(intType, intType, "m_" + suffix, m_context);

	// To wrap around valueUnbounded in case of overflow or underflow, we replace it with a k, given:
	// 1. k + m * intValueRange = valueUnbounded
	// 2. k is in range of the desired integer type
	auto wrap = k.currentValue();
	m_context.addAssertion(valueUnbounded == (k.currentValue() + intValueRange * m.currentValue()));
	m_context.addAssertion(k.currentValue() >= symbMin);
	m_context.addAssertion(k.currentValue() <= symbMax);

	// TODO this could be refined:
	// for unsigned types it's enough to check only the upper bound.
	auto value = smtutil::Expression::ite(
		valueUnbounded > symbMax,
		wrap,
		smtutil::Expression::ite(
			valueUnbounded < symbMin,
			wrap,
			valueUnbounded
		)
	);

	return {value, valueUnbounded};
}

smtutil::Expression SMTEncoder::bitwiseOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	TypePointer const& _commonType
)
{
	static set<Token> validOperators{
		Token::BitAnd,
		Token::BitOr,
		Token::BitXor,
		Token::SHL,
		Token::SHR,
		Token::SAR
	};
	solAssert(validOperators.count(_op), "");
	solAssert(_commonType, "");

	auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_commonType);

	auto bvLeft = smtutil::Expression::int2bv(_left, bvSize);
	auto bvRight = smtutil::Expression::int2bv(_right, bvSize);

	optional<smtutil::Expression> result;
	switch (_op)
	{
		case Token::BitAnd:
			result = bvLeft & bvRight;
			break;
		case Token::BitOr:
			result = bvLeft | bvRight;
			break;
		case Token::BitXor:
			result = bvLeft ^ bvRight;
			break;
		case Token::SHL:
			result = bvLeft << bvRight;
			break;
		case Token::SHR:
			result = bvLeft >> bvRight;
			break;
		case Token::SAR:
			result = isSigned ?
				smtutil::Expression::ashr(bvLeft, bvRight) :
				bvLeft >> bvRight;
			break;
		default:
			solAssert(false, "");
	}

	solAssert(result.has_value(), "");
	return smtutil::Expression::bv2int(*result, isSigned);
}

void SMTEncoder::compareOperation(BinaryOperation const& _op)
{
	auto const& commonType = _op.annotation().commonType;
	solAssert(commonType, "");
	if (smt::isSupportedType(*commonType))
	{
		smtutil::Expression left(expr(_op.leftExpression(), commonType));
		smtutil::Expression right(expr(_op.rightExpression(), commonType));
		Token op = _op.getOperator();
		shared_ptr<smtutil::Expression> value;
		if (smt::isNumber(*commonType))
		{
			value = make_shared<smtutil::Expression>(
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
			solUnimplementedAssert(smt::isBool(*commonType), "Operation not yet supported");
			value = make_shared<smtutil::Expression>(
				op == Token::Equal ? (left == right) :
				/*op == Token::NotEqual*/ (left != right)
			);
		}
		// TODO: check that other values for op are not possible.
		defineExpr(_op, *value);
	}
	else
		m_errorReporter.warning(
			7229_error,
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for comparisons"
		);
}

void SMTEncoder::booleanOperation(BinaryOperation const& _op)
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
			3263_error,
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for boolean operations"
		);
}

void SMTEncoder::bitwiseOperation(BinaryOperation const& _op)
{
	auto op = _op.getOperator();
	solAssert(TokenTraits::isBitOp(op) || TokenTraits::isShiftOp(op), "");
	auto commonType = _op.annotation().commonType;
	solAssert(commonType, "");

	defineExpr(_op, bitwiseOperation(
		_op.getOperator(),
		expr(_op.leftExpression(), commonType),
		expr(_op.rightExpression(), commonType),
		commonType
	));
}

void SMTEncoder::bitwiseNotOperation(UnaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::BitNot, "");

	auto [bvSize, isSigned] = smt::typeBvSizeAndSignedness(_op.annotation().type);

	auto bvOperand = smtutil::Expression::int2bv(expr(_op.subExpression(), _op.annotation().type), bvSize);
	defineExpr(_op, smtutil::Expression::bv2int(~bvOperand, isSigned));
}

pair<smtutil::Expression, smtutil::Expression> SMTEncoder::divModWithSlacks(
	smtutil::Expression _left,
	smtutil::Expression _right,
	IntegerType const& _type
)
{
	IntegerType const* intType = &_type;
	string suffix = "div_mod_" + to_string(m_context.newUniqueId());
	smt::SymbolicIntVariable dSymb(intType, intType, "d_" + suffix, m_context);
	smt::SymbolicIntVariable rSymb(intType, intType, "r_" + suffix, m_context);
	auto d = dSymb.currentValue();
	auto r = rSymb.currentValue();

	// x / y = d and x % y = r iff d * y + r = x and
	// either x >= 0 and 0 <= r < abs(y) (or just 0 <= r < y for unsigned)
	// or     x < 0 and -abs(y) < r <= 0
	m_context.addAssertion(((d * _right) + r) == _left);
	if (_type.isSigned())
		m_context.addAssertion(
			(_left >= 0 && 0 <= r && (_right == 0 || r < smtutil::abs(_right))) ||
			(_left < 0 && ((_right == 0 || 0 - smtutil::abs(_right) < r) && r <= 0))
		);
	else // unsigned version
		m_context.addAssertion(0 <= r && (_right == 0 || r < _right));

	auto divResult = smtutil::Expression::ite(_right == 0, 0, d);
	auto modResult = smtutil::Expression::ite(_right == 0, 0, r);
	return {divResult, modResult};
}

void SMTEncoder::assignment(
	Expression const& _left,
	smtutil::Expression const& _right,
	TypePointer const& _type
)
{
	solAssert(
		_left.annotation().type->category() != Type::Category::Tuple,
		"Tuple assignments should be handled by tupleAssignment."
	);

	Expression const* left = innermostTuple(_left);

	if (!smt::isSupportedType(*_type))
	{
		// Give it a new index anyway to keep the SSA scheme sound.
		if (auto varDecl = identifierToVariable(*left))
			m_context.newValue(*varDecl);
	}
	else if (auto varDecl = identifierToVariable(*left))
		assignment(*varDecl, _right);
	else if (
		dynamic_cast<IndexAccess const*>(left) ||
		dynamic_cast<MemberAccess const*>(left)
	)
		indexOrMemberAssignment(*left, _right);
	else
		solAssert(false, "");
}

void SMTEncoder::tupleAssignment(Expression const& _left, Expression const& _right)
{
	auto lTuple = dynamic_cast<TupleExpression const*>(innermostTuple(_left));
	solAssert(lTuple, "");
	Expression const* right = innermostTuple(_right);

	auto const& lComponents = lTuple->components();

	// If both sides are tuple expressions, we individually and potentially
	// recursively assign each pair of components.
	// This is because of potential type conversion.
	if (auto rTuple = dynamic_cast<TupleExpression const*>(right))
	{
		auto const& rComponents = rTuple->components();
		solAssert(lComponents.size() == rComponents.size(), "");
		for (unsigned i = 0; i < lComponents.size(); ++i)
		{
			if (!lComponents.at(i) || !rComponents.at(i))
				continue;
			auto const& lExpr = *lComponents.at(i);
			auto const& rExpr = *rComponents.at(i);
			if (lExpr.annotation().type->category() == Type::Category::Tuple)
				tupleAssignment(lExpr, rExpr);
			else
			{
				auto type = lExpr.annotation().type;
				assignment(lExpr, expr(rExpr, type), type);
			}
		}
	}
	else
	{
		auto rType = dynamic_cast<TupleType const*>(right->annotation().type);
		solAssert(rType, "");

		auto const& rComponents = rType->components();
		solAssert(lComponents.size() == rComponents.size(), "");

		auto symbRight = expr(*right);
		solAssert(symbRight.sort->kind == smtutil::Kind::Tuple, "");

		for (unsigned i = 0; i < lComponents.size(); ++i)
			if (auto component = lComponents.at(i); component && rComponents.at(i))
				assignment(*component, smtutil::Expression::tuple_get(symbRight, i), component->annotation().type);
	}
}

smtutil::Expression SMTEncoder::compoundAssignment(Assignment const& _assignment)
{
	static map<Token, Token> const compoundToArithmetic{
		{Token::AssignAdd, Token::Add},
		{Token::AssignSub, Token::Sub},
		{Token::AssignMul, Token::Mul},
		{Token::AssignDiv, Token::Div},
		{Token::AssignMod, Token::Mod}
	};
	static map<Token, Token> const compoundToBitwise{
		{Token::AssignBitAnd, Token::BitAnd},
		{Token::AssignBitOr, Token::BitOr},
		{Token::AssignBitXor, Token::BitXor},
		{Token::AssignShl, Token::SHL},
		{Token::AssignShr, Token::SHR},
		{Token::AssignSar, Token::SAR}
	};
	Token op = _assignment.assignmentOperator();
	solAssert(compoundToArithmetic.count(op) || compoundToBitwise.count(op), "");

	auto decl = identifierToVariable(_assignment.leftHandSide());

	if (compoundToBitwise.count(op))
		return bitwiseOperation(
			compoundToBitwise.at(op),
			decl ? currentValue(*decl) : expr(_assignment.leftHandSide()),
			expr(_assignment.rightHandSide()),
			_assignment.annotation().type
		);

	auto values = arithmeticOperation(
		compoundToArithmetic.at(op),
		decl ? currentValue(*decl) : expr(_assignment.leftHandSide()),
		expr(_assignment.rightHandSide()),
		_assignment.annotation().type,
		_assignment
	);
	return values.first;
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, Expression const& _value)
{
	// In general, at this point, the SMT sorts of _variable and _value are the same,
	// even if there is implicit conversion.
	// This is a special case where the SMT sorts are different.
	// For now we are unaware of other cases where this happens, but if they do appear
	// we should extract this into an `implicitConversion` function.
	assignment(_variable, expr(_value, _variable.type()));
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, smtutil::Expression const& _value)
{
	TypePointer type = _variable.type();
	if (type->category() == Type::Category::Mapping)
		arrayAssignment();
	m_context.addAssertion(m_context.newValue(_variable) == _value);
}

SMTEncoder::VariableIndices SMTEncoder::visitBranch(ASTNode const* _statement, smtutil::Expression _condition)
{
	return visitBranch(_statement, &_condition);
}

SMTEncoder::VariableIndices SMTEncoder::visitBranch(ASTNode const* _statement, smtutil::Expression const* _condition)
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

void SMTEncoder::initializeFunctionCallParameters(CallableDeclaration const& _function, vector<smtutil::Expression> const& _callArgs)
{
	auto const& funParams = _function.parameters();
	solAssert(funParams.size() == _callArgs.size(), "");
	for (unsigned i = 0; i < funParams.size(); ++i)
		if (createVariable(*funParams[i]))
		{
			m_context.addAssertion(_callArgs[i] == m_context.newValue(*funParams[i]));
			if (funParams[i]->annotation().type->category() == Type::Category::Mapping)
				m_arrayAssignmentHappened = true;
		}

	vector<VariableDeclaration const*> localVars;
	if (auto const* fun = dynamic_cast<FunctionDefinition const*>(&_function))
		localVars = localVariablesIncludingModifiers(*fun);
	else
		localVars = _function.localVariables();
	for (auto const& variable: localVars)
		if (createVariable(*variable))
		{
			m_context.newValue(*variable);
			m_context.setZeroValue(*variable);
		}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
			{
				m_context.newValue(*retParam);
				m_context.setZeroValue(*retParam);
			}
}

void SMTEncoder::createStateVariables(ContractDefinition const& _contract)
{
	for (auto var: stateVariablesIncludingInheritedAndPrivate(_contract))
		createVariable(*var);
}

void SMTEncoder::initializeStateVariables(ContractDefinition const& _contract)
{
	for (auto var: _contract.stateVariables())
	{
		solAssert(m_context.knownVariable(*var), "");
		m_context.setZeroValue(*var);
	}

	for (auto var: _contract.stateVariables())
		if (var->value())
		{
			var->value()->accept(*this);
			assignment(*var, *var->value());
		}
}

void SMTEncoder::createLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: localVariablesIncludingModifiers(_function))
		createVariable(*variable);

	for (auto const& param: _function.parameters())
		createVariable(*param);

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			createVariable(*retParam);
}

void SMTEncoder::initializeLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: localVariablesIncludingModifiers(_function))
	{
		solAssert(m_context.knownVariable(*variable), "");
		m_context.setZeroValue(*variable);
	}

	for (auto const& param: _function.parameters())
	{
		solAssert(m_context.knownVariable(*param), "");
		m_context.setUnknownValue(*param);
	}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
		{
			solAssert(m_context.knownVariable(*retParam), "");
			m_context.setZeroValue(*retParam);
		}
}

void SMTEncoder::resetStateVariables()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.isStateVariable(); });
}

void SMTEncoder::resetReferences(VariableDeclaration const& _varDecl)
{
	m_context.resetVariables([&](VariableDeclaration const& _var) {
		if (_var == _varDecl)
			return false;

		// If both are state variables no need to clear knowledge.
		if (_var.isStateVariable() && _varDecl.isStateVariable())
			return false;

		return sameTypeOrSubtype(_var.type(), _varDecl.type());
	});
}

void SMTEncoder::resetReferences(TypePointer _type)
{
	m_context.resetVariables([&](VariableDeclaration const& _var) {
		return sameTypeOrSubtype(_var.type(), _type);
	});
}

bool SMTEncoder::sameTypeOrSubtype(TypePointer _a, TypePointer _b)
{
	TypePointer prefix = _a;
	while (
		prefix->category() == Type::Category::Mapping ||
		prefix->category() == Type::Category::Array
	)
	{
		if (*typeWithoutPointer(_b) == *typeWithoutPointer(prefix))
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
}

TypePointer SMTEncoder::typeWithoutPointer(TypePointer const& _type)
{
	if (auto refType = dynamic_cast<ReferenceType const*>(_type))
		return TypeProvider::withLocationIfReference(refType->location(), _type);
	return _type;
}

void SMTEncoder::mergeVariables(set<VariableDeclaration const*> const& _variables, smtutil::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse)
{
	auto cmp = [] (VariableDeclaration const* var1, VariableDeclaration const* var2) {
		return var1->id() < var2->id();
	};
	set<VariableDeclaration const*, decltype(cmp)> sortedVars(begin(_variables), end(_variables), cmp);

	/// Knowledge about references is erased if a reference is assigned,
	/// so those also need their SSA's merged.
	/// This does not cause scope harm since the symbolic variables
	/// are kept alive.
	for (auto const& var: _indicesEndTrue)
	{
		solAssert(_indicesEndFalse.count(var.first), "");
		if (
			_indicesEndFalse.at(var.first) != var.second &&
			!sortedVars.count(var.first)
		)
			sortedVars.insert(var.first);
	}

	for (auto const* decl: sortedVars)
	{
		solAssert(_indicesEndTrue.count(decl) && _indicesEndFalse.count(decl), "");
		auto trueIndex = static_cast<unsigned>(_indicesEndTrue.at(decl));
		auto falseIndex = static_cast<unsigned>(_indicesEndFalse.at(decl));
		solAssert(trueIndex != falseIndex, "");
		m_context.addAssertion(m_context.newValue(*decl) == smtutil::Expression::ite(
			_condition,
			valueAtIndex(*decl, trueIndex),
			valueAtIndex(*decl, falseIndex))
		);
	}
}

smtutil::Expression SMTEncoder::currentValue(VariableDeclaration const& _decl)
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->currentValue();
}

smtutil::Expression SMTEncoder::valueAtIndex(VariableDeclaration const& _decl, unsigned _index)
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->valueAtIndex(_index);
}

bool SMTEncoder::createVariable(VariableDeclaration const& _varDecl)
{
	if (m_context.knownVariable(_varDecl))
		return true;
	bool abstract = m_context.createVariable(_varDecl);
	if (abstract)
	{
		m_errorReporter.warning(
			8115_error,
			_varDecl.location(),
			"Assertion checker does not yet support the type of this variable."
		);
		return false;
	}
	return true;
}

smtutil::Expression SMTEncoder::expr(Expression const& _e, TypePointer _targetType)
{
	if (!m_context.knownExpression(_e))
	{
		m_errorReporter.warning(6031_error, _e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}

	return m_context.expression(_e)->currentValue(_targetType);
}

void SMTEncoder::createExpr(Expression const& _e)
{
	bool abstract = m_context.createExpression(_e);
	if (abstract)
		m_errorReporter.warning(
			8364_error,
			_e.location(),
			"Assertion checker does not yet implement type " + _e.annotation().type->toString()
		);
}

void SMTEncoder::defineExpr(Expression const& _e, smtutil::Expression _value)
{
	createExpr(_e);
	solAssert(_value.sort->kind != smtutil::Kind::Function, "Equality operator applied to type that is not fully supported");
	m_context.addAssertion(expr(_e) == _value);
}

void SMTEncoder::popPathCondition()
{
	solAssert(m_pathConditions.size() > 0, "Cannot pop path condition, empty.");
	m_pathConditions.pop_back();
}

void SMTEncoder::pushPathCondition(smtutil::Expression const& _e)
{
	m_pathConditions.push_back(currentPathConditions() && _e);
}

smtutil::Expression SMTEncoder::currentPathConditions()
{
	if (m_pathConditions.empty())
		return smtutil::Expression(true);
	return m_pathConditions.back();
}

SecondarySourceLocation SMTEncoder::callStackMessage(vector<CallStackEntry> const& _callStack)
{
	SecondarySourceLocation callStackLocation;
	solAssert(!_callStack.empty(), "");
	callStackLocation.append("Callstack:", SourceLocation());
	for (auto const& call: _callStack | boost::adaptors::reversed)
		if (call.second)
			callStackLocation.append("", call.second->location());
	return callStackLocation;
}

pair<CallableDeclaration const*, ASTNode const*> SMTEncoder::popCallStack()
{
	solAssert(!m_callStack.empty(), "");
	auto lastCalled = m_callStack.back();
	m_callStack.pop_back();
	return lastCalled;
}

void SMTEncoder::pushCallStack(CallStackEntry _entry)
{
	m_callStack.push_back(_entry);
}

void SMTEncoder::addPathImpliedExpression(smtutil::Expression const& _e)
{
	m_context.addAssertion(smtutil::Expression::implies(currentPathConditions(), _e));
}

bool SMTEncoder::isRootFunction()
{
	return m_callStack.size() == 1;
}

bool SMTEncoder::visitedFunction(FunctionDefinition const* _funDef)
{
	for (auto const& call: m_callStack)
		if (call.first == _funDef)
			return true;
	return false;
}

SMTEncoder::VariableIndices SMTEncoder::copyVariableIndices()
{
	VariableIndices indices;
	for (auto const& var: m_context.variables())
		indices.emplace(var.first, var.second->index());
	return indices;
}

void SMTEncoder::resetVariableIndices(VariableIndices const& _indices)
{
	for (auto const& var: _indices)
		m_context.variable(*var.first)->setIndex(static_cast<unsigned>(var.second));
}

void SMTEncoder::clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function)
{
	solAssert(_contract, "");
	for (auto var: stateVariablesIncludingInheritedAndPrivate(*_contract))
		m_context.variable(*var)->resetIndex();
	if (_function)
	{
		for (auto const& var: _function->parameters() + _function->returnParameters())
			m_context.variable(*var)->resetIndex();
		for (auto const& var: localVariablesIncludingModifiers(*_function))
			m_context.variable(*var)->resetIndex();
	}
	m_context.state().reset();
}

Expression const* SMTEncoder::leftmostBase(IndexAccess const& _indexAccess)
{
	Expression const* base = &_indexAccess.baseExpression();
	while (auto access = dynamic_cast<IndexAccess const*>(base))
		base = &access->baseExpression();
	return base;
}

TypePointer SMTEncoder::keyType(TypePointer _type)
{
	if (auto const* mappingType = dynamic_cast<MappingType const*>(_type))
		return mappingType->keyType();
	if (
		dynamic_cast<ArrayType const*>(_type) ||
		dynamic_cast<ArraySliceType const*>(_type)
	)
		return TypeProvider::uint256();
	else
		solAssert(false, "");
}

Expression const* SMTEncoder::innermostTuple(Expression const& _expr)
{
	auto const* tuple = dynamic_cast<TupleExpression const*>(&_expr);
	if (!tuple || tuple->isInlineArray())
		return &_expr;

	Expression const* expr = tuple;
	while (tuple && !tuple->isInlineArray() && tuple->components().size() == 1)
	{
		expr = tuple->components().front().get();
		tuple = dynamic_cast<TupleExpression const*>(expr);
	}
	solAssert(expr, "");
	return expr;
}

set<VariableDeclaration const*> SMTEncoder::touchedVariables(ASTNode const& _node)
{
	vector<CallableDeclaration const*> callStack;
	for (auto const& call: m_callStack)
		callStack.push_back(call.first);
	return m_variableUsage.touchedVariables(_node, callStack);
}

Declaration const* SMTEncoder::expressionToDeclaration(Expression const& _expr) const
{
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_expr))
		return identifier->annotation().referencedDeclaration;
	if (auto const* outerMemberAccess = dynamic_cast<MemberAccess const*>(&_expr))
		return outerMemberAccess->annotation().referencedDeclaration;
	return nullptr;
}

VariableDeclaration const* SMTEncoder::identifierToVariable(Expression const& _expr) const
{
	// We do not use `expressionToDeclaration` here because we are not interested in
	// struct.field, for example.
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_expr))
		if (auto const* varDecl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
		{
			solAssert(m_context.knownVariable(*varDecl), "");
			return varDecl;
		}
	return nullptr;
}

MemberAccess const* SMTEncoder::isEmptyPush(Expression const& _expr) const
{
	if (
		auto const* funCall = dynamic_cast<FunctionCall const*>(&_expr);
		funCall && funCall->arguments().empty()
	)
	{
		auto const& funType = dynamic_cast<FunctionType const&>(*funCall->expression().annotation().type);
		if (funType.kind() == FunctionType::Kind::ArrayPush)
			return &dynamic_cast<MemberAccess const&>(funCall->expression());
	}
	return nullptr;
}

string SMTEncoder::extraComment()
{
	string extra;
	if (m_arrayAssignmentHappened)
		extra +=
			"\nNote that array aliasing is not supported,"
			" therefore all mapping information is erased after"
			" a mapping local variable/parameter is assigned.\n"
			"You can re-introduce information using require().";
	return extra;
}

FunctionDefinition const* SMTEncoder::functionCallToDefinition(FunctionCall const& _funCall)
{
	if (*_funCall.annotation().kind != FunctionCallKind::FunctionCall)
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

	return funDef;
}

vector<VariableDeclaration const*> SMTEncoder::stateVariablesIncludingInheritedAndPrivate(ContractDefinition const& _contract)
{
	return fold(
		_contract.annotation().linearizedBaseContracts,
		vector<VariableDeclaration const*>{},
		[](auto&& _acc, auto _contract) { return _acc + _contract->stateVariables(); }
	);
}

vector<VariableDeclaration const*> SMTEncoder::stateVariablesIncludingInheritedAndPrivate(FunctionDefinition const& _function)
{
	return stateVariablesIncludingInheritedAndPrivate(dynamic_cast<ContractDefinition const&>(*_function.scope()));
}

vector<VariableDeclaration const*> SMTEncoder::localVariablesIncludingModifiers(FunctionDefinition const& _function)
{
	return _function.localVariables() + modifiersVariables(_function);
}

vector<VariableDeclaration const*> SMTEncoder::modifiersVariables(FunctionDefinition const& _function)
{
	struct BlockVars: ASTConstVisitor
	{
		BlockVars(Block const& _block) { _block.accept(*this); }
		void endVisit(VariableDeclaration const& _var) { vars.push_back(&_var); }
		vector<VariableDeclaration const*> vars;
	};

	vector<VariableDeclaration const*> vars;
	set<ModifierDefinition const*> visited;
	for (auto invok: _function.modifiers())
	{
		if (!invok)
			continue;
		auto decl = invok->name()->annotation().referencedDeclaration;
		auto const* modifier = dynamic_cast<ModifierDefinition const*>(decl);
		if (!modifier || visited.count(modifier))
			continue;

		visited.insert(modifier);
		vars += applyMap(modifier->parameters(), [](auto _var) { return _var.get(); });
		vars += BlockVars(modifier->body()).vars;
	}
	return vars;
}

SourceUnit const* SMTEncoder::sourceUnitContaining(Scopable const& _scopable)
{
	for (auto const* s = &_scopable; s; s = dynamic_cast<Scopable const*>(s->scope()))
		if (auto const* source = dynamic_cast<SourceUnit const*>(s->scope()))
			return source;
	solAssert(false, "");
}

void SMTEncoder::createReturnedExpressions(FunctionCall const& _funCall)
{
	FunctionDefinition const* funDef = functionCallToDefinition(_funCall);
	if (!funDef)
		return;

	auto const& returnParams = funDef->returnParameters();
	for (auto param: returnParams)
		createVariable(*param);

	if (returnParams.size() > 1)
	{
		auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_funCall));
		solAssert(symbTuple, "");
		auto const& symbComponents = symbTuple->components();
		solAssert(symbComponents.size() == returnParams.size(), "");
		for (unsigned i = 0; i < symbComponents.size(); ++i)
		{
			auto param = returnParams.at(i);
			solAssert(param, "");
			solAssert(m_context.knownVariable(*param), "");
			m_context.addAssertion(symbTuple->component(i) == currentValue(*param));
		}
	}
	else if (returnParams.size() == 1)
		defineExpr(_funCall, currentValue(*returnParams.front()));
}

vector<smtutil::Expression> SMTEncoder::symbolicArguments(FunctionCall const& _funCall)
{
	auto const* function = functionCallToDefinition(_funCall);
	solAssert(function, "");

	vector<smtutil::Expression> args;
	Expression const* calledExpr = &_funCall.expression();
	auto const& funType = dynamic_cast<FunctionType const*>(calledExpr->annotation().type);
	solAssert(funType, "");

	auto const& functionParams = function->parameters();
	auto const& arguments = _funCall.arguments();
	unsigned firstParam = 0;
	if (funType->bound())
	{
		auto const& boundFunction = dynamic_cast<MemberAccess const*>(calledExpr);
		solAssert(boundFunction, "");
		args.push_back(expr(boundFunction->expression(), functionParams.front()->type()));
		firstParam = 1;
	}

	solAssert((arguments.size() + firstParam) == functionParams.size(), "");
	for (unsigned i = 0; i < arguments.size(); ++i)
		args.push_back(expr(*arguments.at(i), functionParams.at(i + firstParam)->type()));

	return args;
}
