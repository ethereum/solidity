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

#include <libsolidity/formal/BMC.h>

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsmtutil/SMTPortfolio.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/CharStreamProvider.h>

#ifdef HAVE_Z3_DLOPEN
#include <z3_version.h>
#endif

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

BMC::BMC(
	smt::EncodingContext& _context,
	UniqueErrorReporter& _errorReporter,
	map<h256, string> const& _smtlib2Responses,
	ReadCallback::Callback const& _smtCallback,
	ModelCheckerSettings const& _settings,
	CharStreamProvider const& _charStreamProvider
):
	SMTEncoder(_context, _settings, _errorReporter, _charStreamProvider),
	m_interface(make_unique<smtutil::SMTPortfolio>(_smtlib2Responses, _smtCallback, _settings.solvers, _settings.timeout))
{
#if defined (HAVE_Z3) || defined (HAVE_CVC4)
	if (m_settings.solvers.cvc4 || m_settings.solvers.z3)
		if (!_smtlib2Responses.empty())
			m_errorReporter.warning(
				5622_error,
				"SMT-LIB2 query responses were given in the auxiliary input, "
				"but this Solidity binary uses an SMT solver (Z3/CVC4) directly."
				"These responses will be ignored."
				"Consider disabling Z3/CVC4 at compilation time in order to use SMT-LIB2 responses."
			);
#endif
}

void BMC::analyze(SourceUnit const& _source, map<ASTNode const*, set<VerificationTargetType>, smt::EncodingContext::IdCompare> _solvedTargets)
{
	if (m_interface->solvers() == 0)
	{
		if (!m_noSolverWarning)
		{
			m_noSolverWarning = true;
			m_errorReporter.warning(
				7710_error,
				SourceLocation(),
				"BMC analysis was not possible since no SMT solver was found and enabled."
			);
		}
		return;
	}

	SMTEncoder::resetSourceAnalysis();

	m_solvedTargets = move(_solvedTargets);
	m_context.setSolver(m_interface.get());
	m_context.reset();
	m_context.setAssertionAccumulation(true);
	m_variableUsage.setFunctionInlining(shouldInlineFunctionCall);
	createFreeConstants(sourceDependencies(_source));
	state().prepareForSourceUnit(_source);
	m_unprovedAmt = 0;

	_source.accept(*this);

	if (m_unprovedAmt > 0 && !m_settings.showUnproved)
		m_errorReporter.warning(
			2788_error,
			{},
			"BMC: " +
			to_string(m_unprovedAmt) +
			" verification condition(s) could not be proved." +
			" Enable the model checker option \"show unproved\" to see all of them." +
			" Consider choosing a specific contract to be verified in order to reduce the solving problems." +
			" Consider increasing the timeout per query."
		);

	// If this check is true, Z3 and CVC4 are not available
	// and the query answers were not provided, since SMTPortfolio
	// guarantees that SmtLib2Interface is the first solver, if enabled.
	if (
		!m_interface->unhandledQueries().empty() &&
		m_interface->solvers() == 1 &&
		m_settings.solvers.smtlib2
	)
	{
		if (!m_noSolverWarning)
		{
			m_noSolverWarning = true;
			m_errorReporter.warning(
				8084_error,
				SourceLocation(),
				"BMC analysis was not possible. No SMT solver (Z3 or CVC4) was available."
				" None of the installed solvers was enabled."
#ifdef HAVE_Z3_DLOPEN
				" Install libz3.so." + to_string(Z3_MAJOR_VERSION) + "." + to_string(Z3_MINOR_VERSION) + " to enable Z3."
#endif
			);
		}
	}
}

bool BMC::shouldInlineFunctionCall(
	FunctionCall const& _funCall,
	ContractDefinition const* _scopeContract,
	ContractDefinition const* _contextContract
)
{
	auto funDef = functionCallToDefinition(_funCall, _scopeContract, _contextContract);
	if (!funDef || !funDef->isImplemented())
		return false;

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (funType.kind() == FunctionType::Kind::External)
		return isTrustedExternalCall(&_funCall.expression());
	else if (funType.kind() != FunctionType::Kind::Internal)
		return false;

	return true;
}

/// AST visitors.

bool BMC::visit(ContractDefinition const& _contract)
{
	initContract(_contract);

	SMTEncoder::visit(_contract);

	return false;
}

void BMC::endVisit(ContractDefinition const& _contract)
{
	if (auto constructor = _contract.constructor())
		constructor->accept(*this);
	else
	{
		/// Visiting implicit constructor - we need a dummy callstack frame
		pushCallStack({nullptr, nullptr});
		inlineConstructorHierarchy(_contract);
		popCallStack();
		/// Check targets created by state variable initialization.
		checkVerificationTargets();
		m_verificationTargets.clear();
	}

	SMTEncoder::endVisit(_contract);
}

bool BMC::visit(FunctionDefinition const& _function)
{
	// Free functions need to be visited in the context of a contract.
	if (!m_currentContract)
		return false;

	auto contract = dynamic_cast<ContractDefinition const*>(_function.scope());
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	if (contract && find(hierarchy.begin(), hierarchy.end(), contract) == hierarchy.end())
		createStateVariables(*contract);

	if (m_callStack.empty())
	{
		reset();
		initFunction(_function);
		if (_function.isConstructor() || _function.isPublic())
			m_context.addAssertion(state().txTypeConstraints() && state().txFunctionConstraints(_function));
		resetStateVariables();
	}

	if (_function.isConstructor())
	{
		solAssert(contract, "");
		inlineConstructorHierarchy(*contract);
	}

	/// Already visits the children.
	SMTEncoder::visit(_function);

	return false;
}

void BMC::endVisit(FunctionDefinition const& _function)
{
	// Free functions need to be visited in the context of a contract.
	if (!m_currentContract)
		return;

	if (isRootFunction())
	{
		checkVerificationTargets();
		m_verificationTargets.clear();
		m_pathConditions.clear();
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
			VerificationTargetType::ConstantCondition,
			expr(_node.condition()),
			&_node.condition()
		);
	m_context.popSolver();

	_node.condition().accept(*this);
	auto conditionExpr = expr(_node.condition());
	// visit true branch
	auto [indicesEndTrue, trueEndPathCondition] = visitBranch(&_node.trueStatement(), conditionExpr);

	// visit false branch
	decltype(indicesEndTrue) indicesEndFalse;
	auto falseEndPathCondition = currentPathConditions() && !conditionExpr;
	if (_node.falseStatement())
		std::tie(indicesEndFalse, falseEndPathCondition) = visitBranch(_node.falseStatement(), !conditionExpr);
	else
		indicesEndFalse = copyVariableIndices();

	// merge the information from branches
	setPathCondition(trueEndPathCondition || falseEndPathCondition);
	mergeVariables(expr(_node.condition()), indicesEndTrue, indicesEndFalse);

	return false;
}

bool BMC::visit(Conditional const& _op)
{
	m_context.pushSolver();
	_op.condition().accept(*this);

	if (isRootFunction())
		addVerificationTarget(
			VerificationTargetType::ConstantCondition,
			expr(_op.condition()),
			&_op.condition()
		);
	m_context.popSolver();

	SMTEncoder::visit(_op);

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
	m_context.resetVariables(touchedVariables(_node));
	decltype(indicesBeforeLoop) indicesAfterLoop;
	if (_node.isDoWhile())
	{
		indicesAfterLoop = visitBranch(&_node.body()).first;
		// TODO the assertions generated in the body should still be active in the condition
		_node.condition().accept(*this);
		if (isRootFunction())
			addVerificationTarget(
				VerificationTargetType::ConstantCondition,
				expr(_node.condition()),
				&_node.condition()
			);
	}
	else
	{
		_node.condition().accept(*this);
		if (isRootFunction())
			addVerificationTarget(
				VerificationTargetType::ConstantCondition,
				expr(_node.condition()),
				&_node.condition()
			);

		indicesAfterLoop = visitBranch(&_node.body(), expr(_node.condition())).first;
	}

	// We reset the execution to before the loop
	// and visit the condition in case it's not a do-while.
	// A do-while's body might have non-precise information
	// in its first run about variables that are touched.
	resetVariableIndices(indicesBeforeLoop);
	if (!_node.isDoWhile())
		_node.condition().accept(*this);

	mergeVariables(expr(_node.condition()), indicesAfterLoop, copyVariableIndices());

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
				VerificationTargetType::ConstantCondition,
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

	auto forCondition = _node.condition() ? expr(*_node.condition()) : smtutil::Expression(true);
	mergeVariables(forCondition, indicesAfterLoop, copyVariableIndices());

	m_loopExecutionHappened = true;
	return false;
}

bool BMC::visit(TryStatement const& _tryStatement)
{
	FunctionCall const* externalCall = dynamic_cast<FunctionCall const*>(&_tryStatement.externalCall());
	solAssert(externalCall && externalCall->annotation().tryCall, "");

	externalCall->accept(*this);
	if (_tryStatement.successClause()->parameters())
		expressionToTupleAssignment(_tryStatement.successClause()->parameters()->parameters(), *externalCall);

	smtutil::Expression clauseId = m_context.newVariable("clause_choice_" + to_string(m_context.newUniqueId()), smtutil::SortProvider::uintSort);
	auto const& clauses = _tryStatement.clauses();
	m_context.addAssertion(clauseId >= 0 && clauseId < clauses.size());
	solAssert(clauses[0].get() == _tryStatement.successClause(), "First clause of TryStatement should be the success clause");
	vector<pair<VariableIndices, smtutil::Expression>> clausesVisitResults;
	for (size_t i = 0; i < clauses.size(); ++i)
		clausesVisitResults.push_back(visitBranch(clauses[i].get()));

	// merge the information from all clauses
	smtutil::Expression pathCondition = clausesVisitResults.front().second;
	auto currentIndices = clausesVisitResults[0].first;
	for (size_t i = 1; i < clauses.size(); ++i)
	{
		mergeVariables(clauseId == i, clausesVisitResults[i].first, currentIndices);
		currentIndices = copyVariableIndices();
		pathCondition = pathCondition || clausesVisitResults[i].second;
	}
	setPathCondition(pathCondition);

	return false;
}

void BMC::endVisit(UnaryOperation const& _op)
{
	SMTEncoder::endVisit(_op);

	if (
		_op.annotation().type->category() == Type::Category::RationalNumber ||
		_op.annotation().type->category() == Type::Category::FixedPoint
	)
		return;

	if (_op.getOperator() == Token::Sub && smt::isInteger(*_op.annotation().type))
		addVerificationTarget(
			VerificationTargetType::UnderOverflow,
			expr(_op),
			&_op
		);
}

void BMC::endVisit(FunctionCall const& _funCall)
{
	auto functionCallKind = *_funCall.annotation().kind;

	if (functionCallKind != FunctionCallKind::FunctionCall)
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
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		auto value = _funCall.arguments().front();
		solAssert(value, "");
		smtutil::Expression thisBalance = state().balance();

		addVerificationTarget(
			VerificationTargetType::Balance,
			thisBalance < expr(*value),
			&_funCall
		);

		SMTEncoder::endVisit(_funCall);
		break;
	}
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
	case FunctionType::Kind::Unwrap:
	case FunctionType::Kind::Wrap:
		[[fallthrough]];
	default:
		SMTEncoder::endVisit(_funCall);
		break;
	}
}

void BMC::endVisit(Return const& _return)
{
	SMTEncoder::endVisit(_return);
	setPathCondition(smtutil::Expression(false));
}

/// Visitor helpers.

void BMC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addVerificationTarget(
		VerificationTargetType::Assert,
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
			VerificationTargetType::ConstantCondition,
			expr(*args.front()),
			args.front().get()
		);
}

void BMC::visitAddMulMod(FunctionCall const& _funCall)
{
	solAssert(_funCall.arguments().at(2), "");
	addVerificationTarget(
		VerificationTargetType::DivByZero,
		expr(*_funCall.arguments().at(2)),
		&_funCall
	);

	SMTEncoder::visitAddMulMod(_funCall);
}

void BMC::inlineFunctionCall(FunctionCall const& _funCall)
{
	solAssert(shouldInlineFunctionCall(_funCall, currentScopeContract(), m_currentContract), "");
	auto funDef = functionCallToDefinition(_funCall, currentScopeContract(), m_currentContract);
	solAssert(funDef, "");

	if (visitedFunction(funDef))
	{
		auto const& returnParams = funDef->returnParameters();
		for (auto param: returnParams)
		{
			m_context.newValue(*param);
			m_context.setUnknownValue(*param);
		}
	}
	else
	{
		initializeFunctionCallParameters(*funDef, symbolicArguments(_funCall, m_currentContract));

		// The reason why we need to pushCallStack here instead of visit(FunctionDefinition)
		// is that there we don't have `_funCall`.
		pushCallStack({funDef, &_funCall});
		pushPathCondition(currentPathConditions());
		auto oldChecked = std::exchange(m_checked, true);
		funDef->accept(*this);
		m_checked = oldChecked;
		popPathCondition();
	}

	createReturnedExpressions(_funCall, m_currentContract);
}

void BMC::internalOrExternalFunctionCall(FunctionCall const& _funCall)
{
	auto const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	if (shouldInlineFunctionCall(_funCall, currentScopeContract(), m_currentContract))
		inlineFunctionCall(_funCall);
	else if (isPublicGetter(_funCall.expression()))
	{
		// Do nothing here.
		// The processing happens in SMT Encoder, but we need to prevent the resetting of the state variables.
	}
	else if (funType.kind() == FunctionType::Kind::Internal)
		m_errorReporter.warning(
			5729_error,
			_funCall.location(),
			"BMC does not yet implement this type of function call."
		);
	else if (funType.kind() == FunctionType::Kind::BareStaticCall)
	{
		// Do nothing here.
		// Neither storage nor balances should be modified.
	}
	else
	{
		m_externalFunctionCallHappened = true;
		resetStorageVariables();
		resetBalances();
	}
}

pair<smtutil::Expression, smtutil::Expression> BMC::arithmeticOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	Type const* _commonType,
	Expression const& _expression
)
{
	// Unchecked does not disable div by 0 checks.
	if (_op == Token::Div || _op == Token::Mod)
		addVerificationTarget(
			VerificationTargetType::DivByZero,
			_right,
			&_expression
		);

	auto values = SMTEncoder::arithmeticOperation(_op, _left, _right, _commonType, _expression);

	if (!m_checked)
		return values;

	auto const* intType = dynamic_cast<IntegerType const*>(_commonType);
	if (!intType)
		intType = TypeProvider::uint256();

	// Mod does not need underflow/overflow checks.
	if (_op == Token::Mod)
		return values;

	VerificationTargetType type;
	// The order matters here:
	// If _op is Div and intType is signed, we only care about overflow.
	if (_op == Token::Div)
	{
		if (intType->isSigned())
			// Signed division can only overflow.
			type = VerificationTargetType::Overflow;
		else
			// Unsigned division cannot underflow/overflow.
			return values;
	}
	else if (intType->isSigned())
		type = VerificationTargetType::UnderOverflow;
	else if (_op == Token::Sub)
		type = VerificationTargetType::Underflow;
	else if (_op == Token::Add || _op == Token::Mul)
		type = VerificationTargetType::Overflow;
	else
		solAssert(false, "");

	addVerificationTarget(
		type,
		values.second,
		&_expression
	);
	return values;
}

void BMC::reset()
{
	m_externalFunctionCallHappened = false;
	m_loopExecutionHappened = false;
}

pair<vector<smtutil::Expression>, vector<string>> BMC::modelExpressions()
{
	vector<smtutil::Expression> expressionsToEvaluate;
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
			smt::smtKind(*type) != smtutil::Kind::Function
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
			string expressionName;
			if (uf->location().hasText())
				expressionName = m_charStreamProvider.charStream(*uf->location().sourceName).text(
					uf->location()
				);
			expressionNames.push_back(move(expressionName));
		}

	return {expressionsToEvaluate, expressionNames};
}

/// Verification targets.

void BMC::checkVerificationTargets()
{
	for (auto& target: m_verificationTargets)
		checkVerificationTarget(target);
}

void BMC::checkVerificationTarget(BMCVerificationTarget& _target)
{
	switch (_target.type)
	{
		case VerificationTargetType::ConstantCondition:
			checkConstantCondition(_target);
			break;
		case VerificationTargetType::Underflow:
			checkUnderflow(_target);
			break;
		case VerificationTargetType::Overflow:
			checkOverflow(_target);
			break;
		case VerificationTargetType::UnderOverflow:
			checkUnderflow(_target);
			checkOverflow(_target);
			break;
		case VerificationTargetType::DivByZero:
			checkDivByZero(_target);
			break;
		case VerificationTargetType::Balance:
			checkBalance(_target);
			break;
		case VerificationTargetType::Assert:
			checkAssert(_target);
			break;
		default:
			solAssert(false, "");
	}
}

void BMC::checkConstantCondition(BMCVerificationTarget& _target)
{
	checkBooleanNotConstant(
		*_target.expression,
		_target.constraints,
		_target.value,
		_target.callStack
	);
}

void BMC::checkUnderflow(BMCVerificationTarget& _target)
{
	solAssert(
		_target.type == VerificationTargetType::Underflow ||
			_target.type == VerificationTargetType::UnderOverflow,
		""
	);

	if (
		m_solvedTargets.count(_target.expression) && (
			m_solvedTargets.at(_target.expression).count(VerificationTargetType::Underflow) ||
			m_solvedTargets.at(_target.expression).count(VerificationTargetType::UnderOverflow)
		)
	)
		return;

	auto const* intType = dynamic_cast<IntegerType const*>(_target.expression->annotation().type);
	if (!intType)
		intType = TypeProvider::uint256();

	checkCondition(
		_target.constraints && _target.value < smt::minValue(*intType),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		4144_error,
		8312_error,
		"Underflow (resulting value less than " + formatNumberReadable(intType->minValue()) + ")",
		"<result>",
		&_target.value
	);
}

void BMC::checkOverflow(BMCVerificationTarget& _target)
{
	solAssert(
		_target.type == VerificationTargetType::Overflow ||
			_target.type == VerificationTargetType::UnderOverflow,
		""
	);

	if (
		m_solvedTargets.count(_target.expression) && (
			m_solvedTargets.at(_target.expression).count(VerificationTargetType::Overflow) ||
			m_solvedTargets.at(_target.expression).count(VerificationTargetType::UnderOverflow)
		)
	)
		return;

	auto const* intType = dynamic_cast<IntegerType const*>(_target.expression->annotation().type);
	if (!intType)
		intType = TypeProvider::uint256();

	checkCondition(
		_target.constraints && _target.value > smt::maxValue(*intType),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		2661_error,
		8065_error,
		"Overflow (resulting value larger than " + formatNumberReadable(intType->maxValue()) + ")",
		"<result>",
		&_target.value
	);
}

void BMC::checkDivByZero(BMCVerificationTarget& _target)
{
	solAssert(_target.type == VerificationTargetType::DivByZero, "");

	if (
		m_solvedTargets.count(_target.expression) &&
		m_solvedTargets.at(_target.expression).count(VerificationTargetType::DivByZero)
	)
		return;

	checkCondition(
		_target.constraints && (_target.value == 0),
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		3046_error,
		5272_error,
		"Division by zero",
		"<result>",
		&_target.value
	);
}

void BMC::checkBalance(BMCVerificationTarget& _target)
{
	solAssert(_target.type == VerificationTargetType::Balance, "");
	checkCondition(
		_target.constraints && _target.value,
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		1236_error,
		4010_error,
		"Insufficient funds",
		"address(this).balance"
	);
}

void BMC::checkAssert(BMCVerificationTarget& _target)
{
	solAssert(_target.type == VerificationTargetType::Assert, "");

	if (
		m_solvedTargets.count(_target.expression) &&
		m_solvedTargets.at(_target.expression).count(_target.type)
	)
		return;

	checkCondition(
		_target.constraints && !_target.value,
		_target.callStack,
		_target.modelExpressions,
		_target.expression->location(),
		4661_error,
		7812_error,
		"Assertion violation"
	);
}

void BMC::addVerificationTarget(
	VerificationTargetType _type,
	smtutil::Expression const& _value,
	Expression const* _expression
)
{
	if (!m_settings.targets.has(_type) || (m_currentContract && !shouldAnalyze(*m_currentContract)))
		return;

	BMCVerificationTarget target{
		{
			_type,
			_value,
			currentPathConditions() && m_context.assertions()
		},
		_expression,
		m_callStack,
		modelExpressions()
	};
	if (_type == VerificationTargetType::ConstantCondition)
		checkVerificationTarget(target);
	else
		m_verificationTargets.emplace_back(move(target));
}

/// Solving.

void BMC::checkCondition(
	smtutil::Expression _condition,
	vector<SMTEncoder::CallStackEntry> const& _callStack,
	pair<vector<smtutil::Expression>, vector<string>> const& _modelExpressions,
	SourceLocation const& _location,
	ErrorId _errorHappens,
	ErrorId _errorMightHappen,
	string const& _description,
	string const& _additionalValueName,
	smtutil::Expression const* _additionalValue
)
{
	m_interface->push();
	m_interface->addAssertion(_condition);

	vector<smtutil::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	tie(expressionsToEvaluate, expressionNames) = _modelExpressions;
	if (_callStack.size())
		if (_additionalValue)
		{
			expressionsToEvaluate.emplace_back(*_additionalValue);
			expressionNames.push_back(_additionalValueName);
		}
	smtutil::CheckResult result;
	vector<string> values;
	tie(result, values) = checkSatisfiableAndGenerateModel(expressionsToEvaluate);

	string extraComment = SMTEncoder::extraComment();
	if (m_loopExecutionHappened)
		extraComment +=
			"\nNote that some information is erased after the execution of loops.\n"
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
	case smtutil::CheckResult::SATISFIABLE:
	{
		solAssert(!_callStack.empty(), "");
		std::ostringstream message;
		message << "BMC: " << _description << " happens here.";

		std::ostringstream modelMessage;
		// Sometimes models have complex smtlib2 expressions that SMTLib2Interface fails to parse.
		if (values.size() == expressionNames.size())
		{
			modelMessage << "Counterexample:\n";
			map<string, string> sortedModel;
			for (size_t i = 0; i < values.size(); ++i)
				if (expressionsToEvaluate.at(i).name != values.at(i))
					sortedModel[expressionNames.at(i)] = values.at(i);

			for (auto const& eval: sortedModel)
				modelMessage << "  " << eval.first << " = " << eval.second << "\n";
		}

		m_errorReporter.warning(
			_errorHappens,
			_location,
			message.str(),
			SecondarySourceLocation().append(modelMessage.str(), SourceLocation{})
			.append(SMTEncoder::callStackMessage(_callStack))
			.append(move(secondaryLocation))
		);
		break;
	}
	case smtutil::CheckResult::UNSATISFIABLE:
		break;
	case smtutil::CheckResult::UNKNOWN:
	{
		++m_unprovedAmt;
		if (m_settings.showUnproved)
			m_errorReporter.warning(_errorMightHappen, _location, "BMC: " + _description + " might happen here.", secondaryLocation);
		break;
	}
	case smtutil::CheckResult::CONFLICTING:
		m_errorReporter.warning(1584_error, _location, "BMC: At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case smtutil::CheckResult::ERROR:
		m_errorReporter.warning(1823_error, _location, "BMC: Error trying to invoke SMT solver.");
		break;
	}

	m_interface->pop();
}

void BMC::checkBooleanNotConstant(
	Expression const& _condition,
	smtutil::Expression const& _constraints,
	smtutil::Expression const& _value,
	vector<SMTEncoder::CallStackEntry> const& _callStack
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

	if (positiveResult == smtutil::CheckResult::ERROR || negatedResult == smtutil::CheckResult::ERROR)
		m_errorReporter.warning(8592_error, _condition.location(), "BMC: Error trying to invoke SMT solver.");
	else if (positiveResult == smtutil::CheckResult::CONFLICTING || negatedResult == smtutil::CheckResult::CONFLICTING)
		m_errorReporter.warning(3356_error, _condition.location(), "BMC: At least two SMT solvers provided conflicting answers. Results might not be sound.");
	else if (positiveResult == smtutil::CheckResult::SATISFIABLE && negatedResult == smtutil::CheckResult::SATISFIABLE)
	{
		// everything fine.
	}
	else if (positiveResult == smtutil::CheckResult::UNKNOWN || negatedResult == smtutil::CheckResult::UNKNOWN)
	{
		// can't do anything.
	}
	else if (positiveResult == smtutil::CheckResult::UNSATISFIABLE && negatedResult == smtutil::CheckResult::UNSATISFIABLE)
		m_errorReporter.warning(2512_error, _condition.location(), "BMC: Condition unreachable.", SMTEncoder::callStackMessage(_callStack));
	else
	{
		string description;
		if (positiveResult == smtutil::CheckResult::SATISFIABLE)
		{
			solAssert(negatedResult == smtutil::CheckResult::UNSATISFIABLE, "");
			description = "BMC: Condition is always true.";
		}
		else
		{
			solAssert(positiveResult == smtutil::CheckResult::UNSATISFIABLE, "");
			solAssert(negatedResult == smtutil::CheckResult::SATISFIABLE, "");
			description = "BMC: Condition is always false.";
		}
		m_errorReporter.warning(
			6838_error,
			_condition.location(),
			description,
			SMTEncoder::callStackMessage(_callStack)
		);
	}
}

pair<smtutil::CheckResult, vector<string>>
BMC::checkSatisfiableAndGenerateModel(vector<smtutil::Expression> const& _expressionsToEvaluate)
{
	smtutil::CheckResult result;
	vector<string> values;
	try
	{
		tie(result, values) = m_interface->check(_expressionsToEvaluate);
	}
	catch (smtutil::SolverError const& _e)
	{
		string description("BMC: Error querying SMT solver");
		if (_e.comment())
			description += ": " + *_e.comment();
		m_errorReporter.warning(8140_error, description);
		result = smtutil::CheckResult::ERROR;
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

smtutil::CheckResult BMC::checkSatisfiable()
{
	return checkSatisfiableAndGenerateModel({}).first;
}

void BMC::assignment(smt::SymbolicVariable& _symVar, smtutil::Expression const& _value)
{
	auto oldVar = _symVar.currentValue();
	auto newVar = _symVar.increaseIndex();
	m_context.addAssertion(smtutil::Expression::ite(
		currentPathConditions(),
		newVar == _value,
		newVar == oldVar
	));
}

