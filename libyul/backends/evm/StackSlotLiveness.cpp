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
 * Stack slot liveness tracker. It performs a backward pass over the CFG collecting
 * liveness information of stack slots in the entry and exit of basic blocks.
 */

#include <libyul/backends/evm/StackSlotLiveness.h>
#include <libyul/backends/evm/StackHelpers.h>

#include <libsolutil/Algorithms.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::yul;

StackLayoutLivenessInfo StackSlotLivenessTracker::run(CFG const& _cfg)
{
	StackLayoutLivenessInfo livenessInfo;
	StackSlotLivenessTracker{livenessInfo}.processEntryPoint(*_cfg.entry);

	for (auto& functionInfo: _cfg.functionInfo | ranges::views::values)
		StackSlotLivenessTracker{livenessInfo}.processEntryPoint(*functionInfo.entry);

	return livenessInfo;
}

StackSlotLivenessTracker::StackSlotLivenessTracker(StackLayoutLivenessInfo& _livenessInfo):
	m_livenessInfo(_livenessInfo)
{
}

StackSlotSet StackSlotLivenessTracker::propagateStackSlotSetThroughOperation(StackSlotSet _exitStack, CFG::Operation const& _operation)
{
	StackSlotSet stack = std::move(_exitStack);

	// We do not track of livenesss of constants. They can always be produce when we need them.
	for (auto it = stack.begin(); it != stack.end();)
	{
		if (canBeFreelyGenerated(*it))
			it = stack.erase(it);
		else
			++it;
	}

	// We do not need to retain variable slots that are reassigned here.
	if (auto const* assignment = std::get_if<CFG::Assignment>(&_operation.operation))
		for (auto&& var: assignment->variables)
			stack.erase(var);

	// We do not retain operation outputs.
	for (auto&& var: _operation.output)
		stack.erase(var);

	// Since stack+_operation.output can be easily shuffled to _exitLayout, the desired layout before the operation
	// is stack+_operation.input;
	stack += _operation.input;

	// Store the exact desired operation entry layout. The stored layout will be recreated by the code transform
	// before executing the operation. However, this recreation can produce slots that can be freely generated or
	// are duplicated, i.e. we can compress the stack afterwards without causing problems for code generation later.
	m_livenessInfo.operationEntrySlots[&_operation] = stack;

	return stack;
}

StackSlotSet StackSlotLivenessTracker::propagateStackSlotSetThroughBlock(StackSlotSet _exitStack, CFG::BasicBlock const& _block)
{
	StackSlotSet stack = _exitStack;
	for (auto&& operation: _block.operations | ranges::views::reverse)
	{
		StackSlotSet newStack = propagateStackSlotSetThroughOperation(stack, operation);
		stack = std::move(newStack);
	}

	return stack;
}

void StackSlotLivenessTracker::processEntryPoint(CFG::BasicBlock const& _entry)
{
	std::list<CFG::BasicBlock const*> toVisit{&_entry};
	std::set<CFG::BasicBlock const*> visited;

	// TODO: check whether visiting only a subset of these in the outer iteration below is enough.
	std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> backwardsJumps = collectBackwardsJumps(_entry);

	while (!toVisit.empty())
	{
		// First calculate stack layouts without walking backwards jumps, i.e. assuming the current preliminary
		// entry layout of the backwards jump target as the initial exit layout of the backwards-jumping block.
		while (!toVisit.empty())
		{
			CFG::BasicBlock const *block = *toVisit.begin();
			toVisit.pop_front();

			if (visited.count(block))
				continue;

			if (std::optional<StackSlotSet> exitLayout = getExitLayoutOrStageDependencies(*block, visited, toVisit))
			{
				visited.emplace(block);
				auto& info = m_livenessInfo.blockInfos[block];
				info.exitSlots = *exitLayout;
				info.entrySlots = propagateStackSlotSetThroughBlock(info.exitSlots, *block);

				for (auto entry: block->entries)
					toVisit.emplace_back(entry);
			}
			else
				continue;
		}

		// Determine which backwards jumps still require fixing and stage revisits of appropriate nodes.
		for (auto [jumpingBlock, target]: backwardsJumps)
			// This block jumps backwards, but does not provide all slots required by the jump target on exit.
			// Therefore we need to visit the subgraph between ``target`` and ``jumpingBlock`` again.
			if (ranges::any_of(
				m_livenessInfo.blockInfos[target].entrySlots,
				[exitLayout = m_livenessInfo.blockInfos[jumpingBlock].exitSlots](StackSlot const& _slot) {
					return !util::contains(exitLayout, _slot);
				}
			))
			{
				// In particular we can visit backwards starting from ``jumpingBlock`` and mark all entries to-be-visited-
				// again until we hit ``target``.
				toVisit.emplace_front(jumpingBlock);
				// Since we are likely to permute the entry layout of ``target``, we also visit its entries again.
				// This is not required for correctness, since the set of stack slots will match, but it may move some
				// required stack shuffling from the loop condition to outside the loop.
				for (CFG::BasicBlock const* entry: target->entries)
					visited.erase(entry);
				util::BreadthFirstSearch<CFG::BasicBlock const*>{{jumpingBlock}}.run(
					[&visited, target = target](CFG::BasicBlock const* _block, auto _addChild) {
						visited.erase(_block);
						if (_block == target)
							return;
						for (auto const* entry: _block->entries)
							_addChild(entry);
					}
				);
				// While the shuffled layout for ``target`` will be compatible, it can be worthwhile propagating
				// it further up once more.
				// This would mean not stopping at _block == target above, resp. even doing visited.clear() here, revisiting the entire graph.
				// This is a tradeoff between the runtime of this process and the optimality of the result.
				// Also note that while visiting the entire graph again *can* be helpful, it can also be detrimental.
			}
	}
}

std::optional<StackSlotSet> StackSlotLivenessTracker::getExitLayoutOrStageDependencies(
	CFG::BasicBlock const& _block,
	std::set<CFG::BasicBlock const*> const& _visited,
	std::list<CFG::BasicBlock const*>& _toVisit
) const
{
	return std::visit(util::GenericVisitor{
		[&](CFG::BasicBlock::MainExit const&) -> std::optional<StackSlotSet>
		{
			// On the exit of the outermost block the stack can be empty.
			return StackSlotSet{};
		},
		[&](CFG::BasicBlock::Jump const& _jump) -> std::optional<StackSlotSet>
		{
			if (_jump.backwards)
			{
				// Choose the best currently known entry layout of the jump target as initial exit.
				// Note that this may not yet be the final layout.
				if (auto* info = util::valueOrNullptr(m_livenessInfo.blockInfos, _jump.target))
					return info->entrySlots;
				return StackSlotSet{};
			}
			// If the current iteration has already visited the jump target, start from its entry layout.
			if (_visited.count(_jump.target))
				return m_livenessInfo.blockInfos.at(_jump.target).entrySlots;
			// Otherwise stage the jump target for visit and defer the current block.
			_toVisit.emplace_front(_jump.target);
			return std::nullopt;
		},
		[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump) -> std::optional<StackSlotSet>
		{
			bool zeroVisited = _visited.count(_conditionalJump.zero);
			bool nonZeroVisited = _visited.count(_conditionalJump.nonZero);
			if (zeroVisited && nonZeroVisited)
			{
				// If the current iteration has already visited both jump targets, start from its entry layout.
				StackSlotSet stack = m_livenessInfo.blockInfos.at(_conditionalJump.zero).entrySlots +
					m_livenessInfo.blockInfos.at(_conditionalJump.nonZero).entrySlots;
				// Additionally, the jump condition has to be at the stack top at exit.
				stack.emplace(_conditionalJump.condition);
				return stack;
			}
			// If one of the jump targets has not been visited, stage it for visit and defer the current block.
			if (!zeroVisited)
				_toVisit.emplace_front(_conditionalJump.zero);
			if (!nonZeroVisited)
				_toVisit.emplace_front(_conditionalJump.nonZero);
			return std::nullopt;
		},
		[&](CFG::BasicBlock::FunctionReturn const& _functionReturn) -> std::optional<StackSlotSet>
		{
			// A function return needs the return variables and the function return label slot on stack.
			yulAssert(_functionReturn.info, "");
			StackSlotSet stack = _functionReturn.info->returnVariables | ranges::views::transform([](auto const& _varSlot){
				return StackSlot{_varSlot};
			}) | ranges::to<StackSlotSet>;
			stack.emplace(FunctionReturnLabelSlot{_functionReturn.info->function});
			return stack;
		},
		[&](CFG::BasicBlock::Terminated const&) -> std::optional<StackSlotSet>
		{
			// A terminating block can have an empty stack on exit.
			return StackSlotSet{};
		},
	}, _block.exit);
}

std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> StackSlotLivenessTracker::collectBackwardsJumps(CFG::BasicBlock const& _entry) const
{
	std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> backwardsJumps;
	util::BreadthFirstSearch<CFG::BasicBlock const*>{{&_entry}}.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				if (_jump.backwards)
					backwardsJumps.emplace_back(_block, _jump.target);
				_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const&) {},
			[&](CFG::BasicBlock::Terminated const&) {},
		}, _block->exit);
	});
	return backwardsJumps;
}
