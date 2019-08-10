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

	return true;
}

void CHC::endVisit(ContractDefinition const& _contract)
{
	if (!shouldVisit(_contract))
		return;

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
