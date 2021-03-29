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
/**
 * Optimizer step that removes `mstore(x, y)` if the memory location `[x, x + 32)` is never read.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Solver.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>

namespace solidity::yul
{

/**
 * Optimization step that removes `mstore(x, y)` if the memory location `[x, x + 32)` is never used.
 * The fact that the location `[x, x + 32)` is determined with the help of a solver. Currently, we
 * use a SMT solver.
 *
 * The step works by encoding all memory reads in the code and trying to prove that the location
 * `[x, x + 32)` is never read.
 *
 * Prerequisite: Disambiguator
 *
 * Works best if the code is in SSA form, and if each variable `x` is only used once as the first
 * parameter in `mstore`.
 */
class MemoryStoreRemover: public ASTWalker, Solver
{
public:
	static constexpr char const* name{"MemoryStoreRemover"};
	static void run(OptimiserStepContext& _context, Block& _ast);

private:
	using ASTWalker::operator();
	void operator()(FunctionCall const& _funtionCall) override;
	void operator()(VariableDeclaration const& _variableDeclaration) override;
	MemoryStoreRemover(
		std::set<YulString> const& _ssaVariables,
		Dialect const& _dialect
	):
		Solver(_ssaVariables, _dialect),
		m_ssaVariables(_ssaVariables)
	{}

	smtutil::Expression encodeEVMBuiltin(
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	) override;

	/// Given a function call `_functionCall`, and a variable `_memoryLocation`, representing a
	/// memory location. Suppose that the `_functionCall` modifies the memory locations `[r, s)`.
	/// This function would add the constraint: `r <= _memoryLocation < s` to the solver.
	///
	/// Only encodes `_functionCall` that calls a builtin.
	void encodeMemoryRead(
		FunctionCall const& _functionCall,
		smtutil::Expression _memoryLocation
	);

	/// The set of all SSA variables
	std::set<YulString> const& m_ssaVariables;
	/// The set of all variables, that are used as key in `mstore(key, value)`. Note that, they also
	/// have to be a SSA variable.
	std::set<YulString> m_memoryKeys;
	/// A vector of builtin function calls that reads from memory.
	std::vector<FunctionCall const*> m_memoryReads;
};

}
