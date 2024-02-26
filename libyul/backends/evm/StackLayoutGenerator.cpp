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

#include <libyul/backends/evm/StackHelpers.h>

#include <libevmasm/GasMeter.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/cxx20.h>
#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/find.hpp>
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

StackLayout StackLayoutGenerator::run(CFG const& _cfg)
{
	StackLayout stackLayout;
	StackLayoutGenerator{stackLayout, nullptr}.processEntryPoint(*_cfg.entry);

	for (auto& functionInfo: _cfg.functionInfo | ranges::views::values)
		StackLayoutGenerator{stackLayout, &functionInfo}.processEntryPoint(*functionInfo.entry, &functionInfo);

	return stackLayout;
}

std::map<YulString, std::vector<StackLayoutGenerator::StackTooDeep>> StackLayoutGenerator::reportStackTooDeep(CFG const& _cfg)
{
	std::map<YulString, std::vector<StackLayoutGenerator::StackTooDeep>> stackTooDeepErrors;
	stackTooDeepErrors[YulString{}] = reportStackTooDeep(_cfg, YulString{});
	for (auto const& function: _cfg.functions)
		if (auto errors = reportStackTooDeep(_cfg, function->name); !errors.empty())
			stackTooDeepErrors[function->name] = std::move(errors);
	return stackTooDeepErrors;
}

std::vector<StackLayoutGenerator::StackTooDeep> StackLayoutGenerator::reportStackTooDeep(CFG const& _cfg, YulString _functionName)
{
	StackLayout stackLayout;
	CFG::FunctionInfo const* functionInfo = nullptr;
	if (!_functionName.empty())
	{
		functionInfo = &ranges::find(
			_cfg.functionInfo,
			_functionName,
			util::mapTuple([](auto&&, auto&& info) { return info.function.name; })
		)->second;
		yulAssert(functionInfo, "Function not found.");
	}

	StackLayoutGenerator generator{stackLayout, functionInfo};
	CFG::BasicBlock const* entry = functionInfo ? functionInfo->entry : _cfg.entry;
	generator.processEntryPoint(*entry);
	return generator.reportStackTooDeep(*entry);
}

StackLayoutGenerator::StackLayoutGenerator(StackLayout& _layout, CFG::FunctionInfo const* _functionInfo):
	m_layout(_layout),
	m_currentFunctionInfo(_functionInfo)
{
}

namespace
{
/// @returns all stack too deep errors that would occur when shuffling @a _source to @a _target.
std::vector<StackLayoutGenerator::StackTooDeep> findStackTooDeep(Stack const& _source, Stack const& _target)
{
	Stack currentStack = _source;
	std::vector<StackLayoutGenerator::StackTooDeep> stackTooDeepErrors;
	auto getVariableChoices = [](auto&& _range) {
		std::vector<YulString> result;
		for (auto const& slot: _range)
			if (auto const* variableSlot = std::get_if<VariableSlot>(&slot))
				if (!util::contains(result, variableSlot->variable.get().name))
					result.push_back(variableSlot->variable.get().name);
		return result;
	};
	::createStackLayout(
		currentStack,
		_target,
		[&](unsigned _i)
		{
			if (_i > 16)
				stackTooDeepErrors.emplace_back(StackLayoutGenerator::StackTooDeep{
					_i - 16,
					getVariableChoices(currentStack | ranges::views::take_last(_i + 1))
				});
		},
		[&](StackSlot const& _slot)
		{
			if (canBeFreelyGenerated(_slot))
				return;
			if (
				auto depth = util::findOffset(currentStack | ranges::views::reverse, _slot);
				depth && *depth >= 16
			)
				stackTooDeepErrors.emplace_back(StackLayoutGenerator::StackTooDeep{
					*depth - 15,
					getVariableChoices(currentStack | ranges::views::take_last(*depth + 1))
				});
		},
		[&]() {}
	);
	return stackTooDeepErrors;
}

/// @returns the ideal stack to have before executing an operation that outputs @a _operationOutput, s.t.
/// shuffling to @a _post is cheap (excluding the input of the operation itself).
/// If @a _generateSlotOnTheFly returns true for a slot, this slot should not occur in the ideal stack, but
/// rather be generated on the fly during shuffling.
template<typename Callable>
Stack createIdealLayout(Stack const& _operationOutput, Stack const& _post, Callable _generateSlotOnTheFly)
{
	struct PreviousSlot { size_t slot; };

	// Determine the number of slots that have to be on stack before executing the operation (excluding
	// the inputs of the operation itself).
	// That is slots that should not be generated on the fly and are not outputs of the operation.
	size_t preOperationLayoutSize = _post.size();
	for (auto const& slot: _post)
		if (util::contains(_operationOutput, slot) || _generateSlotOnTheFly(slot))
			--preOperationLayoutSize;

	// The symbolic layout directly after the operation has the form
	// PreviousSlot{0}, ..., PreviousSlot{n}, [output<0>], ..., [output<m>]
	auto layout = ranges::views::iota(0u, preOperationLayoutSize) |
		ranges::views::transform([](size_t _index) { return PreviousSlot{_index}; }) |
		ranges::to<std::vector<std::variant<PreviousSlot, StackSlot>>>;
	layout += _operationOutput;

	// Shortcut for trivial case.
	if (layout.empty())
		return Stack{};

	// Next we will shuffle the layout to the post stack using ShuffleOperations
	// that are aware of PreviousSlot's.
	struct ShuffleOperations
	{
		std::vector<std::variant<PreviousSlot, StackSlot>>& layout;
		Stack const& post;
		std::set<StackSlot> outputs;
		Multiplicity multiplicity;
		Callable generateSlotOnTheFly;
		ShuffleOperations(
			std::vector<std::variant<PreviousSlot, StackSlot>>& _layout,
			Stack const& _post,
			Callable _generateSlotOnTheFly
		): layout(_layout), post(_post), generateSlotOnTheFly(_generateSlotOnTheFly)
		{
			for (auto const& layoutSlot: layout)
				if (StackSlot const* slot = std::get_if<StackSlot>(&layoutSlot))
					outputs.insert(*slot);

			for (auto const& layoutSlot: layout)
				if (StackSlot const* slot = std::get_if<StackSlot>(&layoutSlot))
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
			yulAssert(!std::holds_alternative<PreviousSlot>(layout.at(layout.size() - _i - 1)) || !std::holds_alternative<PreviousSlot>(layout.back()), "");
			std::swap(layout.at(layout.size() -  _i - 1), layout.back());
		}
		size_t sourceSize() { return layout.size(); }
		size_t targetSize() { return post.size(); }
		void pop() { layout.pop_back(); }
		void pushOrDupTarget(size_t _offset) { layout.push_back(post.at(_offset)); }
	};
	Shuffler<ShuffleOperations>::shuffle(layout, _post, _generateSlotOnTheFly);

	// Now we can construct the ideal layout before the operation.
	// "layout" has shuffled the PreviousSlot{x} to new places using minimal operations to move the operation
	// output in place. The resulting permutation of the PreviousSlot yields the ideal positions of slots
	// before the operation, i.e. if PreviousSlot{2} is at a position at which _post contains VariableSlot{"tmp"},
	// then we want the variable tmp in the slot at offset 2 in the layout before the operation.
	std::vector<std::optional<StackSlot>> idealLayout(_post.size(), std::nullopt);
	for (auto&& [slot, idealPosition]: ranges::zip_view(_post, layout))
		if (PreviousSlot* previousSlot = std::get_if<PreviousSlot>(&idealPosition))
			idealLayout.at(previousSlot->slot) = slot;

	// The tail of layout must have contained the operation outputs and will not have been assigned slots in the last loop.
	while (!idealLayout.empty() && !idealLayout.back())
		idealLayout.pop_back();

	yulAssert(idealLayout.size() == preOperationLayoutSize, "");

	return idealLayout | ranges::views::transform([](std::optional<StackSlot> s) {
		yulAssert(s, "");
		return *s;
	}) | ranges::to<Stack>;
}
}

StackSlotSet StackLayoutGenerator::propagateStackSlotSetThroughOperation(StackSlotSet _exitStack, CFG::Operation const& _operation)
{
	StackSlotSet stack = std::move(_exitStack);

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
	m_layout.operationEntrySlots[&_operation] = stack;

	return stack;
}

StackSlotSet StackLayoutGenerator::propagateStackSlotSetThroughBlock(StackSlotSet _exitStack, CFG::BasicBlock const& _block)
{
	StackSlotSet stack = _exitStack;
	for (auto&& operation: _block.operations | ranges::views::reverse)
	{
		StackSlotSet newStack = propagateStackSlotSetThroughOperation(stack, operation);
		stack = std::move(newStack);
	}

	return stack;
}


Stack StackLayoutGenerator::propagateStackThroughOperation(Stack _entryStack, StackSlotSet _exitSet, CFG::Operation const& _operation)
{
	Stack stack;

	for (auto slot: _entryStack)
		if (_exitSet.count(slot))
			stack.emplace_back(slot);

	m_layout.operationEntryLayout[&_operation] = stack + _operation.input;

	for (auto slot: _operation.output)
		if (_exitSet.count(slot))
			stack.emplace_back(slot);
	return stack;
}

Stack StackLayoutGenerator::propagateStackThroughBlock(Stack _entryStack, CFG::BasicBlock const& _block)
{
	Stack stack = _entryStack;
	for (auto&& [idx, operation]: _block.operations | ranges::view::enumerate)
	{
		StackSlotSet exitSet;
		if (idx < _block.operations.size() - 1) {
			exitSet = m_layout.operationEntrySlots.at(&_block.operations[idx + 1]);
		} else {
			exitSet = m_layout.blockInfos[&_block].exitSlots;
		}
		Stack newStack = propagateStackThroughOperation(stack, exitSet, operation);
		stack = std::move(newStack);
	}
	StackSlotSet exitSet = m_layout.blockInfos[&_block].exitSlots;

	Stack result;
	for (auto slot: stack)
		if (exitSet.count(slot))
			result.emplace_back(slot);

	return result;
}

void StackLayoutGenerator::processEntryPoint(CFG::BasicBlock const& _entry, CFG::FunctionInfo const* _functionInfo)
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
				auto& info = m_layout.blockInfos[block];
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
				m_layout.blockInfos[target].entrySlots,
				[exitLayout = m_layout.blockInfos[jumpingBlock].exitSlots](StackSlot const& _slot) {
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

	stitchConditionalJumpSets(_entry);

	/// Forward pass.
	visited.clear();
	toVisit.emplace_back(&_entry);

	if (_functionInfo)
	{
		Stack& entryLayout = m_layout.blockInfos[&_entry].entryLayout;
		if (_functionInfo->canContinue)
			entryLayout.emplace_back(FunctionReturnLabelSlot{_functionInfo->function});
		for (auto const& param: _functionInfo->parameters | ranges::views::reverse)
			entryLayout.emplace_back(param);
	}

	while (!toVisit.empty())
	{
		CFG::BasicBlock const *block = *toVisit.begin();
		toVisit.pop_front();

		if (visited.count(block))
			continue;
		visited.emplace(block);

		Stack stack = m_layout.blockInfos[block].entryLayout;

		stack = propagateStackThroughBlock(stack, *block);

		for (auto it = std::begin(stack); it != std::end(stack);)
		{
			if (!m_layout.blockInfos[block].exitSlots.count(*it))
				it = stack.erase(it);
			else
				it++;
		}

		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				if (!visited.count(_jump.target))
				{
					m_layout.blockInfos[_jump.target].entryLayout = stack;
					toVisit.emplace_front(_jump.target);
				}
				else
				{
					// TODO: is this the correct validation?
					auto const& layout = m_layout.blockInfos[_jump.target].entryLayout;
					for (auto slot: layout)
						yulAssert(util::contains(stack, slot), stackToString(layout) + " does not contain " + stackSlotToString(slot));
				}
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				if (!(stack.back() == _conditionalJump.condition))
					stack.emplace_back(_conditionalJump.condition);
				Stack postJumpStack = stack;
				postJumpStack.pop_back();
				if (!visited.count(_conditionalJump.zero))
				{
					m_layout.blockInfos[_conditionalJump.zero].entryLayout = postJumpStack;
					toVisit.emplace_front(_conditionalJump.zero);
				}
				else
				{
					// TODO: is this the correct validation?
					auto const& layout = m_layout.blockInfos[_conditionalJump.zero].entryLayout;
					for (auto slot: layout)
						yulAssert(util::contains(postJumpStack, slot), stackToString(postJumpStack) + " does not contain " + stackSlotToString(slot));
				}
				if (!visited.count(_conditionalJump.nonZero))
				{
					m_layout.blockInfos[_conditionalJump.nonZero].entryLayout = postJumpStack;
					toVisit.emplace_front(_conditionalJump.nonZero);
				}
				else
				{
					// TODO: is this the correct validation?
					auto const& layout = m_layout.blockInfos[_conditionalJump.nonZero].entryLayout;
					for (auto slot: layout)
						yulAssert(util::contains(postJumpStack, slot), stackToString(postJumpStack) + " does not contain " + stackSlotToString(slot));
				}
			},
			[&](CFG::BasicBlock::FunctionReturn const&) {},
			[&](CFG::BasicBlock::Terminated const&) {},
		}, block->exit);
		m_layout.blockInfos[block].exitLayout = stack;

	}

	stitchConditionalJumps(_entry);
}

std::optional<StackSlotSet> StackLayoutGenerator::getExitLayoutOrStageDependencies(
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
				if (auto* info = util::valueOrNullptr(m_layout.blockInfos, _jump.target))
					return info->entrySlots;
				return StackSlotSet{};
			}
			// If the current iteration has already visited the jump target, start from its entry layout.
			if (_visited.count(_jump.target))
				return m_layout.blockInfos.at(_jump.target).entrySlots;
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
				StackSlotSet stack = m_layout.blockInfos.at(_conditionalJump.zero).entrySlots +
					m_layout.blockInfos.at(_conditionalJump.nonZero).entrySlots;
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

std::list<std::pair<CFG::BasicBlock const*, CFG::BasicBlock const*>> StackLayoutGenerator::collectBackwardsJumps(CFG::BasicBlock const& _entry) const
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


void StackLayoutGenerator::stitchConditionalJumpSets(CFG::BasicBlock const& _block)
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
				StackSlotSet exitLayout = info.exitSlots;

				// The last block must have produced the condition at the stack top.
				yulAssert(!exitLayout.empty(), "");
				yulAssert(exitLayout.count(_conditionalJump.condition));

				auto fixJumpTargetEntry = [&](StackSlotSet const& _originalEntryLayout) -> StackSlotSet {
					StackSlotSet newEntryLayout;
					for (auto slot: exitLayout)
						if (util::contains(_originalEntryLayout, slot))
							newEntryLayout.emplace(slot);

					// Make sure everything the block being jumped to requires is actually present or can be generated.
					for (auto const& slot: _originalEntryLayout)
						yulAssert(canBeFreelyGenerated(slot) || util::contains(newEntryLayout, slot), "");
					return newEntryLayout;
				};
				zeroTargetInfo.entrySlots = fixJumpTargetEntry(zeroTargetInfo.entrySlots);
				nonZeroTargetInfo.entrySlots = fixJumpTargetEntry(nonZeroTargetInfo.entrySlots);
				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const&)	{},
			[&](CFG::BasicBlock::Terminated const&) { },
		}, _block->exit);
	});
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
				yulAssert(exitLayout.back() == _conditionalJump.condition, stackToString(exitLayout) + " was expected to end in conditon " + stackSlotToString(_conditionalJump.condition));
				// The condition is consumed by the jump.
				exitLayout.pop_back();

				auto fixJumpTargetEntry = [&](Stack const& _originalEntryLayout) -> Stack {
					Stack newEntryLayout = exitLayout;
					// Whatever the block being jumped to does not actually require, can be marked as junk.
					for (auto& slot: newEntryLayout)
						if (!util::contains(_originalEntryLayout, slot))
							slot = JunkSlot{};
					// Make sure everything the block being jumped to requires is actually present or can be generated.
					{
						bool sanity = true;
						for (auto const& slot: _originalEntryLayout)
							sanity = sanity && (canBeFreelyGenerated(slot) || util::contains(newEntryLayout, slot));
						yulAssert(sanity, stackToString(newEntryLayout) + " is missing something from " + stackToString(_originalEntryLayout));
					}
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

std::vector<StackLayoutGenerator::StackTooDeep> StackLayoutGenerator::reportStackTooDeep(CFG::BasicBlock const& _entry) const
{
	std::vector<StackTooDeep> stackTooDeepErrors;
	util::BreadthFirstSearch<CFG::BasicBlock const*> breadthFirstSearch{{&_entry}};
	breadthFirstSearch.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		Stack currentStack = m_layout.blockInfos.at(_block).entryLayout;

		for (auto const& operation: _block->operations)
		{
			Stack& operationEntry = m_layout.operationEntryLayout.at(&operation);

			stackTooDeepErrors += findStackTooDeep(currentStack, operationEntry);
			currentStack = operationEntry;
			for (size_t i = 0; i < operation.input.size(); i++)
				currentStack.pop_back();
			currentStack += operation.output;
		}
		// Do not attempt to create the exit layout m_layout.blockInfos.at(_block).exitLayout here,
		// since the code generator will directly move to the target entry layout.

		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				Stack const& targetLayout = m_layout.blockInfos.at(_jump.target).entryLayout;
				stackTooDeepErrors += findStackTooDeep(currentStack, targetLayout);

				if (!_jump.backwards)
					_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				for (Stack const& targetLayout: {
					m_layout.blockInfos.at(_conditionalJump.zero).entryLayout,
					m_layout.blockInfos.at(_conditionalJump.nonZero).entryLayout
				})
					stackTooDeepErrors += findStackTooDeep(currentStack, targetLayout);

				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const&) {},
			[&](CFG::BasicBlock::Terminated const&) {},
		}, _block->exit);
	});
	return stackTooDeepErrors;
}

Stack StackLayoutGenerator::compressStack(Stack _stack)
{
	std::optional<size_t> firstDupOffset;
	do
	{
		if (firstDupOffset)
		{
			std::swap(_stack.at(*firstDupOffset), _stack.back());
			_stack.pop_back();
			firstDupOffset.reset();
		}
		for (auto&& [depth, slot]: _stack | ranges::views::reverse | ranges::views::enumerate)
			if (canBeFreelyGenerated(slot))
			{
				firstDupOffset = _stack.size() - depth - 1;
				break;
			}
			else if (auto dupDepth = util::findOffset(_stack | ranges::views::reverse | ranges::views::drop(depth + 1), slot))
				if (depth + *dupDepth <= 16)
				{
					firstDupOffset = _stack.size() - depth - 1;
					break;
				}
	}
	while (firstDupOffset);
	return _stack;
}

void StackLayoutGenerator::fillInJunk(CFG::BasicBlock const& _block, CFG::FunctionInfo const* _functionInfo)
{
	/// Recursively adds junk to the subgraph starting on @a _entry.
	/// Since it is only called on cut-vertices, the full subgraph retains proper stack balance.
	auto addJunkRecursive = [&](CFG::BasicBlock const* _entry, size_t _numJunk) {
		util::BreadthFirstSearch<CFG::BasicBlock const*> breadthFirstSearch{{_entry}};
		breadthFirstSearch.run([&](CFG::BasicBlock const* _block, auto _addChild) {
			auto& blockInfo = m_layout.blockInfos.at(_block);
			blockInfo.entryLayout = Stack{_numJunk, JunkSlot{}} + std::move(blockInfo.entryLayout);
			for (auto const& operation: _block->operations)
			{
				auto& operationEntryLayout = m_layout.operationEntryLayout.at(&operation);
				operationEntryLayout = Stack{_numJunk, JunkSlot{}} + std::move(operationEntryLayout);
			}
			blockInfo.exitLayout = Stack{_numJunk, JunkSlot{}} + std::move(blockInfo.exitLayout);

			std::visit(util::GenericVisitor{
				[&](CFG::BasicBlock::MainExit const&) {},
				[&](CFG::BasicBlock::Jump const& _jump)
				{
					_addChild(_jump.target);
				},
				[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
				{
					_addChild(_conditionalJump.zero);
					_addChild(_conditionalJump.nonZero);
				},
				[&](CFG::BasicBlock::FunctionReturn const&) { yulAssert(false); },
				[&](CFG::BasicBlock::Terminated const&) {},
			}, _block->exit);
		});
	};
	/// @returns the number of operations required to transform @a _source to @a _target.
	auto evaluateTransform = [&](Stack _source, Stack const& _target) -> size_t {
		size_t opGas = 0;
		auto swap = [&](unsigned _swapDepth)
		{
			if (_swapDepth > 16)
				opGas += 1000;
			else
				opGas += evmasm::GasMeter::runGas(evmasm::swapInstruction(_swapDepth), langutil::EVMVersion());
		};
		auto dupOrPush = [&](StackSlot const& _slot)
		{
			if (canBeFreelyGenerated(_slot))
				opGas += evmasm::GasMeter::runGas(evmasm::pushInstruction(32), langutil::EVMVersion());
			else
			{
				if (auto depth = util::findOffset(_source | ranges::views::reverse, _slot))
				{
					if (*depth < 16)
						opGas += evmasm::GasMeter::runGas(evmasm::dupInstruction(static_cast<unsigned>(*depth + 1)), langutil::EVMVersion());
					else
						opGas += 1000;
				}
				else
				{
					// This has to be a previously unassigned return variable.
					// We at least sanity-check that it is among the return variables at all.
					yulAssert(m_currentFunctionInfo && std::holds_alternative<VariableSlot>(_slot));
					yulAssert(util::contains(m_currentFunctionInfo->returnVariables, std::get<VariableSlot>(_slot)));
					// Strictly speaking the cost of the PUSH0 depends on the targeted EVM version, but the difference
					// will not matter here.
					opGas += evmasm::GasMeter::runGas(evmasm::pushInstruction(0), langutil::EVMVersion());;
				}
			}
		};
		auto pop = [&]() { opGas += evmasm::GasMeter::runGas(evmasm::Instruction::POP,langutil::EVMVersion()); };
		createStackLayout(_source, _target, swap, dupOrPush, pop);
		return opGas;
	};
	/// @returns the number of junk slots to be prepended to @a _targetLayout for an optimal transition from
	/// @a _entryLayout to @a _targetLayout.
	auto getBestNumJunk = [&](Stack const& _entryLayout, Stack const& _targetLayout) -> size_t {
		size_t bestCost = evaluateTransform(_entryLayout, _targetLayout);
		size_t bestNumJunk = 0;
		size_t maxJunk = _entryLayout.size();
		for (size_t numJunk = 1; numJunk <= maxJunk; ++numJunk)
		{
			size_t cost = evaluateTransform(_entryLayout, Stack{numJunk, JunkSlot{}} + _targetLayout);
			if (cost < bestCost)
			{
				bestCost = cost;
				bestNumJunk = numJunk;
			}
		}
		return bestNumJunk;
	};

	if (_functionInfo && !_functionInfo->canContinue && _block.allowsJunk())
	{
		size_t bestNumJunk = getBestNumJunk(
			_functionInfo->parameters | ranges::views::reverse | ranges::to<Stack>,
			m_layout.blockInfos.at(&_block).entryLayout
		);
		if (bestNumJunk > 0)
			addJunkRecursive(&_block, bestNumJunk);
	}

	/// Traverses the CFG and at each block that allows junk, i.e. that is a cut-vertex that never leads to a function
	/// return, checks if adding junk reduces the shuffling cost upon entering and if so recursively adds junk
	/// to the spanned subgraph.
	util::BreadthFirstSearch<CFG::BasicBlock const*>{{&_block}}.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		if (_block->allowsJunk())
		{
			auto& blockInfo = m_layout.blockInfos.at(_block);
			Stack entryLayout = blockInfo.entryLayout;
			Stack const& nextLayout = _block->operations.empty() ? blockInfo.exitLayout : m_layout.operationEntryLayout.at(&_block->operations.front());
			if (entryLayout != nextLayout)
			{
				size_t bestNumJunk = getBestNumJunk(
					entryLayout,
					nextLayout
				);
				if (bestNumJunk > 0)
				{
					addJunkRecursive(_block, bestNumJunk);
					blockInfo.entryLayout = entryLayout;
				}
			}
		}
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
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
}
