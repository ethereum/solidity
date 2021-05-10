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

#include <libyul/optimiser/SMTSolver.h>
#include <libyul/optimiser/OptimizerUtilities.h>

#include <libyul/AST.h>
#include <libyul/Utilities.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::smtutil;
using namespace solidity;
using namespace std;

SMTSolver::SMTSolver(set<YulString> const& _ssaVariables, Dialect const& _dialect):
	m_ssaVariables(_ssaVariables),
	m_solver(make_unique<smtutil::SMTPortfolio>()),
	m_dialect(_dialect)
{ }

smtutil::Expression SMTSolver::encodeExpression(Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto instruction = toEVMInstruction(m_dialect, _functionCall.functionName.name))
				return encodeEVMBuiltin(*instruction, _functionCall.arguments);

			return newRestrictedVariable();
		},
		[&](Identifier const& _identifier)
		{
			if (
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				return m_variables.at(_identifier.name);
			else
				return newRestrictedVariable();
		},
		[&](Literal const& _literal)
		{
			return literalValue(_literal);
		}
	}, _expression);
}

smtutil::Expression SMTSolver::int2bv(smtutil::Expression _arg)
{
	return smtutil::Expression::int2bv(std::move(_arg), 256);
}

smtutil::Expression SMTSolver::bv2int(smtutil::Expression _arg)
{
	return smtutil::Expression::bv2int(std::move(_arg));
}

smtutil::Expression SMTSolver::newVariable()
{
	return m_solver->newVariable(uniqueName(), defaultSort());
}

smtutil::Expression SMTSolver::newRestrictedVariable(bigint _maxValue)
{
	smtutil::Expression var = newVariable();
	m_solver->addAssertion(0 <= var && var <= smtutil::Expression(_maxValue));
	return var;
}

string SMTSolver::uniqueName()
{
	return "expr_" + to_string(m_varCounter++);
}

shared_ptr<Sort> SMTSolver::defaultSort() const
{
	return SortProvider::intSort();
}

smtutil::Expression SMTSolver::booleanValue(smtutil::Expression _value)
{
	return smtutil::Expression::ite(_value, constantValue(1), constantValue(0));
}

smtutil::Expression SMTSolver::constantValue(bigint _value)
{
	return _value;
}

smtutil::Expression SMTSolver::literalValue(Literal const& _literal)
{
	return smtutil::Expression(valueOfLiteral(_literal));
}

smtutil::Expression SMTSolver::twosComplementToSigned(smtutil::Expression _value)
{
	return smtutil::Expression::ite(
		_value < smtutil::Expression(bigint(1) << 255),
		_value,
		_value - smtutil::Expression(bigint(1) << 256)
	);
}

smtutil::Expression SMTSolver::signedToTwosComplement(smtutil::Expression _value)
{
	return smtutil::Expression::ite(
		_value >= 0,
		_value,
		_value + smtutil::Expression(bigint(1) << 256)
	);
}

smtutil::Expression SMTSolver::wrap(smtutil::Expression _value)
{
	smtutil::Expression rest = newRestrictedVariable();
	smtutil::Expression multiplier = newVariable();
	m_solver->addAssertion(_value == multiplier * smtutil::Expression(bigint(1) << 256) + rest);
	return rest;
}

void SMTSolver::encodeVariableDeclaration(VariableDeclaration const& _varDecl)
{
	if (
		_varDecl.variables.size() == 1 &&
		_varDecl.value &&
		m_ssaVariables.count(_varDecl.variables.front().name)
	)
	{
		YulString const& variableName = _varDecl.variables.front().name;
		bool const inserted = m_variables.insert({
			variableName,
			m_solver->newVariable("yul_" + variableName.str(), defaultSort())
		}).second;
		yulAssert(inserted, "");
		m_solver->addAssertion(
			m_variables.at(variableName) == encodeExpression(*_varDecl.value)
		);
	}
}
