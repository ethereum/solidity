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
#pragma once

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

// because of instruction
#include <libyul/backends/evm/EVMDialect.h>

#include <libsmtutil/SolverInterface.h>

#include <memory>

namespace solidity::smtutil
{
class SolverInterface;
class Expression;
struct Sort;
}

namespace solidity::yul
{

/**
 * A base class for SMT based optimization steps
 */
class Solver
{
protected:
	/// A helper function that encodes VariableDeclaration
	void encodeVariableDeclaration(VariableDeclaration const& _varDecl);

	Solver(
		std::set<YulString> const& _ssaVariables,
		Dialect const& _dialect
	);

	/// The encoding for a builtin. The type of encoding determines what we are
	/// solving for.
	virtual smtutil::Expression encodeEVMBuiltin(
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	) = 0;

	smtutil::Expression encodeExpression(
		Expression const& _expression
	);


	smtutil::Expression int2bv(smtutil::Expression _arg) const;
	smtutil::Expression bv2int(smtutil::Expression _arg) const;

	smtutil::Expression newVariable();
	virtual smtutil::Expression newRestrictedVariable(bigint _maxValue=(bigint(1) << 256) - 1);
	std::string uniqueName();

	virtual std::shared_ptr<smtutil::Sort> defaultSort() const;
	virtual smtutil::Expression booleanValue(smtutil::Expression _value) const;
	virtual smtutil::Expression constantValue(bigint _value) const;
	virtual smtutil::Expression literalValue(Literal const& _literal) const;
	virtual smtutil::Expression unsignedToSigned(smtutil::Expression _value);
	virtual smtutil::Expression signedToUnsigned(smtutil::Expression _value);
	virtual smtutil::Expression wrap(smtutil::Expression _value);

	std::set<YulString> const& m_ssaVariables;
	std::unique_ptr<smtutil::SolverInterface> m_solver;
	std::map<YulString, smtutil::Expression> m_variables;

	Dialect const& m_dialect;
private:

	size_t m_varCounter = 0;
};

}
