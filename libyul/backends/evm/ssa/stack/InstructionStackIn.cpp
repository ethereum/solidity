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

#include <libyul/backends/evm/ssa/stack/InstructionStackIn.h>

#include <libyul/backends/evm/ssa/Stack.h>

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/none_of.hpp>

#include <concepts>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::stack;

namespace
{
/// Simulates the shuffle towards `tail + args` to decide two things about the tail:
///
/// - Which args take their copy out of the tail ("moved") instead of being duplicated
/// - Which tail slots are dropped to bring a needed copy within DUP/SWAP reach
///
/// Moved args are `SWAP`ped to the top and taken off, then the args are generated in order. The depth of a DUP source
/// automatically accounts for whatever args already sit above it.
struct InstructionStackInBuilder
{
	/// the amount of times `_slot` appears in `args` from the `args`-relative offset `_fromArgsIndex`.
	std::size_t remainingArgsCount(StackSlot const& _slot, std::size_t const _fromArgsIndex) const
	{
		yulAssert(_fromArgsIndex <= args.size());
		return static_cast<std::size_t>(std::count(
			args.begin() + static_cast<std::ptrdiff_t>(_fromArgsIndex),
			args.end(),
			_slot
		));
	}

	/// A slot the tail does not need to keep: junk, or a value that is neither live out nor an argument
	bool isDead(StackSlot const& _slot) const
	{
		if (_slot.isJunk())
			return true;
		if (!_slot.isValue())
			return false;
		return !liveOut.contains(_slot) && remainingArgsCount(_slot, 0) == 0;
	}

	/// An arg whose on-stack copy can be moved into the args region instead of being duplicated:
	/// - it is not a literal (a literal should be DUPed if in reach or PUSHed otherwise),
	/// - this is its last use among the args, and
	/// - after the operation it is either dead or spilled (so the stack may lose its copy)
	bool isMovable(std::size_t const _argIndex) const
	{
		StackSlot const& arg = args[_argIndex];
		return
			arg.isVariable() &&
			(!liveOut.contains(arg) || spillSet.isSpilled(arg)) &&
			remainingArgsCount(arg, _argIndex + 1) == 0 &&
			ranges::contains(stack, arg);
	}

	/// A slot that can be dropped to bring something else within reach without a new spill: dead, or a spilled
	/// value (reloadable) that is not an argument
	bool isDroppableForReach(StackSlot const& _slot) const
	{
		return
			isDead(_slot) ||
			(
				_slot.isVariable() &&
				spillSet.isSpilled(_slot) &&
				remainingArgsCount(_slot, 0) == 0
			);
	}

	/// Erases droppable slots above `_offset` in `_data` until `_outOfReach(_offset)` no longer
	/// holds or nothing droppable is left. Only slots above `_offset` are erased, so the offset
	/// stays valid throughout while the slot's depth shrinks with each erasure.
	template<std::predicate<StackOffset> OutOfReachFunction>
	void dropForReach(StackData& _data, StackOffset _offset, OutOfReachFunction const& _outOfReach) const
	{
		std::size_t candidate = _offset.value + 1;
		while (_outOfReach(_offset) && candidate < _data.size())
			if (isDroppableForReach(_data[candidate]))
				_data.erase(_data.begin() + static_cast<std::ptrdiff_t>(candidate));
			else
				++candidate;
	}

	StackData build() const
	{
		StackData data = stack;
		// simulating what a shuffler might produce based on args requirements, liveness, and reachability
		Stack sim(data);

		// Moved args take their copy out of the tail the way a SWAP does: the old top falls into the vacated position.
		// They are pushed back as args below.
		std::vector<std::uint8_t> moved(args.size(), false);
		for (std::size_t i = 0; i < args.size(); ++i)
			moved[i] = isMovable(i);

		// moved args whose copy is still in the tail
		std::vector<std::uint8_t> pending = moved;

		// If a pending copy is out of SWAP reach, drop slots above it
		for (std::size_t i = 0; i < args.size(); ++i)
			if (pending[i])
				dropForReach(
					data,
					sim.depthToOffset(*sim.findSlotDepth(args[i])),  // shallowest copy of args[i] in the tail
					[&](StackOffset const& _offset) { return sim.offsetToDepth(_offset).value > reachableStackDepth; }
				);

		// Takes the top off the tail if it is the copy of a pending moved arg
		auto const takeTop = [&] {
			for (std::size_t j = 0; j < args.size(); ++j)
				if (pending[j] && !sim.empty() && args[j] == sim.top())
				{
					pending[j] = false;
					sim.pop();
					return true;
				}
			return false;
		};

		// simulate tail elements being swapped up into args by removing them from the `sim`, they are later
		// concatenated back on top
		for (std::size_t i = 0; i < args.size(); ++i)
		{
			// remove pending moved args already on top, they leave without a swap
			while (takeTop()) {}
			if (!pending[i])
				continue;
			// the pending slot is swapped; this doesn't use `sim.swap` to bypass the swap target validity asserts
			data[sim.depthToOffset(*sim.findSlotDepth(args[i])).value] = data.back();
			sim.pop();
			pending[i] = false;
		}

		// Now generate the args in order on top of the tail
		yulAssert(ranges::none_of(pending, std::identity{}));
		for (std::size_t i = 0; i < args.size(); ++i)
		{
			if (!moved[i] && !canBeFreelyGenerated(args[i]) && !spillSet.isSpilled(args[i]))
				if (auto const source = sim.findSlotDepth(args[i]))
					dropForReach(
						data,
						sim.depthToOffset(*source),
						[&](StackOffset const& _offset) { return sim.offsetToDepth(_offset).value + 1 > reachableStackDepth; }
					);
			sim.push(args[i]);
		}
		return data;
	}

	StackData const& stack;
	StackData const& args;
	StackSlotLiveness const& liveOut;
	spill::SpillSet const& spillSet;
};
}

StackData stack::buildInstructionStackIn
(
	StackData const& _stack,
	StackData const& _args,
	StackSlotLiveness const& _liveOut,
	spill::SpillSet const& _spillSet
)
{
	return InstructionStackInBuilder{.stack = _stack, .args = _args, .liveOut = _liveOut, .spillSet = _spillSet}.build();
}
