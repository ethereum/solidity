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
 * @file SemanticInformation.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Helper to provide semantic information about assembly items.
 */

#pragma once

#include <libevmasm/Instruction.h>

namespace dev
{
namespace eth
{

class AssemblyItem;

/**
 * Helper functions to provide context-independent information about assembly items.
 */
struct SemanticInformation
{
	/// @returns true if the given items starts a new block for common subexpression analysis.
	static bool breaksCSEAnalysisBlock(AssemblyItem const& _item);
	/// @returns true if the item is a two-argument operation whose value does not depend on the
	/// order of its arguments.
	static bool isCommutativeOperation(AssemblyItem const& _item);
	static bool isDupInstruction(AssemblyItem const& _item);
	static bool isSwapInstruction(AssemblyItem const& _item);
	static bool isJumpInstruction(AssemblyItem const& _item);
	static bool altersControlFlow(AssemblyItem const& _item);
	/// @returns false if the value put on the stack by _item depends on anything else than
	/// the information in the current block header, memory, storage or stack.
	static bool isDeterministic(AssemblyItem const& _item);
	/// @returns true if the given instruction modifies memory.
	static bool invalidatesMemory(solidity::Instruction _instruction);
	/// @returns true if the given instruction modifies storage (even indirectly).
	static bool invalidatesStorage(solidity::Instruction _instruction);
};

}
}
