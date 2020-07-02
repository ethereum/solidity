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
/**
 * Optimisation stage that moves Yul variables from stack to memory.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libsolutil/Common.h>

namespace solidity::yul
{

/**
 * Optimisation stage that moves Yul variables from stack to memory.
 * It takes a map from functions names and variable names to memory offsets.
 * It then transforms the AST as follows:
 *
 * Single variable declarations are replaced by mstore's as follows:
 *   If a is in the map, replace
 *     let a
 *   by
 *     mstore(<memory offset for a>, 0)
 *   respectively, replace
 *     let a := expr
 *   by
 *     mstore(<memory offset for a>, expr)
 *
 * In a multi-variable declaration, variables to be moved are replaced by fresh variables and then moved to memory:
 *   If b and d are in the map, replace
 *     let a, b, c, d := f()
 *   by
 *     let _1, _2, _3, _4 := f()
 *     mstore(<memory offset for d>, _4)
 *     mstore(<memory offset for b>, _2)
 *     let c := _3
 *     let a := _1
 *
 * Assignments to single variables are replaced by mstore's:
 *   If a is in the map, replace
 *     a := expr
 *   by
 *     mstore(<memory offset for a>, expr)
 *
 * Assignments to multiple variables are split up similarly to multi-variable declarations:
 *   If b and d are in the map, replace
 *     a, b, c, d := f()
 *   by
 *     let _1, _2, _3, _4 := f()
 *     mstore(<memory offset for d>, _4)
 *     mstore(<memory offset for b>, _2)
 *     c := _3
 *     a := _1
 *
 * Replace all references to a variable ``a`` in the map by ``mload(<memory offset for a>)``.
 *
 * If a visited function has arguments or return parameters that are contained in the map,
 * the entire function is skipped (no local variables in the function will be moved at all).
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class StackToMemoryMover: ASTModifier
{
public:
	StackToMemoryMover(
		OptimiserStepContext& _context,
		u256 _reservedMemory,
		std::map<YulString, std::map<YulString, uint64_t>> const& _memoryOffsets
	);

	using ASTModifier::operator();

	void operator()(FunctionDefinition& _functionDefinition) override;
	void operator()(Block& _block) override;
	void visit(Expression& _expression) override;
private:
	YulString memoryOffset(YulString _variable);
	u256 m_reservedMemory;
	std::map<YulString, std::map<YulString, uint64_t>> const& m_memorySlots;
	NameDispenser& m_nameDispenser;
	std::map<YulString, uint64_t> const* m_currentFunctionMemorySlots = nullptr;
};

}