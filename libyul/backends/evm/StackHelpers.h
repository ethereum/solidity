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
#include <libyul/backends/evm/StackShuffle.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>

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

// TODO: move Shuffler to StackShuffle.h
/// Helper class that can perform shuffling of a source stack layout to a target stack layout via
/// abstracted shuffle operations.
class Shuffler
{
public:
	/// Executes the stack shuffling operations.
	/// Each iteration performs exactly one operation that modifies the stack.
	/// After `shuffle`, source and target have the same size and all slots in the source layout are
	/// compatible with the slots at the same target offset.
	static void shuffle(ShuffleOperations& _ops)
	{
		bool needsMoreShuffling = true;
		// The shuffling algorithm should always terminate in polynomial time, but we provide a limit
		// in case it does not terminate due to a bug.
		size_t iterationCount = 0;
		while (iterationCount < 1000 && (needsMoreShuffling = shuffleStep(_ops)))
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
	static bool shuffleStep(ShuffleOperations& _ops)
	{
		_ops.updateMultiplicity();

		// All source slots are final.
		if (ranges::all_of(
			ranges::views::iota(0u, _ops.sourceSize()),
			[&](size_t _index) { return _ops.isCompatible(_index, _index); }
		))
		{
			// Bring up all remaining target slots, if any, or terminate otherwise.
			if (_ops.sourceSize() < _ops.targetSize())
			{
				if (!dupDeepSlotIfRequired(_ops))
					yulAssert(bringUpTargetSlot(_ops, _ops.sourceSize()), "");
				return true;
			}
			return false;
		}

		size_t sourceTop = _ops.sourceSize() - 1;
		// If we no longer need the current stack top, we pop it, unless we need an arbitrary slot at this position
		// in the target.
		if (
			_ops.sourceMultiplicity(sourceTop) < 0 &&
			!_ops.targetIsArbitrary(sourceTop)
		)
		{
			_ops.pop();
			return true;
		}

		yulAssert(_ops.targetSize() > 0, "");

		// If the top is not supposed to be exactly what is on top right now, try to find a lower position to swap it to.
		if (!_ops.isCompatible(sourceTop, sourceTop) || _ops.targetIsArbitrary(sourceTop))
			for (size_t offset: ranges::views::iota(0u, std::min(_ops.sourceSize(), _ops.targetSize())))
				// It makes sense to swap to a lower position, if
				if (
					!_ops.isCompatible(offset, offset) && // The lower slot is not already in position.
					!_ops.sourceIsSame(offset, sourceTop) && // We would not just swap identical slots.
					_ops.isCompatible(sourceTop, offset) // The lower position wants to have this slot.
				)
				{
					// We cannot swap that deep.
					if (_ops.sourceSize() - offset - 1 > 16)
					{
						// If there is a reachable slot to be removed, park the current top there.
						for (size_t swapDepth: ranges::views::iota(1u, 17u) | ranges::views::reverse)
							if (_ops.sourceMultiplicity(_ops.sourceSize() - 1 - swapDepth) < 0)
							{
								_ops.swap(swapDepth);
								if (_ops.targetIsArbitrary(sourceTop))
									// Usually we keep a slot that is to-be-removed, if the current top is arbitrary.
									// However, since we are in a stack-too-deep situation, pop it immediately
									// to compress the stack (we can always push back junk in the end).
									_ops.pop();
								return true;
							}
						// Otherwise we rely on stack compression or stack-to-memory.
					}
					_ops.swap(_ops.sourceSize() - offset - 1);
					return true;
				}

		// ops.sourceSize() > ops.targetSize() cannot be true anymore, since if the source top is no longer required,
		// we already popped it, and if it is required, we already swapped it down to a suitable target position.
		yulAssert(_ops.sourceSize() <= _ops.targetSize(), "");

		// If a lower slot should be removed, try to bring up the slot that should end up there and bring it up.
		// Note that after the cases above, there will always be a target slot to duplicate in this case.
		for (size_t offset: ranges::views::iota(0u, _ops.sourceSize()))
			if (
				!_ops.isCompatible(offset, offset) && // The lower slot is not already in position.
				_ops.sourceMultiplicity(offset) < 0 && // We have too many copies of this slot.
				offset <= _ops.targetSize() && // There is a target slot at this position.
				!_ops.targetIsArbitrary(offset) // And that target slot is not arbitrary.
			)
			{
				if (!dupDeepSlotIfRequired(_ops))
					yulAssert(bringUpTargetSlot(_ops, offset), "");
				return true;
			}

		// At this point we want to keep all slots.
		for (size_t i = 0; i < _ops.sourceSize(); ++i)
			yulAssert(_ops.sourceMultiplicity(i) >= 0, "");
		yulAssert(_ops.sourceSize() <= _ops.targetSize(), "");

		// If the top is not in position, try to find a slot that wants to be at the top and swap it up.
		if (!_ops.isCompatible(sourceTop, sourceTop))
			for (size_t sourceOffset: ranges::views::iota(0u, _ops.sourceSize()))
				if (
					!_ops.isCompatible(sourceOffset, sourceOffset) &&
					_ops.isCompatible(sourceOffset, sourceTop)
				)
				{
					_ops.swap(_ops.sourceSize() - sourceOffset - 1);
					return true;
				}

		// If we still need more slots, produce a suitable one.
		if (_ops.sourceSize() < _ops.targetSize())
		{
			if (!dupDeepSlotIfRequired(_ops))
				yulAssert(bringUpTargetSlot(_ops, _ops.sourceSize()), "");
			return true;
		}

		// The stack has the correct size, each slot has the correct number of copies and the top is in position.
		yulAssert(_ops.sourceSize() == _ops.targetSize(), "");
		size_t size = _ops.sourceSize();
		for (size_t i = 0; i < _ops.sourceSize(); ++i)
			yulAssert(_ops.sourceMultiplicity(i) == 0 && (_ops.targetIsArbitrary(i) || _ops.targetMultiplicity(i) == 0), "");
		yulAssert(_ops.isCompatible(sourceTop, sourceTop), "");

		auto swappableOffsets = ranges::views::iota(size > 17 ? size - 17 : 0u, size);

		// If we find a lower slot that is out of position, but also compatible with the top, swap that up.
		for (size_t offset: swappableOffsets)
			if (!_ops.isCompatible(offset, offset) && _ops.isCompatible(sourceTop, offset))
			{
				_ops.swap(size - offset - 1);
				return true;
			}
		// Swap up any reachable slot that is still out of position.
		for (size_t offset: swappableOffsets)
			if (!_ops.isCompatible(offset, offset) && !_ops.sourceIsSame(offset, sourceTop))
			{
				_ops.swap(size - offset - 1);
				return true;
			}
		// We are in a stack-too-deep situation and try to reduce the stack size.
		// If the current top is merely kept since the target slot is arbitrary, pop it.
		if (_ops.targetIsArbitrary(sourceTop) && _ops.sourceMultiplicity(sourceTop) <= 0)
		{
			_ops.pop();
			return true;
		}
		// If any reachable slot is merely kept, since the target slot is arbitrary, swap it up and pop it.
		for (size_t offset: swappableOffsets)
			if (_ops.targetIsArbitrary(offset) && _ops.sourceMultiplicity(offset) <= 0)
			{
				_ops.swap(size - offset - 1);
				_ops.pop();
				return true;
			}
		// We cannot avoid a stack-too-deep error. Repeat the above without restricting to reachable slots.
		for (size_t offset: ranges::views::iota(0u, size))
			if (!_ops.isCompatible(offset, offset) && _ops.isCompatible(sourceTop, offset))
			{
				_ops.swap(size - offset - 1);
				return true;
			}
		for (size_t offset: ranges::views::iota(0u, size))
			if (!_ops.isCompatible(offset, offset) && !_ops.sourceIsSame(offset, sourceTop))
			{
				_ops.swap(size - offset - 1);
				return true;
			}
		yulAssert(false, "");

		// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
		throw std::exception();
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
	// TODO: refactor IndexedStack
	std::vector<StackSlot> indexedSlots;
	IndexingMap indexer;
	auto indexTransform = ranges::views::transform([&](auto const& _slot) { return indexer[_slot]; });
	IndexedStack _targetStackIndexed = _targetStack | indexTransform | ranges::to<IndexedStack>;
	IndexedStack _currentStackIndexed = _currentStack | indexTransform | ranges::to<IndexedStack>;
	indexedSlots = indexer.indexedSlots();

	auto swapIndexed = [&](unsigned _index) {
		_swap(_index);
		std::swap(_currentStack.at(_currentStack.size() - _index - 1), _currentStack.back());
	};
	auto pushOrDupIndexed = [&](size_t _index) {
		_pushOrDup(indexedSlots.at(_index));
		_currentStack.push_back(indexedSlots.at(_index));
	};
	auto popIndexed = [&]() {
		_pop();
		_currentStack.pop_back();
	};

	StackShuffleOperations ops(
		_currentStackIndexed,
		_targetStackIndexed,
		swapIndexed,
		pushOrDupIndexed,
		popIndexed,
		indexedSlots.size(),
		0 // junkIndex
	);
	Shuffler shuffler{};
	shuffler.shuffle(ops);

	yulAssert(_currentStack.size() == _targetStack.size(), "");
	for (auto&& [current, target]: ranges::zip_view(_currentStack, _targetStack))
		if (std::holds_alternative<JunkSlot>(target))
			current = JunkSlot{};
		else
			yulAssert(current == target, "");
}

}
