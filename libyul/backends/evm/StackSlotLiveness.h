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
 * Stack slot liveness tracker.
 */

#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>

namespace solidity::yul
{

using StackSlotSet = std::set<StackSlot>;

struct StackLayoutLivenessInfo
{
	struct BlockInfo
	{
		/// Set of stack slots required on entry of this block.
		StackSlotSet entrySlots;
		/// Set of stack slots required at the exit of this block.
		StackSlotSet exitSlots;
	};
	std::map<CFG::BasicBlock const*, BlockInfo> blockInfos;
	/// For each operation the set of stack slots required on stack when entering the operation.
	std::map<CFG::Operation const*, StackSlotSet> operationEntrySlots;
};

class StackSlotLivenessTracker
{
public:
	static StackLayoutLivenessInfo run(CFG const& _cfg);

private:
	StackSlotLivenessTracker(StackLayoutLivenessInfo& _context);
	StackSlotSet propagateStackSlotSetThroughOperation(StackSlotSet _exitStack, CFG::Operation const& _operation);
	StackSlotSet propagateStackSlotSetThroughBlock(StackSlotSet _exitStack, CFG::BasicBlock const& _block);
	/// Main algorithm walking the graph from entry to exit and propagating back the stack layouts to the entries.
	/// Iteratively reruns itself along backwards jumps until the layout is stabilized.
	void processEntryPoint(CFG::BasicBlock const& _entry);

	/// @returns the best known exit layout of @a _block, if all dependencies are already @a _visited.
	/// If not, adds the dependencies to @a _dependencyList and @returns std::nullopt.
	std::optional<StackSlotSet> getExitLayoutOrStageDependencies(
		CFG::BasicBlock const& _block,
		std::set<CFG::BasicBlock const*> const& _visited,
		std::list<CFG::BasicBlock const*>& _dependencyList
	) const;

	/// @returns a pair of ``{jumpingBlock, targetBlock}`` for each backwards jump in the graph starting at @a _entry.
	std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> collectBackwardsJumps(CFG::BasicBlock const& _entry) const;

	StackLayoutLivenessInfo& m_livenessInfo;
};

}
