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

	SMTEncoder::visit(*m_currentFunction);

	return false;
}

void CHC::endVisit(FunctionDefinition const& _function)
{
	if (!shouldVisit(_function))
		return;

	solAssert(m_currentFunction == &_function, "Inlining internal function calls not yet implemented");
	m_currentFunction = nullptr;

	SMTEncoder::endVisit(_function);
}

bool CHC::visit(IfStatement const& _if)
{
	solAssert(m_currentFunction, "");

	SMTEncoder::visit(_if);

	return false;
}

void CHC::endVisit(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind != FunctionCallKind::Unset, "");

	if (_funCall.annotation().kind == FunctionCallKind::FunctionCall)
	{
		FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);
		if (funType.kind() == FunctionType::Kind::Assert)
			visitAssert(_funCall);
	}

	SMTEncoder::endVisit(_funCall);

	createReturnedExpressions(_funCall);
}

void CHC::visitAssert(FunctionCall const&)
{
}

void CHC::reset()
{
	m_stateSorts.clear();
	m_stateVariables.clear();
	m_verificationTargets.clear();
	m_safeAssertions.clear();
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

smt::SortPointer CHC::constructorSort()
{
	solAssert(m_currentContract, "");
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	if (!m_currentContract->constructor())
		return make_shared<smt::FunctionSort>(vector<smt::SortPointer>{}, boolSort);
	return functionSort(*m_currentContract->constructor());
}

smt::SortPointer CHC::interfaceSort()
{
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	return make_shared<smt::FunctionSort>(
		m_stateSorts,
		boolSort
	);
}

smt::SortPointer CHC::functionSort(FunctionDefinition const& _function)
{
	auto boolSort = make_shared<smt::Sort>(smt::Kind::Bool);
	auto const& funType = dynamic_cast<FunctionType const&>(*_function.type());
	return make_shared<smt::FunctionSort>(
		smt::smtSort(funType.parameterTypes()),
		boolSort
	);
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
