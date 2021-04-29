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

#include <libyul/optimiser/Solver.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>

#include <libyul/Dialect.h>
#include <libyul/AST.h>

// because of instruction
#include <libyul/backends/evm/EVMDialect.h>

namespace solidity::yul
{

class CatchMemoryViolators: public ASTWalker, Solver
{
public:
	static constexpr char const* name{"CatchMemoryViolators"};
	static void run(OptimiserStepContext& _context, Block& _ast);
private:
	using ASTWalker::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(VariableDeclaration const& _variableDeclaration) override;

	CatchMemoryViolators(
		std::set<YulString> const& _ssaVariables,
		Dialect const& _dialect
	):
		Solver(_ssaVariables, _dialect)
	{}

	smtutil::Expression encodeEVMBuiltin(
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	) override;

	smtutil::Expression encodeExpression(Expression const& _expression) override;

	// Returns a variable with bounds `freeMemoryPointer <= variable < 2**32`
	smtutil::Expression safeMemoryRestrictedVariable();
};

}
