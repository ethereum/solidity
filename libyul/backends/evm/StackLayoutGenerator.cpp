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

#include <libyul/backends/evm/StackLayoutGenerator.h>

#include <libyul/backends/evm/OptimizedEVMCodeTransform.h>
#include <libyul/backends/evm/StackHelpers.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/cxx20.h>
#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/all.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/take_last.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

StackLayoutGenerator::StackLayoutGenerator(StackLayout& _layout): m_layout(_layout)
{
}

namespace
{
struct PreviousSlot { size_t slot; };

template<typename Callable>
Stack createIdealLayout(Stack const& _post, vector<variant<PreviousSlot, StackSlot>> _layout, Callable _generateSlotOnTheFly)
{
	if (_layout.empty())
		return Stack{};

	struct ShuffleOperations
	{
		vector<variant<PreviousSlot, StackSlot>>& layout;
		Stack const& post;
		std::set<StackSlot> outputs;
		std::map<StackSlot, int> multiplicity;
		Callable generateSlotOnTheFly;
		ShuffleOperations(
			vector<variant<PreviousSlot, StackSlot>>& _layout,
			Stack const& _post,
			Callable _generateSlotOnTheFly
		): layout(_layout), post(_post), generateSlotOnTheFly(_generateSlotOnTheFly)
		{
			for (auto const& layoutSlot: layout)
				if (StackSlot const* slot = get_if<StackSlot>(&layoutSlot))
					outputs.insert(*slot);

			for (auto const& layoutSlot: layout)
				if (StackSlot const* slot = get_if<StackSlot>(&layoutSlot))
					--multiplicity[*slot];
			for (auto&& slot: post)
				if (outputs.count(slot) || generateSlotOnTheFly(slot))
					++multiplicity[slot];
		}
		bool isCompatible(size_t _source, size_t _target)
		{
			return
				_source < layout.size() &&
				_target < post.size() &&
				(
					std::holds_alternative<JunkSlot>(post.at(_target)) ||
					std::visit(util::GenericVisitor{
						[&](PreviousSlot const&) {
							return !outputs.count(post.at(_target)) && !generateSlotOnTheFly(post.at(_target));
						},
						[&](StackSlot const& _s) { return _s == post.at(_target); }
					}, layout.at(_source))
				);
		}
		bool sourceIsSame(size_t _lhs, size_t _rhs)
		{
			return std::visit(util::GenericVisitor{
				[&](PreviousSlot const&, PreviousSlot const&) { return true; },
				[&](StackSlot const& _lhs, StackSlot const& _rhs) { return _lhs == _rhs; },
				[&](auto const&, auto const&) { return false; }
			}, layout.at(_lhs), layout.at(_rhs));
		}
		int sourceMultiplicity(size_t _offset)
		{
			return std::visit(util::GenericVisitor{
				[&](PreviousSlot const&) { return 0; },
				[&](StackSlot const& _s) { return multiplicity.at(_s); }
			}, layout.at(_offset));
		}
		int targetMultiplicity(size_t _offset)
		{
			if (!outputs.count(post.at(_offset)) && !generateSlotOnTheFly(post.at(_offset)))
				return 0;
			return multiplicity.at(post.at(_offset));
		}
		bool targetIsArbitrary(size_t _offset)
		{
			return _offset < post.size() && std::holds_alternative<JunkSlot>(post.at(_offset));
		}
		void swap(size_t _i)
		{
			yulAssert(!holds_alternative<PreviousSlot>(layout.at(layout.size() - _i - 1)) || !holds_alternative<PreviousSlot>(layout.back()), "");
			std::swap(layout.at(layout.size() -  _i - 1), layout.back());
		}
		size_t sourceSize() { return layout.size(); }
		size_t targetSize() { return post.size(); }
		void pop() { layout.pop_back(); }
		void pushOrDupTarget(size_t _offset) { layout.push_back(post.at(_offset)); }
	};

	Shuffler<ShuffleOperations>::shuffle(_layout, _post, _generateSlotOnTheFly);

	// Now we can construct the ideal layout before the operation.
	// "layout" has the declared variables in the desired position and
	// for any PreviousSlot{x}, x yields the ideal place of the slot before the declaration.
	vector<optional<StackSlot>> idealLayout(_post.size(), nullopt);
	for (auto const& [slot, idealPosition]: ranges::zip_view(_post, _layout))
		if (PreviousSlot* previousSlot = std::get_if<PreviousSlot>(&idealPosition))
			idealLayout.at(previousSlot->slot) = slot;

	while (!idealLayout.empty() && !idealLayout.back())
		idealLayout.pop_back();

	return idealLayout | ranges::views::transform([](optional<StackSlot> s) {
		yulAssert(s, "");
		return *s;
	}) | ranges::to<Stack>;
}
}

Stack StackLayoutGenerator::propagateStackThroughOperation(Stack _exitStack, CFG::Operation const& _operation)
{
	Stack& stack = _exitStack;

	// This is a huge tradeoff between code size, gas cost and stack size.
	auto generateSlotOnTheFly = [&](StackSlot const&) {
		//return stack.size() > 12 && canBeFreelyGenerated(_slot);
		// return canBeFreelyGenerated(_slot);
		return false;
	};

	size_t previousLayoutSize = stack.size();
	for (auto const& slot: stack)
		if (util::findOffset(_operation.output, slot) || generateSlotOnTheFly(slot))
				--previousLayoutSize;

	auto layout = ranges::views::iota(0u, previousLayoutSize) |
		ranges::views::transform([](size_t _index) { return PreviousSlot{_index}; }) |
		ranges::to<vector<variant<PreviousSlot, StackSlot>>>;
	// The call produces a known sequence of values.
	layout += _operation.output;

	stack = createIdealLayout(stack, layout, generateSlotOnTheFly);

	if (auto const* assignment = get_if<CFG::Assignment>(&_operation.operation))
		for (auto& stackSlot: stack)
			if (auto const* varSlot = get_if<VariableSlot>(&stackSlot))
				yulAssert(!util::findOffset(assignment->variables, *varSlot), "");

	for (StackSlot const& input: _operation.input)
		stack.emplace_back(input);

	m_layout.operationEntryLayout[&_operation] = stack;

	// Remove anything from the stack top that can be freely generated or dupped from deeper on the stack.
	while (!stack.empty())
	{
		if (canBeFreelyGenerated(stack.back()))
			stack.pop_back();
		else if (auto offset = util::findOffset(stack | ranges::views::reverse | ranges::views::drop(1), stack.back()))
		{
			if (*offset + 2 < 16)
				stack.pop_back();
			else
				break;
		}
		else
			break;
	}

	// TODO: there may be a better criterion than overall stack size.
	if (stack.size() > 12)
		// Deduplicate and remove slots that can be freely generated.
		stack = compressStack(move(stack));
	return stack;
}

Stack StackLayoutGenerator::compressStack(Stack _stack)
{
	optional<size_t> firstDupOffset;
	do
	{
		if (firstDupOffset)
		{
			if (_stack.size() - *firstDupOffset - 1 > 1)
				std::swap(_stack.at(*firstDupOffset + 1), _stack.back());
			std::swap(_stack.at(*firstDupOffset), _stack.back());
			_stack.pop_back();
			firstDupOffset.reset();
		}
		for (auto&& [offset, slot]: _stack | ranges::views::enumerate)
			if (canBeFreelyGenerated(slot) || util::findOffset(_stack | ranges::views::take(offset), slot))
				firstDupOffset = offset;
	}
	while (firstDupOffset);
	return _stack;
}

Stack StackLayoutGenerator::propagateStackThroughBlock(Stack _exitStack, CFG::BasicBlock const& _block)
{
	Stack stack = std::move(_exitStack);
	for (auto& operation: _block.operations | ranges::views::reverse)
		stack = propagateStackThroughOperation(stack, operation);
	return stack;
}

void StackLayoutGenerator::processEntryPoint(CFG::BasicBlock const& _entry)
{
	std::list<CFG::BasicBlock const*> toVisit{&_entry};
	std::set<CFG::BasicBlock const*> visited;

	while (!toVisit.empty())
	{
		// TODO: calculate backwardsJumps only once.
		std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> backwardsJumps;
		while (!toVisit.empty())
		{
			CFG::BasicBlock const *block = *toVisit.begin();
			toVisit.pop_front();

			if (visited.count(block))
				continue;

			if (std::optional<Stack> exitLayout = std::visit(util::GenericVisitor{
				[&](CFG::BasicBlock::MainExit const&) -> std::optional<Stack>
				{
					visited.emplace(block);
					return Stack{};
				},
				[&](CFG::BasicBlock::Jump const& _jump) -> std::optional<Stack>
				{
					if (_jump.backwards)
					{
						visited.emplace(block);
						backwardsJumps.emplace_back(block, _jump.target);
						if (auto* info = util::valueOrNullptr(m_layout.blockInfos, _jump.target))
							return info->entryLayout;
						return Stack{};
					}
					if (visited.count(_jump.target))
					{
						visited.emplace(block);
						return m_layout.blockInfos.at(_jump.target).entryLayout;
					}
					toVisit.emplace_front(_jump.target);
					return nullopt;
				},
				[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump) -> std::optional<Stack>
				{
					bool zeroVisited = visited.count(_conditionalJump.zero);
					bool nonZeroVisited = visited.count(_conditionalJump.nonZero);
					if (zeroVisited && nonZeroVisited)
					{
						Stack stack = combineStack(
							m_layout.blockInfos.at(_conditionalJump.zero).entryLayout,
							m_layout.blockInfos.at(_conditionalJump.nonZero).entryLayout
						);
						stack.emplace_back(_conditionalJump.condition);
						visited.emplace(block);
						return stack;
					}
					if (!zeroVisited)
						toVisit.emplace_front(_conditionalJump.zero);
					if (!nonZeroVisited)
						toVisit.emplace_front(_conditionalJump.nonZero);
					return nullopt;
				},
				[&](CFG::BasicBlock::FunctionReturn const& _functionReturn) -> std::optional<Stack>
				{
					visited.emplace(block);
					yulAssert(_functionReturn.info, "");
					Stack stack = _functionReturn.info->returnVariables | ranges::views::transform([](auto const& _varSlot){
						return StackSlot{_varSlot};
					}) | ranges::to<Stack>;
					stack.emplace_back(FunctionReturnLabelSlot{});
					return stack;
				},
				[&](CFG::BasicBlock::Terminated const&) -> std::optional<Stack>
				{
					visited.emplace(block);
					return Stack{};
				},
			}, block->exit))
			{
				// We can skip the visit, if we have seen this precise exit layout already last time.
				// Note: if the entire graph is revisited in the backwards jump check below, doing
				//       this seems to break things; not sure why.
				// Note: since I don't quite understand why doing this can break things, I comment
				//       it out for now, since not aborting in those cases should always be safe.
				// if (auto* previousInfo = util::valueOrNullptr(m_layout.blockInfos, block))
				//	if (previousInfo->exitLayout == *exitLayout)
				//		continue;
				auto& info = m_layout.blockInfos[block];
				info.exitLayout = *exitLayout;
				info.entryLayout = propagateStackThroughBlock(info.exitLayout, *block);

				for (auto entry: block->entries)
					toVisit.emplace_back(entry);
			}
			else
				continue;
		}

		for (auto [block, target]: backwardsJumps)
			if (ranges::any_of(
				m_layout.blockInfos[target].entryLayout,
				[exitLayout = m_layout.blockInfos[block].exitLayout](StackSlot const& _slot) {
					return !util::findOffset(exitLayout, _slot);
				}
			))
			{
				// This block jumps backwards, but does not provide all slots required by the jump target on exit.
				// Therefore we need to visit the subgraph between ``target`` and ``block`` again.
				// In particular we can visit backwards starting from ``block`` and mark all entries to-be-visited-
				// again until we hit ``target``.
				toVisit.emplace_front(block);
				// Since we are likely to change the entry layout of ``target``, we also visit its entries again.
				for (CFG::BasicBlock const* entry: target->entries)
					visited.erase(entry);
				util::BreadthFirstSearch<CFG::BasicBlock const*>{{block}}.run(
					[&visited, target = target](CFG::BasicBlock const* _block, auto _addChild) {
						visited.erase(_block);
						if (_block == target)
							return;
						for (auto const* entry: _block->entries)
							_addChild(entry);
					}
				);
				// TODO: while the above is enough, the layout of ``target`` might change in the process.
				// While the shuffled layout for ``target`` will be compatible, it can be worthwhile propagating
				// it further up once more.
				// This would mean not stopping at _block == target above or even doing visited.clear() here, revisiting the entire graph.
				// This is a tradeoff between the runtime of this process and the optimality of the result.
				// Also note that while visiting the entire graph again *can* be helpful, it can also be detrimental.
				// Also note that for some reason using visited.clear() is incompatible with skipping the revisit
				// of already seen exit layouts above, I'm not sure yet why.
			}
	}

	stitchConditionalJumps(_entry);
	fixStackTooDeep(_entry);
}

Stack StackLayoutGenerator::combineStack(Stack const& _stack1, Stack const& _stack2)
{
	// TODO: there is probably a better way than brute-forcing. This has n! complexity or worse, so
	// we can't keep it like this.

	Stack commonPrefix;
	for (auto&& [slot1, slot2]: ranges::zip_view(_stack1, _stack2))
	{
		if (!(slot1 == slot2))
			break;
		commonPrefix.emplace_back(slot1);
	}

	Stack stack1Tail = _stack1 | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>;
	Stack stack2Tail = _stack2 | ranges::views::drop(commonPrefix.size()) | ranges::to<Stack>;

	if (stack1Tail.empty())
		return commonPrefix + compressStack(stack2Tail);
	if (stack2Tail.empty())
		return commonPrefix + compressStack(stack1Tail);

	Stack candidate;
	for (auto slot: stack1Tail)
		if (!util::findOffset(candidate, slot))
			candidate.emplace_back(slot);
	for (auto slot: stack2Tail)
		if (!util::findOffset(candidate, slot))
			candidate.emplace_back(slot);
	cxx20::erase_if(candidate, [](StackSlot const& slot) {
		return holds_alternative<LiteralSlot>(slot) || holds_alternative<FunctionCallReturnLabelSlot>(slot);
	});

	std::map<size_t, Stack> sortedCandidates;

	// TODO: surprisingly this works for rather comparably large candidate size, but we should probably
	// set up some limit, since this will quickly explode otherwise.
	// Ideally we would then have a better fallback mechanism - although returning any naive union of both stacks
	// like ``candidate`` itself may just be fine.
	//	if (candidate.size() > 8)
	//		return candidate;

	auto evaluate = [&](Stack const& _candidate) -> size_t {
		size_t numOps = 0;
		Stack testStack = _candidate;
		auto swap = [&](unsigned _swapDepth) { ++numOps; if (_swapDepth > 16) numOps += 1000; };
		auto dupOrPush = [&](StackSlot const& _slot)
		{
			if (canBeFreelyGenerated(_slot))
				return;
			auto depth = util::findOffset(ranges::concat_view(commonPrefix, testStack) | ranges::views::reverse, _slot);
			if (depth && *depth >= 16)
				numOps += 1000;
		};
		createStackLayout(testStack, stack1Tail, swap, dupOrPush, [&](){} );
		testStack = _candidate;
		createStackLayout(testStack, stack2Tail, swap, dupOrPush, [&](){});
		return numOps;
	};

	// See https://en.wikipedia.org/wiki/Heap's_algorithm
	size_t n = candidate.size();
	sortedCandidates.insert(std::make_pair(evaluate(candidate), candidate));
	std::vector<size_t> c(n, 0);
	size_t i = 1;
	while (i < n)
	{
		if (c[i] < i)
		{
			if (i & 1)
				std::swap(candidate.front(), candidate[i]);
			else
				std::swap(candidate[c[i]], candidate[i]);
			sortedCandidates.insert(std::make_pair(evaluate(candidate), candidate));
			++c[i];
			++i;
		}
		else
		{
			c[i] = 0;
			++i;
		}
	}

	return commonPrefix + sortedCandidates.begin()->second;
}

void StackLayoutGenerator::stitchConditionalJumps(CFG::BasicBlock const& _block)
{
	util::BreadthFirstSearch<CFG::BasicBlock const*> breadthFirstSearch{{&_block}};
	breadthFirstSearch.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		auto& info = m_layout.blockInfos.at(_block);
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				if (!_jump.backwards)
					_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				auto& zeroTargetInfo = m_layout.blockInfos.at(_conditionalJump.zero);
				auto& nonZeroTargetInfo = m_layout.blockInfos.at(_conditionalJump.nonZero);
				Stack exitLayout = info.exitLayout;

				// The last block must have produced the condition at the stack top.
				yulAssert(!exitLayout.empty(), "");
				yulAssert(exitLayout.back() == _conditionalJump.condition, "");
				// The condition is consumed by the jump.
				exitLayout.pop_back();

				auto fixJumpTargetEntry = [&](Stack const& _originalEntryLayout) -> Stack {
					Stack newEntryLayout = exitLayout;
					// Whatever the block being jumped to does not actually require, can be marked as junk.
					for (auto& slot: newEntryLayout)
						if (!util::findOffset(_originalEntryLayout, slot))
							slot = JunkSlot{};
					// Make sure everything the block being jumped to requires is actually present or can be generated.
					for (auto const& slot: _originalEntryLayout)
						yulAssert(canBeFreelyGenerated(slot) || util::findOffset(newEntryLayout, slot), "");
					return newEntryLayout;
				};
				zeroTargetInfo.entryLayout = fixJumpTargetEntry(zeroTargetInfo.entryLayout);
				nonZeroTargetInfo.entryLayout = fixJumpTargetEntry(nonZeroTargetInfo.entryLayout);
				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const&)	{},
			[&](CFG::BasicBlock::Terminated const&) { },
		}, _block->exit);
	});
}

void StackLayoutGenerator::fixStackTooDeep(CFG::BasicBlock const& _block)
{
	// This is just an initial proof of concept. Doing this in a clever way and in all cases will take some doing.
	// It might be enough to keep it at fixing inner-block issues and leave inter-block issues to the stack limit
	// evader.
	// TODO: make sure this really always terminates.
	util::BreadthFirstSearch<CFG::BasicBlock const*> breadthFirstSearch{{&_block}};
	breadthFirstSearch.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		Stack stack;
		stack = m_layout.blockInfos.at(_block).entryLayout;

		size_t cursor = 0;

		while (cursor < _block->operations.size())
		{
			Stack& operationEntry = m_layout.operationEntryLayout.at(&_block->operations.at(cursor));

			auto unreachable = OptimizedEVMCodeTransform::tryCreateStackLayout(stack, operationEntry, m_currentFunctionReturnVariables);
			if (unreachable.empty())
			{
				stack = operationEntry;
				if (cursor < _block->operations.size())
				{
					CFG::Operation const& operation = _block->operations.at(cursor);
					for (size_t i = 0; i < operation.input.size(); i++)
						stack.pop_back();
					stack += operation.output;
				}
				++cursor;
			}
			else
			{
				if (cursor == 0)
					break;
				CFG::Operation const& previousOperation = _block->operations.at(cursor - 1);
				if (auto const* assignment = get_if<CFG::Assignment>(&previousOperation.operation))
				{
					for (auto& slot: unreachable)
						if (VariableSlot const* varSlot = get_if<VariableSlot>(&slot))
							if (util::findOffset(assignment->variables, *varSlot))
							{
								// TODO: proper warning
								std::cout << "Unreachable slot after assignment." << std::endl;
								std::cout << "CANNOT FIX YET" << std::endl;
								return;
							}
				}
				Stack& previousEntry = m_layout.operationEntryLayout.at(&previousOperation);
				Stack newStack = ranges::concat_view(
					previousEntry | ranges::views::take(previousEntry.size() - previousOperation.input.size()),
					unreachable,
					previousEntry | ranges::views::take_last(previousOperation.input.size())
				) | ranges::to<Stack>;
				previousEntry = newStack;
				--cursor;
				if (cursor > 0)
				{
					CFG::Operation const& ancestorOperation = _block->operations.at(cursor - 1);
					Stack& ancestorEntry = m_layout.operationEntryLayout.at(&ancestorOperation);
					stack = ancestorEntry | ranges::views::take(ancestorEntry.size() - ancestorOperation.input.size()) | ranges::to<Stack>;
					stack += ancestorOperation.output;
				}
				else
					// Stop at block entry, hope for the best and continue downwards again.
					break;
			}
		}
		stack = m_layout.blockInfos.at(_block).exitLayout;

		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				auto unreachable = OptimizedEVMCodeTransform::tryCreateStackLayout(stack, m_layout.blockInfos.at(_jump.target).entryLayout, m_currentFunctionReturnVariables);
				if (!unreachable.empty())
					// TODO: proper warning
					std::cout << "UNREACHABLE SLOTS AT JUMP: " << stackToString(unreachable) << std::endl
						<< "CANNOT FIX YET" << std::endl;

				if (!_jump.backwards)
					_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				auto unreachable = OptimizedEVMCodeTransform::tryCreateStackLayout(stack, m_layout.blockInfos.at(_conditionalJump.zero).entryLayout, m_currentFunctionReturnVariables);
				if (!unreachable.empty())
					// TODO: proper warning
					std::cout
						<< "UNREACHABLE SLOTS AT CONDITIONAL JUMP: " << stackToString(unreachable) << std::endl
						<< "CANNOT FIX YET" << std::endl;
				unreachable = OptimizedEVMCodeTransform::tryCreateStackLayout(stack, m_layout.blockInfos.at(_conditionalJump.nonZero).entryLayout, m_currentFunctionReturnVariables);
				if (!unreachable.empty())
					// TODO: proper warning
					std::cout
						<< "UNREACHABLE SLOTS AT CONDITIONAL JUMP: " << stackToString(unreachable) << std::endl
						<< "CANNOT FIX YET" << std::endl;

				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const&) {},
			[&](CFG::BasicBlock::Terminated const&) { },
		}, _block->exit);

	});
}

StackLayout StackLayoutGenerator::run(CFG const& _dfg)
{
	StackLayout stackLayout;
	StackLayoutGenerator stackLayoutGenerator{stackLayout};

	stackLayoutGenerator.processEntryPoint(*_dfg.entry);
	for (auto& functionInfo: _dfg.functionInfo | ranges::views::values)
	{
		stackLayoutGenerator.m_currentFunctionReturnVariables = functionInfo.returnVariables;
		stackLayoutGenerator.processEntryPoint(*functionInfo.entry);
	}

	return stackLayout;
}
