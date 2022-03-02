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
 * @file SemanticInformation.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Helper to provide semantic information about assembly items.
 */

#pragma once

#include <libevmasm/Instruction.h>

#include <optional>
#include <vector>

namespace solidity::evmasm
{

class AssemblyItem;

/**
 * Helper functions to provide context-independent information about assembly items.
 */
struct SemanticInformation
{
	/// Corresponds to the effect that a YUL-builtin has on a generic data location (storage, memory
	/// and other blockchain state).
	enum Effect
	{
		None,
		Read,
		Write
	};

	enum class Location { Storage, Memory };

	/**
	 * Represents a read or write operation from or to one of the data locations.
	 */
	struct Operation
	{
		Location location;
		Effect effect;
		/// Start of affected area as an index into the parameters.
		/// Unknown if not provided.
		std::optional<size_t> startParameter;
		/// Length of the affected area as an index into the parameters (if this is an opcode).
		/// Unknown if neither this nor lengthConstant is provided.
		std::optional<size_t> lengthParameter;
		/// Length as a constant.
		/// Unknown if neither this nor lengthArgument is provided.
		std::optional<size_t> lengthConstant;
	};

	/// @returns the sequence of read write operations performed by the instruction.
	/// Order matters.
	/// For external calls, there is just one unknown read and one unknown write operation,
	/// event though there might be multiple.
	static std::vector<Operation> readWriteOperations(Instruction _instruction);

	/// @returns true if the given items starts a new block for common subexpression analysis.
	/// @param _msizeImportant if false, consider an operation non-breaking if its only side-effect is that it modifies msize.
	static bool breaksCSEAnalysisBlock(AssemblyItem const& _item, bool _msizeImportant);
	/// @returns true if the item is a two-argument operation whose value does not depend on the
	/// order of its arguments.
	static bool isCommutativeOperation(AssemblyItem const& _item);
	static bool isDupInstruction(AssemblyItem const& _item);
	static bool isSwapInstruction(AssemblyItem const& _item);
	static bool isJumpInstruction(AssemblyItem const& _item);
	static bool altersControlFlow(AssemblyItem const& _item);
	static bool terminatesControlFlow(Instruction _instruction);
	static bool reverts(Instruction _instruction);
	/// @returns false if the value put on the stack by _item depends on anything else than
	/// the information in the current block header, memory, storage or stack.
	static bool isDeterministic(AssemblyItem const& _item);
	/// @returns true if the instruction can be moved or copied (together with its arguments)
	/// without altering the semantics. This means it cannot depend on storage or memory,
	/// cannot have any side-effects, but it can depend on a call-constant state of the blockchain.
	static bool movable(Instruction _instruction);
	/// If true, the expressions in this code can be moved or copied (together with their arguments)
	/// across control flow branches and instructions as long as these instructions' 'effects' do
	/// not influence the 'effects' of the aforementioned expressions.
	static bool movableApartFromEffects(Instruction _instruction);
	/// @returns true if the instruction can be removed without changing the semantics.
	/// This does not mean that it has to be deterministic or retrieve information from
	/// somewhere else than purely the values of its arguments.
	static bool canBeRemoved(Instruction _instruction);
	/// @returns true if the instruction can be removed without changing the semantics.
	/// This does not mean that it has to be deterministic or retrieve information from
	/// somewhere else than purely the values of its arguments.
	/// If true, the instruction is still allowed to influence the value returned by the
	/// msize instruction.
	static bool canBeRemovedIfNoMSize(Instruction _instruction);
	static Effect memory(Instruction _instruction);
	static Effect storage(Instruction _instruction);
	static Effect otherState(Instruction _instruction);
	static bool invalidInPureFunctions(Instruction _instruction);
	static bool invalidInViewFunctions(Instruction _instruction);
};

}
