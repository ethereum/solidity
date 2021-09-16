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

#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take.hpp>

namespace solidity::yul
{

inline std::string stackSlotToString(StackSlot const& _slot)
{
	return std::visit(util::GenericVisitor{
		[](FunctionCallReturnLabelSlot const& _ret) -> std::string { return "RET[" + _ret.call.get().functionName.name.str() + "]"; },
		[](FunctionReturnLabelSlot const&) -> std::string { return "RET"; },
		[](VariableSlot const& _var) { return _var.variable.get().name.str(); },
		[](LiteralSlot const& _lit) { return toCompactHexWithPrefix(_lit.value); },
		[](TemporarySlot const& _tmp) -> std::string { return "TMP[" + _tmp.call.get().functionName.name.str() + ", " + std::to_string(_tmp.index) + "]"; },
		[](JunkSlot const&) -> std::string { return "JUNK"; }
	}, _slot);
}

inline std::string stackToString(Stack const& _stack)
{
	std::string result("[ ");
	for (auto const& slot: _stack)
		result += stackSlotToString(slot) + ' ';
	result += ']';
	return result;
}


// Abstraction of stack shuffling operations. Can be defined as actual concept once we switch to C++20.
// Used as an interface for the stack shuffler below.
// The shuffle operation class is expected to internally keep track of a current stack layout (the "source layout")
// that the shuffler is supposed to shuffle to a fixed target stack layout.
// The shuffler works iteratively. At each iteration it instantiates an instance of the shuffle operations and
// queries it for various information about the current source stack layout and the target layout, as described
// in the interface below.
// Based on that information the shuffler decides which is the next optimal operation to perform on the stack
// and calls the corresponding entry point in the shuffling operations (swap, pushOrDupTarget or pop).
/*
template<typename ShuffleOperations>
concept ShuffleOperationConcept = requires(ShuffleOperations ops, size_t sourceOffset, size_t targetOffset, size_t depth) {
	// Returns true, iff the current slot at sourceOffset in source layout is a suitable slot at targetOffset.
	{ ops.isCompatible(sourceOffset, targetOffset) } -> std::convertible_to<bool>;
	// Returns true, iff the slots at the two given source offsets are identical.
	{ ops.sourceIsSame(sourceOffset, sourceOffset) } -> std::convertible_to<bool>;
	// Returns a positive integer n, if the slot at the given source offset needs n more copies.
	// Returns a negative integer -n, if the slot at the given source offsets occurs n times too many.
	// Returns zero if the amount of occurrences, in the current source layout, of the slot at the given source offset
	// matches the desired amount of occurrences in the target.
	{ ops.sourceMultiplicity(sourceOffset) } -> std::convertible_to<int>;
	// Returns a positive integer n, if the slot at the given target offset needs n more copies.
	// Returns a negative integer -n, if the slot at the given target offsets occurs n times too many.
	// Returns zero if the amount of occurrences, in the current source layout, of the slot at the given target offset
	// matches the desired amount of occurrences in the target.
	{ ops.targetMultiplicity(targetOffset) } -> std::convertible_to<int>;
	// Returns true, iff any slot is compatible with the given target offset.
	{ ops.targetIsArbitrary(targetOffset) } -> std::convertible_to<bool>;
	// Returns the number of slots in the source layout.
	{ ops.sourceSize() } -> std::convertible_to<size_t>;
	// Returns the number of slots in the target layout.
	{ ops.targetSize() } -> std::convertible_to<size_t>;
	// Swaps the top most slot in the source with the slot `depth` slots below the top.
	// In terms of EVM opcodes this is supposed to be a `SWAP<depth>`.
	// In terms of vectors this is supposed to be `std::swap(source.at(source.size() - depth - 1, source.top))`.
	{ ops.swap(depth) };
	// Pops the top most slot in the source, i.e. the slot at offset ops.sourceSize() - 1.
	// In terms of EVM opcodes this is `POP`.
	// In terms of vectors this is `source.pop();`.
	{ ops.pop() };
	// Dups or pushes the slot that is supposed to end up at the given target offset.
	{ ops.pushOrDupTarget(targetOffset) };
};
*/
/// Helper class that can perform shuffling of a source stack layout to a target stack layout via
/// abstracted shuffle operations.
template</*ShuffleOperationConcept*/ typename ShuffleOperations>
class Shuffler
{
public:
	/// Executes the stack shuffling operations. Instantiates an instance of ShuffleOperations
	/// in each iteration. Each iteration performs exactly one operation that modifies the stack.
	/// After `shuffle`, source and target have the same size and all slots in the source layout are
	/// compatible with the slots at the same target offset.
	template<typename... Args>
	static void shuffle(Args&&... args)
	{
		bool needsMoreShuffling = true;
		// The shuffling algorithm should always terminate in polynomial time, but we provide a limit
		// in case it does not terminate due to a bug.
		size_t iterationCount = 0;
		while (iterationCount < 1000 && (needsMoreShuffling = shuffleStep(std::forward<Args>(args)...)))
			++iterationCount;
		yulAssert(!needsMoreShuffling, "Could not create stack layout after 1000 iterations.");
	}
private:
	// If dupping an ideal slot causes a slot that will still be required to become unreachable, then dup
	// the latter slot first.
	// @returns true, if it performed a dup.
	static bool dupDeepSlotIfRequired(ShuffleOperations& _ops)
	{
		// Check if the stack is large enough for anything to potentially become unreachable.
		if (_ops.sourceSize() < 15)
			return false;
		// Check whether any deep slot might still be needed later (i.e. we still need to reach it with a DUP or SWAP).
		for (size_t sourceOffset: ranges::views::iota(0u, _ops.sourceSize() - 15))
		{
			// This slot needs to be moved.
			if (!_ops.isCompatible(sourceOffset, sourceOffset))
			{
				// If the current top fixes the slot, swap it down now.
				if (_ops.isCompatible(_ops.sourceSize() - 1, sourceOffset))
				{
					_ops.swap(_ops.sourceSize() - sourceOffset - 1);
					return true;
				}
				// Bring up a slot to fix this now, if possible.
				if (bringUpTargetSlot(_ops, sourceOffset))
					return true;
				// Otherwise swap up the slot that will fix the offending slot.
				for (auto offset: ranges::views::iota(sourceOffset + 1, _ops.sourceSize()))
					if (_ops.isCompatible(offset, sourceOffset))
					{
						_ops.swap(_ops.sourceSize() - offset - 1);
						return true;
					}
				// Otherwise give up - we will need stack compression or stack limit evasion.
			}
			// We need another copy of this slot.
			else if (_ops.sourceMultiplicity(sourceOffset) > 0)
			{
				// If this slot occurs again later, we skip this occurrence.
				if (ranges::any_of(
					ranges::views::iota(sourceOffset + 1, _ops.sourceSize()),
					[&](size_t _offset) { return _ops.sourceIsSame(sourceOffset, _offset); }
				))
					continue;
				// Bring up the target slot that would otherwise become unreachable.
				for (size_t targetOffset: ranges::views::iota(0u, _ops.targetSize()))
					if (!_ops.targetIsArbitrary(targetOffset) && _ops.isCompatible(sourceOffset, targetOffset))
					{
						_ops.pushOrDupTarget(targetOffset);
						return true;
					}
			}
		}
		return false;
	}
	/// Finds a slot to dup or push with the aim of eventually fixing @a _targetOffset in the target.
	/// In the simplest case, the slot at @a _targetOffset has a multiplicity > 0, i.e. it can directly be dupped or pushed
	/// and the next iteration will fix @a _targetOffset.
	/// But, in general, there may already be enough copies of the slot that is supposed to end up at @a _targetOffset
	/// on stack, s.t. it cannot be dupped again. In that case there has to be a copy of the desired slot on stack already
	/// elsewhere that is not yet in place (`nextOffset` below). The fact that ``nextOffset`` is not in place means that
	/// we can (recursively) try bringing up the slot that is supposed to end up at ``nextOffset`` in the *target*.
	/// When the target slot at ``nextOffset`` is fixed, the current source slot at ``nextOffset`` will be
	/// at the stack top, which is the slot required at @a _targetOffset.
	static bool bringUpTargetSlot(ShuffleOperations& _ops, size_t _targetOffset)
	{
		std::list<size_t> toVisit{_targetOffset};
		std::set<size_t> visited;

		while (!toVisit.empty())
		{
			auto offset = *toVisit.begin();
			toVisit.erase(toVisit.begin());
			visited.emplace(offset);
			if (_ops.targetMultiplicity(offset) > 0)
			{
				_ops.pushOrDupTarget(offset);
				return true;
			}
			// There must be another slot we can dup/push that will lead to the target slot at ``offset`` to be fixed.
			for (auto nextOffset: ranges::views::iota(0u, std::min(_ops.sourceSize(), _ops.targetSize())))
				if (
					!_ops.isCompatible(nextOffset, nextOffset) &&
					_ops.isCompatible(nextOffset, offset)
				)
					if (!visited.count(nextOffset))
						toVisit.emplace_back(nextOffset);
		}
		return false;
	}
	/// Performs a single stack operation, transforming the source layout closer to the target layout.
	template<typename... Args>
	static bool shuffleStep(Args&&... args)
	{
		ShuffleOperations ops{std::forward<Args>(args)...};

		// All source slots are final.
		if (ranges::all_of(
			ranges::views::iota(0u, ops.sourceSize()),
			[&](size_t _index) { return ops.isCompatible(_index, _index); }
		))
		{
			// Bring up all remaining target slots, if any, or terminate otherwise.
			if (ops.sourceSize() < ops.targetSize())
			{
				if (!dupDeepSlotIfRequired(ops))
					yulAssert(bringUpTargetSlot(ops, ops.sourceSize()), "");
				return true;
			}
			return false;
		}

		size_t sourceTop = ops.sourceSize() - 1;
		// If we no longer need the current stack top, we pop it, unless we need an arbitrary slot at this position
		// in the target.
		if (
			ops.sourceMultiplicity(sourceTop) < 0 &&
			!ops.targetIsArbitrary(sourceTop)
		)
		{
			ops.pop();
			return true;
		}

		yulAssert(ops.targetSize() > 0, "");

		// If the top is not supposed to be exactly what is on top right now, try to find a lower position to swap it to.
		if (!ops.isCompatible(sourceTop, sourceTop) || ops.targetIsArbitrary(sourceTop))
			for (size_t offset: ranges::views::iota(0u, std::min(ops.sourceSize(), ops.targetSize())))
				// It makes sense to swap to a lower position, if
				if (
					!ops.isCompatible(offset, offset) && // The lower slot is not already in position.
					!ops.sourceIsSame(offset, sourceTop) && // We would not just swap identical slots.
					ops.isCompatible(sourceTop, offset) // The lower position wants to have this slot.
				)
				{
					// We cannot swap that deep.
					if (ops.sourceSize() - offset - 1 > 16)
					{
						// If there is a reachable slot to be removed, park the current top there.
						for (size_t swapDepth: ranges::views::iota(1u, 17u) | ranges::views::reverse)
							if (ops.sourceMultiplicity(ops.sourceSize() - 1 - swapDepth) < 0)
							{
								ops.swap(swapDepth);
								return true;
							}
						// Otherwise we rely on stack compression or stack-to-memory.
					}
					ops.swap(ops.sourceSize() - offset - 1);
					return true;
				}

		// ops.sourceSize() > ops.targetSize() cannot be true anymore, since if the source top is no longer required,
		// we already popped it, and if it is required, we already swapped it down to a suitable target position.
		yulAssert(ops.sourceSize() <= ops.targetSize(), "");

		// If a lower slot should be removed, try to bring up the slot that should end up there and bring it up.
		// Note that after the cases above, there will always be a target slot to duplicate in this case.
		for (size_t offset: ranges::views::iota(0u, ops.sourceSize()))
			if (
				!ops.isCompatible(offset, offset) && // The lower slot is not already in position.
				ops.sourceMultiplicity(offset) < 0 && // We have too many copies of this slot.
				offset <= ops.targetSize() && // There is a target slot at this position.
				!ops.targetIsArbitrary(offset) // And that target slot is not arbitrary.
			)
			{
				if (!dupDeepSlotIfRequired(ops))
					yulAssert(bringUpTargetSlot(ops, offset), "");
				return true;
			}

		// At this point we want to keep all slots.
		for (size_t i = 0; i < ops.sourceSize(); ++i)
			yulAssert(ops.sourceMultiplicity(i) >= 0, "");
		yulAssert(ops.sourceSize() <= ops.targetSize(), "");

		// If the top is not in position, try to find a slot that wants to be at the top and swap it up.
		if (!ops.isCompatible(sourceTop, sourceTop))
			for (size_t sourceOffset: ranges::views::iota(0u, ops.sourceSize()))
				if (
					!ops.isCompatible(sourceOffset, sourceOffset) &&
					ops.isCompatible(sourceOffset, sourceTop)
				)
				{
					ops.swap(ops.sourceSize() - sourceOffset - 1);
					return true;
				}

		// If we still need more slots, produce a suitable one.
		if (ops.sourceSize() < ops.targetSize())
		{
			if (!dupDeepSlotIfRequired(ops))
				yulAssert(bringUpTargetSlot(ops, ops.sourceSize()), "");
			return true;
		}

		// The stack has the correct size, each slot has the correct number of copies and the top is in position.
		yulAssert(ops.sourceSize() == ops.targetSize(), "");
		size_t size = ops.sourceSize();
		for (size_t i = 0; i < ops.sourceSize(); ++i)
			yulAssert(ops.sourceMultiplicity(i) == 0 && (ops.targetIsArbitrary(i) || ops.targetMultiplicity(i) == 0), "");
		yulAssert(ops.isCompatible(sourceTop, sourceTop), "");

		// If we find a lower slot that is out of position, but also compatible with the top, swap that up.
		for (size_t offset: ranges::views::iota(0u, size))
			if (!ops.isCompatible(offset, offset) && ops.isCompatible(sourceTop, offset))
			{
				ops.swap(size - offset - 1);
				return true;
			}
		// Swap up any slot that is still out of position.
		for (size_t offset: ranges::views::iota(0u, size))
			if (!ops.isCompatible(offset, offset) && !ops.sourceIsSame(offset, sourceTop))
			{
				ops.swap(size - offset - 1);
				return true;
			}
		yulAssert(false, "");
	}
};


/// Transforms @a _currentStack to @a _targetStack, invoking the provided shuffling operations.
/// Modifies @a _currentStack itself after each invocation of the shuffling operations.
/// @a _swap is a function with signature void(unsigned) that is called when the top most slot is swapped with
/// the slot `depth` slots below the top. In terms of EVM opcodes this is supposed to be a `SWAP<depth>`.
/// @a _pushOrDup is a function with signature void(StackSlot const&) that is called to push or dup the slot given as
/// its argument to the stack top.
/// @a _pop is a function with signature void() that is called when the top most slot is popped.
template<typename Swap, typename PushOrDup, typename Pop>
void createStackLayout(Stack& _currentStack, Stack const& _targetStack, Swap _swap, PushOrDup _pushOrDup, Pop _pop)
{
	struct ShuffleOperations
	{
		Stack& currentStack;
		Stack const& targetStack;
		Swap swapCallback;
		PushOrDup pushOrDupCallback;
		Pop popCallback;
		std::map<StackSlot, int> multiplicity;
		ShuffleOperations(
			Stack& _currentStack,
			Stack const& _targetStack,
			Swap _swap,
			PushOrDup _pushOrDup,
			Pop _pop
		):
			currentStack(_currentStack),
			targetStack(_targetStack),
			swapCallback(_swap),
			pushOrDupCallback(_pushOrDup),
			popCallback(_pop)
		{
			for (auto const& slot: currentStack)
				--multiplicity[slot];
			for (auto&& [offset, slot]: targetStack | ranges::views::enumerate)
				if (std::holds_alternative<JunkSlot>(slot) && offset < currentStack.size())
					++multiplicity[currentStack.at(offset)];
				else
					++multiplicity[slot];
		}
		bool isCompatible(size_t _source, size_t _target)
		{
			return
				_source < currentStack.size() &&
				_target < targetStack.size() &&
				(
					std::holds_alternative<JunkSlot>(targetStack.at(_target)) ||
					currentStack.at(_source) == targetStack.at(_target)
				);
		}
		bool sourceIsSame(size_t _lhs, size_t _rhs) { return currentStack.at(_lhs) == currentStack.at(_rhs); }
		int sourceMultiplicity(size_t _offset) { return multiplicity.at(currentStack.at(_offset)); }
		int targetMultiplicity(size_t _offset) { return multiplicity.at(targetStack.at(_offset)); }
		bool targetIsArbitrary(size_t offset)
		{
			return offset < targetStack.size() && std::holds_alternative<JunkSlot>(targetStack.at(offset));
		}
		void swap(size_t _i)
		{
			swapCallback(static_cast<unsigned>(_i));
			std::swap(currentStack.at(currentStack.size() - _i - 1), currentStack.back());
		}
		size_t sourceSize() { return currentStack.size(); }
		size_t targetSize() { return targetStack.size(); }
		void pop()
		{
			popCallback();
			currentStack.pop_back();
		}
		void pushOrDupTarget(size_t _offset)
		{
			auto const& targetSlot = targetStack.at(_offset);
			pushOrDupCallback(targetSlot);
			currentStack.push_back(targetSlot);
		}
	};

	Shuffler<ShuffleOperations>::shuffle(_currentStack, _targetStack, _swap, _pushOrDup, _pop);

	yulAssert(_currentStack.size() == _targetStack.size(), "");
	for (auto&& [current, target]: ranges::zip_view(_currentStack, _targetStack))
		if (std::holds_alternative<JunkSlot>(target))
			current = JunkSlot{};
		else
			yulAssert(current == target, "");
}

}
