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

#include <libsolidity/formal/BMC.h>

#include <libsolidity/formal/SMTPortfolio.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

BMC::BMC(smt::EncodingContext& _context, ErrorReporter& _errorReporter, map<h256, string> const& _smtlib2Responses):
	SMTEncoder(_context),
	m_outerErrorReporter(_errorReporter),
	m_interface(make_shared<smt::SMTPortfolio>(_smtlib2Responses))
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

void BMC::analyze(SourceUnit const& _source, set<Expression const*> _safeAssertions)
{
	solAssert(_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker), "");

	m_safeAssertions += move(_safeAssertions);
	m_context.setSolver(m_interface);
	m_context.clear();
	m_context.setAssertionAccumulation(true);
	m_variableUsage.setFunctionInlining(true);

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
			m_outerErrorReporter.warning(
				SourceLocation(),
				"BMC analysis was not possible since no integrated SMT solver (Z3 or CVC4) was found."
			);
		}
	}
	else
		m_outerErrorReporter.append(m_errorReporter.errors());

	m_errorReporter.clear();
}

bool BMC::shouldInlineFunctionCall(FunctionCall const& _funCall)
{
	FunctionDefinition const* funDef = functionCallToDefinition(_funCall);
	if (!funDef || !funDef->isImplemented())
		return false;

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (funType.kind() == FunctionType::Kind::External)
	{
		auto memberAccess = dynamic_cast<MemberAccess const*>(&_funCall.expression());
		if (!memberAccess)
			return false;

		auto identifier = dynamic_cast<Identifier const*>(&memberAccess->expression());
		if (!(
			identifier &&
			identifier->name() == "this" &&
			identifier->annotation().referencedDeclaration &&
			dynamic_cast<MagicVariableDeclaration const*>(identifier->annotation().referencedDeclaration)
		))
			return false;
	}
	else if (funType.kind() != FunctionType::Kind::Internal)
		return false;

	return true;
}

/// AST visitors.

bool BMC::visit(ContractDefinition const& _contract)
{
	initContract(_contract);

	/// Check targets created by state variable initialization.
	smt::Expression constraints = m_context.assertions();
	checkVerificationTargets(constraints);
	m_verificationTargets.clear();

	SMTEncoder::visit(_contract);

	return false;
}

void BMC::endVisit(ContractDefinition const& _contract)
{
	SMTEncoder::endVisit(_contract);
}

bool BMC::visit(FunctionDefinition const& _function)
{
	auto contract = dynamic_cast<ContractDefinition const*>(_function.scope());
	solAssert(contract, "");
	solAssert(m_currentContract, "");
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	if (find(hierarchy.begin(), hierarchy.end(), contract) == hierarchy.end())
		initializeStateVariables(*contract);

	if (m_callStack.empty())
		reset();

	/// Already visits the children.
	SMTEncoder::visit(_function);

	return false;
}

void BMC::endVisit(FunctionDefinition const& _function)
{
	if (isRootFunction())
	{
		smt::Expression constraints = m_context.assertions();
		checkVerificationTargets(constraints);
		m_verificationTargets.clear();
	}

	SMTEncoder::endVisit(_function);
}

bool BMC::visit(IfStatement const& _node)
{
	// This check needs to be done in its own context otherwise
	// constraints from the If body might influence it.
	m_context.pushSolver();
	_node.condition().accept(*this);

	// We ignore called functions here because they have
	// specific input values.
	if (isRootFunction())
		addVerificationTarget(
			VerificationTarget::Type::ConstantCondition,
			expr(_node.condition()),
			&_node.condition()
		);
	m_context.popSolver();

	SMTEncoder::visit(_node);

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
bool BMC::visit(WhileStatement const& _node)
{
	auto indicesBeforeLoop = copyVariableIndices();
	auto touchedVars = touchedVariables(_node);
	m_context.resetVariables(touchedVars);
	decltype(indicesBeforeLoop) indicesAfterLoop;
	if (_node.isDoWhile())
	{
		indicesAfterLoop = visitBranch(&_node.body());
		// TODO the assertions generated in the body should still be active in the condition
		_node.condition().accept(*this);
		if (isRootFunction())
			addVerificationTarget(
				VerificationTarget::Type::ConstantCondition,
				expr(_node.condition()),
				&_node.condition()
			);
	}
	else
	{
		_node.condition().accept(*this);
		if (isRootFunction())
			addVerificationTarget(
				VerificationTarget::Type::ConstantCondition,
				expr(_node.condition()),
				&_node.condition()
			);

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
bool BMC::visit(ForStatement const& _node)
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

	m_context.resetVariables(touchedVars);

	if (_node.condition())
	{
		_node.condition()->accept(*this);
		if (isRootFunction())
			addVerificationTarget(
				VerificationTarget::Type::ConstantCondition,
				expr(*_node.condition()),
				_node.condition()
			);
	}

	m_context.pushSolver();
	if (_node.condition())
		m_context.addAssertion(expr(*_node.condition()));
	_node.body().accept(*this);
	if (_node.loopExpression())
		_node.loopExpression()->accept(*this);
	m_context.popSolver();

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

void BMC::endVisit(UnaryOperation const& _op)
{
	SMTEncoder::endVisit(_op);

	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;

	switch (_op.getOperator())
	{
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
		addVerificationTarget(
			VerificationTarget::Type::UnderOverflow,
			expr(_op),
			&_op
		);
		break;
	case Token::Sub: // -
		if (_op.annotation().type->category() == Type::Category::Integer)
			addVerificationTarget(
				VerificationTarget::Type::UnderOverflow,
				expr(_op),
				&_op
			);
		break;
	default:
		break;
	}
}

void BMC::endVisit(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind != FunctionCallKind::Unset, "");
	if (_funCall.annotation().kind != FunctionCallKind::FunctionCall)
	{
		SMTEncoder::endVisit(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	switch (funType.kind())
	{
	case FunctionType::Kind::Assert:
		visitAssert(_funCall);
		SMTEncoder::endVisit(_funCall);
		break;
	case FunctionType::Kind::Require:
		visitRequire(_funCall);
		SMTEncoder::endVisit(_funCall);
		break;
	case FunctionType::Kind::Internal:
	case FunctionType::Kind::External:
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::BareStaticCall:
	case FunctionType::Kind::Creation:
		SMTEncoder::endVisit(_funCall);
		internalOrExternalFunctionCall(_funCall);
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		SMTEncoder::endVisit(_funCall);
		abstractFunctionCall(_funCall);
		break;
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		SMTEncoder::endVisit(_funCall);
		auto value = _funCall.arguments().front();
		solAssert(value, "");
		smt::Expression thisBalance = m_context.balance();

		addVerificationTarget(
			VerificationTarget::Type::Balance,
			thisBalance < expr(*value),
			&_funCall
		);
		break;
	}
	default:
		SMTEncoder::endVisit(_funCall);
		break;
	}
}

/// Visitor helpers.

void BMC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addVerificationTarget(
		VerificationTarget::Type::Assert,
		expr(*args.front()),
		&_funCall
	);
}

void BMC::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	if (isRootFunction())
		addVerificationTarget(
			VerificationTarget::Type::ConstantCondition,
			expr(*args.front()),
			args.front().get()
		);
}

void BMC::inlineFunctionCall(FunctionCall const& _funCall)
{
	solAssert(shouldInlineFunctionCall(_funCall), "");
	FunctionDefinition const* funDef = functionCallToDefinition(_funCall);
	solAssert(funDef, "");

	if (visitedFunction(funDef))
	{
		auto const& returnParams = funDef->returnParameters();
		for (auto param: returnParams)
		{
			m_context.newValue(*param);
			m_context.setUnknownValue(*param);
		}

		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not support recursive function calls.",
			SecondarySourceLocation().append("Starting from function:", funDef->location())
		);
	}
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
		// If an internal function is called to initialize
		// a state variable.
		if (m_callStack.empty())
			initFunction(*funDef);
		funDef->accept(*this);
	}

	createReturnedExpressions(_funCall);
}

void BMC::abstractFunctionCall(FunctionCall const& _funCall)
{
	vector<smt::Expression> smtArguments;
	for (auto const& arg: _funCall.arguments())
		smtArguments.push_back(expr(*arg));
	defineExpr(_funCall, (*m_context.expression(_funCall.expression()))(smtArguments));
	m_uninterpretedTerms.insert(&_funCall);
	setSymbolicUnknownValue(expr(_funCall), _funCall.annotation().type, m_context);
}

void BMC::internalOrExternalFunctionCall(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (shouldInlineFunctionCall(_funCall))
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

pair<smt::Expression, smt::Expression> BMC::arithmeticOperation(
	Token _op,
	smt::Expression const& _left,
	smt::Expression const& _right,
	TypePointer const& _commonType,
	Expression const& _expression
)
{
	if (_op == Token::Div || _op == Token::Mod)
		addVerificationTarget(
			VerificationTarget::Type::DivByZero,
			_right,
			&_expression
		);

	auto values = SMTEncoder::arithmeticOperation(_op, _left, _right, _commonType, _expression);

	addVerificationTarget(
		VerificationTarget::Type::UnderOverflow,
		values.second,
		&_expression
	);
	return values;
}

void BMC::resetStorageReferences()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.hasReferenceOrMappingType(); });
}

void BMC::reset()
{
	m_externalFunctionCallHappened = false;
	m_loopExecutionHappened = false;
}

pair<vector<smt::Expression>, vector<string>> BMC::modelExpressions()
{
	vector<smt::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	for (auto const& var: m_context.variables())
		if (var.first->type()->isValueType())
		{
			expressionsToEvaluate.emplace_back(currentValue(*var.first));
			expressionNames.push_back(var.first->name());
		}
	for (auto const& var: m_context.globalSymbols())
	{
		auto const& type = var.second->type();
		if (
			type->isValueType() &&
			smt::smtKind(type->category()) != smt::Kind::Function
		)
		{
			expressionsToEvaluate.emplace_back(var.second->currentValue());
			expressionNames.push_back(var.first);
		}
	}
	for (auto const& uf: m_uninterpretedTerms)
		if (uf->annotation().type->isValueType())
		{
			expressionsToEvaluate.emplace_back(expr(*uf));
			expressionNames.push_back(uf->location().text());
		}

	return {expressionsToEvaluate, expressionNames};
}

/// Verification targets.

void BMC::checkVerificationTargets(smt::Expression const& _constraints)
{
	for (auto& target: m_verificationTargets)
		checkVerificationTarget(target, _constraints);
}

void BMC::checkVerificationTarget(VerificationTarget& _target, smt::Expression const& _constraints)
{
	switch (_target.type)
	{
		case VerificationTarget::Type::ConstantCondition:
			checkConstantCondition(_target);
			break;
		case VerificationTarget::Type::Underflow:
			checkUnderflow(_target, _constraints);
			break;
		case VerificationTarget::Type::Overflow:
			checkOverflow(_target, _constraints);
			break;
		case VerificationTarget::Type::UnderOverflow:
			checkUnderflow(_target, _constraints);
			checkOverflow(_target, _constraints);
			break;
		case VerificationTarget::Type::DivByZero:
			checkDivByZero(_target);
			break;
		case VerificationTarget::Type::Balance:
			checkBalance(_target);
			break;
		case VerificationTarget::Type::Assert:
			checkAssert(_target);
			break;
		default:
			solAssert(false, "");
	}
}

void BMC::checkConstantCondition(VerificationTarget& _target)
{
	checkBooleanNotConstant(
		*_target.expression,
		_target.constraints,
		_target.value,
		_target.callStack,
		"Condition is always $VALUE."
	);
}

void BMC::checkUnderflow(VerificationTarget& _target, smt::Expression const& _constraints)
{
	solAssert(
		_target.type == VerificationTarget::Type::Underflow ||
			_target.type == VerificationTarget::Type::UnderOverflow,
		""
	);
	auto intType = dynamic_cast<IntegerType const*>(_target.expression->annotation().type);
	solAssert(intType, "");
	checkCondition(
		_target.constraints && _constraints && _target.value < smt::minValue(*intType),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		"Underflow (resulting value less than " + formatNumberReadable(intType->minValue()) + ")",
		"<result>",
		&_target.value
	);
}

void BMC::checkOverflow(VerificationTarget& _target, smt::Expression const& _constraints)
{
	solAssert(
		_target.type == VerificationTarget::Type::Overflow ||
			_target.type == VerificationTarget::Type::UnderOverflow,
		""
	);
	auto intType = dynamic_cast<IntegerType const*>(_target.expression->annotation().type);
	solAssert(intType, "");
	checkCondition(
		_target.constraints && _constraints && _target.value > smt::maxValue(*intType),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		"Overflow (resulting value larger than " + formatNumberReadable(intType->maxValue()) + ")",
		"<result>",
		&_target.value
	);
}

void BMC::checkDivByZero(VerificationTarget& _target)
{
	solAssert(_target.type == VerificationTarget::Type::DivByZero, "");
	checkCondition(
		_target.constraints && (_target.value == 0),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		"Division by zero",
		"<result>",
		&_target.value
	);
}

void BMC::checkBalance(VerificationTarget& _target)
{
	solAssert(_target.type == VerificationTarget::Type::Balance, "");
	checkCondition(
		_target.constraints && _target.value,
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		"Insufficient funds",
		"address(this).balance"
	);
}

void BMC::checkAssert(VerificationTarget& _target)
{
	solAssert(_target.type == VerificationTarget::Type::Assert, "");
	if (!m_safeAssertions.count(_target.expression))
		checkCondition(
			_target.constraints && !_target.value,
			_target.callStack,
			_target.modelExpressions,
			_target.expression->location(),
			"Assertion violation"
		);
}

void BMC::addVerificationTarget(
	VerificationTarget::Type _type,
	smt::Expression const& _value,
	Expression const* _expression
)
{
	VerificationTarget target{
		_type,
		_value,
		currentPathConditions() && m_context.assertions(),
		_expression,
		m_callStack,
		modelExpressions()
	};
	if (_type == VerificationTarget::Type::ConstantCondition)
		checkVerificationTarget(target);
	else
		m_verificationTargets.emplace_back(move(target));
}

/// Solving.

void BMC::checkCondition(
	smt::Expression _condition,
	vector<SMTEncoder::CallStackEntry> const& callStack,
	pair<vector<smt::Expression>, vector<string>> const& _modelExpressions,
	SourceLocation const& _location,
	string const& _description,
	string const& _additionalValueName,
	smt::Expression const* _additionalValue
)
{
	m_interface->push();
	m_interface->addAssertion(_condition);

	vector<smt::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	tie(expressionsToEvaluate, expressionNames) = _modelExpressions;
	if (callStack.size())
		if (_additionalValue)
		{
			expressionsToEvaluate.emplace_back(*_additionalValue);
			expressionNames.push_back(_additionalValueName);
		}
	smt::CheckResult result;
	vector<string> values;
	tie(result, values) = checkSatisfiableAndGenerateModel(expressionsToEvaluate);

	string extraComment = SMTEncoder::extraComment();
	if (m_loopExecutionHappened)
		extraComment +=
			"\nNote that some information is erased after the execution of loops.\n"
			"You can re-introduce information using require().";
	if (m_externalFunctionCallHappened)
		extraComment+=
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
		if (callStack.size())
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
				.append(SMTEncoder::callStackMessage(callStack))
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

void BMC::checkBooleanNotConstant(
	Expression const& _condition,
	smt::Expression const& _constraints,
	smt::Expression const& _value,
	vector<SMTEncoder::CallStackEntry> const& _callStack,
	string const& _description
)
{
	// Do not check for const-ness if this is a constant.
	if (dynamic_cast<Literal const*>(&_condition))
		return;

	m_interface->push();
	m_interface->addAssertion(_constraints && _value);
	auto positiveResult = checkSatisfiable();
	m_interface->pop();

	m_interface->push();
	m_interface->addAssertion(_constraints && !_value);
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
		m_errorReporter.warning(_condition.location(), "Condition unreachable.", SMTEncoder::callStackMessage(_callStack));
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
			SMTEncoder::callStackMessage(_callStack)
		);
	}
}

pair<smt::CheckResult, vector<string>>
BMC::checkSatisfiableAndGenerateModel(vector<smt::Expression> const& _expressionsToEvaluate)
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

smt::CheckResult BMC::checkSatisfiable()
{
	return checkSatisfiableAndGenerateModel({}).first;
}

