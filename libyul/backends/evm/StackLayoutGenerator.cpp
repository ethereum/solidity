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
 * It performs a forward pass over the CFG.
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
	StackLayoutLivenessInfo livenessInfo = StackSlotLivenessTracker::run(_cfg);
	StackLayout stackLayout;
	StackLayoutGenerator{stackLayout, nullptr, livenessInfo}.processEntryPoint(*_cfg.entry);

	for (auto& functionInfo: _cfg.functionInfo | ranges::views::values)
		StackLayoutGenerator{stackLayout, &functionInfo, livenessInfo}.processEntryPoint(*functionInfo.entry, &functionInfo);

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

	StackLayoutLivenessInfo livenessInfo = StackSlotLivenessTracker::run(_cfg);
	StackLayoutGenerator generator{stackLayout, functionInfo, livenessInfo};
	CFG::BasicBlock const* entry = functionInfo ? functionInfo->entry : _cfg.entry;
	generator.processEntryPoint(*entry);
	return generator.reportStackTooDeep(*entry);
}

StackLayoutGenerator::StackLayoutGenerator(StackLayout& _layout, CFG::FunctionInfo const* _functionInfo, StackLayoutLivenessInfo const& _livenessInfo):
	m_layout(_layout),
	m_currentFunctionInfo(_functionInfo),
	m_livenessInfo(_livenessInfo)
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
	for (auto&& [idx, operation]: _block.operations | ranges::views::enumerate)
	{
		StackSlotSet exitSet;
		if (idx < _block.operations.size() - 1)
			exitSet = m_livenessInfo.operationEntrySlots.at(&_block.operations[idx + 1]);
		else
			exitSet = m_livenessInfo.blockInfos.at(&_block).exitSlots;
		Stack newStack = propagateStackThroughOperation(stack, exitSet, operation);
		stack = std::move(newStack);
	}
	StackSlotSet exitSet = m_livenessInfo.blockInfos.at(&_block).exitSlots;
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
			if (!m_livenessInfo.blockInfos.at(block).exitSlots.count(*it))
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
						yulAssert(util::contains(stack, slot), stackToString(stack) + " does not contain " + stackSlotToString(slot));
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
