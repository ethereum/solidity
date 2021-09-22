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

#include <range/v3/algorithm/for_each.hpp>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>

#ifdef HAVE_Z3_DLOPEN
#include <z3_version.h>
#endif

#include <charconv>
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
	UniqueErrorReporter& _errorReporter,
	[[maybe_unused]] map<util::h256, string> const& _smtlib2Responses,
	[[maybe_unused]] ReadCallback::Callback const& _smtCallback,
	ModelCheckerSettings const& _settings,
	CharStreamProvider const& _charStreamProvider
):
	SMTEncoder(_context, _settings, _errorReporter, _charStreamProvider)
{
	bool usesZ3 = m_settings.solvers.z3;
#ifdef HAVE_Z3
	usesZ3 = usesZ3 && Z3Interface::available();
#else
	usesZ3 = false;
#endif
	if (!usesZ3 && m_settings.solvers.smtlib2)
		m_interface = make_unique<CHCSmtLib2Interface>(_smtlib2Responses, _smtCallback, m_settings.timeout);
}

void CHC::analyze(SourceUnit const& _source)
{
	if (!m_settings.solvers.z3 && !m_settings.solvers.smtlib2)
	{
		if (!m_noSolverWarning)
		{
			m_noSolverWarning = true;
			m_errorReporter.warning(
				7649_error,
				SourceLocation(),
				"CHC analysis was not possible since no Horn solver was enabled."
			);
		}
		return;
	}

	resetSourceAnalysis();

	auto sources = sourceDependencies(_source);
	collectFreeFunctions(sources);
	createFreeConstants(sources);
	state().prepareForSourceUnit(_source);

	for (auto const* source: sources)
		defineInterfacesAndSummaries(*source);
	for (auto const* source: sources)
		source->accept(*this);

	checkVerificationTargets();

	bool ranSolver = true;
	// If ranSolver is true here it's because an SMT solver callback was
	// actually given and the queries were solved.
	if (auto const* smtLibInterface = dynamic_cast<CHCSmtLib2Interface const*>(m_interface.get()))
		ranSolver = smtLibInterface->unhandledQueries().empty();
	if (!ranSolver && !m_noSolverWarning)
	{
		m_noSolverWarning = true;
		m_errorReporter.warning(
			3996_error,
			SourceLocation(),
#ifdef HAVE_Z3_DLOPEN
			"CHC analysis was not possible since libz3.so." + to_string(Z3_MAJOR_VERSION) + "." + to_string(Z3_MINOR_VERSION) + " was not found."
#else
			"CHC analysis was not possible. No Horn solver was available."
			" None of the installed solvers was enabled."
#endif
		);
	}
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

	m_scopes.push_back(&_contract);

	m_stateVariables = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract);
	solAssert(m_currentContract, "");

	SMTEncoder::visit(_contract);
	return false;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	for (auto base: _contract.annotation().linearizedBaseContracts)
	{
		if (auto constructor = base->constructor())
			constructor->accept(*this);
		defineContractInitializer(*base, _contract);
	}

	auto const& entry = *createConstructorBlock(_contract, "implicit_constructor_entry");

	// In case constructors use uninitialized state variables,
	// they need to be zeroed.
	// This is not part of `initialConstraints` because it's only true here,
	// at the beginning of the deployment routine.
	smtutil::Expression zeroes(true);
	for (auto var: stateVariablesIncludingInheritedAndPrivate(_contract))
		zeroes = zeroes && currentValue(*var) == smt::zeroValue(var->type());
	// The contract's address might already have funds before deployment,
	// so the balance must be at least `msg.value`, but not equals.
	auto initialBalanceConstraint = state().balance(state().thisAddress()) >= state().txMember("msg.value");
	addRule(smtutil::Expression::implies(
		initialConstraints(_contract) && zeroes && initialBalanceConstraint,
		predicate(entry)
	), entry.functor().name);
	setCurrentBlock(entry);

	solAssert(!m_errorDest, "");
	m_errorDest = m_constructorSummaries.at(&_contract);
	// We need to evaluate the base constructor calls (arguments) from derived -> base
	auto baseArgs = baseArguments(_contract);
	for (auto base: _contract.annotation().linearizedBaseContracts)
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
	m_errorDest = nullptr;
	// Then call initializer_Base from base -> derived
	for (auto base: _contract.annotation().linearizedBaseContracts | ranges::views::reverse)
	{
		errorFlag().increaseIndex();
		m_context.addAssertion(smt::constructorCall(*m_contractInitializers.at(&_contract).at(base), m_context));
		connectBlocks(m_currentBlock, summary(_contract), errorFlag().currentValue() > 0);
		m_context.addAssertion(errorFlag().currentValue() == 0);
	}

	connectBlocks(m_currentBlock, summary(_contract));

	setCurrentBlock(*m_constructorSummaries.at(&_contract));

	solAssert(&_contract == m_currentContract, "");
	if (shouldAnalyze(_contract))
	{
		auto constructor = _contract.constructor();
		auto txConstraints = state().txTypeConstraints();
		if (!constructor || !constructor->isPayable())
			txConstraints = txConstraints && state().txNonPayableConstraint();
		m_queryPlaceholders[&_contract].push_back({txConstraints, errorFlag().currentValue(), m_currentBlock});
		connectBlocks(m_currentBlock, interface(), txConstraints && errorFlag().currentValue() == 0);
	}

	solAssert(m_scopes.back() == &_contract, "");
	m_scopes.pop_back();

	SMTEncoder::endVisit(_contract);
}

bool CHC::visit(FunctionDefinition const& _function)
{
	// Free functions need to be visited in the context of a contract.
	if (!m_currentContract)
		return false;

	if (
		!_function.isImplemented() ||
		abstractAsNondet(_function)
	)
	{
		smtutil::Expression conj(true);
		if (
			_function.stateMutability() == StateMutability::Pure ||
			_function.stateMutability() == StateMutability::View
		)
			conj = conj && currentEqualInitialVarsConstraints(stateVariablesIncludingInheritedAndPrivate(_function));

		conj = conj && errorFlag().currentValue() == 0;
		addRule(smtutil::Expression::implies(conj, summary(_function)), "summary_function_" + to_string(_function.id()));
		return false;
	}

	// No inlining.
	solAssert(!m_currentFunction, "Function inlining should not happen in CHC.");
	m_currentFunction = &_function;

	m_scopes.push_back(&_function);

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
	// Free functions need to be visited in the context of a contract.
	if (!m_currentContract)
		return;

	if (
		!_function.isImplemented() ||
		abstractAsNondet(_function)
	)
		return;

	solAssert(m_currentFunction && m_currentContract, "");
	// No inlining.
	solAssert(m_currentFunction == &_function, "");

	solAssert(m_scopes.back() == &_function, "");
	m_scopes.pop_back();

	connectBlocks(m_currentBlock, summary(_function));
	setCurrentBlock(*m_summaries.at(m_currentContract).at(&_function));

	// Query placeholders for constructors are not created here because
	// of contracts without constructors.
	// Instead, those are created in endVisit(ContractDefinition).
	if (
		!_function.isConstructor() &&
		_function.isPublic() &&
		contractFunctions(*m_currentContract).count(&_function) &&
		shouldAnalyze(*m_currentContract)
	)
	{
		defineExternalFunctionInterface(_function, *m_currentContract);
		setCurrentBlock(*m_interfaces.at(m_currentContract));

		// Create the rule
		// interface \land externalFunctionEntry => interface'
		auto ifacePre = smt::interfacePre(*m_interfaces.at(m_currentContract), *m_currentContract, m_context);
		auto sum = externalSummary(_function);

		m_queryPlaceholders[&_function].push_back({sum, errorFlag().currentValue(), ifacePre});
		connectBlocks(ifacePre, interface(), sum && errorFlag().currentValue() == 0);
	}

	m_currentFunction = nullptr;

	SMTEncoder::endVisit(_function);
}

bool CHC::visit(Block const& _block)
{
	m_scopes.push_back(&_block);
	return SMTEncoder::visit(_block);
}

void CHC::endVisit(Block const& _block)
{
	solAssert(m_scopes.back() == &_block, "");
	m_scopes.pop_back();
	SMTEncoder::endVisit(_block);
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
	m_scopes.push_back(&_for);

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

void CHC::endVisit(ForStatement const& _for)
{
	solAssert(m_scopes.back() == &_for, "");
	m_scopes.pop_back();
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
	case FunctionType::Kind::BareCall:
		externalFunctionCall(_funCall);
		SMTEncoder::endVisit(_funCall);
		break;
	case FunctionType::Kind::DelegateCall:
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
	case FunctionType::Kind::Unwrap:
	case FunctionType::Kind::Wrap:
		[[fallthrough]];
	default:
		SMTEncoder::endVisit(_funCall);
		break;
	}


	createReturnedExpressions(_funCall, m_currentContract);
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

bool CHC::visit(TryCatchClause const& _tryStatement)
{
	m_scopes.push_back(&_tryStatement);
	return SMTEncoder::visit(_tryStatement);
}

void CHC::endVisit(TryCatchClause const& _tryStatement)
{
	solAssert(m_scopes.back() == &_tryStatement, "");
	m_scopes.pop_back();
}

bool CHC::visit(TryStatement const& _tryStatement)
{
	FunctionCall const* externalCall = dynamic_cast<FunctionCall const*>(&_tryStatement.externalCall());
	solAssert(externalCall && externalCall->annotation().tryCall, "");
	solAssert(m_currentFunction, "");

	auto tryHeaderBlock = createBlock(&_tryStatement, PredicateType::FunctionBlock, "try_header_");
	auto afterTryBlock = createBlock(&m_currentFunction->body(), PredicateType::FunctionBlock);

	auto const& clauses = _tryStatement.clauses();
	solAssert(clauses[0].get() == _tryStatement.successClause(), "First clause of TryStatement should be the success clause");
	auto clauseBlocks = applyMap(clauses, [this](ASTPointer<TryCatchClause> clause) {
		return createBlock(clause.get(), PredicateType::FunctionBlock, "try_clause_" + std::to_string(clause->id()));
	});

	connectBlocks(m_currentBlock, predicate(*tryHeaderBlock));
	setCurrentBlock(*tryHeaderBlock);
	// Visit everything, except the actual external call.
	externalCall->expression().accept(*this);
	ASTNode::listAccept(externalCall->arguments(), *this);
	// Branch directly to all catch clauses, since in these cases, any effects of the external call are reverted.
	for (size_t i = 1; i < clauseBlocks.size(); ++i)
		connectBlocks(m_currentBlock, predicate(*clauseBlocks[i]));
	// Only now visit the actual call to record its effects and connect to the success clause.
	endVisit(*externalCall);
	if (_tryStatement.successClause()->parameters())
		expressionToTupleAssignment(_tryStatement.successClause()->parameters()->parameters(), *externalCall);

	connectBlocks(m_currentBlock, predicate(*clauseBlocks[0]));

	for (size_t i = 0; i < clauses.size(); ++i)
	{
		setCurrentBlock(*clauseBlocks[i]);
		clauses[i]->accept(*this);
		connectBlocks(m_currentBlock, predicate(*afterTryBlock));
	}
	setCurrentBlock(*afterTryBlock);

	return false;
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
	verificationTargetEncountered(&_funCall, VerificationTargetType::Assert, errorCondition);
}

void CHC::visitAddMulMod(FunctionCall const& _funCall)
{
	solAssert(_funCall.arguments().at(2), "");

	verificationTargetEncountered(&_funCall, VerificationTargetType::DivByZero, expr(*_funCall.arguments().at(2)) == 0);

	SMTEncoder::visitAddMulMod(_funCall);
}

void CHC::internalFunctionCall(FunctionCall const& _funCall)
{
	solAssert(m_currentContract, "");

	auto function = functionCallToDefinition(_funCall, currentScopeContract(), m_currentContract);
	if (function)
	{
		if (m_currentFunction && !m_currentFunction->isConstructor())
			m_callGraph[m_currentFunction].insert(function);
		else
			m_callGraph[m_currentContract].insert(function);
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
	auto [callExpr, callOptions] = functionCallExpression(_funCall);

	if (isTrustedExternalCall(callExpr))
	{
		externalFunctionCallToTrustedCode(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*callExpr->annotation().type);
	auto kind = funType.kind();
	solAssert(
		kind == FunctionType::Kind::External ||
		kind == FunctionType::Kind::BareCall ||
		kind == FunctionType::Kind::BareStaticCall,
		""
	);

	bool usesStaticCall = kind == FunctionType::Kind::BareStaticCall;

	solAssert(m_currentContract, "");
	auto function = functionCallToDefinition(_funCall, currentScopeContract(), m_currentContract);
	if (function)
	{
		usesStaticCall |= function->stateMutability() == StateMutability::Pure ||
			function->stateMutability() == StateMutability::View;
		for (auto var: function->returnParameters())
			m_context.variable(*var)->increaseIndex();
	}

	if (!m_currentFunction || m_currentFunction->isConstructor())
		return;

	if (callOptions)
	{
		optional<unsigned> valueIndex;
		for (auto&& [i, name]: callOptions->names() | ranges::views::enumerate)
			if (name && *name == "value")
			{
				valueIndex = i;
				break;
			}
		solAssert(valueIndex, "");
		state().addBalance(state().thisAddress(), 0 - expr(*callOptions->options().at(*valueIndex)));
	}

	auto preCallState = vector<smtutil::Expression>{state().state()} + currentStateVariables();

	if (!usesStaticCall)
	{
		state().newState();
		for (auto const* var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();
	}

	auto error = errorFlag().increaseIndex();

	Predicate const& callPredicate = *createSymbolicBlock(
		nondetInterfaceSort(*m_currentContract, state()),
		"nondet_call_" + uniquePrefix(),
		PredicateType::ExternalCallUntrusted,
		&_funCall
	);
	auto postCallState = vector<smtutil::Expression>{state().state()} + currentStateVariables();
	vector<smtutil::Expression> stateExprs{error, state().thisAddress(), state().abi(), state().crypto()};

	auto nondet = (*m_nondetInterfaces.at(m_currentContract))(stateExprs + preCallState + postCallState);
	auto nondetCall = callPredicate(stateExprs + preCallState + postCallState);

	addRule(smtutil::Expression::implies(nondet, nondetCall), nondetCall.name);

	m_context.addAssertion(nondetCall);
	solAssert(m_errorDest, "");
	connectBlocks(m_currentBlock, predicate(*m_errorDest), errorFlag().currentValue() > 0);

	// To capture the possibility of a reentrant call, we record in the call graph that the  current function
	// can call any of the external methods of the current contract.
	if (m_currentFunction)
		for (auto const* definedFunction: contractFunctions(*m_currentContract))
			if (!definedFunction->isConstructor() && definedFunction->isPublic())
				m_callGraph[m_currentFunction].insert(definedFunction);

	m_context.addAssertion(errorFlag().currentValue() == 0);
}

void CHC::externalFunctionCallToTrustedCode(FunctionCall const& _funCall)
{
	solAssert(m_currentContract, "");
	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::External || kind == FunctionType::Kind::BareStaticCall, "");

	auto function = functionCallToDefinition(_funCall, currentScopeContract(), m_currentContract);
	if (!function)
		return;

	// External call creates a new transaction.
	auto originalTx = state().tx();
	auto txOrigin = state().txMember("tx.origin");
	state().newTx();
	// set the transaction sender as this contract
	m_context.addAssertion(state().txMember("msg.sender") == state().thisAddress());
	// set the transaction value as 0
	m_context.addAssertion(state().txMember("msg.value") == 0);
	// set the origin to be the current transaction origin
	m_context.addAssertion(state().txMember("tx.origin") == txOrigin);

	smtutil::Expression pred = predicate(_funCall);

	auto txConstraints = state().txTypeConstraints() && state().txFunctionConstraints(*function);
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

	auto memberAccess = dynamic_cast<MemberAccess const*>(cleanExpression(_arrayPop.expression()));
	solAssert(memberAccess, "");
	auto symbArray = dynamic_pointer_cast<SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");

	verificationTargetEncountered(&_arrayPop, VerificationTargetType::PopEmptyArray, symbArray->length() <= 0);
}

void CHC::makeOutOfBoundsVerificationTarget(IndexAccess const& _indexAccess)
{
	if (_indexAccess.annotation().type->category() == Type::Category::TypeType)
		return;

	auto baseType = _indexAccess.baseExpression().annotation().type;

	optional<smtutil::Expression> length;
	if (smt::isArray(*baseType))
		length = dynamic_cast<smt::SymbolicArrayVariable const&>(
			*m_context.expression(_indexAccess.baseExpression())
		).length();
	else if (auto const* type = dynamic_cast<FixedBytesType const*>(baseType))
		length = smtutil::Expression(static_cast<size_t>(type->numBytes()));

	optional<smtutil::Expression> target;
	if (
		auto index = _indexAccess.indexExpression();
		index && length
	)
		target = expr(*index) < 0 || expr(*index) >= *length;

	if (target)
		verificationTargetEncountered(&_indexAccess, VerificationTargetType::OutOfBounds, *target);
}

pair<smtutil::Expression, smtutil::Expression> CHC::arithmeticOperation(
	Token _op,
	smtutil::Expression const& _left,
	smtutil::Expression const& _right,
	Type const* _commonType,
	frontend::Expression const& _expression
)
{
	// Unchecked does not disable div by 0 checks.
	if (_op == Token::Mod || _op == Token::Div)
		verificationTargetEncountered(&_expression, VerificationTargetType::DivByZero, _right == 0);

	auto values = SMTEncoder::arithmeticOperation(_op, _left, _right, _commonType, _expression);

	if (!m_checked)
		return values;

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
		verificationTargetEncountered(&_expression, VerificationTargetType::Overflow, values.second > intType->maxValue());
	else if (intType->isSigned())
	{
		verificationTargetEncountered(&_expression, VerificationTargetType::Underflow, values.second < intType->minValue());
		verificationTargetEncountered(&_expression, VerificationTargetType::Overflow, values.second > intType->maxValue());
	}
	else if (_op == Token::Sub)
		verificationTargetEncountered(&_expression, VerificationTargetType::Underflow, values.second < intType->minValue());
	else if (_op == Token::Add || _op == Token::Mul)
		verificationTargetEncountered(&_expression, VerificationTargetType::Overflow, values.second > intType->maxValue());
	else
		solAssert(false, "");
	return values;
}

void CHC::resetSourceAnalysis()
{
	SMTEncoder::resetSourceAnalysis();

	m_safeTargets.clear();
	m_unsafeTargets.clear();
	m_unprovedTargets.clear();
	m_functionTargetIds.clear();
	m_verificationTargets.clear();
	m_queryPlaceholders.clear();
	m_callGraph.clear();
	m_summaries.clear();
	m_externalSummaries.clear();
	m_interfaces.clear();
	m_nondetInterfaces.clear();
	m_constructorSummaries.clear();
	m_contractInitializers.clear();
	Predicate::reset();
	ArraySlicePredicate::reset();
	m_blockCounter = 0;

	bool usesZ3 = false;
#ifdef HAVE_Z3
	usesZ3 = m_settings.solvers.z3 && Z3Interface::available();
	if (usesZ3)
	{
		/// z3::fixedpoint does not have a reset mechanism, so we need to create another.
		m_interface.reset(new Z3CHCInterface(m_settings.timeout));
		auto z3Interface = dynamic_cast<Z3CHCInterface const*>(m_interface.get());
		solAssert(z3Interface, "");
		m_context.setSolver(z3Interface->z3Interface());
	}
#endif
	if (!usesZ3)
	{
		auto smtlib2Interface = dynamic_cast<CHCSmtLib2Interface*>(m_interface.get());
		solAssert(smtlib2Interface, "");
		smtlib2Interface->reset();
		m_context.setSolver(smtlib2Interface->smtlib2Interface());
	}

	m_context.reset();
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
	resetStorageVariables();
	resetBalances();
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
		for (auto const& var: localVariablesIncludingModifiers(*_function, _contract))
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
	struct ASTNodeCompare: EncodingContext::IdCompare
	{
		bool operator<(ASTNodeCompare _other) const { return operator()(node, _other.node); }
		ASTNode const* node;
	};
	solidity::util::BreadthFirstSearch<ASTNodeCompare>{{{{}, _txRoot}}}.run([&](auto _node, auto&& _addChild) {
		verificationTargetsIds.insert(m_functionTargetIds[_node.node].begin(), m_functionTargetIds[_node.node].end());
		for (ASTNode const* called: m_callGraph[_node.node])
			_addChild({{}, called});
	});
	return verificationTargetsIds;
}

optional<CHC::CHCNatspecOption> CHC::natspecOptionFromString(string const& _option)
{
	static map<string, CHCNatspecOption> options{
		{"abstract-function-nondet", CHCNatspecOption::AbstractFunctionNondet}
	};
	if (options.count(_option))
		return options.at(_option);
	return {};
}

set<CHC::CHCNatspecOption> CHC::smtNatspecTags(FunctionDefinition const& _function)
{
	set<CHC::CHCNatspecOption> options;
	string smtStr = "custom:smtchecker";
	for (auto const& [tag, value]: _function.annotation().docTags)
		if (tag == smtStr)
		{
			string const& content = value.content;
			if (auto option = natspecOptionFromString(content))
				options.insert(*option);
			else
				m_errorReporter.warning(3130_error, _function.location(), "Unknown option for \"" + smtStr + "\": \"" + content + "\"");
		}
	return options;
}

bool CHC::abstractAsNondet(FunctionDefinition const& _function)
{
	return smtNatspecTags(_function).count(CHCNatspecOption::AbstractFunctionNondet);
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

Predicate const* CHC::createSymbolicBlock(SortPointer _sort, string const& _name, PredicateType _predType, ASTNode const* _node, ContractDefinition const* _contractContext)
{
	auto const* block = Predicate::create(_sort, _name, _predType, m_context, _node, _contractContext, m_scopes);
	m_interface->registerRelation(block->functor());
	return block;
}

void CHC::defineInterfacesAndSummaries(SourceUnit const& _source)
{
	for (auto const& node: _source.nodes())
		if (auto const* contract = dynamic_cast<ContractDefinition const*>(node.get()))
		{
			string suffix = contract->name() + "_" + to_string(contract->id());
			m_interfaces[contract] = createSymbolicBlock(interfaceSort(*contract, state()), "interface_" + uniquePrefix() + "_" + suffix, PredicateType::Interface, contract);
			m_nondetInterfaces[contract] = createSymbolicBlock(nondetInterfaceSort(*contract, state()), "nondet_interface_" + uniquePrefix() + "_" + suffix, PredicateType::NondetInterface, contract);
			m_constructorSummaries[contract] = createConstructorBlock(*contract, "summary_constructor");

			for (auto const* var: stateVariablesIncludingInheritedAndPrivate(*contract))
				if (!m_context.knownVariable(*var))
					createVariable(*var);

			/// Base nondeterministic interface that allows
			/// 0 steps to be taken, used as base for the inductive
			/// rule for each function.
			auto const& iface = *m_nondetInterfaces.at(contract);
			addRule(smtutil::Expression::implies(errorFlag().currentValue() == 0, smt::nondetInterface(iface, *contract, m_context, 0, 0)), "base_nondet");

			auto const& resolved = contractFunctions(*contract);
			for (auto const* function: contractFunctionsWithoutVirtual(*contract) + allFreeFunctions())
			{
				for (auto var: function->parameters())
					createVariable(*var);
				for (auto var: function->returnParameters())
					createVariable(*var);
				for (auto const* var: localVariablesIncludingModifiers(*function, contract))
					createVariable(*var);

				m_summaries[contract].emplace(function, createSummaryBlock(*function, *contract));

				if (!function->isConstructor() && function->isPublic() && resolved.count(function))
				{
					m_externalSummaries[contract].emplace(function, createSummaryBlock(*function, *contract));

					auto state1 = stateVariablesAtIndex(1, *contract);
					auto state2 = stateVariablesAtIndex(2, *contract);

					auto errorPre = errorFlag().currentValue();
					auto nondetPre = smt::nondetInterface(iface, *contract, m_context, 0, 1);
					auto errorPost = errorFlag().increaseIndex();
					auto nondetPost = smt::nondetInterface(iface, *contract, m_context, 0, 2);

					vector<smtutil::Expression> args{errorPost, state().thisAddress(), state().abi(), state().crypto(), state().tx(), state().state(1)};
					args += state1 +
						applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 0); }) +
						vector<smtutil::Expression>{state().state(2)} +
						state2 +
						applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 1); }) +
						applyMap(function->returnParameters(), [this](auto _var) { return valueAtIndex(*_var, 1); });

					connectBlocks(nondetPre, nondetPost, errorPre == 0 && (*m_externalSummaries.at(contract).at(function))(args));
				}
			}
		}
}

void CHC::defineExternalFunctionInterface(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	// Create a rule that represents an external call to this function.
	// This contains more things than the function body itself,
	// such as balance updates because of ``msg.value``.
	auto functionEntryBlock = createBlock(&_function, PredicateType::FunctionBlock);
	auto functionPred = predicate(*functionEntryBlock);
	addRule(functionPred, functionPred.name);
	setCurrentBlock(*functionEntryBlock);

	m_context.addAssertion(initialConstraints(_contract, &_function));
	m_context.addAssertion(state().txTypeConstraints() && state().txFunctionConstraints(_function));

	// The contract may have received funds through a selfdestruct or
	// block.coinbase, which do not trigger calls into the contract.
	// So the only constraint we can add here is that the balance of
	// the contract grows by at least `msg.value`.
	SymbolicIntVariable k{TypeProvider::uint256(), TypeProvider::uint256(), "funds_" + to_string(m_context.newUniqueId()), m_context};
	m_context.addAssertion(k.currentValue() >= state().txMember("msg.value"));
	// Assume that address(this).balance cannot overflow.
	m_context.addAssertion(smt::symbolicUnknownConstraints(state().balance(state().thisAddress()) + k.currentValue(), TypeProvider::uint256()));
	state().addBalance(state().thisAddress(), k.currentValue());

	errorFlag().increaseIndex();
	m_context.addAssertion(summaryCall(_function));

	connectBlocks(functionPred, externalSummary(_function));
}

void CHC::defineContractInitializer(ContractDefinition const& _contract, ContractDefinition const& _contextContract)
{
	m_contractInitializers[&_contextContract][&_contract] = createConstructorBlock(_contract, "contract_initializer");
	auto const& implicitConstructorPredicate = *createConstructorBlock(_contract, "contract_initializer_entry");

	auto implicitFact = smt::constructor(implicitConstructorPredicate, m_context);
	addRule(smtutil::Expression::implies(initialConstraints(_contract), implicitFact), implicitFact.name);
	setCurrentBlock(implicitConstructorPredicate);

	auto prevErrorDest = m_errorDest;
	m_errorDest = m_contractInitializers.at(&_contextContract).at(&_contract);
	for (auto var: _contract.stateVariables())
		if (var->value())
		{
			var->value()->accept(*this);
			assignment(*var, *var->value());
		}
	m_errorDest = prevErrorDest;

	auto const& afterInit = *createConstructorBlock(_contract, "contract_initializer_after_init");
	connectBlocks(m_currentBlock, predicate(afterInit));
	setCurrentBlock(afterInit);

	if (auto constructor = _contract.constructor())
	{
		errorFlag().increaseIndex();
		m_context.addAssertion(smt::functionCall(*m_summaries.at(&_contextContract).at(constructor), &_contextContract, m_context));
		connectBlocks(m_currentBlock, initializer(_contract, _contextContract), errorFlag().currentValue() > 0);
		m_context.addAssertion(errorFlag().currentValue() == 0);
	}

	connectBlocks(m_currentBlock, initializer(_contract, _contextContract));
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

smtutil::Expression CHC::initializer(ContractDefinition const& _contract, ContractDefinition const& _contractContext)
{
	return predicate(*m_contractInitializers.at(&_contractContext).at(&_contract));
}

smtutil::Expression CHC::summary(ContractDefinition const& _contract)
{
	return predicate(*m_constructorSummaries.at(&_contract));
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	return smt::function(*m_summaries.at(&_contract).at(&_function), &_contract, m_context);
}

smtutil::Expression CHC::summaryCall(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	return smt::functionCall(*m_summaries.at(&_contract).at(&_function), &_contract, m_context);
}

smtutil::Expression CHC::externalSummary(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	return smt::function(*m_externalSummaries.at(&_contract).at(&_function), &_contract, m_context);
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");
	return summary(_function, *m_currentContract);
}

smtutil::Expression CHC::summaryCall(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");
	return summaryCall(_function, *m_currentContract);
}

smtutil::Expression CHC::externalSummary(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");
	return externalSummary(_function, *m_currentContract);
}

Predicate const* CHC::createBlock(ASTNode const* _node, PredicateType _predType, string const& _prefix)
{
	auto block = createSymbolicBlock(
		sort(_node),
		"block_" + uniquePrefix() + "_" + _prefix + predicateName(_node),
		_predType,
		_node,
		m_currentContract
	);

	solAssert(m_currentFunction, "");
	return block;
}

Predicate const* CHC::createSummaryBlock(FunctionDefinition const& _function, ContractDefinition const& _contract, PredicateType _type)
{
	return createSymbolicBlock(
		functionSort(_function, &_contract, state()),
		"summary_" + uniquePrefix() + "_" + predicateName(&_function, &_contract),
		_type,
		&_function,
		&_contract
	);
}

Predicate const* CHC::createConstructorBlock(ContractDefinition const& _contract, string const& _prefix)
{
	return createSymbolicBlock(
		constructorSort(_contract, state()),
		_prefix + "_" + uniquePrefix() + "_" + contractSuffix(_contract),
		PredicateType::ConstructorSummary,
		&_contract,
		&_contract
	);
}

void CHC::createErrorBlock()
{
	m_errorPredicate = createSymbolicBlock(
		arity0FunctionSort(),
		"error_target_" + to_string(m_context.newUniqueId()),
		PredicateType::Error
	);
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
	conj = conj && currentEqualInitialVarsConstraints(stateVariablesIncludingInheritedAndPrivate(_contract));

	FunctionDefinition const* function = _function ? _function : _contract.constructor();
	if (function)
		conj = conj && currentEqualInitialVarsConstraints(applyMap(function->parameters(), [](auto&& _var) -> VariableDeclaration const* { return _var.get(); }));

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

smtutil::Expression CHC::currentEqualInitialVarsConstraints(vector<VariableDeclaration const*> const& _vars) const
{
	return fold(_vars, smtutil::Expression(true), [this](auto&& _conj, auto _var) {
		return move(_conj) && currentValue(*_var) == m_context.variable(*_var)->valueAtIndex(0);
	});
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
	case PredicateType::InternalCall:
	case PredicateType::ExternalCallTrusted:
	case PredicateType::ExternalCallUntrusted:
		return smt::function(_block, m_currentContract, m_context);
	case PredicateType::FunctionBlock:
	case PredicateType::FunctionErrorBlock:
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

	solAssert(m_currentContract, "");
	auto function = functionCallToDefinition(_funCall, currentScopeContract(), m_currentContract);
	if (!function)
		return smtutil::Expression(true);

	auto contractAddressValue = [this](FunctionCall const& _f) {
		auto [callExpr, callOptions] = functionCallExpression(_f);

		FunctionType const& funType = dynamic_cast<FunctionType const&>(*callExpr->annotation().type);
		if (funType.kind() == FunctionType::Kind::Internal)
			return state().thisAddress();
		if (MemberAccess const* callBase = dynamic_cast<MemberAccess const*>(callExpr))
			return expr(callBase->expression());
		solAssert(false, "Unreachable!");
	};
	errorFlag().increaseIndex();
	vector<smtutil::Expression> args{errorFlag().currentValue(), contractAddressValue(_funCall), state().abi(), state().crypto(), state().tx(), state().state()};

	auto const* contract = function->annotation().contract;
	auto const& hierarchy = m_currentContract->annotation().linearizedBaseContracts;
	solAssert(kind != FunctionType::Kind::Internal || function->isFree() || (contract && contract->isLibrary()) || contains(hierarchy, contract), "");

	bool usesStaticCall = function->stateMutability() == StateMutability::Pure || function->stateMutability() == StateMutability::View;

	args += currentStateVariables(*m_currentContract);
	args += symbolicArguments(_funCall, m_currentContract);
	if (!m_currentContract->isLibrary() && !usesStaticCall)
	{
		state().newState();
		for (auto const& var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();
	}
	args += vector<smtutil::Expression>{state().state()};
	args += currentStateVariables(*m_currentContract);

	for (auto var: function->parameters() + function->returnParameters())
	{
		if (m_context.knownVariable(*var))
			m_context.variable(*var)->increaseIndex();
		else
			createVariable(*var);
		args.push_back(currentValue(*var));
	}

	Predicate const& summary = *m_summaries.at(m_currentContract).at(function);
	auto from = smt::function(summary, m_currentContract, m_context);
	Predicate const& callPredicate = *createSummaryBlock(
		*function,
		*m_currentContract,
		kind == FunctionType::Kind::Internal ? PredicateType::InternalCall : PredicateType::ExternalCallTrusted
	);
	auto to = smt::function(callPredicate, m_currentContract, m_context);
	addRule(smtutil::Expression::implies(from, to), to.name);

	return callPredicate(args);
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
		if (m_settings.solvers.z3)
		{
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
		}
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
	VerificationTargetType _type,
	smtutil::Expression const& _errorCondition
)
{
	if (!m_settings.targets.has(_type))
		return;

	if (!(m_currentContract || m_currentFunction))
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

	Predicate const* localBlock = m_currentFunction ?
		createBlock(m_currentFunction, PredicateType::FunctionErrorBlock) :
		createConstructorBlock(*m_currentContract, "local_error");

	auto pred = predicate(*localBlock);
	connectBlocks(
		m_currentBlock,
		pred,
		_errorCondition && errorFlag().currentValue() == errorId
	);
	solAssert(m_errorDest, "");
	addRule(smtutil::Expression::implies(pred, predicate(*m_errorDest)), pred.name);

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

		if (target.type == VerificationTargetType::PopEmptyArray)
		{
			solAssert(dynamic_cast<FunctionCall const*>(target.errorNode), "");
			errorType = "Empty array \"pop\"";
			errorReporterId = 2529_error;
		}
		else if (target.type == VerificationTargetType::OutOfBounds)
		{
			solAssert(dynamic_cast<IndexAccess const*>(target.errorNode), "");
			errorType = "Out of bounds access";
			errorReporterId = 6368_error;
		}
		else if (
			target.type == VerificationTargetType::Underflow ||
			target.type == VerificationTargetType::Overflow
		)
		{
			auto const* expr = dynamic_cast<Expression const*>(target.errorNode);
			solAssert(expr, "");
			auto const* intType = dynamic_cast<IntegerType const*>(expr->annotation().type);
			if (!intType)
				intType = TypeProvider::uint256();

			if (target.type == VerificationTargetType::Underflow)
			{
				errorType = "Underflow (resulting value less than " + formatNumberReadable(intType->minValue()) + ")";
				errorReporterId = 3944_error;
			}
			else if (target.type == VerificationTargetType::Overflow)
			{
				errorType = "Overflow (resulting value larger than " + formatNumberReadable(intType->maxValue()) + ")";
				errorReporterId = 4984_error;
			}
		}
		else if (target.type == VerificationTargetType::DivByZero)
		{
			errorType = "Division by zero";
			errorReporterId = 4281_error;
		}
		else if (target.type == VerificationTargetType::Assert)
		{
			errorType = "Assertion violation";
			errorReporterId = 6328_error;
		}
		else
			solAssert(false, "");

		checkAndReportTarget(target, errorReporterId, errorType + " happens here.", errorType + " might happen here.");
		checkedErrorIds.insert(target.errorId);
	}

	auto toReport = m_unsafeTargets;
	if (m_settings.showUnproved)
		for (auto const& [node, targets]: m_unprovedTargets)
			for (auto const& [target, info]: targets)
				toReport[node].emplace(target, info);

	for (auto const& [node, targets]: toReport)
		for (auto const& [target, info]: targets)
			m_errorReporter.warning(
				info.error,
				info.location,
				info.message
			);

	if (!m_settings.showUnproved && !m_unprovedTargets.empty())
		m_errorReporter.warning(
			5840_error,
			{},
			"CHC: " +
			to_string(m_unprovedTargets.size()) +
			" verification condition(s) could not be proved." +
			" Enable the model checker option \"show unproved\" to see all of them." +
			" Consider choosing a specific contract to be verified in order to reduce the solving problems." +
			" Consider increasing the timeout per query."
		);

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
		auto cex = generateCounterexample(model, error().name);
		if (cex)
			m_unsafeTargets[_target.errorNode][_target.type] = {
				_errorReporterId,
				location,
				"CHC: " + _satMsg + "\nCounterexample:\n" + *cex
			};
		else
			m_unsafeTargets[_target.errorNode][_target.type] = {
				_errorReporterId,
				location,
				"CHC: " + _satMsg
			};
	}
	else if (!_unknownMsg.empty())
		m_unprovedTargets[_target.errorNode][_target.type] = {
			_errorReporterId,
			location,
			"CHC: " + _unknownMsg
		};
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

We run a BFS on the DAG from the root node collecting the reachable function summaries from the given node.
When a function summary is seen, the search continues with that summary as the new root for its subgraph.
The result of the search is a callgraph containing:
- Functions calls needed to reach the root node, that is, transaction entry points.
- Functions called by other functions (internal calls or external calls/internal transactions).
The BFS visit order and the shape of the DAG described in the previous paragraph guarantee that the order of
the function summaries in the callgraph of the error node is the reverse transaction trace.

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

	auto callGraph = summaryCalls(_graph, *rootId);

	auto nodePred = [&](auto _node) { return Predicate::predicate(_graph.nodes.at(_node).name); };
	auto nodeArgs = [&](auto _node) { return _graph.nodes.at(_node).arguments; };

	bool first = true;
	for (auto summaryId: callGraph.at(*rootId))
	{
		CHCSolverInterface::CexNode const& summaryNode = _graph.nodes.at(summaryId);
		Predicate const* summaryPredicate = Predicate::predicate(summaryNode.name);
		auto const& summaryArgs = summaryNode.arguments;

		auto stateVars = summaryPredicate->stateVariables();
		solAssert(stateVars.has_value(), "");
		auto stateValues = summaryPredicate->summaryStateValues(summaryArgs);
		solAssert(stateValues.size() == stateVars->size(), "");

		if (first)
		{
			first = false;
			/// Generate counterexample message local to the failed target.
			localState = formatVariableModel(*stateVars, stateValues, ", ") + "\n";

			if (auto calledFun = summaryPredicate->programFunction())
			{
				auto inValues = summaryPredicate->summaryPostInputValues(summaryArgs);
				auto const& inParams = calledFun->parameters();
				if (auto inStr = formatVariableModel(inParams, inValues, "\n"); !inStr.empty())
					localState += inStr + "\n";
				auto outValues = summaryPredicate->summaryPostOutputValues(summaryArgs);
				auto const& outParams = calledFun->returnParameters();
				if (auto outStr = formatVariableModel(outParams, outValues, "\n"); !outStr.empty())
					localState += outStr + "\n";

				optional<unsigned> localErrorId;
				solidity::util::BreadthFirstSearch<unsigned> bfs{{summaryId}};
				bfs.run([&](auto _nodeId, auto&& _addChild) {
					auto const& children = _graph.edges.at(_nodeId);
					if (
						children.size() == 1 &&
						nodePred(children.front())->isFunctionErrorBlock()
					)
					{
						localErrorId = children.front();
						bfs.abort();
					}
					ranges::for_each(children, _addChild);
				});

				if (localErrorId.has_value())
				{
					auto const* localError = nodePred(*localErrorId);
					solAssert(localError && localError->isFunctionErrorBlock(), "");
					auto const [localValues, localVars] = localError->localVariableValues(nodeArgs(*localErrorId));
					if (auto localStr = formatVariableModel(localVars, localValues, "\n"); !localStr.empty())
						localState += localStr + "\n";
				}
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

		string txCex = summaryPredicate->formatSummaryCall(summaryArgs, m_charStreamProvider);

		list<string> calls;
		auto dfs = [&](unsigned parent, unsigned node, unsigned depth, auto&& _dfs) -> void {
			auto pred = nodePred(node);
			auto parentPred = nodePred(parent);
			solAssert(pred && pred->isSummary(), "");
			solAssert(parentPred && parentPred->isSummary(), "");
			auto callTraceSize = calls.size();
			if (!pred->isConstructorSummary())
				for (unsigned v: callGraph[node])
					_dfs(node, v, depth + 1, _dfs);
			calls.push_front(string(depth * 4, ' ') + pred->formatSummaryCall(nodeArgs(node), m_charStreamProvider));
			if (pred->isInternalCall())
				calls.front() += " -- internal call";
			else if (pred->isExternalCallTrusted())
				calls.front() += " -- trusted external call";
			else if (pred->isExternalCallUntrusted())
			{
				calls.front() += " -- untrusted external call";
				if (calls.size() > callTraceSize + 1)
					calls.front() += ", synthesized as:";
			}
			else if (pred->isFunctionSummary() && parentPred->isExternalCallUntrusted())
				calls.front() += " -- reentrant call";
		};
		dfs(summaryId, summaryId, 0, dfs);
		path.emplace_back(boost::algorithm::join(calls, "\n"));
	}

	return localState + "\nTransaction trace:\n" + boost::algorithm::join(path | ranges::views::reverse, "\n");
}

map<unsigned, vector<unsigned>> CHC::summaryCalls(CHCSolverInterface::CexGraph const& _graph, unsigned _root)
{
	map<unsigned, vector<unsigned>> calls;

	auto compare = [&](unsigned _a, unsigned _b) {
		auto extract = [&](string const& _s) {
			// We want to sort sibling predicates in the counterexample graph by their unique predicate id.
			// For most predicates, this actually doesn't matter.
			// The cases where this matters are internal and external function calls which have the form:
			// summary_<CALLID>_<suffix>
			// nondet_call_<CALLID>_<suffix>
			// Those have the extra unique <CALLID> numbers based on the traversal order, and are necessary
			// to infer the call order so that's shown property in the counterexample trace.
			// Predicates that do not have a CALLID have a predicate id at the end of <suffix>,
			// so the assertion below should still hold.
			auto beg = _s.data();
			while (beg != _s.data() + _s.size() && !isdigit(*beg)) ++beg;
			auto end = beg;
			while (end != _s.data() + _s.size() && isdigit(*end)) ++end;

			solAssert(beg != end, "Expected to find numerical call or predicate id.");

			int result;
			auto [p, ec] = std::from_chars(beg, end, result);
			solAssert(ec == std::errc(), "Id should be a number.");

			return result;
		};
		return extract(_graph.nodes.at(_a).name) > extract(_graph.nodes.at(_b).name);
	};

	queue<pair<unsigned, unsigned>> q;
	q.push({_root, _root});
	while (!q.empty())
	{
		auto [node, root] = q.front();
		q.pop();

		Predicate const* nodePred = Predicate::predicate(_graph.nodes.at(node).name);
		Predicate const* rootPred = Predicate::predicate(_graph.nodes.at(root).name);
		if (nodePred->isSummary() && (
			_root == root ||
			nodePred->isInternalCall() ||
			nodePred->isExternalCallTrusted() ||
			nodePred->isExternalCallUntrusted() ||
			rootPred->isExternalCallUntrusted()
		))
		{
			calls[root].push_back(node);
			root = node;
		}
		auto const& edges = _graph.edges.at(node);
		for (unsigned v: set<unsigned, decltype(compare)>(begin(edges), end(edges), compare))
			q.push({v, root});
	}

	return calls;
}

string CHC::cex2dot(CHCSolverInterface::CexGraph const& _cex)
{
	string dot = "digraph {\n";

	auto pred = [&](CHCSolverInterface::CexNode const& _node) {
		vector<string> args = applyMap(
			_node.arguments,
			[&](auto const& arg) { return arg.name; }
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

SymbolicIntVariable& CHC::errorFlag()
{
	return state().errorFlag();
}
