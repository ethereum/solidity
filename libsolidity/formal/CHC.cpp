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

	string interfaceName = "interface_" + _contract.name() + "_" + to_string(_contract.id());
	m_interfacePredicate = createBlock(interfaceSort(),	interfaceName);

	// TODO create static instances for Bool/Int sorts in SolverInterface.
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	auto errorFunctionSort = make_shared<smt::FunctionSort>(
		vector<smt::SortPointer>(),
		boolSort
	);
	m_errorPredicate = createBlock(errorFunctionSort, "error");

	// If the contract has a constructor it is handled as a function.
	// Otherwise we zero-initialize all state vars.
	// TODO take into account state vars init values.
	if (!_contract.constructor())
	{
		string constructorName = "constructor_" + _contract.name() + "_" + to_string(_contract.id());
		m_constructorPredicate = createBlock(constructorSort(), constructorName);

		for (auto const& var: m_stateVariables)
		{
			auto const& symbVar = m_context.variable(*var);
			symbVar->increaseIndex();
			m_interface->declareVariable(symbVar->currentName(), *symbVar->sort());
			m_context.setZeroValue(*symbVar);
		}

		smt::Expression constructorAppl = (*m_constructorPredicate)({});
		m_interface->addRule(constructorAppl, constructorName);

		smt::Expression constructorInterface = smt::Expression::implies(
			constructorAppl && m_context.assertions(),
			interface()
		);
		m_interface->addRule(constructorInterface, constructorName + "_to_" + interfaceName);
	}

	return true;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	if (!shouldVisit(_contract))
		return;

	auto errorAppl = (*m_errorPredicate)({});
	for (auto const& target: m_verificationTargets)
		if (query(errorAppl, target->location()))
			m_safeAssertions.insert(target);

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

	createFunctionBlock(*m_currentFunction);

	// Rule Interface -> FunctionEntry, uses no constraints.
	smt::Expression interfaceFunction = smt::Expression::implies(
		interface(),
		predicateCurrent(m_currentFunction)
	);
	m_interface->addRule(
		interfaceFunction,
		m_interfacePredicate->currentName() + "_to_" + m_predicates.at(m_currentFunction)->currentName()
	);

	pushBlock(predicateCurrent(m_currentFunction));

	createFunctionBlock(m_currentFunction->body());

	// Rule FunctionEntry -> FunctionBody, also no constraints.
	smt::Expression functionBody = smt::Expression::implies(
		predicateEntry(m_currentFunction),
		predicateBodyCurrent(&m_currentFunction->body())
	);
	m_interface->addRule(
		functionBody,
		m_predicates.at(m_currentFunction)->currentName() + "_to_" + m_predicates.at(&m_currentFunction->body())->currentName()
	);

	pushBlock(predicateBodyCurrent(&m_currentFunction->body()));
	// We need to re-add the constraints that were created for initialization of variables.
	m_context.addAssertion(initAssertions);

	solAssert(m_functionBlocks == 0, "");
	m_functionBlocks = 2;

	SMTEncoder::visit(*m_currentFunction);

	return false;
}

void CHC::endVisit(FunctionDefinition const& _function)
{
	if (!shouldVisit(_function))
		return;

	solAssert(m_currentFunction == &_function, "Inlining internal function calls not yet implemented");

	// Create Function Exit block.
	createFunctionBlock(*m_currentFunction);

	// Rule FunctionBody -> FunctionExit.
	smt::Expression bodyFunction = smt::Expression::implies(
		predicateEntry(&_function.body()) && m_context.assertions(),
		predicateCurrent(&_function)
	);
	m_interface->addRule(
		bodyFunction,
		m_predicates.at(&_function.body())->currentName() + "_to_" + m_predicates.at(&_function.body())->currentName()
	);

	// Rule FunctionExit -> Interface, uses no constraints.
	smt::Expression functionInterface = smt::Expression::implies(
		predicateCurrent(&_function),
		interface()
	);
	m_interface->addRule(
		functionInterface,
		m_predicates.at(&_function)->currentName() + "_to_" + m_interfacePredicate->currentName()
	);

	m_currentFunction = nullptr;
	solAssert(m_path.size() == m_functionBlocks, "");
	for (unsigned i = 0; i < m_path.size(); ++i)
		m_context.popSolver();
	m_functionBlocks = 0;
	m_path.clear();

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
	eraseKnowledge();
	m_context.resetVariables(touchedVariables(_while));
	return false;
}

bool CHC::visit(ForStatement const& _for)
{
	eraseKnowledge();
	m_context.resetVariables(touchedVariables(_for));
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

void CHC::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");

	solAssert(!m_path.empty(), "");

	smt::Expression assertNeg = !(m_context.expression(*args.front())->currentValue());
	smt::Expression assertionError = smt::Expression::implies(
		m_path.back() && m_context.assertions() && currentPathConditions() && assertNeg,
		error()
	);
	string predicateName = "assert_" + to_string(_funCall.id());
	m_interface->addRule(assertionError, predicateName + "_to_error");

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

void CHC::reset()
{
	m_stateSorts.clear();
	m_stateVariables.clear();
	m_verificationTargets.clear();
	m_safeAssertions.clear();
	m_unknownFunctionCallSeen = false;
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
		_function.isImplemented()
	)
		return true;
	return false;
}

void CHC::pushBlock(smt::Expression const& _block)
{
	m_context.pushSolver();
	m_path.push_back(_block);
}

void CHC::popBlock()
{
	m_context.popSolver();
	m_path.pop_back();
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

smt::SortPointer CHC::sort(Block const& _block)
{
	if (m_nodeSorts.count(&_block))
		return m_nodeSorts.at(&_block);

	solAssert(_block.scope() == m_currentFunction, "");

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
	return m_nodeSorts[&_block] = move(functionBodySort);
}

unique_ptr<smt::SymbolicFunctionVariable> CHC::createBlock(smt::SortPointer _sort, string const& _name)
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

void CHC::createFunctionBlock(FunctionDefinition const& _function)
{
	if (m_predicates.count(&_function))
	{
		m_predicates.at(&_function)->increaseIndex();
		m_interface->registerRelation(m_predicates.at(&_function)->currentValue());
	}
	else
		m_predicates[&_function] = createBlock(
			sort(_function),
			predicateName(_function)
		);
}

void CHC::createFunctionBlock(Block const& _block)
{
	solAssert(_block.scope() == m_currentFunction, "");
	if (m_predicates.count(&_block))
	{
		m_predicates.at(&_block)->increaseIndex();
		m_interface->registerRelation(m_predicates.at(&_block)->currentValue());
	}
	else
		m_predicates[&_block] = createBlock(
			sort(_block),
			predicateName(*m_currentFunction) + "_body"
		);
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

string CHC::predicateName(FunctionDefinition const& _function)
{
	string functionName = _function.isConstructor() ?
		"constructor" :
		_function.isFallback() ?
			"fallback" :
			"function_" + _function.name();
	return functionName + "_" + to_string(_function.id());
}

smt::Expression CHC::predicateCurrent(ASTNode const* _node)
{
	return (*m_predicates.at(_node))(currentFunctionVariables());
}

smt::Expression CHC::predicateBodyCurrent(ASTNode const* _node)
{
	return (*m_predicates.at(_node))(currentBlockVariables());
}

smt::Expression CHC::predicateEntry(ASTNode const* _node)
{
	solAssert(!m_path.empty(), "");
	return (*m_predicates.at(_node))(m_path.back().arguments);
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
