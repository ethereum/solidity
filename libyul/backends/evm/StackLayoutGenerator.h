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
 * Stack layout generator for Yul to EVM code generation.
 */

#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libyul/backends/evm/StackSlotLiveness.h>

#include <map>

namespace solidity::yul
{

struct StackLayout
{
	struct BlockInfo
	{
		/// Complete stack layout that is required for entering a block.
		Stack entryLayout;
		/// The resulting stack layout after executing the block.
		Stack exitLayout;
	};
	std::map<CFG::BasicBlock const*, BlockInfo> blockInfos;
	/// For each operation the complete stack layout that:
	/// - has the slots required for the operation at the stack top.
	/// - will have the operation result in a layout that makes it easy to achieve the next desired layout.
	std::map<CFG::Operation const*, Stack> operationEntryLayout;
};

class StackLayoutGenerator
{
public:
	struct StackTooDeep
	{
		/// Number of slots that need to be saved.
		size_t deficit = 0;
		/// Set of variables, eliminating which would decrease the stack deficit.
		std::vector<YulString> variableChoices;
	};

	static StackLayout run(CFG const& _cfg);
	/// @returns a map from function names to the stack too deep errors occurring in that function.
	/// Requires @a _cfg to be a control flow graph generated from disambiguated Yul.
	/// The empty string is mapped to the stack too deep errors of the main entry point.
	static std::map<YulString, std::vector<StackTooDeep>> reportStackTooDeep(CFG const& _cfg);
	/// @returns all stack too deep errors in the function named @a _functionName.
	/// Requires @a _cfg to be a control flow graph generated from disambiguated Yul.
	/// If @a _functionName is empty, the stack too deep errors of the main entry point are reported instead.
	static std::vector<StackTooDeep> reportStackTooDeep(CFG const& _cfg, YulString _functionName);

private:
	StackLayoutGenerator(StackLayout& _context, CFG::FunctionInfo const* _functionInfo, StackLayoutLivenessInfo const& _livenessInfo);

	/// @returns the optimal entry stack layout, s.t. @a _operation can be applied to it and
	/// the result can be transformed to @a _exitStack with minimal stack shuffling.
	/// Simultaneously stores the entry layout required for executing the operation in m_layout.
	Stack propagateStackThroughOperation(Stack _entryStack, StackSlotSet _exitSet, CFG::Operation const& _operation);

	/// @returns the desired stack layout at the entry of @a _block, assuming the layout after
	/// executing the block should be @a _exitStack.
	Stack propagateStackThroughBlock(Stack _entryStack, CFG::BasicBlock const& _block);

	/// Main algorithm walking the graph from entry to exit and propagating back the stack layouts to the entries.
	/// Iteratively reruns itself along backwards jumps until the layout is stabilized.
	void processEntryPoint(CFG::BasicBlock const& _entry, CFG::FunctionInfo const* _functionInfo = nullptr);

	/// After the main algorithms, layouts at conditional jumps are merely compatible, i.e. the exit layout of the
	/// jumping block is a superset of the entry layout of the target block. This function modifies the entry layouts
	/// of conditional jump targets, s.t. the entry layout of target blocks match the exit layout of the jumping block
	/// exactly, except that slots not required after the jump are marked as `JunkSlot`s.
	void stitchConditionalJumps(CFG::BasicBlock const& _block);

	/// Walks through the CFG and reports any stack too deep errors that would occur when generating code for it
	/// without countermeasures.
	std::vector<StackTooDeep> reportStackTooDeep(CFG::BasicBlock const& _entry) const;

	StackLayout& m_layout;
	CFG::FunctionInfo const* m_currentFunctionInfo = nullptr;
	StackLayoutLivenessInfo const& m_livenessInfo;
};

}
