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

#include <libsolidity/formal/CHC.h>

#ifdef HAVE_Z3
#include <libsolidity/formal/Z3CHCInterface.h>
#endif

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/TypeProvider.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

CHC::CHC(smt::EncodingContext& _context, ErrorReporter& _errorReporter):
	SMTEncoder(_context),
#ifdef HAVE_Z3
	m_interface(make_shared<smt::Z3CHCInterface>()),
#endif
	m_outerErrorReporter(_errorReporter)
{
}

void CHC::analyze(SourceUnit const& _source)
{
	solAssert(_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker), "");

#ifdef HAVE_Z3
	auto z3Interface = dynamic_pointer_cast<smt::Z3CHCInterface>(m_interface);
	solAssert(z3Interface, "");
	m_context.setSolver(z3Interface->z3Interface());
	m_context.clear();
	m_context.setAssertionAccumulation(false);
	m_variableUsage.setFunctionInlining(false);

	_source.accept(*this);
#endif
}

bool CHC::visit(ContractDefinition const& _contract)
{
	if (!shouldVisit(_contract))
		return false;

	reset();

	if (!SMTEncoder::visit(_contract))
		return false;

	m_stateVariables = _contract.stateVariablesIncludingInherited();

	for (auto const& var: m_stateVariables)
		// SMT solvers do not support function types as arguments.
		if (var->type()->category() == Type::Category::Function)
			m_stateSorts.push_back(make_shared<smt::Sort>(smt::Kind::Int));
		else
			m_stateSorts.push_back(smt::smtSort(*var->type()));

	clearIndices();

	string interfaceName = "interface_" + _contract.name() + "_" + to_string(_contract.id());
	m_interfacePredicate = createSymbolicBlock(interfaceSort(), interfaceName);

	// TODO create static instances for Bool/Int sorts in SolverInterface.
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	auto errorFunctionSort = make_shared<smt::FunctionSort>(
		vector<smt::SortPointer>(),
		boolSort
	);
	m_errorPredicate = createSymbolicBlock(errorFunctionSort, "error");

	// If the contract has a constructor it is handled as a function.
	// Otherwise we zero-initialize all state vars.
	// TODO take into account state vars init values.
	if (!_contract.constructor())
	{
		string constructorName = "constructor_" + _contract.name() + "_" + to_string(_contract.id());
		m_constructorPredicate = createSymbolicBlock(constructorSort(), constructorName);
		smt::Expression constructorPred = (*m_constructorPredicate)({});
		addRule(constructorPred, constructorName);

		for (auto const& var: m_stateVariables)
		{
			auto const& symbVar = m_context.variable(*var);
			symbVar->increaseIndex();
			m_interface->declareVariable(symbVar->currentName(), *symbVar->sort());
			m_context.setZeroValue(*symbVar);
		}

		connectBlocks(constructorPred, interface());
	}

	return true;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	if (!shouldVisit(_contract))
		return;

	for (unsigned i = 0; i < m_verificationTargets.size(); ++i)
	{
		auto const& target = m_verificationTargets.at(i);
		auto errorAppl = error(i + 1);
		if (query(errorAppl, target->location()))
			m_safeAssertions.insert(target);
	}

	SMTEncoder::endVisit(_contract);
}

bool CHC::visit(FunctionDefinition const& _function)
{
	if (!shouldVisit(_function))
		return false;

	solAssert(!m_currentFunction, "Inlining internal function calls not yet implemented");
	m_currentFunction = &_function;

	initFunction(_function);

	// Store the constraints related to variable initialization.
	smt::Expression const& initAssertions = m_context.assertions();
	m_context.pushSolver();

	solAssert(m_functionBlocks == 0, "");

	createBlock(m_currentFunction);
	createBlock(&m_currentFunction->body(), "block_");

	auto functionPred = predicate(m_currentFunction);
	auto bodyPred = predicate(&m_currentFunction->body());

	connectBlocks(interface(), functionPred);
	connectBlocks(functionPred, bodyPred);

	m_context.popSolver();

	pushBlock(&m_currentFunction->body());

	// We need to re-add the constraints that were created for initialization of variables.
	m_context.addAssertion(initAssertions);

	SMTEncoder::visit(*m_currentFunction);

	return false;
}

void CHC::endVisit(FunctionDefinition const& _function)
{
	if (!shouldVisit(_function))
		return;

	solAssert(m_currentFunction == &_function, "Inlining internal function calls not yet implemented");

	// Function Exit block.
	createBlock(m_currentFunction);
	connectBlocks(m_path.back(), predicate(&_function));

	// Rule FunctionExit -> Interface, uses no constraints.
	clearIndices();
	m_context.pushSolver();
	connectBlocks(predicate(&_function), interface());
	m_context.popSolver();

	m_currentFunction = nullptr;
	solAssert(m_path.size() == m_functionBlocks, "");
	while (m_functionBlocks > 0)
		popBlock();

	solAssert(m_path.empty(), "");

	SMTEncoder::endVisit(_function);
}

bool CHC::visit(IfStatement const& _if)
{
	solAssert(m_currentFunction, "");

	bool unknownFunctionCallWasSeen = m_unknownFunctionCallSeen;
	m_unknownFunctionCallSeen = false;

	SMTEncoder::visit(_if);

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

	if (_while.isDoWhile())
		_while.body().accept(*this);

	visitLoop(
		_while,
		&_while.condition(),
		_while.body(),
		nullptr
	);

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

	if (auto init = _for.initializationExpression())
		init->accept(*this);

	visitLoop(
		_for,
		_for.condition(),
		_for.body(),
		_for.loopExpression()
	);

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
	case FunctionType::Kind::External:
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::BareStaticCall:
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

void CHC::endVisit(Break const&)
{
	solAssert(m_breakDest, "");
	m_breakSeen = true;
}

void CHC::endVisit(Continue const&)
{
	solAssert(m_continueDest, "");
	m_continueSeen = true;
}

void CHC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");

	solAssert(!m_path.empty(), "");

	createErrorBlock();

	smt::Expression assertNeg = !(m_context.expression(*args.front())->currentValue());
	connectBlocks(m_path.back(), error(), currentPathConditions() && assertNeg);

	m_verificationTargets.push_back(&_funCall);
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

void CHC::visitLoop(
	BreakableStatement const& _loop,
	Expression const* _condition,
	Statement const& _body,
	ASTNode const* _postLoop
)
{
	bool breakWasSeen = m_breakSeen;
	bool continueWasSeen = m_continueSeen;
	m_breakSeen = false;
	m_continueSeen = false;

	solAssert(m_currentFunction, "");
	auto const& functionBody = m_currentFunction->body();

	createBlock(&_loop, "loop_header_");
	createBlock(&_body, "loop_body_");
	createBlock(&functionBody, "block_");

	connectBlocks(m_path.back(), predicate(&_loop));

	// We need to save the next block here because new blocks
	// might be created inside the loop body.
	// This will be m_path.back() in the end of this function.
	pushBlock(&functionBody);

	smt::Expression loopHeader = predicate(&_loop);
	pushBlock(&_loop);

	if (_condition)
		_condition->accept(*this);
	auto condition = _condition ? expr(*_condition) : smt::Expression(true);

	connectBlocks(loopHeader, predicate(&_body), condition);
	connectBlocks(loopHeader, predicate(&functionBody), !condition);

	// Loop body visit.
	pushBlock(&_body);

	m_breakDest = &functionBody;
	m_continueDest = _postLoop ? _postLoop : &_loop;

	auto functionBlocks = m_functionBlocks;
	_body.accept(*this);
	if (_postLoop)
	{
		createBlock(_postLoop, "loop_post_");
		connectBlocks(m_path.back(), predicate(_postLoop));
		pushBlock(_postLoop);
		_postLoop->accept(*this);
	}

	// Back edge.
	connectBlocks(m_path.back(), predicate(&_loop));

	// Pop all function blocks created by nested inner loops
	// to adjust the assertion context.
	for (unsigned i = m_functionBlocks; i > functionBlocks; --i)
		popBlock();
	m_functionBlocks = functionBlocks;

	// Loop body
	popBlock();
	// Loop header
	popBlock();

	// New function block starts with indices = 0
	clearIndices();

	if (m_breakSeen || m_continueSeen)
	{
		eraseKnowledge();
		m_context.resetVariables([](VariableDeclaration const&) { return true; });
	}

	m_breakSeen = breakWasSeen;
	m_continueSeen = continueWasSeen;
}

void CHC::reset()
{
	m_stateSorts.clear();
	m_stateVariables.clear();
	m_verificationTargets.clear();
	m_safeAssertions.clear();
	m_unknownFunctionCallSeen = false;
	m_breakSeen = false;
	m_continueSeen = false;
}

void CHC::eraseKnowledge()
{
	resetStateVariables();
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.hasReferenceOrMappingType(); });
}

bool CHC::shouldVisit(ContractDefinition const& _contract) const
{
	if (
		_contract.isLibrary() ||
		_contract.isInterface()
	)
		return false;
	return true;
}

bool CHC::shouldVisit(FunctionDefinition const& _function) const
{
	if (
		_function.isPublic() &&
		_function.isImplemented() &&
		!_function.isConstructor()
	)
		return true;
	return false;
}

void CHC::pushBlock(ASTNode const* _node)
{
	clearIndices();
	m_context.pushSolver();
	m_path.push_back(predicate(_node));
	++m_functionBlocks;
}

void CHC::popBlock()
{
	m_context.popSolver();
	m_path.pop_back();
	--m_functionBlocks;
}

smt::SortPointer CHC::constructorSort()
{
	solAssert(m_currentContract, "");
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	if (!m_currentContract->constructor())
		return make_shared<smt::FunctionSort>(vector<smt::SortPointer>{}, boolSort);
	return sort(*m_currentContract->constructor());
}

smt::SortPointer CHC::interfaceSort()
{
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	return make_shared<smt::FunctionSort>(
		m_stateSorts,
		boolSort
	);
}

smt::SortPointer CHC::sort(FunctionDefinition const& _function)
{
	if (m_nodeSorts.count(&_function))
		return m_nodeSorts.at(&_function);

	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	vector<smt::SortPointer> varSorts;
	for (auto const& var: _function.parameters() + _function.returnParameters())
		varSorts.push_back(smt::smtSort(*var->type()));
	auto sort = make_shared<smt::FunctionSort>(
		m_stateSorts + varSorts,
		boolSort
	);
	return m_nodeSorts[&_function] = move(sort);
}

smt::SortPointer CHC::sort(ASTNode const* _node)
{
	if (m_nodeSorts.count(_node))
		return m_nodeSorts.at(_node);

	if (auto funDef = dynamic_cast<FunctionDefinition const*>(_node))
		return sort(*funDef);

	auto fSort = dynamic_pointer_cast<smt::FunctionSort>(sort(*m_currentFunction));
	solAssert(fSort, "");

	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	vector<smt::SortPointer> varSorts;
	for (auto const& var: m_currentFunction->localVariables())
		varSorts.push_back(smt::smtSort(*var->type()));
	auto functionBodySort = make_shared<smt::FunctionSort>(
		fSort->domain + varSorts,
		boolSort
	);
	return m_nodeSorts[_node] = move(functionBodySort);
}

unique_ptr<smt::SymbolicFunctionVariable> CHC::createSymbolicBlock(smt::SortPointer _sort, string const& _name)
{
	auto block = make_unique<smt::SymbolicFunctionVariable>(
		_sort,
		_name,
		m_context
	);
	m_interface->registerRelation(block->currentValue());
	return block;
}

smt::Expression CHC::constructor()
{
	solAssert(m_currentContract, "");

	if (!m_currentContract->constructor())
		return (*m_constructorPredicate)({});

	vector<smt::Expression> paramExprs;
	for (auto const& var: m_currentContract->constructor()->parameters())
		paramExprs.push_back(m_context.variable(*var)->currentValue());
	return (*m_constructorPredicate)(paramExprs);
}

smt::Expression CHC::interface()
{
	vector<smt::Expression> paramExprs;
	for (auto const& var: m_stateVariables)
		paramExprs.push_back(m_context.variable(*var)->currentValue());
	return (*m_interfacePredicate)(paramExprs);
}

smt::Expression CHC::error()
{
	return (*m_errorPredicate)({});
}

smt::Expression CHC::error(unsigned _idx)
{
	return m_errorPredicate->valueAtIndex(_idx)({});
}

void CHC::createBlock(ASTNode const* _node, string const& _prefix)
{
	if (m_predicates.count(_node))
	{
		m_predicates.at(_node)->increaseIndex();
		m_interface->registerRelation(m_predicates.at(_node)->currentValue());
	}
	else
		m_predicates[_node] = createSymbolicBlock(sort(_node), _prefix + predicateName(_node));
}

void CHC::createErrorBlock()
{
	solAssert(m_errorPredicate, "");
	m_errorPredicate->increaseIndex();
	m_interface->registerRelation(m_errorPredicate->currentValue());
}

void CHC::connectBlocks(smt::Expression const& _from, smt::Expression const& _to, smt::Expression const& _constraints)
{
	smt::Expression edge = smt::Expression::implies(
		_from && m_context.assertions() && _constraints,
		_to
	);
	addRule(edge, _from.name + "_to_" + _to.name);
}

vector<smt::Expression> CHC::currentFunctionVariables()
{
	solAssert(m_currentFunction, "");
	vector<smt::Expression> paramExprs;
	for (auto const& var: m_stateVariables)
		paramExprs.push_back(m_context.variable(*var)->currentValue());
	for (auto const& var: m_currentFunction->parameters() + m_currentFunction->returnParameters())
		paramExprs.push_back(m_context.variable(*var)->currentValue());
	return paramExprs;
}

vector<smt::Expression> CHC::currentBlockVariables()
{
	solAssert(m_currentFunction, "");
	vector<smt::Expression> paramExprs;
	for (auto const& var: m_currentFunction->localVariables())
		paramExprs.push_back(m_context.variable(*var)->currentValue());
	return currentFunctionVariables() + paramExprs;
}

void CHC::clearIndices()
{
	for (auto const& var: m_stateVariables)
		m_context.variable(*var)->resetIndex();
	if (m_currentFunction)
	{
		for (auto const& var: m_currentFunction->parameters() + m_currentFunction->returnParameters())
			m_context.variable(*var)->resetIndex();
		for (auto const& var: m_currentFunction->localVariables())
			m_context.variable(*var)->resetIndex();
	}
}

string CHC::predicateName(ASTNode const* _node)
{
	string prefix;
	if (auto funDef = dynamic_cast<FunctionDefinition const*>(_node))
	{
		prefix = funDef->isConstructor() ?
			"constructor" :
			funDef->isFallback() ?
				"fallback" :
				"function_" + funDef->name();
		prefix += "_";
	}
	return prefix + to_string(_node->id());
}

smt::Expression CHC::predicate(ASTNode const* _node)
{
	if (dynamic_cast<FunctionDefinition const*>(_node))
		return (*m_predicates.at(_node))(currentFunctionVariables());
	return (*m_predicates.at(_node))(currentBlockVariables());
}

void CHC::addRule(smt::Expression const& _rule, string const& _ruleName)
{
	m_interface->addRule(_rule, _ruleName);
}

bool CHC::query(smt::Expression const& _query, langutil::SourceLocation const& _location)
{
	smt::CheckResult result;
	vector<string> values;
	tie(result, values) = m_interface->query(_query);
	switch (result)
	{
	case smt::CheckResult::SATISFIABLE:
		break;
	case smt::CheckResult::UNSATISFIABLE:
		return true;
	case smt::CheckResult::UNKNOWN:
		break;
	case smt::CheckResult::CONFLICTING:
		m_outerErrorReporter.warning(_location, "At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case smt::CheckResult::ERROR:
		m_outerErrorReporter.warning(_location, "Error trying to invoke SMT solver.");
		break;
	}
	return false;
}
