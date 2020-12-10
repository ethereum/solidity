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

#include <libsolidity/formal/CHC.h>

#ifdef HAVE_Z3
#include <libsmtutil/Z3CHCInterface.h>
#endif

#include <libsolidity/formal/ArraySlicePredicate.h>
#include <libsolidity/formal/PredicateInstance.h>
#include <libsolidity/formal/PredicateSort.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/TypeProvider.h>

#include <libsmtutil/CHCSmtLib2Interface.h>
#include <libsolutil/Algorithms.h>

#include <boost/range/adaptor/reversed.hpp>

#ifdef HAVE_Z3_DLOPEN
#include <z3_version.h>
#endif

#include <queue>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::smtutil;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

CHC::CHC(
	EncodingContext& _context,
	ErrorReporter& _errorReporter,
	[[maybe_unused]] map<util::h256, string> const& _smtlib2Responses,
	[[maybe_unused]] ReadCallback::Callback const& _smtCallback,
	SMTSolverChoice _enabledSolvers,
	optional<unsigned> _timeout
):
	SMTEncoder(_context),
	m_outerErrorReporter(_errorReporter),
	m_enabledSolvers(_enabledSolvers),
	m_queryTimeout(_timeout)
{
	bool usesZ3 = _enabledSolvers.z3;
#ifdef HAVE_Z3
	usesZ3 = usesZ3 && Z3Interface::available();
#else
	usesZ3 = false;
#endif
	if (!usesZ3)
		m_interface = make_unique<CHCSmtLib2Interface>(_smtlib2Responses, _smtCallback, m_queryTimeout);
}

void CHC::analyze(SourceUnit const& _source)
{
	solAssert(_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker), "");

	/// This is currently used to abort analysis of SourceUnits
	/// containing file level functions or constants.
	if (SMTEncoder::analyze(_source))
	{
		resetSourceAnalysis();

		set<SourceUnit const*, EncodingContext::IdCompare> sources;
		sources.insert(&_source);
		for (auto const& source: _source.referencedSourceUnits(true))
			sources.insert(source);
		for (auto const* source: sources)
			defineInterfacesAndSummaries(*source);
		for (auto const* source: sources)
			source->accept(*this);

		checkVerificationTargets();
	}

	bool ranSolver = true;
	if (auto const* smtLibInterface = dynamic_cast<CHCSmtLib2Interface const*>(m_interface.get()))
		ranSolver = smtLibInterface->unhandledQueries().empty();
	if (!ranSolver && !m_noSolverWarning)
	{
		m_noSolverWarning = true;
		m_outerErrorReporter.warning(
			3996_error,
			SourceLocation(),
#ifdef HAVE_Z3_DLOPEN
			"CHC analysis was not possible since libz3.so." + to_string(Z3_MAJOR_VERSION) + "." + to_string(Z3_MINOR_VERSION) + " was not found."
#else
			"CHC analysis was not possible since no integrated z3 SMT solver was found."
#endif
		);
	}
	else
		m_outerErrorReporter.append(m_errorReporter.errors());

	m_errorReporter.clear();
}

vector<string> CHC::unhandledQueries() const
{
	if (auto smtlib2 = dynamic_cast<CHCSmtLib2Interface const*>(m_interface.get()))
		return smtlib2->unhandledQueries();

	return {};
}

bool CHC::visit(ContractDefinition const& _contract)
{
	resetContractAnalysis();
	initContract(_contract);
	clearIndices(&_contract);

	m_stateVariables = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract);
	solAssert(m_currentContract, "");

	SMTEncoder::visit(_contract);
	return false;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	if (auto constructor = _contract.constructor())
		constructor->accept(*this);

	defineContractInitializer(_contract);

	auto const& entry = *createConstructorBlock(_contract, "implicit_constructor_entry");

	// In case constructors use uninitialized state variables,
	// they need to be zeroed.
	// This is not part of `initialConstraints` because it's only true here,
	// at the beginning of the deployment routine.
	smtutil::Expression zeroes(true);
	for (auto var: stateVariablesIncludingInheritedAndPrivate(_contract))
		zeroes = zeroes && currentValue(*var) == smt::zeroValue(var->type());
	addRule(smtutil::Expression::implies(initialConstraints(_contract) && zeroes, predicate(entry)), entry.functor().name);
	setCurrentBlock(entry);

	solAssert(!m_errorDest, "");
	m_errorDest = m_constructorSummaries.at(&_contract);
	// We need to evaluate the base constructor calls (arguments) from derived -> base
	auto baseArgs = baseArguments(_contract);
	for (auto base: _contract.annotation().linearizedBaseContracts)
	{
		if (base != &_contract)
		{
			m_callGraph[&_contract].insert(base);

			auto baseConstructor = base->constructor();
			if (baseConstructor && baseArgs.count(base))
			{
				vector<ASTPointer<Expression>> const& args = baseArgs.at(base);
				auto const& params = baseConstructor->parameters();
				solAssert(params.size() == args.size(), "");
				for (unsigned i = 0; i < params.size(); ++i)
				{
					args.at(i)->accept(*this);
					if (params.at(i))
					{
						solAssert(m_context.knownVariable(*params.at(i)), "");
						m_context.addAssertion(currentValue(*params.at(i)) == expr(*args.at(i), params.at(i)->type()));
					}
				}
			}
		}
	}
	m_errorDest = nullptr;
	// Then call initializer_Base from base -> derived
	for (auto base: _contract.annotation().linearizedBaseContracts | boost::adaptors::reversed)
	{
		errorFlag().increaseIndex();
		m_context.addAssertion(smt::constructorCall(*m_contractInitializers.at(base), m_context));
		connectBlocks(m_currentBlock, summary(_contract), errorFlag().currentValue() > 0);
		m_context.addAssertion(errorFlag().currentValue() == 0);
	}

	connectBlocks(m_currentBlock, summary(_contract));

	setCurrentBlock(*m_constructorSummaries.at(&_contract));
	m_queryPlaceholders[&_contract].push_back({smtutil::Expression(true), errorFlag().currentValue(), m_currentBlock});
	connectBlocks(m_currentBlock, interface(), errorFlag().currentValue() == 0);

	SMTEncoder::endVisit(_contract);
}

bool CHC::visit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
	{
		addRule(summary(_function), "summary_function_" + to_string(_function.id()));
		return false;
	}

	// No inlining.
	solAssert(!m_currentFunction, "Function inlining should not happen in CHC.");
	m_currentFunction = &_function;

	initFunction(_function);

	auto functionEntryBlock = createBlock(m_currentFunction, PredicateType::FunctionBlock);
	auto bodyBlock = createBlock(&m_currentFunction->body(), PredicateType::FunctionBlock);

	auto functionPred = predicate(*functionEntryBlock);
	auto bodyPred = predicate(*bodyBlock);

	addRule(functionPred, functionPred.name);

	solAssert(m_currentContract, "");
	m_context.addAssertion(initialConstraints(*m_currentContract, &_function));

	connectBlocks(functionPred, bodyPred);

	setCurrentBlock(*bodyBlock);

	solAssert(!m_errorDest, "");
	m_errorDest = m_summaries.at(m_currentContract).at(&_function);
	SMTEncoder::visit(*m_currentFunction);
	m_errorDest = nullptr;

	return false;
}

void CHC::endVisit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
		return;

	solAssert(m_currentFunction && m_currentContract, "");
	// No inlining.
	solAssert(m_currentFunction == &_function, "");

	connectBlocks(m_currentBlock, summary(_function));
	setCurrentBlock(*m_summaries.at(m_currentContract).at(&_function));

	// Query placeholders for constructors are not created here because
	// of contracts without constructors.
	// Instead, those are created in endVisit(ContractDefinition).
	if (!_function.isConstructor())
	{
		auto sum = summary(_function);
		auto ifacePre = smt::interfacePre(*m_interfaces.at(m_currentContract), *m_currentContract, m_context);
		if (_function.isPublic())
		{
			auto txConstraints = m_context.state().txConstraints(_function);
			m_queryPlaceholders[&_function].push_back({txConstraints && sum, errorFlag().currentValue(), ifacePre});
			connectBlocks(ifacePre, interface(), txConstraints && sum && errorFlag().currentValue() == 0);
		}
	}

	m_currentFunction = nullptr;

	SMTEncoder::endVisit(_function);
}

bool CHC::visit(IfStatement const& _if)
{
	solAssert(m_currentFunction, "");

	bool unknownFunctionCallWasSeen = m_unknownFunctionCallSeen;
	m_unknownFunctionCallSeen = false;

	solAssert(m_currentFunction, "");
	auto const& functionBody = m_currentFunction->body();

	auto ifHeaderBlock = createBlock(&_if, PredicateType::FunctionBlock, "if_header_");
	auto trueBlock = createBlock(&_if.trueStatement(), PredicateType::FunctionBlock, "if_true_");
	auto falseBlock = _if.falseStatement() ? createBlock(_if.falseStatement(), PredicateType::FunctionBlock, "if_false_") : nullptr;
	auto afterIfBlock = createBlock(&functionBody, PredicateType::FunctionBlock);

	connectBlocks(m_currentBlock, predicate(*ifHeaderBlock));

	setCurrentBlock(*ifHeaderBlock);
	_if.condition().accept(*this);
	auto condition = expr(_if.condition());

	connectBlocks(m_currentBlock, predicate(*trueBlock), condition);
	if (_if.falseStatement())
		connectBlocks(m_currentBlock, predicate(*falseBlock), !condition);
	else
		connectBlocks(m_currentBlock, predicate(*afterIfBlock), !condition);

	setCurrentBlock(*trueBlock);
	_if.trueStatement().accept(*this);
	connectBlocks(m_currentBlock, predicate(*afterIfBlock));

	if (_if.falseStatement())
	{
		setCurrentBlock(*falseBlock);
		_if.falseStatement()->accept(*this);
		connectBlocks(m_currentBlock, predicate(*afterIfBlock));
	}

	setCurrentBlock(*afterIfBlock);

	if (m_unknownFunctionCallSeen)
		eraseKnowledge();

	m_unknownFunctionCallSeen = unknownFunctionCallWasSeen;

	return false;
}

bool CHC::visit(WhileStatement const& _while)
{
	bool unknownFunctionCallWasSeen = m_unknownFunctionCallSeen;
	m_unknownFunctionCallSeen = false;

	solAssert(m_currentFunction, "");
	auto const& functionBody = m_currentFunction->body();

	auto namePrefix = string(_while.isDoWhile() ? "do_" : "") + "while";
	auto loopHeaderBlock = createBlock(&_while, PredicateType::FunctionBlock, namePrefix + "_header_");
	auto loopBodyBlock = createBlock(&_while.body(), PredicateType::FunctionBlock, namePrefix + "_body_");
	auto afterLoopBlock = createBlock(&functionBody, PredicateType::FunctionBlock);

	auto outerBreakDest = m_breakDest;
	auto outerContinueDest = m_continueDest;
	m_breakDest = afterLoopBlock;
	m_continueDest = loopHeaderBlock;

	if (_while.isDoWhile())
		_while.body().accept(*this);

	connectBlocks(m_currentBlock, predicate(*loopHeaderBlock));

	setCurrentBlock(*loopHeaderBlock);

	_while.condition().accept(*this);
	auto condition = expr(_while.condition());

	connectBlocks(m_currentBlock, predicate(*loopBodyBlock), condition);
	connectBlocks(m_currentBlock, predicate(*afterLoopBlock), !condition);

	// Loop body visit.
	setCurrentBlock(*loopBodyBlock);
	_while.body().accept(*this);

	m_breakDest = outerBreakDest;
	m_continueDest = outerContinueDest;

	// Back edge.
	connectBlocks(m_currentBlock, predicate(*loopHeaderBlock));
	setCurrentBlock(*afterLoopBlock);

	if (m_unknownFunctionCallSeen)
		eraseKnowledge();

	m_unknownFunctionCallSeen = unknownFunctionCallWasSeen;

	return false;
}

bool CHC::visit(ForStatement const& _for)
{
	bool unknownFunctionCallWasSeen = m_unknownFunctionCallSeen;
	m_unknownFunctionCallSeen = false;

	solAssert(m_currentFunction, "");
	auto const& functionBody = m_currentFunction->body();

	auto loopHeaderBlock = createBlock(&_for, PredicateType::FunctionBlock, "for_header_");
	auto loopBodyBlock = createBlock(&_for.body(), PredicateType::FunctionBlock, "for_body_");
	auto afterLoopBlock = createBlock(&functionBody, PredicateType::FunctionBlock);
	auto postLoop = _for.loopExpression();
	auto postLoopBlock = postLoop ? createBlock(postLoop, PredicateType::FunctionBlock, "for_post_") : nullptr;

	auto outerBreakDest = m_breakDest;
	auto outerContinueDest = m_continueDest;
	m_breakDest = afterLoopBlock;
	m_continueDest = postLoop ? postLoopBlock : loopHeaderBlock;

	if (auto init = _for.initializationExpression())
		init->accept(*this);

	connectBlocks(m_currentBlock, predicate(*loopHeaderBlock));
	setCurrentBlock(*loopHeaderBlock);

	auto condition = smtutil::Expression(true);
	if (auto forCondition = _for.condition())
	{
		forCondition->accept(*this);
		condition = expr(*forCondition);
	}

	connectBlocks(m_currentBlock, predicate(*loopBodyBlock), condition);
	connectBlocks(m_currentBlock, predicate(*afterLoopBlock), !condition);

	// Loop body visit.
	setCurrentBlock(*loopBodyBlock);
	_for.body().accept(*this);

	if (postLoop)
	{
		connectBlocks(m_currentBlock, predicate(*postLoopBlock));
		setCurrentBlock(*postLoopBlock);
		postLoop->accept(*this);
	}

	m_breakDest = outerBreakDest;
	m_continueDest = outerContinueDest;

	// Back edge.
	connectBlocks(m_currentBlock, predicate(*loopHeaderBlock));
	setCurrentBlock(*afterLoopBlock);

	if (m_unknownFunctionCallSeen)
		eraseKnowledge();

	m_unknownFunctionCallSeen = unknownFunctionCallWasSeen;

	return false;
}

void CHC::endVisit(FunctionCall const& _funCall)
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
	case FunctionType::Kind::Internal:
		internalFunctionCall(_funCall);
		break;
	case FunctionType::Kind::External:
	case FunctionType::Kind::BareStaticCall:
		externalFunctionCall(_funCall);
		SMTEncoder::endVisit(_funCall);
		break;
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::Creation:
		SMTEncoder::endVisit(_funCall);
		unknownFunctionCall(_funCall);
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		[[fallthrough]];
	default:
		SMTEncoder::endVisit(_funCall);
		break;
	}

	createReturnedExpressions(_funCall);
}

void CHC::endVisit(Break const& _break)
{
	solAssert(m_breakDest, "");
	connectBlocks(m_currentBlock, predicate(*m_breakDest));

	// Add an unreachable ghost node to collect unreachable statements after a break.
	auto breakGhost = createBlock(&_break, PredicateType::FunctionBlock, "break_ghost_");
	m_currentBlock = predicate(*breakGhost);
}

void CHC::endVisit(Continue const& _continue)
{
	solAssert(m_continueDest, "");
	connectBlocks(m_currentBlock, predicate(*m_continueDest));

	// Add an unreachable ghost node to collect unreachable statements after a continue.
	auto continueGhost = createBlock(&_continue, PredicateType::FunctionBlock, "continue_ghost_");
	m_currentBlock = predicate(*continueGhost);
}

void CHC::endVisit(IndexRangeAccess const& _range)
{
	createExpr(_range);

	auto baseArray = dynamic_pointer_cast<SymbolicArrayVariable>(m_context.expression(_range.baseExpression()));
	auto sliceArray = dynamic_pointer_cast<SymbolicArrayVariable>(m_context.expression(_range));
	solAssert(baseArray && sliceArray, "");

	auto const& sliceData = ArraySlicePredicate::create(sliceArray->sort(), m_context);
	if (!sliceData.first)
	{
		for (auto pred: sliceData.second.predicates)
			m_interface->registerRelation(pred->functor());
		for (auto const& rule: sliceData.second.rules)
			addRule(rule, "");
	}

	auto start = _range.startExpression() ? expr(*_range.startExpression()) : 0;
	auto end = _range.endExpression() ? expr(*_range.endExpression()) : baseArray->length();
	auto slicePred = (*sliceData.second.predicates.at(0))({
		baseArray->elements(),
		sliceArray->elements(),
		start,
		end
	});

	m_context.addAssertion(slicePred);
	m_context.addAssertion(sliceArray->length() == end - start);
}

void CHC::endVisit(Return const& _return)
{
	SMTEncoder::endVisit(_return);

	connectBlocks(m_currentBlock, predicate(*m_returnDests.back()));

	// Add an unreachable ghost node to collect unreachable statements after a return.
	auto returnGhost = createBlock(&_return, PredicateType::FunctionBlock, "return_ghost_");
	m_currentBlock = predicate(*returnGhost);
}

void CHC::pushInlineFrame(CallableDeclaration const& _callable)
{
	m_returnDests.push_back(createBlock(&_callable, PredicateType::FunctionBlock, "return_"));
}

void CHC::popInlineFrame(CallableDeclaration const& _callable)
{
	solAssert(!m_returnDests.empty(), "");
	auto const& ret = *m_returnDests.back();
	solAssert(ret.programNode() == &_callable, "");
	connectBlocks(m_currentBlock, predicate(ret));
	setCurrentBlock(ret);
	m_returnDests.pop_back();
}

void CHC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");

	solAssert(m_currentContract, "");
	solAssert(m_currentFunction, "");
	auto errorCondition = !m_context.expression(*args.front())->currentValue();
	verificationTargetEncountered(&_funCall, VerificationTarget::Type::Assert, errorCondition);
}

void CHC::visitAddMulMod(FunctionCall const& _funCall)
{
	solAssert(_funCall.arguments().at(2), "");

	verificationTargetEncountered(&_funCall, VerificationTarget::Type::DivByZero, expr(*_funCall.arguments().at(2)) == 0);

	SMTEncoder::visitAddMulMod(_funCall);
}

void CHC::internalFunctionCall(FunctionCall const& _funCall)
{
	solAssert(m_currentContract, "");

	auto const* function = functionCallToDefinition(_funCall);
	if (function)
	{
		if (m_currentFunction && !m_currentFunction->isConstructor())
			m_callGraph[m_currentFunction].insert(function);
		else
			m_callGraph[m_currentContract].insert(function);
		auto const* contract = function->annotation().contract;

		// Libraries can have constants as their "state" variables,
		// so we need to ensure they were constructed correctly.
		if (contract->isLibrary())
			m_context.addAssertion(interface(*contract));
	}

	m_context.addAssertion(predicate(_funCall));

	solAssert(m_errorDest, "");
	connectBlocks(
		m_currentBlock,
		predicate(*m_errorDest),
		errorFlag().currentValue() > 0
	);
	m_context.addAssertion(errorFlag().currentValue() == 0);
}

void CHC::externalFunctionCall(FunctionCall const& _funCall)
{
	/// In external function calls we do not add a "predicate call"
	/// because we do not trust their function body anyway,
	/// so we just add the nondet_interface predicate.

	solAssert(m_currentContract, "");
	if (isTrustedExternalCall(&_funCall.expression()))
	{
		externalFunctionCallToTrustedCode(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::External || kind == FunctionType::Kind::BareStaticCall, "");

	auto const* function = functionCallToDefinition(_funCall);
	if (!function)
		return;

	for (auto var: function->returnParameters())
		m_context.variable(*var)->increaseIndex();

	auto preCallState = vector<smtutil::Expression>{state().state()} + currentStateVariables();
	bool usesStaticCall = kind == FunctionType::Kind::BareStaticCall ||
		function->stateMutability() == StateMutability::Pure ||
		function->stateMutability() == StateMutability::View;
	if (!usesStaticCall)
	{
		state().newState();
		for (auto const* var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();
	}

	auto postCallState = vector<smtutil::Expression>{state().state()} + currentStateVariables();
	auto nondet = (*m_nondetInterfaces.at(m_currentContract))(preCallState + postCallState);
	// TODO this could instead add the summary of the called function, where that summary
	// basically has the nondet interface of this summary as a constraint.
	m_context.addAssertion(nondet);

	m_context.addAssertion(errorFlag().currentValue() == 0);
}

void CHC::externalFunctionCallToTrustedCode(FunctionCall const& _funCall)
{
	solAssert(m_currentContract, "");
	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::External || kind == FunctionType::Kind::BareStaticCall, "");

	auto const* function = functionCallToDefinition(_funCall);
	if (!function)
		return;

	// External call creates a new transaction.
	auto originalTx = state().tx();
	auto txOrigin = state().txMember("tx.origin");
	state().newTx();
	// set the transaction sender as this contract
	m_context.addAssertion(state().txMember("msg.sender") == state().thisAddress());
	// set the origin to be the current transaction origin
	m_context.addAssertion(state().txMember("tx.origin") == txOrigin);

	smtutil::Expression pred = predicate(_funCall);

	auto txConstraints = m_context.state().txConstraints(*function);
	m_context.addAssertion(pred && txConstraints);
	// restore the original transaction data
	state().newTx();
	m_context.addAssertion(originalTx == state().tx());

	solAssert(m_errorDest, "");
	connectBlocks(
		m_currentBlock,
		predicate(*m_errorDest),
		(errorFlag().currentValue() > 0)
	);
	m_context.addAssertion(errorFlag().currentValue() == 0);
}

void CHC::unknownFunctionCall(FunctionCall const&)
{
	/// Function calls are not handled at the moment,
	/// so always erase knowledge.
	/// TODO remove when function calls get predicates/blocks.
	eraseKnowledge();

	/// Used to erase outer scope knowledge in loops and ifs.
	/// TODO remove when function calls get predicates/blocks.
	m_unknownFunctionCallSeen = true;
}

void CHC::makeArrayPopVerificationTarget(FunctionCall const& _arrayPop)
{
	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_arrayPop.expression().annotation().type);
	solAssert(funType.kind() == FunctionType::Kind::ArrayPop, "");

	auto memberAccess = dynamic_cast<MemberAccess const*>(&_arrayPop.expression());
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");

	verificationTargetEncountered(&_arrayPop, VerificationTarget::Type::PopEmptyArray, symbArray->length() <= 0);
}

pair<smtutil::Expression, smtutil::Expression> CHC::arithmeticOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	TypePointer const& _commonType,
	frontend::Expression const& _expression
)
{
	if (_op == Token::Mod || _op == Token::Div)
		verificationTargetEncountered(&_expression, VerificationTarget::Type::DivByZero, _right == 0);

	auto values = SMTEncoder::arithmeticOperation(_op, _left, _right, _commonType, _expression);

	IntegerType const* intType = nullptr;
	if (auto const* type = dynamic_cast<IntegerType const*>(_commonType))
		intType = type;
	else
		intType = TypeProvider::uint256();

	// Mod does not need underflow/overflow checks.
	// Div only needs overflow check for signed types.
	if (_op == Token::Mod || (_op == Token::Div && !intType->isSigned()))
		return values;

	if (_op == Token::Div)
		verificationTargetEncountered(&_expression, VerificationTarget::Type::Overflow, values.second > intType->maxValue());
	else if (intType->isSigned())
	{
		verificationTargetEncountered(&_expression, VerificationTarget::Type::Underflow, values.second < intType->minValue());
		verificationTargetEncountered(&_expression, VerificationTarget::Type::Overflow, values.second > intType->maxValue());
	}
	else if (_op == Token::Sub)
		verificationTargetEncountered(&_expression, VerificationTarget::Type::Underflow, values.second < intType->minValue());
	else if (_op == Token::Add || _op == Token::Mul)
		verificationTargetEncountered(&_expression, VerificationTarget::Type::Overflow, values.second > intType->maxValue());
	else
		solAssert(false, "");
	return values;
}

void CHC::resetSourceAnalysis()
{
	m_safeTargets.clear();
	m_unsafeTargets.clear();
	m_functionTargetIds.clear();
	m_verificationTargets.clear();
	m_queryPlaceholders.clear();
	m_callGraph.clear();
	m_summaries.clear();
	m_interfaces.clear();
	m_nondetInterfaces.clear();
	m_constructorSummaries.clear();
	m_contractInitializers.clear();
	Predicate::reset();
	ArraySlicePredicate::reset();
	m_blockCounter = 0;

	bool usesZ3 = false;
#ifdef HAVE_Z3
	usesZ3 = m_enabledSolvers.z3 && Z3Interface::available();
	if (usesZ3)
	{
		/// z3::fixedpoint does not have a reset mechanism, so we need to create another.
		m_interface.reset(new Z3CHCInterface(m_queryTimeout));
		auto z3Interface = dynamic_cast<Z3CHCInterface const*>(m_interface.get());
		solAssert(z3Interface, "");
		m_context.setSolver(z3Interface->z3Interface());
	}
#endif
	if (!usesZ3)
	{
		auto smtlib2Interface = dynamic_cast<CHCSmtLib2Interface*>(m_interface.get());
		smtlib2Interface->reset();
		solAssert(smtlib2Interface, "");
		m_context.setSolver(smtlib2Interface->smtlib2Interface());
	}

	m_context.clear();
	m_context.resetUniqueId();
	m_context.setAssertionAccumulation(false);
}

void CHC::resetContractAnalysis()
{
	m_stateVariables.clear();
	m_unknownFunctionCallSeen = false;
	m_breakDest = nullptr;
	m_continueDest = nullptr;
	m_returnDests.clear();
	errorFlag().resetIndex();
}

void CHC::eraseKnowledge()
{
	resetStateVariables();
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.hasReferenceOrMappingType(); });
}

void CHC::clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function)
{
	SMTEncoder::clearIndices(_contract, _function);
	for (auto const* var: m_stateVariables)
		/// SSA index 0 is reserved for state variables at the beginning
		/// of the current transaction.
		m_context.variable(*var)->increaseIndex();
	if (_function)
	{
		for (auto const& var: _function->parameters() + _function->returnParameters())
			m_context.variable(*var)->increaseIndex();
		for (auto const& var: localVariablesIncludingModifiers(*_function))
			m_context.variable(*var)->increaseIndex();
	}

	state().newState();
}

void CHC::setCurrentBlock(Predicate const& _block)
{
	if (m_context.solverStackHeigh() > 0)
		m_context.popSolver();
	solAssert(m_currentContract, "");
	clearIndices(m_currentContract, m_currentFunction);
	m_context.pushSolver();
	m_currentBlock = predicate(_block);
}

set<unsigned> CHC::transactionVerificationTargetsIds(ASTNode const* _txRoot)
{
	set<unsigned> verificationTargetsIds;
	solidity::util::BreadthFirstSearch<ASTNode const*>{{_txRoot}}.run([&](auto const* function, auto&& _addChild) {
		verificationTargetsIds.insert(m_functionTargetIds[function].begin(), m_functionTargetIds[function].end());
		for (auto const* called: m_callGraph[function])
		_addChild(called);
	});
	return verificationTargetsIds;
}

SortPointer CHC::sort(FunctionDefinition const& _function)
{
	return functionBodySort(_function, m_currentContract, state());
}

SortPointer CHC::sort(ASTNode const* _node)
{
	if (auto funDef = dynamic_cast<FunctionDefinition const*>(_node))
		return sort(*funDef);

	solAssert(m_currentFunction, "");
	return functionBodySort(*m_currentFunction, m_currentContract, state());
}

Predicate const* CHC::createSymbolicBlock(SortPointer _sort, string const& _name, PredicateType _predType, ASTNode const* _node)
{
	auto const* block = Predicate::create(_sort, _name, _predType, m_context, _node);
	m_interface->registerRelation(block->functor());
	return block;
}

void CHC::defineInterfacesAndSummaries(SourceUnit const& _source)
{
	for (auto const& node: _source.nodes())
		if (auto const* contract = dynamic_cast<ContractDefinition const*>(node.get()))
		{
			string suffix = contract->name() + "_" + to_string(contract->id());
			m_interfaces[contract] = createSymbolicBlock(interfaceSort(*contract, state()), "interface_" + suffix, PredicateType::Interface, contract);
			m_nondetInterfaces[contract] = createSymbolicBlock(nondetInterfaceSort(*contract, state()), "nondet_interface_" + suffix, PredicateType::NondetInterface, contract);
			m_constructorSummaries[contract] = createConstructorBlock(*contract, "summary_constructor");
			m_contractInitializers[contract] = createConstructorBlock(*contract, "contract_initializer");

			for (auto const* var: stateVariablesIncludingInheritedAndPrivate(*contract))
				if (!m_context.knownVariable(*var))
					createVariable(*var);

			/// Base nondeterministic interface that allows
			/// 0 steps to be taken, used as base for the inductive
			/// rule for each function.
			auto const& iface = *m_nondetInterfaces.at(contract);
			addRule(smt::nondetInterface(iface, *contract, m_context, 0, 0), "base_nondet");

			for (auto const* base: contract->annotation().linearizedBaseContracts)
				for (auto const* function: base->definedFunctions())
				{
					for (auto var: function->parameters())
						createVariable(*var);
					for (auto var: function->returnParameters())
						createVariable(*var);
					for (auto const* var: localVariablesIncludingModifiers(*function))
						createVariable(*var);

					m_summaries[contract].emplace(function, createSummaryBlock(*function, *contract));

					if (
						!function->isConstructor() &&
						function->isPublic() &&
						!base->isLibrary() &&
						!base->isInterface()
					)
					{
						auto state1 = stateVariablesAtIndex(1, *contract);
						auto state2 = stateVariablesAtIndex(2, *contract);

						auto nondetPre = smt::nondetInterface(iface, *contract, m_context, 0, 1);
						auto nondetPost = smt::nondetInterface(iface, *contract, m_context, 0, 2);

						vector<smtutil::Expression> args{errorFlag().currentValue(), state().thisAddress(), state().crypto(), state().tx(), state().state(1)};
						args += state1 +
							applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 0); }) +
							vector<smtutil::Expression>{state().state(2)} +
							state2 +
							applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 1); }) +
							applyMap(function->returnParameters(), [this](auto _var) { return valueAtIndex(*_var, 1); });

						connectBlocks(nondetPre, nondetPost, (*m_summaries.at(contract).at(function))(args));
					}
			}
		}
}

void CHC::defineContractInitializer(ContractDefinition const& _contract)
{
	auto const& implicitConstructorPredicate = *createConstructorBlock(_contract, "contract_initializer_entry");

	auto implicitFact = smt::constructor(implicitConstructorPredicate, m_context);
	addRule(smtutil::Expression::implies(initialConstraints(_contract), implicitFact), implicitFact.name);
	setCurrentBlock(implicitConstructorPredicate);

	solAssert(!m_errorDest, "");
	m_errorDest = m_contractInitializers.at(&_contract);
	for (auto var: _contract.stateVariables())
		if (var->value())
		{
			var->value()->accept(*this);
			assignment(*var, *var->value());
		}
	m_errorDest = nullptr;

	auto const& afterInit = *createConstructorBlock(_contract, "contract_initializer_after_init");
	connectBlocks(m_currentBlock, predicate(afterInit));
	setCurrentBlock(afterInit);

	if (auto constructor = _contract.constructor())
	{
		errorFlag().increaseIndex();
		m_context.addAssertion(smt::functionCall(*m_summaries.at(&_contract).at(constructor), &_contract, m_context));
		connectBlocks(m_currentBlock, initializer(_contract), errorFlag().currentValue() > 0);
		m_context.addAssertion(errorFlag().currentValue() == 0);
	}

	connectBlocks(m_currentBlock, initializer(_contract));
}

smtutil::Expression CHC::interface()
{
	solAssert(m_currentContract, "");
	return interface(*m_currentContract);
}

smtutil::Expression CHC::interface(ContractDefinition const& _contract)
{
	return ::interface(*m_interfaces.at(&_contract), _contract, m_context);
}

smtutil::Expression CHC::error()
{
	return (*m_errorPredicate)({});
}

smtutil::Expression CHC::error(unsigned _idx)
{
	return m_errorPredicate->functor(_idx)({});
}

smtutil::Expression CHC::initializer(ContractDefinition const& _contract)
{
	return predicate(*m_contractInitializers.at(&_contract));
}

smtutil::Expression CHC::summary(ContractDefinition const& _contract)
{
	return predicate(*m_constructorSummaries.at(&_contract));
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	return smt::function(*m_summaries.at(&_contract).at(&_function), &_contract, m_context);
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");
	return summary(_function, *m_currentContract);
}

Predicate const* CHC::createBlock(ASTNode const* _node, PredicateType _predType, string const& _prefix)
{
	auto block = createSymbolicBlock(
		sort(_node),
		"block_" + uniquePrefix() + "_" + _prefix + predicateName(_node),
		_predType,
		_node
	);

	solAssert(m_currentFunction, "");
	return block;
}

Predicate const* CHC::createSummaryBlock(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	return createSymbolicBlock(
		functionSort(_function, &_contract, state()),
		"summary_" + uniquePrefix() + "_" + predicateName(&_function, &_contract),
		PredicateType::FunctionSummary,
		&_function
	);
}

Predicate const* CHC::createConstructorBlock(ContractDefinition const& _contract, string const& _prefix)
{
	return createSymbolicBlock(
		constructorSort(_contract, state()),
		_prefix + "_" + contractSuffix(_contract) + "_" + uniquePrefix(),
		PredicateType::ConstructorSummary,
		&_contract
	);
}

void CHC::createErrorBlock()
{
	m_errorPredicate = createSymbolicBlock(arity0FunctionSort(), "error_target_" + to_string(m_context.newUniqueId()), PredicateType::Error);
	m_interface->registerRelation(m_errorPredicate->functor());
}

void CHC::connectBlocks(smtutil::Expression const& _from, smtutil::Expression const& _to, smtutil::Expression const& _constraints)
{
	smtutil::Expression edge = smtutil::Expression::implies(
		_from && m_context.assertions() && _constraints,
		_to
	);
	addRule(edge, _from.name + "_to_" + _to.name);
}

smtutil::Expression CHC::initialConstraints(ContractDefinition const& _contract, FunctionDefinition const* _function)
{
	smtutil::Expression conj = state().state() == state().state(0);
	conj = conj && errorFlag().currentValue() == 0;
	for (auto var: stateVariablesIncludingInheritedAndPrivate(_contract))
		conj = conj && m_context.variable(*var)->valueAtIndex(0) == currentValue(*var);

	FunctionDefinition const* function = _function ? _function : _contract.constructor();
	if (function)
		for (auto var: function->parameters())
			conj = conj && m_context.variable(*var)->valueAtIndex(0) == currentValue(*var);

	return conj;
}

vector<smtutil::Expression> CHC::initialStateVariables()
{
	return stateVariablesAtIndex(0);
}

vector<smtutil::Expression> CHC::stateVariablesAtIndex(unsigned _index)
{
	solAssert(m_currentContract, "");
	return stateVariablesAtIndex(_index, *m_currentContract);
}

vector<smtutil::Expression> CHC::stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract)
{
	return applyMap(
		SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract),
		[&](auto _var) { return valueAtIndex(*_var, _index); }
	);
}

vector<smtutil::Expression> CHC::currentStateVariables()
{
	solAssert(m_currentContract, "");
	return currentStateVariables(*m_currentContract);
}

vector<smtutil::Expression> CHC::currentStateVariables(ContractDefinition const& _contract)
{
	return applyMap(SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract), [this](auto _var) { return currentValue(*_var); });
}

string CHC::predicateName(ASTNode const* _node, ContractDefinition const* _contract)
{
	string prefix;
	if (auto funDef = dynamic_cast<FunctionDefinition const*>(_node))
	{
		prefix += TokenTraits::toString(funDef->kind());
		if (!funDef->name().empty())
			prefix += "_" + funDef->name() + "_";
	}
	else if (m_currentFunction && !m_currentFunction->name().empty())
		prefix += m_currentFunction->name();

	auto contract = _contract ? _contract : m_currentContract;
	solAssert(contract, "");
	return prefix + "_" + to_string(_node->id()) + "_" + to_string(contract->id());
}

smtutil::Expression CHC::predicate(Predicate const& _block)
{
	switch (_block.type())
	{
	case PredicateType::Interface:
		solAssert(m_currentContract, "");
		return ::interface(_block, *m_currentContract, m_context);
	case PredicateType::ConstructorSummary:
		return constructor(_block, m_context);
	case PredicateType::FunctionSummary:
		return smt::function(_block, m_currentContract, m_context);
	case PredicateType::FunctionBlock:
		solAssert(m_currentFunction, "");
		return functionBlock(_block, *m_currentFunction, m_currentContract, m_context);
	case PredicateType::Error:
		return _block({});
	case PredicateType::NondetInterface:
		// Nondeterministic interface predicates are handled differently.
		solAssert(false, "");
	case PredicateType::Custom:
		// Custom rules are handled separately.
		solAssert(false, "");
	}
	solAssert(false, "");
}

smtutil::Expression CHC::predicate(FunctionCall const& _funCall)
{
	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::Internal || kind == FunctionType::Kind::External || kind == FunctionType::Kind::BareStaticCall, "");

	auto const* function = functionCallToDefinition(_funCall);
	if (!function)
		return smtutil::Expression(true);

	auto contractAddressValue = [this](FunctionCall const& _f) {
		FunctionType const& funType = dynamic_cast<FunctionType const&>(*_f.expression().annotation().type);
		if (funType.kind() == FunctionType::Kind::Internal)
			return state().thisAddress();
		if (MemberAccess const* callBase = dynamic_cast<MemberAccess const*>(&_f.expression()))
			return expr(callBase->expression());
		solAssert(false, "Unreachable!");
	};
	errorFlag().increaseIndex();
	vector<smtutil::Expression> args{errorFlag().currentValue(), contractAddressValue(_funCall), state().crypto(), state().tx(), state().state()};

	auto const* contract = function->annotation().contract;
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	solAssert(kind != FunctionType::Kind::Internal || contract->isLibrary() || contains(hierarchy, contract), "");

	/// If the call is to a library, we use that library as the called contract.
	/// If the call is to a contract not in the inheritance hierarchy, we also use that as the called contract.
	/// Otherwise, the call is to some contract in the inheritance hierarchy of the current contract.
	/// In this case we use current contract as the called one since the interfaces/predicates are different.
	auto const* calledContract = contains(hierarchy, contract) ?  m_currentContract : contract;
	solAssert(calledContract, "");

	bool usesStaticCall = function->stateMutability() == StateMutability::Pure || function->stateMutability() == StateMutability::View;

	args += currentStateVariables(*calledContract);
	args += symbolicArguments(_funCall);
	if (!calledContract->isLibrary() && !usesStaticCall)
	{
		state().newState();
		for (auto const& var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();
	}
	args += vector<smtutil::Expression>{state().state()};
	args += currentStateVariables(*calledContract);

	for (auto var: function->parameters() + function->returnParameters())
	{
		if (m_context.knownVariable(*var))
			m_context.variable(*var)->increaseIndex();
		else
			createVariable(*var);
		args.push_back(currentValue(*var));
	}

	return (*m_summaries.at(calledContract).at(function))(args);
}

void CHC::addRule(smtutil::Expression const& _rule, string const& _ruleName)
{
	m_interface->addRule(_rule, _ruleName);
}

pair<CheckResult, CHCSolverInterface::CexGraph> CHC::query(smtutil::Expression const& _query, langutil::SourceLocation const& _location)
{
	CheckResult result;
	CHCSolverInterface::CexGraph cex;
	tie(result, cex) = m_interface->query(_query);
	switch (result)
	{
	case CheckResult::SATISFIABLE:
	{
#ifdef HAVE_Z3
		// Even though the problem is SAT, Spacer's pre processing makes counterexamples incomplete.
		// We now disable those optimizations and check whether we can still solve the problem.
		auto* spacer = dynamic_cast<Z3CHCInterface*>(m_interface.get());
		solAssert(spacer, "");
		spacer->setSpacerOptions(false);

		CheckResult resultNoOpt;
		CHCSolverInterface::CexGraph cexNoOpt;
		tie(resultNoOpt, cexNoOpt) = m_interface->query(_query);

		if (resultNoOpt == CheckResult::SATISFIABLE)
			cex = move(cexNoOpt);

		spacer->setSpacerOptions(true);
#endif
		break;
	}
	case CheckResult::UNSATISFIABLE:
		break;
	case CheckResult::UNKNOWN:
		break;
	case CheckResult::CONFLICTING:
		m_errorReporter.warning(1988_error, _location, "CHC: At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case CheckResult::ERROR:
		m_errorReporter.warning(1218_error, _location, "CHC: Error trying to invoke SMT solver.");
		break;
	}
	return {result, cex};
}

void CHC::verificationTargetEncountered(
	ASTNode const* const _errorNode,
	VerificationTarget::Type _type,
	smtutil::Expression const& _errorCondition
)
{
	solAssert(m_currentContract || m_currentFunction, "");
	SourceUnit const* source = m_currentContract ? sourceUnitContaining(*m_currentContract) : sourceUnitContaining(*m_currentFunction);
	solAssert(source, "");
	if (!source->annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		return;

	bool scopeIsFunction = m_currentFunction && !m_currentFunction->isConstructor();
	auto errorId = newErrorId();
	solAssert(m_verificationTargets.count(errorId) == 0, "Error ID is not unique!");
	m_verificationTargets.emplace(errorId, CHCVerificationTarget{{_type, _errorCondition, smtutil::Expression(true)}, errorId, _errorNode});
	if (scopeIsFunction)
		m_functionTargetIds[m_currentFunction].push_back(errorId);
	else
		m_functionTargetIds[m_currentContract].push_back(errorId);
	auto previousError = errorFlag().currentValue();
	errorFlag().increaseIndex();

	// create an error edge to the summary
	solAssert(m_errorDest, "");
	connectBlocks(
		m_currentBlock,
		predicate(*m_errorDest),
		_errorCondition && errorFlag().currentValue() == errorId
	);

	m_context.addAssertion(errorFlag().currentValue() == previousError);
}

void CHC::checkVerificationTargets()
{
	// The verification conditions have been collected per function where they have been encountered (m_verificationTargets).
	// Also, all possible contexts in which an external function can be called has been recorded (m_queryPlaceholders).
	// Here we combine every context in which an external function can be called with all possible verification conditions
	// in its call graph. Each such combination forms a unique verification target.
	vector<CHCVerificationTarget> verificationTargets;
	for (auto const& [function, placeholders]: m_queryPlaceholders)
	{
		auto functionTargets = transactionVerificationTargetsIds(function);
		for (auto const& placeholder: placeholders)
			for (unsigned id: functionTargets)
			{
				auto const& target = m_verificationTargets.at(id);
				verificationTargets.push_back(CHCVerificationTarget{
					{target.type, placeholder.fromPredicate, placeholder.constraints && placeholder.errorExpression == target.errorId},
					target.errorId,
					target.errorNode
				});
			}
	}

	set<unsigned> checkedErrorIds;
	for (auto const& target: verificationTargets)
	{
		string errorType;
		ErrorId errorReporterId;

		if (target.type == VerificationTarget::Type::PopEmptyArray)
		{
			solAssert(dynamic_cast<FunctionCall const*>(target.errorNode), "");
			errorType = "Empty array \"pop\"";
			errorReporterId = 2529_error;
		}
		else if (
			target.type == VerificationTarget::Type::Underflow ||
			target.type == VerificationTarget::Type::Overflow
		)
		{
			auto const* expr = dynamic_cast<Expression const*>(target.errorNode);
			solAssert(expr, "");
			auto const* intType = dynamic_cast<IntegerType const*>(expr->annotation().type);
			if (!intType)
				intType = TypeProvider::uint256();

			if (target.type == VerificationTarget::Type::Underflow)
			{
				errorType = "Underflow (resulting value less than " + formatNumberReadable(intType->minValue()) + ")";
				errorReporterId = 3944_error;
			}
			else if (target.type == VerificationTarget::Type::Overflow)
			{
				errorType = "Overflow (resulting value larger than " + formatNumberReadable(intType->maxValue()) + ")";
				errorReporterId = 4984_error;
			}
		}
		else if (target.type == VerificationTarget::Type::DivByZero)
		{
			errorType = "Division by zero";
			errorReporterId = 4281_error;
		}
		else if (target.type == VerificationTarget::Type::Assert)
		{
			errorType = "Assertion violation";
			errorReporterId = 6328_error;
		}
		else
			solAssert(false, "");

		checkAndReportTarget(target, errorReporterId, errorType + " happens here.", errorType + " might happen here.");
		checkedErrorIds.insert(target.errorId);
	}

	// There can be targets in internal functions that are not reachable from the external interface.
	// These are safe by definition and are not even checked by the CHC engine, but this information
	// must still be reported safe by the BMC engine.
	set<unsigned> allErrorIds;
	for (auto const& entry: m_functionTargetIds)
		for (unsigned id: entry.second)
			allErrorIds.insert(id);

	set<unsigned> unreachableErrorIds;
	set_difference(
		allErrorIds.begin(),
		allErrorIds.end(),
		checkedErrorIds.begin(),
		checkedErrorIds.end(),
		inserter(unreachableErrorIds, unreachableErrorIds.begin())
	);
	for (auto id: unreachableErrorIds)
		m_safeTargets[m_verificationTargets.at(id).errorNode].insert(m_verificationTargets.at(id).type);
}

void CHC::checkAndReportTarget(
	CHCVerificationTarget const& _target,
	ErrorId _errorReporterId,
	string _satMsg,
	string _unknownMsg
)
{
	if (m_unsafeTargets.count(_target.errorNode) && m_unsafeTargets.at(_target.errorNode).count(_target.type))
		return;

	createErrorBlock();
	connectBlocks(_target.value, error(), _target.constraints);
	auto const& location = _target.errorNode->location();
	auto const& [result, model] = query(error(), location);
	if (result == CheckResult::UNSATISFIABLE)
		m_safeTargets[_target.errorNode].insert(_target.type);
	else if (result == CheckResult::SATISFIABLE)
	{
		solAssert(!_satMsg.empty(), "");
		m_unsafeTargets[_target.errorNode].insert(_target.type);
		auto cex = generateCounterexample(model, error().name);
		if (cex)
			m_errorReporter.warning(
				_errorReporterId,
				location,
				"CHC: " + _satMsg + "\nCounterexample:\n" + *cex
			);
		else
			m_errorReporter.warning(
				_errorReporterId,
				location,
				"CHC: " + _satMsg
			);
	}
	else if (!_unknownMsg.empty())
		m_errorReporter.warning(
			_errorReporterId,
			location,
			"CHC: " + _unknownMsg
		);
}

/**
The counterexample DAG has the following properties:
1) The root node represents the reachable error predicate.
2) The root node has 1 or 2 children:
	- One of them is the summary of the function that was called and led to that node.
	If this is the only child, this function must be the constructor.
	- If it has 2 children, the function is not the constructor and the other child is the interface node,
	that is, it represents the state of the contract before the function described above was called.
3) Interface nodes also have property 2.

The following algorithm starts collecting function summaries at the root node and repeats
for each interface node seen.
Each function summary collected represents a transaction, and the final order is reversed.

The first function summary seen contains the values for the state, input and output variables at the
error point.
*/
optional<string> CHC::generateCounterexample(CHCSolverInterface::CexGraph const& _graph, string const& _root)
{
	optional<unsigned> rootId;
	for (auto const& [id, node]: _graph.nodes)
		if (node.name == _root)
		{
			rootId = id;
			break;
		}
	if (!rootId)
		return {};

	vector<string> path;
	string localState;

	unsigned node = *rootId;
	/// The first summary node seen in this loop represents the last transaction.
	bool lastTxSeen = false;
	while (_graph.edges.at(node).size() >= 1)
	{
		auto const& edges = _graph.edges.at(node);
		solAssert(edges.size() <= 2, "");

		unsigned summaryId = edges.at(0);
		optional<unsigned> interfaceId;
		if (edges.size() == 2)
		{
			interfaceId = edges.at(1);
			if (!Predicate::predicate(_graph.nodes.at(summaryId).name)->isSummary())
				swap(summaryId, *interfaceId);
			auto interfacePredicate = Predicate::predicate(_graph.nodes.at(*interfaceId).name);
			solAssert(interfacePredicate && interfacePredicate->isInterface(), "");
		}
		/// The children are unordered, so we need to check which is the summary and
		/// which is the interface.

		Predicate const* summaryPredicate = Predicate::predicate(_graph.nodes.at(summaryId).name);
		solAssert(summaryPredicate && summaryPredicate->isSummary(), "");
		/// At this point property 2 from the function description is verified for this node.
		vector<smtutil::Expression> summaryArgs = _graph.nodes.at(summaryId).arguments;

		FunctionDefinition const* calledFun = summaryPredicate->programFunction();
		ContractDefinition const* calledContract = summaryPredicate->programContract();

		solAssert((calledFun && !calledContract) || (!calledFun && calledContract), "");
		auto stateVars = summaryPredicate->stateVariables();
		solAssert(stateVars.has_value(), "");
		auto stateValues = summaryPredicate->summaryStateValues(summaryArgs);
		solAssert(stateValues.size() == stateVars->size(), "");

		/// This summary node is the end of a tx.
		/// If it is the first summary node seen in this loop, it is the summary
		/// of the public/external function that was called when the error was reached,
		/// but not necessarily the summary of the function that contains the error.
		if (!lastTxSeen)
		{
			lastTxSeen = true;
			/// Generate counterexample message local to the failed target.
			localState = formatVariableModel(*stateVars, stateValues, ", ") + "\n";
			if (calledFun)
			{
				auto inValues = summaryPredicate->summaryPostInputValues(summaryArgs);
				auto const& inParams = calledFun->parameters();
				localState += formatVariableModel(inParams, inValues, "\n") + "\n";
				auto outValues = summaryPredicate->summaryPostOutputValues(summaryArgs);
				auto const& outParams = calledFun->returnParameters();
				localState += formatVariableModel(outParams, outValues, "\n") + "\n";
			}
		}
		else
		{
			auto modelMsg = formatVariableModel(*stateVars, stateValues, ", ");
			/// We report the state after every tx in the trace except for the last, which is reported
			/// first in the code above.
			if (!modelMsg.empty())
				path.emplace_back("State: " + modelMsg);
		}

		string txCex = summaryPredicate->formatSummaryCall(summaryArgs);
		path.emplace_back(txCex);

		/// Stop when we reach the summary of the analyzed constructor.
		if (summaryPredicate->type() == PredicateType::ConstructorSummary)
			break;

		/// Recurse on the next interface node which represents the previous transaction.
		node = *interfaceId;
	}

	return localState + "\nTransaction trace:\n" + boost::algorithm::join(boost::adaptors::reverse(path), "\n");
}

string CHC::cex2dot(CHCSolverInterface::CexGraph const& _cex)
{
	string dot = "digraph {\n";

	auto pred = [&](CHCSolverInterface::CexNode const& _node) {
		vector<string> args = applyMap(
			_node.arguments,
			[&](auto const& arg) {
				solAssert(arg.arguments.empty(), "");
				return arg.name;
			}
		);
		return "\"" + _node.name + "(" + boost::algorithm::join(args, ", ") + ")\"";
	};

	for (auto const& [u, vs]: _cex.edges)
		for (auto v: vs)
			dot += pred(_cex.nodes.at(v)) + " -> " + pred(_cex.nodes.at(u)) + "\n";

	dot += "}";
	return dot;
}

string CHC::uniquePrefix()
{
	return to_string(m_blockCounter++);
}

string CHC::contractSuffix(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + to_string(_contract.id());
}

unsigned CHC::newErrorId()
{
	unsigned errorId = m_context.newUniqueId();
	// We need to make sure the error id is not zero,
	// because error id zero actually means no error in the CHC encoding.
	if (errorId == 0)
		errorId = m_context.newUniqueId();
	return errorId;
}

SymbolicState& CHC::state()
{
	return m_context.state();
}

SymbolicIntVariable& CHC::errorFlag()
{
	return state().errorFlag();
}
