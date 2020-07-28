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

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/TypeProvider.h>

#include <libsmtutil/CHCSmtLib2Interface.h>
#include <libsolutil/Algorithms.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <queue>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::smtutil;
using namespace solidity::frontend;

CHC::CHC(
	smt::EncodingContext& _context,
	ErrorReporter& _errorReporter,
	map<util::h256, string> const& _smtlib2Responses,
	ReadCallback::Callback const& _smtCallback,
	[[maybe_unused]] smtutil::SMTSolverChoice _enabledSolvers
):
	SMTEncoder(_context),
	m_outerErrorReporter(_errorReporter),
	m_enabledSolvers(_enabledSolvers)
{
#ifdef HAVE_Z3
	if (_enabledSolvers.z3)
		m_interface = make_unique<smtutil::Z3CHCInterface>();
#endif
	if (!m_interface)
		m_interface = make_unique<smtutil::CHCSmtLib2Interface>(_smtlib2Responses, _smtCallback);
}

void CHC::analyze(SourceUnit const& _source)
{
	solAssert(_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker), "");

	bool usesZ3 = false;
#ifdef HAVE_Z3
	usesZ3 = m_enabledSolvers.z3;
	if (usesZ3)
	{
		auto z3Interface = dynamic_cast<smtutil::Z3CHCInterface const*>(m_interface.get());
		solAssert(z3Interface, "");
		m_context.setSolver(z3Interface->z3Interface());
	}
#endif
	if (!usesZ3)
	{
		auto smtlib2Interface = dynamic_cast<smtutil::CHCSmtLib2Interface const*>(m_interface.get());
		solAssert(smtlib2Interface, "");
		m_context.setSolver(smtlib2Interface->smtlib2Interface());
	}
	m_context.clear();
	m_context.setAssertionAccumulation(false);
	m_variableUsage.setFunctionInlining(false);

	resetSourceAnalysis();

	auto genesisSort = make_shared<smtutil::FunctionSort>(
		vector<smtutil::SortPointer>(),
		smtutil::SortProvider::boolSort
	);
	m_genesisPredicate = createSymbolicBlock(genesisSort, "genesis");
	addRule(genesis(), "genesis");

	set<SourceUnit const*, IdCompare> sources;
	sources.insert(&_source);
	for (auto const& source: _source.referencedSourceUnits(true))
		sources.insert(source);
	for (auto const* source: sources)
		defineInterfacesAndSummaries(*source);
	for (auto const* source: sources)
		source->accept(*this);

	checkVerificationTargets();
}

vector<string> CHC::unhandledQueries() const
{
	if (auto smtlib2 = dynamic_cast<smtutil::CHCSmtLib2Interface const*>(m_interface.get()))
		return smtlib2->unhandledQueries();

	return {};
}

bool CHC::visit(ContractDefinition const& _contract)
{
	resetContractAnalysis();

	initContract(_contract);

	m_stateVariables = stateVariablesIncludingInheritedAndPrivate(_contract);
	m_stateSorts = stateSorts(_contract);

	clearIndices(&_contract);

	string suffix = _contract.name() + "_" + to_string(_contract.id());
	m_errorPredicate = createSymbolicBlock(arity0FunctionSort(), "error_" + suffix);
	m_constructorSummaryPredicate = createSymbolicBlock(constructorSort(), "summary_constructor_" + suffix);
	m_symbolFunction[m_constructorSummaryPredicate->currentFunctionValue().name] = &_contract;
	m_implicitConstructorPredicate = createSymbolicBlock(arity0FunctionSort(), "implicit_constructor_" + suffix);
	auto stateExprs = currentStateVariables();
	setCurrentBlock(*m_interfaces.at(m_currentContract), &stateExprs);

	SMTEncoder::visit(_contract);
	return false;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	auto implicitConstructor = (*m_implicitConstructorPredicate)({});
	connectBlocks(genesis(), implicitConstructor);
	m_currentBlock = implicitConstructor;
	m_context.addAssertion(m_error.currentValue() == 0);

	if (auto constructor = _contract.constructor())
		constructor->accept(*this);
	else
		inlineConstructorHierarchy(_contract);

	connectBlocks(m_currentBlock, summary(_contract), m_error.currentValue() == 0);

	clearIndices(m_currentContract, nullptr);
	vector<smtutil::Expression> symbArgs = currentFunctionVariables(*m_currentContract);
	setCurrentBlock(*m_constructorSummaryPredicate, &symbArgs);

	addAssertVerificationTarget(m_currentContract, m_currentBlock, smtutil::Expression(true), m_error.currentValue());
	connectBlocks(m_currentBlock, interface(), m_error.currentValue() == 0);

	SMTEncoder::endVisit(_contract);
}

bool CHC::visit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
	{
		connectBlocks(genesis(), summary(_function));
		return false;
	}

	// This is the case for base constructor inlining.
	if (m_currentFunction)
	{
		solAssert(m_currentFunction->isConstructor(), "");
		solAssert(_function.isConstructor(), "");
		solAssert(_function.scope() != m_currentContract, "");
		SMTEncoder::visit(_function);
		return false;
	}

	solAssert(!m_currentFunction, "Function inlining should not happen in CHC.");
	m_currentFunction = &_function;

	initFunction(_function);

	auto functionEntryBlock = createBlock(m_currentFunction);
	auto bodyBlock = createBlock(&m_currentFunction->body());

	auto functionPred = predicate(*functionEntryBlock, currentFunctionVariables());
	auto bodyPred = predicate(*bodyBlock);

	if (_function.isConstructor())
		connectBlocks(m_currentBlock, functionPred);
	else
		connectBlocks(genesis(), functionPred);

	m_context.addAssertion(m_error.currentValue() == 0);
	for (auto const* var: m_stateVariables)
		m_context.addAssertion(m_context.variable(*var)->valueAtIndex(0) == currentValue(*var));
	for (auto const& var: _function.parameters())
		m_context.addAssertion(m_context.variable(*var)->valueAtIndex(0) == currentValue(*var));

	connectBlocks(functionPred, bodyPred);

	setCurrentBlock(*bodyBlock);

	SMTEncoder::visit(*m_currentFunction);

	return false;
}

void CHC::endVisit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
		return;

	solAssert(m_currentFunction && m_currentContract, "");

	// This is the case for base constructor inlining.
	if (m_currentFunction != &_function)
	{
		solAssert(m_currentFunction && m_currentFunction->isConstructor(), "");
		solAssert(_function.isConstructor(), "");
		solAssert(_function.scope() != m_currentContract, "");
	}
	else
	{
		// We create an extra exit block for constructors that simply
		// connects to the interface in case an explicit constructor
		// exists in the hierarchy.
		// It is not connected directly here, as normal functions are,
		// because of the case where there are only implicit constructors.
		// This is done in endVisit(ContractDefinition).
		if (_function.isConstructor())
		{
			string suffix = m_currentContract->name() + "_" + to_string(m_currentContract->id());
			auto constructorExit = createSymbolicBlock(constructorSort(), "constructor_exit_" + suffix);
			connectBlocks(m_currentBlock, predicate(*constructorExit, currentFunctionVariables(*m_currentContract)));

			clearIndices(m_currentContract, m_currentFunction);
			auto stateExprs = currentFunctionVariables(*m_currentContract);
			setCurrentBlock(*constructorExit, &stateExprs);
		}
		else
		{
			auto assertionError = m_error.currentValue();
			auto sum = summary(_function);
			connectBlocks(m_currentBlock, sum);

			auto iface = interface();

			auto stateExprs = initialStateVariables();
			setCurrentBlock(*m_interfaces.at(m_currentContract), &stateExprs);

			if (_function.isPublic())
			{
				addAssertVerificationTarget(&_function, m_currentBlock, sum, assertionError);
				connectBlocks(m_currentBlock, iface, sum && (assertionError == 0));
			}
		}
		m_currentFunction = nullptr;
	}

	SMTEncoder::endVisit(_function);
}

bool CHC::visit(IfStatement const& _if)
{
	solAssert(m_currentFunction, "");

	bool unknownFunctionCallWasSeen = m_unknownFunctionCallSeen;
	m_unknownFunctionCallSeen = false;

	solAssert(m_currentFunction, "");
	auto const& functionBody = m_currentFunction->body();

	auto ifHeaderBlock = createBlock(&_if, "if_header_");
	auto trueBlock = createBlock(&_if.trueStatement(), "if_true_");
	auto falseBlock = _if.falseStatement() ? createBlock(_if.falseStatement(), "if_false_") : nullptr;
	auto afterIfBlock = createBlock(&functionBody);

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
	auto loopHeaderBlock = createBlock(&_while, namePrefix + "_header_");
	auto loopBodyBlock = createBlock(&_while.body(), namePrefix + "_body_");
	auto afterLoopBlock = createBlock(&functionBody);

	auto outerBreakDest = m_breakDest;
	auto outerContinueDest = m_continueDest;
	m_breakDest = afterLoopBlock.get();
	m_continueDest = loopHeaderBlock.get();

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

	auto loopHeaderBlock = createBlock(&_for, "for_header_");
	auto loopBodyBlock = createBlock(&_for.body(), "for_body_");
	auto afterLoopBlock = createBlock(&functionBody);
	auto postLoop = _for.loopExpression();
	auto postLoopBlock = postLoop ? createBlock(postLoop, "for_post_") : nullptr;

	auto outerBreakDest = m_breakDest;
	auto outerContinueDest = m_continueDest;
	m_breakDest = afterLoopBlock.get();
	m_continueDest = postLoop ? postLoopBlock.get() : loopHeaderBlock.get();

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
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		SMTEncoder::endVisit(_funCall);
		unknownFunctionCall(_funCall);
		break;
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
	auto breakGhost = createBlock(&_break, "break_ghost_");
	m_currentBlock = predicate(*breakGhost);
}

void CHC::endVisit(Continue const& _continue)
{
	solAssert(m_continueDest, "");
	connectBlocks(m_currentBlock, predicate(*m_continueDest));
	auto continueGhost = createBlock(&_continue, "continue_ghost_");
	m_currentBlock = predicate(*continueGhost);
}

void CHC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");

	solAssert(m_currentContract, "");
	solAssert(m_currentFunction, "");
	if (m_currentFunction->isConstructor())
		m_functionAssertions[m_currentContract].insert(&_funCall);
	else
		m_functionAssertions[m_currentFunction].insert(&_funCall);

	auto previousError = m_error.currentValue();
	m_error.increaseIndex();

	connectBlocks(
		m_currentBlock,
		m_currentFunction->isConstructor() ? summary(*m_currentContract) : summary(*m_currentFunction),
		currentPathConditions() && !m_context.expression(*args.front())->currentValue() && (
			m_error.currentValue() == newErrorId(_funCall)
		)
	);

	m_context.addAssertion(m_error.currentValue() == previousError);
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

	auto previousError = m_error.currentValue();

	m_context.addAssertion(predicate(_funCall));

	connectBlocks(
		m_currentBlock,
		(m_currentFunction && !m_currentFunction->isConstructor()) ? summary(*m_currentFunction) : summary(*m_currentContract),
		(m_error.currentValue() > 0)
	);
	m_context.addAssertion(m_error.currentValue() == 0);
	m_error.increaseIndex();
	m_context.addAssertion(m_error.currentValue() == previousError);
}

void CHC::externalFunctionCall(FunctionCall const& _funCall)
{
	solAssert(m_currentContract, "");

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	auto kind = funType.kind();
	solAssert(kind == FunctionType::Kind::External || kind == FunctionType::Kind::BareStaticCall, "");

	auto const* function = functionCallToDefinition(_funCall);
	if (!function)
		return;

	for (auto var: function->returnParameters())
		m_context.variable(*var)->increaseIndex();

	auto preCallState = currentStateVariables();
	bool usesStaticCall = kind == FunctionType::Kind::BareStaticCall ||
		function->stateMutability() == StateMutability::Pure ||
		function->stateMutability() == StateMutability::View;
	if (!usesStaticCall)
		for (auto const* var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();

	auto nondet = (*m_nondetInterfaces.at(m_currentContract))(preCallState + currentStateVariables());
	m_symbolFunction[nondet.name] = &_funCall;
	m_context.addAssertion(nondet);

	m_context.addAssertion(m_error.currentValue() == 0);
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
	auto symbArray = dynamic_pointer_cast<smt::SymbolicArrayVariable>(m_context.expression(memberAccess->expression()));
	solAssert(symbArray, "");

	auto previousError = m_error.currentValue();
	m_error.increaseIndex();

	addArrayPopVerificationTarget(&_arrayPop, m_error.currentValue());
	connectBlocks(
		m_currentBlock,
		m_currentFunction->isConstructor() ? summary(*m_currentContract) : summary(*m_currentFunction),
		currentPathConditions() && symbArray->length() <= 0 && m_error.currentValue() == newErrorId(_arrayPop)
	);

	m_context.addAssertion(m_error.currentValue() == previousError);
}

void CHC::resetSourceAnalysis()
{
	m_verificationTargets.clear();
	m_safeTargets.clear();
	m_unsafeTargets.clear();
	m_functionAssertions.clear();
	m_errorIds.clear();
	m_callGraph.clear();
	m_summaries.clear();
	m_symbolFunction.clear();
}

void CHC::resetContractAnalysis()
{
	m_stateSorts.clear();
	m_stateVariables.clear();
	m_unknownFunctionCallSeen = false;
	m_breakDest = nullptr;
	m_continueDest = nullptr;
	m_error.resetIndex();
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
		for (auto const& var: _function->localVariables())
			m_context.variable(*var)->increaseIndex();
	}
}

void CHC::setCurrentBlock(
	smt::SymbolicFunctionVariable const& _block,
	vector<smtutil::Expression> const* _arguments
)
{
	if (m_context.solverStackHeigh() > 0)
		m_context.popSolver();
	solAssert(m_currentContract, "");
	clearIndices(m_currentContract, m_currentFunction);
	m_context.pushSolver();
	if (_arguments)
		m_currentBlock = predicate(_block, *_arguments);
	else
		m_currentBlock = predicate(_block);
}

set<frontend::Expression const*, CHC::IdCompare> CHC::transactionAssertions(ASTNode const* _txRoot)
{
	set<Expression const*, IdCompare> assertions;
	solidity::util::BreadthFirstSearch<ASTNode const*>{{_txRoot}}.run([&](auto const* function, auto&& _addChild) {
		assertions.insert(m_functionAssertions[function].begin(), m_functionAssertions[function].end());
		for (auto const* called: m_callGraph[function])
		_addChild(called);
	});
	return assertions;
}

vector<VariableDeclaration const*> CHC::stateVariablesIncludingInheritedAndPrivate(ContractDefinition const& _contract)
{
	return fold(
		_contract.annotation().linearizedBaseContracts,
		vector<VariableDeclaration const*>{},
		[](auto&& _acc, auto _contract) { return _acc + _contract->stateVariables(); }
	);
}

vector<VariableDeclaration const*> CHC::stateVariablesIncludingInheritedAndPrivate(FunctionDefinition const& _function)
{
	return stateVariablesIncludingInheritedAndPrivate(dynamic_cast<ContractDefinition const&>(*_function.scope()));
}

vector<smtutil::SortPointer> CHC::stateSorts(ContractDefinition const& _contract)
{
	return applyMap(
		stateVariablesIncludingInheritedAndPrivate(_contract),
		[](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); }
	);
}

smtutil::SortPointer CHC::constructorSort()
{
	solAssert(m_currentContract, "");
	if (auto const* constructor = m_currentContract->constructor())
		return sort(*constructor);

	return make_shared<smtutil::FunctionSort>(
		vector<smtutil::SortPointer>{smtutil::SortProvider::uintSort} + m_stateSorts,
		smtutil::SortProvider::boolSort
	);
}

smtutil::SortPointer CHC::interfaceSort()
{
	solAssert(m_currentContract, "");
	return interfaceSort(*m_currentContract);
}

smtutil::SortPointer CHC::nondetInterfaceSort()
{
	solAssert(m_currentContract, "");
	return nondetInterfaceSort(*m_currentContract);
}

smtutil::SortPointer CHC::interfaceSort(ContractDefinition const& _contract)
{
	return make_shared<smtutil::FunctionSort>(
		stateSorts(_contract),
		smtutil::SortProvider::boolSort
	);
}

smtutil::SortPointer CHC::nondetInterfaceSort(ContractDefinition const& _contract)
{
	auto sorts = stateSorts(_contract);
	return make_shared<smtutil::FunctionSort>(
		sorts + sorts,
		smtutil::SortProvider::boolSort
	);
}

smtutil::SortPointer CHC::arity0FunctionSort()
{
	return make_shared<smtutil::FunctionSort>(
		vector<smtutil::SortPointer>(),
		smtutil::SortProvider::boolSort
	);
}

/// A function in the symbolic CFG requires:
/// - Index of failed assertion. 0 means no assertion failed.
/// - 2 sets of state variables:
///   - State variables at the beginning of the current function, immutable
///   - Current state variables
///    At the beginning of the function these must equal set 1
/// - 2 sets of input variables:
///   - Input variables at the beginning of the current function, immutable
///   - Current input variables
///    At the beginning of the function these must equal set 1
/// - 1 set of output variables
smtutil::SortPointer CHC::sort(FunctionDefinition const& _function)
{
	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	auto inputSorts = applyMap(_function.parameters(), smtSort);
	auto outputSorts = applyMap(_function.returnParameters(), smtSort);
	return make_shared<smtutil::FunctionSort>(
		vector<smtutil::SortPointer>{smtutil::SortProvider::uintSort} + m_stateSorts + inputSorts + m_stateSorts + inputSorts + outputSorts,
		smtutil::SortProvider::boolSort
	);
}

smtutil::SortPointer CHC::sort(ASTNode const* _node)
{
	if (auto funDef = dynamic_cast<FunctionDefinition const*>(_node))
		return sort(*funDef);

	auto fSort = dynamic_pointer_cast<smtutil::FunctionSort>(sort(*m_currentFunction));
	solAssert(fSort, "");

	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	return make_shared<smtutil::FunctionSort>(
		fSort->domain + applyMap(m_currentFunction->localVariables(), smtSort),
		smtutil::SortProvider::boolSort
	);
}

smtutil::SortPointer CHC::summarySort(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	auto stateVariables = stateVariablesIncludingInheritedAndPrivate(_contract);
	auto sorts = stateSorts(_contract);

	auto smtSort = [](auto _var) { return smt::smtSortAbstractFunction(*_var->type()); };
	auto inputSorts = applyMap(_function.parameters(), smtSort);
	auto outputSorts = applyMap(_function.returnParameters(), smtSort);
	return make_shared<smtutil::FunctionSort>(
		vector<smtutil::SortPointer>{smtutil::SortProvider::uintSort} +
			sorts +
			inputSorts +
			sorts +
			inputSorts +
			outputSorts,
		smtutil::SortProvider::boolSort
	);
}

unique_ptr<smt::SymbolicFunctionVariable> CHC::createSymbolicBlock(smtutil::SortPointer _sort, string const& _name)
{
	auto block = make_unique<smt::SymbolicFunctionVariable>(
		_sort,
		_name,
		m_context
	);
	m_interface->registerRelation(block->currentFunctionValue());
	return block;
}

void CHC::defineInterfacesAndSummaries(SourceUnit const& _source)
{
	for (auto const& node: _source.nodes())
		if (auto const* contract = dynamic_cast<ContractDefinition const*>(node.get()))
			for (auto const* base: contract->annotation().linearizedBaseContracts)
			{
				string suffix = base->name() + "_" + to_string(base->id());
				m_interfaces[base] = createSymbolicBlock(interfaceSort(*base), "interface_" + suffix);
				m_nondetInterfaces[base] = createSymbolicBlock(nondetInterfaceSort(*base), "nondet_interface_" + suffix);

				for (auto const* var: stateVariablesIncludingInheritedAndPrivate(*base))
					if (!m_context.knownVariable(*var))
						createVariable(*var);

				/// Base nondeterministic interface that allows
				/// 0 steps to be taken, used as base for the inductive
				/// rule for each function.
				auto const& iface = *m_nondetInterfaces.at(base);
				auto state0 = stateVariablesAtIndex(0, *base);
				addRule(iface(state0 + state0), "base_nondet");

				for (auto const* function: base->definedFunctions())
				{
					for (auto var: function->parameters())
						createVariable(*var);
					for (auto var: function->returnParameters())
						createVariable(*var);
					for (auto const* var: function->localVariables())
						createVariable(*var);

					m_summaries[contract].emplace(function, createSummaryBlock(*function, *contract));

					if (
						!function->isConstructor() &&
						function->isPublic() &&
						!base->isLibrary() &&
						!base->isInterface()
					)
					{
						auto state1 = stateVariablesAtIndex(1, *base);
						auto state2 = stateVariablesAtIndex(2, *base);

						auto nondetPre = iface(state0 + state1);
						auto nondetPost = iface(state0 + state2);

						vector<smtutil::Expression> args{m_error.currentValue()};
						args += state1 +
							applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 0); }) +
							state2 +
							applyMap(function->parameters(), [this](auto _var) { return valueAtIndex(*_var, 1); }) +
							applyMap(function->returnParameters(), [this](auto _var) { return valueAtIndex(*_var, 1); });

						connectBlocks(nondetPre, nondetPost, (*m_summaries.at(base).at(function))(args));
					}
				}
			}
}

smtutil::Expression CHC::interface()
{
	auto paramExprs = applyMap(
		m_stateVariables,
		[this](auto _var) { return m_context.variable(*_var)->currentValue(); }
	);
	return (*m_interfaces.at(m_currentContract))(paramExprs);
}

smtutil::Expression CHC::interface(ContractDefinition const& _contract)
{
	return (*m_interfaces.at(&_contract))(stateVariablesAtIndex(0, _contract));
}

smtutil::Expression CHC::error()
{
	return (*m_errorPredicate)({});
}

smtutil::Expression CHC::error(unsigned _idx)
{
	return m_errorPredicate->functionValueAtIndex(_idx)({});
}

smtutil::Expression CHC::summary(ContractDefinition const& _contract)
{
	if (auto const* constructor = _contract.constructor())
		return (*m_constructorSummaryPredicate)(
			currentFunctionVariables(*constructor)
		);

	return (*m_constructorSummaryPredicate)(
		vector<smtutil::Expression>{m_error.currentValue()} +
		currentStateVariables()
	);
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	vector<smtutil::Expression> args{m_error.currentValue()};
	auto contract = _function.annotation().contract;
	args += contract->isLibrary() ? stateVariablesAtIndex(0, *contract) : initialStateVariables(_contract);
	args += applyMap(_function.parameters(), [this](auto _var) { return valueAtIndex(*_var, 0); });
	args += contract->isLibrary() ? stateVariablesAtIndex(1, *contract) : currentStateVariables(_contract);
	args += applyMap(_function.parameters(), [this](auto _var) { return currentValue(*_var); });
	args += applyMap(_function.returnParameters(), [this](auto _var) { return currentValue(*_var); });
	return (*m_summaries.at(&_contract).at(&_function))(args);
}

smtutil::Expression CHC::summary(FunctionDefinition const& _function)
{
	solAssert(m_currentContract, "");
	return summary(_function, *m_currentContract);
}

unique_ptr<smt::SymbolicFunctionVariable> CHC::createBlock(ASTNode const* _node, string const& _prefix)
{
	auto block = createSymbolicBlock(sort(_node),
		"block_" +
		uniquePrefix() +
		"_" +
		_prefix +
		predicateName(_node));

	solAssert(m_currentFunction, "");
	m_symbolFunction[block->currentFunctionValue().name] = m_currentFunction;
	return block;
}

unique_ptr<smt::SymbolicFunctionVariable> CHC::createSummaryBlock(FunctionDefinition const& _function, ContractDefinition const& _contract)
{
	auto block = createSymbolicBlock(summarySort(_function, _contract),
		"summary_" +
		uniquePrefix() +
		"_" +
		predicateName(&_function, &_contract));

	m_symbolFunction[block->currentFunctionValue().name] = &_function;
	return block;
}

void CHC::createErrorBlock()
{
	solAssert(m_errorPredicate, "");
	m_errorPredicate->increaseIndex();
	m_interface->registerRelation(m_errorPredicate->currentFunctionValue());
}

void CHC::connectBlocks(smtutil::Expression const& _from, smtutil::Expression const& _to, smtutil::Expression const& _constraints)
{
	smtutil::Expression edge = smtutil::Expression::implies(
		_from && m_context.assertions() && _constraints,
		_to
	);
	addRule(edge, _from.name + "_to_" + _to.name);
}

vector<smtutil::Expression> CHC::initialStateVariables()
{
	return stateVariablesAtIndex(0);
}

vector<smtutil::Expression> CHC::initialStateVariables(ContractDefinition const& _contract)
{
	return stateVariablesAtIndex(0, _contract);
}

vector<smtutil::Expression> CHC::stateVariablesAtIndex(unsigned _index)
{
	solAssert(m_currentContract, "");
	return stateVariablesAtIndex(_index, *m_currentContract);
}

vector<smtutil::Expression> CHC::stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract)
{
	return applyMap(
		stateVariablesIncludingInheritedAndPrivate(_contract),
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
	return applyMap(stateVariablesIncludingInheritedAndPrivate(_contract), [this](auto _var) { return currentValue(*_var); });
}

vector<smtutil::Expression> CHC::currentFunctionVariables()
{
	solAssert(m_currentFunction, "");
	return currentFunctionVariables(*m_currentFunction);
}

vector<smtutil::Expression> CHC::currentFunctionVariables(FunctionDefinition const& _function)
{
	vector<smtutil::Expression> initInputExprs;
	vector<smtutil::Expression> mutableInputExprs;
	for (auto const& var: _function.parameters())
	{
		initInputExprs.push_back(m_context.variable(*var)->valueAtIndex(0));
		mutableInputExprs.push_back(m_context.variable(*var)->currentValue());
	}
	auto returnExprs = applyMap(_function.returnParameters(), [this](auto _var) { return currentValue(*_var); });
	return vector<smtutil::Expression>{m_error.currentValue()} +
		initialStateVariables() +
		initInputExprs +
		currentStateVariables() +
		mutableInputExprs +
		returnExprs;
}

vector<smtutil::Expression> CHC::currentFunctionVariables(ContractDefinition const& _contract)
{
	if (auto const* constructor = _contract.constructor())
		return currentFunctionVariables(*constructor);

	return vector<smtutil::Expression>{m_error.currentValue()} + currentStateVariables();
}

vector<smtutil::Expression> CHC::currentBlockVariables()
{
	if (m_currentFunction)
		return currentFunctionVariables() + applyMap(m_currentFunction->localVariables(), [this](auto _var) { return currentValue(*_var); });

	return currentFunctionVariables();
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

smtutil::Expression CHC::predicate(smt::SymbolicFunctionVariable const& _block)
{
	return _block(currentBlockVariables());
}

smtutil::Expression CHC::predicate(
	smt::SymbolicFunctionVariable const& _block,
	vector<smtutil::Expression> const& _arguments
)
{
	return _block(_arguments);
}

smtutil::Expression CHC::predicate(FunctionCall const& _funCall)
{
	auto const* function = functionCallToDefinition(_funCall);
	if (!function)
		return smtutil::Expression(true);

	m_error.increaseIndex();
	vector<smtutil::Expression> args{m_error.currentValue()};
	auto const* contract = function->annotation().contract;
	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
	bool otherContract = contract->isLibrary() ||
		funType.kind() == FunctionType::Kind::External ||
		funType.kind() == FunctionType::Kind::BareStaticCall;

	args += otherContract ? stateVariablesAtIndex(0, *contract) : currentStateVariables();
	args += symbolicArguments(_funCall);
	if (!otherContract)
		for (auto const& var: m_stateVariables)
			m_context.variable(*var)->increaseIndex();
	args += otherContract ? stateVariablesAtIndex(1, *contract) : currentStateVariables();

	for (auto var: function->parameters() + function->returnParameters())
	{
		if (m_context.knownVariable(*var))
			m_context.variable(*var)->increaseIndex();
		else
			createVariable(*var);
		args.push_back(currentValue(*var));
	}

	if (otherContract)
		return (*m_summaries.at(contract).at(function))(args);

	solAssert(m_currentContract, "");
	return (*m_summaries.at(m_currentContract).at(function))(args);
}

void CHC::addRule(smtutil::Expression const& _rule, string const& _ruleName)
{
	m_interface->addRule(_rule, _ruleName);
}

pair<smtutil::CheckResult, CHCSolverInterface::CexGraph> CHC::query(smtutil::Expression const& _query, langutil::SourceLocation const& _location)
{
	smtutil::CheckResult result;
	CHCSolverInterface::CexGraph cex;
	tie(result, cex) = m_interface->query(_query);
	switch (result)
	{
	case smtutil::CheckResult::SATISFIABLE:
	{
#ifdef HAVE_Z3
		// Even though the problem is SAT, Spacer's pre processing makes counterexamples incomplete.
		// We now disable those optimizations and check whether we can still solve the problem.
		auto* spacer = dynamic_cast<Z3CHCInterface*>(m_interface.get());
		solAssert(spacer, "");
		spacer->setSpacerOptions(false);

		smtutil::CheckResult resultNoOpt;
		CHCSolverInterface::CexGraph cexNoOpt;
		tie(resultNoOpt, cexNoOpt) = m_interface->query(_query);

		if (resultNoOpt == smtutil::CheckResult::SATISFIABLE)
			cex = move(cexNoOpt);

		spacer->setSpacerOptions(true);
#endif
		break;
	}
	case smtutil::CheckResult::UNSATISFIABLE:
		break;
	case smtutil::CheckResult::UNKNOWN:
		break;
	case smtutil::CheckResult::CONFLICTING:
		m_outerErrorReporter.warning(1988_error, _location, "At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case smtutil::CheckResult::ERROR:
		m_outerErrorReporter.warning(1218_error, _location, "Error trying to invoke SMT solver.");
		break;
	}
	return {result, cex};
}

void CHC::addVerificationTarget(
	ASTNode const* _scope,
	VerificationTarget::Type _type,
	smtutil::Expression _from,
	smtutil::Expression _constraints,
	smtutil::Expression _errorId
)
{
	m_verificationTargets.emplace(_scope, CHCVerificationTarget{{_type, _from, _constraints}, _errorId});
}

void CHC::addAssertVerificationTarget(ASTNode const* _scope, smtutil::Expression _from, smtutil::Expression _constraints, smtutil::Expression _errorId)
{
	addVerificationTarget(_scope, VerificationTarget::Type::Assert, _from, _constraints, _errorId);
}

void CHC::addArrayPopVerificationTarget(ASTNode const* _scope, smtutil::Expression _errorId)
{
	solAssert(m_currentContract, "");
	solAssert(m_currentFunction, "");

	if (m_currentFunction->isConstructor())
		addVerificationTarget(_scope, VerificationTarget::Type::PopEmptyArray, summary(*m_currentContract), smtutil::Expression(true), _errorId);
	else
	{
		auto iface = (*m_interfaces.at(m_currentContract))(initialStateVariables());
		auto sum = summary(*m_currentFunction);
		addVerificationTarget(_scope, VerificationTarget::Type::PopEmptyArray, iface, sum, _errorId);
	}
}

void CHC::checkVerificationTargets()
{
	for (auto const& [scope, target]: m_verificationTargets)
	{
		if (target.type == VerificationTarget::Type::Assert)
			checkAssertTarget(scope, target);
		else
		{
			string satMsg;
			string unknownMsg;
			ErrorId errorReporterId;

			if (target.type == VerificationTarget::Type::PopEmptyArray)
			{
				solAssert(dynamic_cast<FunctionCall const*>(scope), "");
				satMsg = "Empty array \"pop\" detected here";
				unknownMsg = "Empty array \"pop\" might happen here.";
				errorReporterId = 2529_error;
			}
			else
				solAssert(false, "");

			auto it = m_errorIds.find(scope->id());
			solAssert(it != m_errorIds.end(), "");
			checkAndReportTarget(scope, target, it->second, errorReporterId, satMsg, unknownMsg);
		}
	}
}

void CHC::checkAssertTarget(ASTNode const* _scope, CHCVerificationTarget const& _target)
{
	solAssert(_target.type == VerificationTarget::Type::Assert, "");
	auto assertions = transactionAssertions(_scope);
	for (auto const* assertion: assertions)
	{
		auto it = m_errorIds.find(assertion->id());
		solAssert(it != m_errorIds.end(), "");
		unsigned errorId = it->second;

		checkAndReportTarget(assertion, _target, errorId, 6328_error, "Assertion violation happens here");
	}
}

void CHC::checkAndReportTarget(
	ASTNode const* _scope,
	CHCVerificationTarget const& _target,
	unsigned _errorId,
	ErrorId _errorReporterId,
	string _satMsg,
	string _unknownMsg
)
{
	if (m_unsafeTargets.count(_scope) && m_unsafeTargets.at(_scope).count(_target.type))
		return;

	createErrorBlock();
	connectBlocks(_target.value, error(), _target.constraints && (_target.errorId == _errorId));
	auto const& [result, model] = query(error(), _scope->location());
	if (result == smtutil::CheckResult::UNSATISFIABLE)
		m_safeTargets[_scope].insert(_target.type);
	else if (result == smtutil::CheckResult::SATISFIABLE)
	{
		solAssert(!_satMsg.empty(), "");
		m_unsafeTargets[_scope].insert(_target.type);
		auto cex = generateCounterexample(model, error().name);
		if (cex)
			m_outerErrorReporter.warning(
				_errorReporterId,
				_scope->location(),
				_satMsg,
				SecondarySourceLocation().append(" for:\n" + *cex, SourceLocation{})
			);
		else
			m_outerErrorReporter.warning(
				_errorReporterId,
				_scope->location(),
				_satMsg + "."
			);
	}
	else if (!_unknownMsg.empty())
		m_outerErrorReporter.warning(
			_errorReporterId,
			_scope->location(),
			_unknownMsg
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
		if (node.first == _root)
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
			if (_graph.nodes.at(summaryId).first.rfind("summary", 0) != 0)
				swap(summaryId, *interfaceId);
			solAssert(_graph.nodes.at(*interfaceId).first.rfind("interface", 0) == 0, "");
		}
		/// The children are unordered, so we need to check which is the summary and
		/// which is the interface.

		solAssert(_graph.nodes.at(summaryId).first.rfind("summary", 0) == 0, "");
		/// At this point property 2 from the function description is verified for this node.

		auto const& summaryNode = _graph.nodes.at(summaryId);
		solAssert(m_symbolFunction.count(summaryNode.first), "");

		FunctionDefinition const* calledFun = nullptr;
		ContractDefinition const* calledContract = nullptr;
		if (auto const* contract = dynamic_cast<ContractDefinition const*>(m_symbolFunction.at(summaryNode.first)))
		{
			if (auto const* constructor = contract->constructor())
				calledFun = constructor;
			else
				calledContract = contract;
		}
		else if (auto const* fun = dynamic_cast<FunctionDefinition const*>(m_symbolFunction.at(summaryNode.first)))
			calledFun = fun;
		else
			solAssert(false, "");

		solAssert((calledFun && !calledContract) || (!calledFun && calledContract), "");
		auto const& stateVars = calledFun ? stateVariablesIncludingInheritedAndPrivate(*calledFun) : stateVariablesIncludingInheritedAndPrivate(*calledContract);
		/// calledContract != nullptr implies that the constructor of the analyzed contract is implicit and
		/// therefore takes no parameters.

		/// This summary node is the end of a tx.
		/// If it is the first summary node seen in this loop, it is the summary
		/// of the public/external function that was called when the error was reached,
		/// but not necessarily the summary of the function that contains the error.
		if (!lastTxSeen)
		{
			lastTxSeen = true;
			/// Generate counterexample message local to the failed target.
			localState = formatStateCounterexample(stateVars, calledFun, summaryNode.second) + "\n";
			if (calledFun)
			{
				/// The signature of a summary predicate is: summary(error, preStateVars, preInputVars, postInputVars, outputVars).
				auto const& inParams = calledFun->parameters();
				unsigned initLocals = stateVars.size() * 2 + 1 + inParams.size();
				/// In this loop we are interested in postInputVars.
				for (unsigned i = initLocals; i < initLocals + inParams.size(); ++i)
				{
					auto param = inParams.at(i - initLocals);
					if (param->type()->isValueType())
						localState += param->name() + " = " + summaryNode.second.at(i) + "\n";
				}
				auto const& outParams = calledFun->returnParameters();
				initLocals += inParams.size();
				/// In this loop we are interested in outputVars.
				for (unsigned i = initLocals; i < initLocals + outParams.size(); ++i)
				{
					auto param = outParams.at(i - initLocals);
					if (param->type()->isValueType())
						localState += param->name() + " = " + summaryNode.second.at(i) + "\n";
				}
			}
		}
		else
			/// We report the state after every tx in the trace except for the last, which is reported
			/// first in the code above.
			path.emplace_back("State: " + formatStateCounterexample(stateVars, calledFun, summaryNode.second));

		string txCex = calledContract ? "constructor()" : formatFunctionCallCounterexample(stateVars, *calledFun, summaryNode.second);
		path.emplace_back(txCex);

		/// Recurse on the next interface node which represents the previous transaction
		/// or stop.
		if (interfaceId)
			node = *interfaceId;
		else
			break;
	}

	return localState + "\nTransaction trace:\n" + boost::algorithm::join(boost::adaptors::reverse(path), "\n");
}

string CHC::formatStateCounterexample(vector<VariableDeclaration const*> const& _stateVars, FunctionDefinition const* _function, vector<string> const& _summaryValues)
{
	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postInputVars, outputVars).
	/// The signature of an implicit constructor summary predicate is: summary(error, postStateVars).
	/// Here we are interested in postStateVars.
	vector<string>::const_iterator stateFirst;
	vector<string>::const_iterator stateLast;
	if (_function)
	{
		stateFirst = _summaryValues.begin() + 1 + static_cast<int>(_stateVars.size()) + static_cast<int>(_function->parameters().size());
		stateLast = stateFirst + static_cast<int>(_stateVars.size());
	}
	else
	{
		stateFirst = _summaryValues.begin() + 1;
		stateLast = stateFirst + static_cast<int>(_stateVars.size());
	}

	solAssert(stateFirst >= _summaryValues.begin() && stateFirst <= _summaryValues.end(), "");
	solAssert(stateLast >= _summaryValues.begin() && stateLast <= _summaryValues.end(), "");
	vector<string> stateArgs(stateFirst, stateLast);
	solAssert(stateArgs.size() == _stateVars.size(), "");

	vector<string> stateCex;
	for (unsigned i = 0; i < stateArgs.size(); ++i)
	{
		auto var = _stateVars.at(i);
		if (var->type()->isValueType())
			stateCex.emplace_back(var->name() + " = " + stateArgs.at(i));
	}

	return boost::algorithm::join(stateCex, ", ");
}

string CHC::formatFunctionCallCounterexample(vector<VariableDeclaration const*> const& _stateVars, FunctionDefinition const& _function, vector<string> const& _summaryValues)
{
	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postInputVars, outputVars).
	/// Here we are interested in preInputVars.
	vector<string>::const_iterator first = _summaryValues.begin() + static_cast<int>(_stateVars.size()) + 1;
	vector<string>::const_iterator last = first + static_cast<int>(_function.parameters().size());
	solAssert(first >= _summaryValues.begin() && first <= _summaryValues.end(), "");
	solAssert(last >= _summaryValues.begin() && last <= _summaryValues.end(), "");
	vector<string> functionArgsCex(first, last);
	vector<string> functionArgs;

	auto const& params = _function.parameters();
	solAssert(params.size() == functionArgsCex.size(), "");
	for (unsigned i = 0; i < params.size(); ++i)
		if (params[i]->type()->isValueType())
			functionArgs.emplace_back(functionArgsCex[i]);
		else
			functionArgs.emplace_back(params[i]->name());

	string fName = _function.isConstructor() ? "constructor" :
		_function.isFallback() ? "fallback" :
		_function.isReceive() ? "receive" :
		_function.name();
	return fName + "(" + boost::algorithm::join(functionArgs, ", ") + ")";
}

string CHC::cex2dot(smtutil::CHCSolverInterface::CexGraph const& _cex)
{
	string dot = "digraph {\n";

	auto pred = [&](CHCSolverInterface::CexNode const& _node) {
		return "\"" + _node.first + "(" + boost::algorithm::join(_node.second, ", ") + ")\"";
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

unsigned CHC::newErrorId(frontend::Expression const& _expr)
{
	unsigned errorId = m_context.newUniqueId();
	// We need to make sure the error id is not zero,
	// because error id zero actually means no error in the CHC encoding.
	if (errorId == 0)
		errorId = m_context.newUniqueId();
	m_errorIds.emplace(_expr.id(), errorId);
	return errorId;
}
