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

#include <libyul/backends/evm/ssa/stack/Shuffler.h>

#include <libyul/backends/evm/ssa/spill/SpillSet.h>

#include <libyul/backends/evm/ssa/ShuffleTrace.h>
#include <libyul/backends/evm/ssa/Stack.h>

#include <libyul/Exceptions.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/algorithm/set_algorithm.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/algorithm/stable_sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::stack;

namespace
{

bool isSpilled(StackSlot const& _slot, spill::SpillSet const& _spills)
{
	return _slot.isValue() && !_slot.isLiteralValue() && _spills.isSpilled(_slot.value());
}

/// Every target offset is either served by a source slot (retained) or left empty (generated); source slots without
/// a target offset are surplus. Slots already in place are retained first; the remaining target offsets take the
/// closest unused copy, deepest first, so what is left over is generated near the top, where that is cheapest.
class Mapping
{
public:
	/// A copy that sits in a wildcard slot can either be moved to the target offset demanding it or left there,
	/// serving the demand with a duplicate
	enum class WildcardSlotsStrategy : std::uint8_t { Leave, Take };

	Mapping(
		StackData const& _source,
		StackData const& _target,
		std::vector<std::uint8_t> const& _dropped,  // dropped means that the slot is scheduled to be popped
		WildcardSlotsStrategy const _wildcardSlotsStrategy
	):
		m_source(_source),
		m_target(_target),
		m_dropped(_dropped),
		m_targetOf(_source.size()),
		m_sourceOf(_target.size())
	{
		yulAssert(_dropped.size() == _source.size());

		std::size_t const common = std::min(_source.size(), _target.size());

		// slots already in place get mapped directly
		for (std::size_t j = 0; j < common; ++j)
			if (_source[j] == _target[j] && mappable(j))
				map(j, j);

		// the remaining target offsets take the closest unused copy, deepest first: what is left over is
		// generated on top, where that is cheapest
		for (std::size_t j = 0; j < _target.size(); ++j)
			if (!hasSourceAssigned(j) && !isWildcardTarget(j))
				if (std::optional<std::size_t> const copy = closestCopy(j, CopyLocation::OutsideWildcardSlots))
					map(*copy, j);

		// copies sitting in wildcard slots are taken or left according to the strategy
		for (std::size_t j = 0; j < _target.size(); ++j)
			if (!hasSourceAssigned(j) && !isWildcardTarget(j))
				if (std::optional<std::size_t> const copy = closestCopy(j, CopyLocation::InWildcardSlots))
				{
					// a function return label is always taken, can't be duped or pushed
					if (_target[j].isFunctionReturnLabel())
						map(*copy, j);
					else
					{
						m_sawWildcardCopy = true;
						if (_wildcardSlotsStrategy == WildcardSlotsStrategy::Take)
							map(*copy, j);
					}
				}

		// wildcards keep whatever sits there
		for (std::size_t j = 0; j < common; ++j)
			if (isWildcardTarget(j) && !hasSourceAssigned(j) && mappable(j))
				map(j, j);

		// the remaining wildcards absorb the closest surplus slot instead of having it popped
		for (std::size_t j = 0; j < _target.size(); ++j)
			if (isWildcardTarget(j) && !hasSourceAssigned(j))
			{
				std::optional<std::size_t> best;
				for (std::size_t o = 0; o < _source.size(); ++o)
					if (mappable(o) && (!best.has_value() || absDiff(o, j) < absDiff(*best, j)))
						best = o;
				if (best.has_value())
					map(*best, j);
			}

		// everything else is pushed/duped/loaded
	}

	/// The target offset served by the source slot at `_sourceOffset`; no value means surplus.
	std::optional<std::size_t> targetOf(std::size_t const _sourceOffset) const { return m_targetOf[_sourceOffset]; }
	/// The source offset serving `_targetOffset`; no value means generated.
	std::optional<std::size_t> sourceOf(std::size_t const _targetOffset) const { return m_sourceOf[_targetOffset]; }
	/// Whether any target offset's demand was answered by a copy sitting in a wildcard slot, i.e. whether the
	/// `WildcardSlotsStrategy` had anything to decide. Identical for both strategies on the same inputs.
	bool sawWildcardCopy() const { return m_sawWildcardCopy; }
	/// Hands out the target-indexed source side, ending the map's life
	std::vector<std::optional<std::size_t>> release() && { return std::move(m_sourceOf); }

private:
	/// Whether the source slot is not scheduled to be dropped and also not yet pointing to a target
	bool mappable(std::size_t const _sourceOffset) const
	{
		return !m_dropped[_sourceOffset] && !targetOf(_sourceOffset).has_value();
	}

	/// Whether a source slot already serves `_targetOffset`
	bool hasSourceAssigned(std::size_t const _targetOffset) const { return sourceOf(_targetOffset).has_value(); }

	/// Whether the target at `_targetOffset` is junk, i.e. accepts whatever ends up there
	bool isWildcardTarget(std::size_t const _targetOffset) const { return m_target[_targetOffset].isJunk(); }

	/// Whether the source slot at `_sourceOffset` stands at an offset that exists in the target and is junk
	/// there, i.e. a copy that may stay put
	bool inWildcardSlot(std::size_t const _sourceOffset) const
	{
		return _sourceOffset < m_target.size() && isWildcardTarget(_sourceOffset);
	}

	/// Which copies `closestCopy` considers: only those standing in wildcard slots or only those that do not
	enum class CopyLocation : std::uint8_t { OutsideWildcardSlots, InWildcardSlots };

	/// The closest mappable copy of the slot demanded at `_targetOffset` among the copies at `_location`;
	/// ties go to the shallower copy
	std::optional<std::size_t> closestCopy(std::size_t const _targetOffset, CopyLocation const _location) const
	{
		bool const inWildcardSlots = _location == CopyLocation::InWildcardSlots;
		std::optional<std::size_t> best;
		for (std::size_t i = 0; i < m_source.size(); ++i)
			if (m_source[i] == m_target[_targetOffset] && mappable(i) && inWildcardSlot(i) == inWildcardSlots)
				if (!best.has_value() || absDiff(i, _targetOffset) <= absDiff(*best, _targetOffset))
					best = i;
		return best;
	}

	static std::size_t absDiff(std::size_t const _a, std::size_t const _b)
	{
		return _a > _b ? _a - _b : _b - _a;
	}

	void map(std::size_t const _sourceOffset, std::size_t const _targetOffset)
	{
		yulAssert(mappable(_sourceOffset));
		yulAssert(!hasSourceAssigned(_targetOffset));
		m_targetOf[_sourceOffset] = _targetOffset;
		m_sourceOf[_targetOffset] = _sourceOffset;
	}

	StackData const& m_source;
	StackData const& m_target;
	std::vector<std::uint8_t> const& m_dropped;
	std::vector<std::optional<std::size_t>> m_targetOf;
	std::vector<std::optional<std::size_t>> m_sourceOf;
	bool m_sawWildcardCopy = false;
};

/// A position that is not a position
std::size_t constexpr empty = std::numeric_limits<std::size_t>::max();

/// Where a slot on an `Emission`'s working stack is headed: the target offset it is destined for, or no offset
/// at all for a surplus slot, which is to be popped. This is what tells two slots holding the same value apart.
using Destination = std::optional<StackOffset>;

/// The correspondence between the working stack's positions and the destinations of the slots standing there.
/// No destination ever occurs twice.
class DestinationMap
{
public:
	DestinationMap(Mapping const& _mapping, std::size_t const _sourceSize, std::size_t const _targetSize):
		m_positionOf(_targetSize)
	{
		m_destinationOf.reserve(_sourceSize);
		for (std::size_t offset = 0; offset < _sourceSize; ++offset)
		{
			std::optional<std::size_t> const target = _mapping.targetOf(offset);
			m_destinationOf.push_back(target ? Destination{StackOffset{*target}} : std::nullopt);
			if (target)
				bind(StackOffset{*target}, StackOffset{offset});
		}
	}

	/// The destination of the slot at `_pos`, or none for a surplus slot
	Destination const& destinationOf(StackOffset const _pos) const { return m_destinationOf[_pos.value]; }

	/// The position of the slot bound for `_destination`, or none if no slot is bound for it
	std::optional<StackOffset> positionOf(StackOffset const _destination) const
	{
		return m_positionOf[_destination.value];
	}

	/// Exchanges the destinations of the slots at `_a` and `_b`
	void swapDestinations(StackOffset const _a, StackOffset const _b)
	{
		std::swap(m_destinationOf[_a.value], m_destinationOf[_b.value]);
		if (Destination const& destination = m_destinationOf[_a.value])
			m_positionOf[destination->value] = _a;
		if (Destination const& destination = m_destinationOf[_b.value])
			m_positionOf[destination->value] = _b;
	}

	/// Forgets the top slot's destination, if any
	void pop()
	{
		if (Destination const& destination = m_destinationOf.back())
			m_positionOf[destination->value].reset();
		m_destinationOf.pop_back();
	}

	/// Binds the freshly pushed top slot to `_destination`, which no slot may be bound for yet
	void push(StackOffset const _destination)
	{
		StackOffset const top{m_destinationOf.size()};
		m_destinationOf.emplace_back(_destination);
		bind(_destination, top);
	}

	/// Hands out the position-indexed side, ending the map's life
	std::vector<Destination> release() && { return std::move(m_destinationOf); }

private:
	void bind(StackOffset const _destination, StackOffset const _pos)
	{
		yulAssert(!m_positionOf[_destination.value].has_value(), "destination already bound to a slot");
		m_positionOf[_destination.value] = _pos;
	}

	std::vector<Destination> m_destinationOf;
	std::vector<std::optional<StackOffset>> m_positionOf;
};

/// One attempt to emit a plan's shuffle. Works on a copy of the source stack and records every operation in a
/// trace. The attempt has a fixed structure: surplus slots are popped, the retained slots are permuted at the
/// lowest stack height, then every target offset gets its slot bottom-up and the rest is one final permutation.
class Emission
{
public:
	/// The slot at `offset` of the working stack is out of reach by `excess` slots (might be recoverable)
	struct Blocked
	{
		StackOffset offset;
		std::size_t excess;
	};

	/// What an attempt leaves behind. On success `blocked` is empty and `data` holds the shuffled stack; on
	/// failure `trace`, `data` and `destinations` reflect the working state of the aborted attempt, which victim
	/// selection inspects.
	struct Result
	{
		std::optional<Blocked> blocked;
		ShuffleTrace trace;
		StackData data;
		std::vector<Destination> destinations;
	};

	Emission(
		StackData const& _source,
		StackData const& _target,
		Mapping const& _mapping,
		spill::SpillSet const& _spills,
		std::size_t const _maxSwapDepth,
		std::size_t const _maxDupDepth
	):
		m_target(_target),
		m_mapping(_mapping),
		m_spills(_spills),
		m_maxSwapDepth(_maxSwapDepth),
		m_maxDupDepth(_maxDupDepth),
		m_data(_source),
		m_destination(_mapping, _source.size(), _target.size()),
		m_generated(_target.size(), false)
	{
		m_pendingGenerations = static_cast<std::size_t>(ranges::count_if(
			ranges::views::iota(std::size_t{0}, _target.size()),
			[&](std::size_t const _offset) {
				return !_mapping.sourceOf(_offset).has_value();
			}
		));
	}

	Emission(Emission const&) = delete;
	Emission(Emission&&) = delete;
	Emission& operator=(Emission const&) = delete;
	Emission& operator=(Emission&&) = delete;

	[[nodiscard]] Result run() &&
	{
		std::optional<Blocked> const blocked = emit();
		return {
			.blocked = blocked,
			.trace = std::move(m_trace),
			.data = std::move(m_data),
			.destinations = std::move(m_destination).release()
		};
	}

private:
	/// The offsets of the working stack, bottom to top
	[[nodiscard]] auto stackOffsets() const
	{
		return
			ranges::views::iota(std::size_t{0}, m_data.size()) |
			ranges::views::transform([](std::size_t const _offset) { return StackOffset{_offset}; });
	}

	// pop all surplus or bail if surplus that is scheduled for pop isn't reachable
	[[nodiscard]] std::optional<Blocked> removeSurplus()
	{
		while (!m_stack.empty())
		{
			StackOffset shallowestSurplus{empty};
			StackOffset deepestReachableSurplus{empty};
			for (StackOffset const pos: stackOffsets() | ranges::views::reverse)
				if (isSurplus(pos))
				{
					if (shallowestSurplus == empty)
						shallowestSurplus = pos;
					if (isSwapReachable(pos))
						deepestReachableSurplus = pos;
				}
			if (shallowestSurplus == empty)
				break;
			// there is a surplus but we can't reach it
			if (deepestReachableSurplus == empty)
				return blockSwapUnreachable(shallowestSurplus);

			if (
				StackOffset const stackTopOffset{m_stack.size() - 1};
				shallowestSurplus != stackTopOffset
			)
			{
				// a surplus slot equal to the top takes over the top's destination, so the top itself can be popped
				StackOffset equalSurplus{empty};
				for (StackOffset const pos: stackOffsets() | ranges::views::reverse | ranges::views::drop(1))
					if (isSurplus(pos) && m_stack[pos] == m_stack.top())
					{
						equalSurplus = pos;
						break;
					}
				if (equalSurplus != empty)
					m_destination.swapDestinations(equalSurplus, stackTopOffset);
				else
					swapWith(deepestReachableSurplus);
			}
			pop();
		}
		return std::nullopt;
	}

	/// Produces the slot for `_targetOffset`
	[[nodiscard]] std::optional<Blocked> produce(StackOffset const _targetOffset)
	{
		StackSlot const& slot = m_target[_targetOffset.value];
		auto const copy = shallowestCopyPosition(slot);
		if (slot.isJunk())
			push(slot, _targetOffset);
		else if (copy && isDupReachable(*copy))
			dup(*copy, _targetOffset);
		else if (canBeFreelyGenerated(slot) || isSpilled(slot, m_spills))
			push(slot, _targetOffset);
		else if (copy)
			return blockDupUnreachable(*copy);
		else
			yulAssert(false, "generated slot has no copy on the stack and is not spilled");
		yulAssert(!m_generated[_targetOffset.value] && !m_mapping.sourceOf(_targetOffset.value).has_value());
		m_generated[_targetOffset.value] = true;
		--m_pendingGenerations;
		return std::nullopt;
	}

	/// Produces the slot for `_targetOffset` and moves it toward its place right away: if the offset exists
	/// already and holds a slot that is not final, a single swap places the produced slot and floats the other
	/// one, which may be its own placement. An equal slot there just takes over the destination.
	[[nodiscard]] std::optional<Blocked> generate(StackOffset const _targetOffset)
	{
		if (std::optional<Blocked> blocked = produce(_targetOffset))
			return blocked;
		// `produce` left the slot on top; swap it down only if its offset exists strictly below the top:
		// as the top itself it is in place already, beyond the height it has to wait on top anyway
		if (_targetOffset.value + 1 < m_data.size() && !isFinal(_targetOffset))
		{
			if (m_data[_targetOffset.value] == m_data.back())
				// an equal slot stands at the offset: retag instead of swapping two equal slots
				m_destination.swapDestinations(_targetOffset, StackOffset{m_data.size() - 1});
			else if (isSwapReachable(_targetOffset))
				swapWith(_targetOffset);
			// out of swap reach: leave the slot on top; buildBottomUp re-checks reach when filling the offset
		}
		return std::nullopt;
	}

	/// The stack is at its lowest height now, so deep target offsets are at their most reachable.
	///
	/// This method leaves every slot either final (retained slots whose target offset exists already go
	/// there) or parked in an offset whose slot will be generated later. Slots bound for offsets
	/// above the current height park in the holes (one already in a hole stays, the others take the holes
	/// in order of their targets) and float to the top for free when the hole's slot is generated and
	/// swapped into place.
	[[nodiscard]] std::optional<Blocked> permuteAtLowestHeight()
	{
		if (m_stack.empty())
			return std::nullopt;

		// the desired offset for the slot bound for each destination with `empty` where no slot is bound for
		// the destination (those slots are generated later, so the entry is never consulted)
		std::vector desiredOffsetByDestination(m_target.size(), empty);
		// offsets whose target values will be generated later
		std::vector<std::size_t> holes;
		std::vector<std::size_t> parked;
		for (StackOffset const pos: stackOffsets())
		{
			// all surplus is gone, so every slot has a destination
			yulAssert(destinationOf(pos).has_value());
			StackOffset const destination = *destinationOf(pos);
			// target offset at offset pos has no source (ie the value has to be generated)
			bool const isHole = !m_mapping.sourceOf(pos.value).has_value();
			// can't serve the destination yet
			bool const isParked = destination >= m_data.size();
			if (!isParked)  // the destination can be served
				desiredOffsetByDestination[destination.value] = destination.value;
			if (isParked && isHole)  // parked and already standing in a hole, doesn't need to move (for now)
				desiredOffsetByDestination[destination.value] = pos.value;
			if (isParked && !isHole)  // parked but in the way: needs a free hole to wait in
				parked.push_back(destination.value);
			if (!isParked && isHole)  // a free hole; an occupied one is not on offer for the parked slots below
				holes.push_back(pos.value);
		}

		// after `removeSurplus`, every !parked slot's destination is a distinct offset below the current height,
		// and every non-hole position below the height is the destination of exactly one retained slot.
		// So parked-not-in-hole slots and free holes are of the same magnitude.
		yulAssert(parked.size() == holes.size());
		ranges::sort(parked);
		for (std::size_t i = 0; i < holes.size(); ++i)
			desiredOffsetByDestination[parked[i]] = holes[i];
		return permute(std::move(desiredOffsetByDestination));
	}

	/// Builds the target bottom-up. Every offset either holds its slot already, gets a retained slot that is
	/// floating above it, or gets a freshly generated slot.
	/// Once everything is generated the rest is one final permutation.
	[[nodiscard]] std::optional<Blocked> buildBottomUp()
	{
		for (StackOffset targetOffset{0}; targetOffset < m_target.size(); ++targetOffset.value)
		{
			// all is generated, the final permutation
			if (m_pendingGenerations == 0)
			{
				yulAssert(m_data.size() == m_target.size());
				// every slot goes to the offset it is bound for, so the desired offset of each destination
				// is that destination itself
				return permute(ranges::views::iota(std::size_t{0}, m_target.size()) | ranges::to<std::vector>);
			}

			// a target offset that needs something DUPed urgently before it goes out of dup reach
			std::optional<StackOffset> urgentToDup;
			for (StackOffset offset = targetOffset; offset < m_target.size(); ++offset.value)  // going bottom-up so we can start from targetOffset
			{
				// only slots that are not generated and don't have a source assigned (ie need to be duped) can be urgent
				if (m_mapping.sourceOf(offset.value).has_value() || m_generated[offset.value])
					continue;
				StackSlot const& slot = m_target[offset.value];
				if (slot.isJunk() || canBeFreelyGenerated(slot) || isSpilled(slot, m_spills))
					continue;
				if (auto const sourceCopy = shallowestCopyPosition(slot))
				{
					if (!isDupReachable(*sourceCopy))
						return blockDupUnreachable(*sourceCopy);
					// a copy sitting at the offset being filled floats to the top when it is, so no hurry
					if (depthOf(*sourceCopy).value == m_maxDupDepth && *sourceCopy != targetOffset && !urgentToDup)
						urgentToDup = offset;
				}
			}

			if (
				urgentToDup.has_value() &&  // if there is a slot that is about to go out of dup range but demands more copies
				*urgentToDup != targetOffset &&  // and it's not the target offset anyways
				m_data.size() - targetOffset.value < m_maxSwapDepth  // and duping it doesn't make the target go out of swap range
			)
			{
				// generate it
				if (std::optional<Blocked> blocked = generate(*urgentToDup))
					return blocked;
				// and revisit the current target in the next iteration (with unsigned wrapping this is also fine for 0)
				--targetOffset.value;
				continue;
			}

			// a slot whose offset is exactly where the result of a dup would end up is in place for free,
			// as long as nothing is urgent and targetOffset stays in reach for the slot generated after it
			StackOffset const sourceTop{m_data.size()};
			if (
				!urgentToDup &&  // nothing urgent
				sourceTop > targetOffset &&  // the new top sits above the current targetOffset
				sourceTop < m_target.size()	&&  // the new top is in the target offset range
				!m_mapping.sourceOf(sourceTop.value).has_value() &&   // the target at the source top pos needs a dup
				!m_generated[sourceTop.value] &&  // and it's also not generated
				sourceTop.value - targetOffset.value < m_maxSwapDepth  // targetOffset stays in swap reach
			)
			{
				// we generate the slot demanded at source top
				if (std::optional<Blocked> blocked = generate(sourceTop))
					return blocked;
				// revisit target offset
				--targetOffset.value;
				continue;
			}

			if (
				m_mapping.sourceOf(targetOffset.value).has_value() ||  // there is a proper source slot for the target
				m_generated[targetOffset.value]  // or it's generated (pushed/loaded etc)
			)
			{
				// We go bottom-up, so the slot that should go into targetOffset is somewhere above
				// Any equal slot that is not in place will do the trick: the one at `targetOffset` itself, else the shallowest one
				std::optional<StackOffset> const boundForTarget = m_destination.positionOf(targetOffset);
				yulAssert(
					boundForTarget.has_value() && *boundForTarget >= targetOffset,
					"slot bound for the offset being filled is missing or already below it"
				);
				StackOffset const sourceForTargetOffset = *boundForTarget;
				StackOffset pos = sourceForTargetOffset;
				if (m_data[targetOffset.value] == m_data[sourceForTargetOffset.value])
					// if the slot currently occupying targetOffset happens to be an equal copy of that value we're done
					// and can set `pos` directly to the target offset
					pos = targetOffset;
				else
					// otherwise search if there is an equal, movable copy shallower than carrier
					for (
						StackOffset const candidate: stackOffsets() | ranges::views::reverse | ranges::views::take(depthOf(sourceForTargetOffset).value)
					)
						if (m_data[candidate.value] == m_data[sourceForTargetOffset.value] && !isFinal(candidate))
						{
							pos = candidate;
							break;
						}

				// we picked a valid `pos`
				yulAssert(m_data[pos.value] == m_data[sourceForTargetOffset.value]);

				// update the destinations if needed
				m_destination.swapDestinations(pos, sourceForTargetOffset);

				// we're already done
				if (pos == targetOffset)
					continue;

				// if `pos` is not already at the top of the stack, swap it up
				if (pos.value != m_data.size() - 1)
				{
					if (!isSwapReachable(pos))
						return blockSwapUnreachable(pos);  // bail
					swapWith(pos);
				}
			}
			else
			{
				// the slot needs to be DUPed
				if (std::optional<Blocked> blocked = generate(targetOffset))
					return blocked;

				// `generate` might have already placed the slot into the target offset, then we're done for this offset
				if (isFinal(targetOffset))
					continue;
			}

			// we might have to swap the top down into the target offset
			yulAssert(!isFinal(targetOffset));
			if (targetOffset != m_data.size() - 1)
			{
				if (!isSwapReachable(targetOffset))
					return blockSwapUnreachable(targetOffset);
				swapWith(targetOffset);
			}
		}
		yulAssert(m_data.size() == m_target.size());
		return std::nullopt;
	}

	[[nodiscard]] std::optional<Blocked> emit()
	{
		// removes all surplus (slots that have no destination in the target)
		if (std::optional<Blocked> blocked = removeSurplus())
			return blocked;
		if (std::optional<Blocked> blocked = permuteAtLowestHeight())
			return blocked;
		return buildBottomUp();
	}

	/// Brings every slot to the offset `_desiredOffsetByDestination` assigns to its destination
	[[nodiscard]] std::optional<Blocked> permute(std::vector<std::size_t> _desiredOffsetByDestination)
	{
		// the desired offset of the slot at `_pos`, looked up under the destination it is bound for
		auto const desiredOf = [&](std::size_t const _pos) -> std::size_t& {
			Destination const& destination = destinationOf(StackOffset{_pos});
			// every slot has a destination at this point, surplus is gone before any permutation
			yulAssert(destination.has_value());
			return _desiredOffsetByDestination[destination->value];
		};

		// Equal slots are interchangeable: among them, those already at one of their desired offsets stay, the
		// others take the remaining offsets. Otherwise equal slots would pass each other in cycles.
		{
			std::vector<std::size_t> positions = ranges::views::iota(std::size_t{0}, m_data.size()) | ranges::to<std::vector>;
			// stability needed for reproducibility across platforms (sort doesn't guarantee order for ties)
			// also it keeps chunks of positions that belong to same data in ascending order
			ranges::stable_sort(
				positions,
				[&](std::size_t const _a, std::size_t const _b) { return m_data[_a] < m_data[_b]; }
			);
			for (ranges::subrange const group: positions | ranges::views::chunk_by(
				[&](std::size_t const _a, std::size_t const _b) { return m_data[_a] == m_data[_b]; }
			))
			{
				// the group is empty or contains only a single element (ie no second, equal slot exists)
				if (ranges::distance(group) < 2)
					continue;
				// the group's desired offsets
				std::vector<std::size_t> desired = group | ranges::views::transform(desiredOf) | ranges::to<std::vector>;
				// sorted needed for set operations
				ranges::sort(desired);
				// slots already standing on one of the group's desired offsets stay put
				// stayers = group \cap desired => desiredOf(pos) = pos
				std::vector<std::size_t> stayers;
				ranges::set_intersection(group, desired, std::back_inserter(stayers));
				for (std::size_t const pos: stayers)
					desiredOf(pos) = pos;
				// the others take the vacant offsets, both sides ascending, so nothing crosses
				// movers = group ∖ desired
				std::vector<std::size_t> movers;
				ranges::set_difference(group, desired, std::back_inserter(movers));
				// vacant = desired ∖ group => desiredOf(movers[i]) = vacant[i]
				std::vector<std::size_t> vacant;
				ranges::set_difference(desired, group, std::back_inserter(vacant));

				// desired is built with exactly one entry per group member, so |G| = |D|, i.e.,
				// |movers| =  |G ∖ D| = |G| − |G \cap D| and |vacant| = |D ∖ G| = |D| − |D \cap G|
				yulAssert(movers.size() == vacant.size());
				for (std::size_t i = 0; i < movers.size(); ++i)
					desiredOf(movers[i]) = vacant[i];
			}
		}

		while (true)
		{
			std::size_t const top = m_data.size() - 1;
			if (
				StackOffset const desiredOfTop{desiredOf(top)};
				desiredOfTop != top
			)
			{
				// an equal slot standing at the desired offset serves the top's destination just as well:
				// exchange the destinations instead of swapping two indistinguishable slots
				if (m_data[desiredOfTop.value] == m_data[top])
				{
					m_destination.swapDestinations(desiredOfTop, StackOffset{top});
					continue;
				}
				if (!isSwapReachable(desiredOfTop))
					return blockSwapUnreachable(desiredOfTop);
				swapWith(desiredOfTop);
				continue;
			}
			StackOffset misplaced{empty};
			for (std::size_t const pos: ranges::views::iota(std::size_t{0}, top) | ranges::views::reverse)
				if (desiredOf(pos) != pos)  // not already in place
				{
					misplaced = StackOffset{pos};  // we found something misplaced
					break;
				}

			if (misplaced == empty)
				return std::nullopt;  // nothing misplaced was found, we're done

			// a misplaced slot equal to the top takes over the top's destination, making the top the misplaced
			// one; it then travels to its destination directly instead of dislodging an in-place slot
			if (m_data[misplaced.value] == m_data[top])
			{
				m_destination.swapDestinations(misplaced, StackOffset{top});
				continue;
			}

			if (!isSwapReachable(misplaced))
				return blockSwapUnreachable(misplaced);  // can't reach it, abort

			swapWith(misplaced);
		}
	}

	/// Whether the slot at `_pos` has no target offset to go to and is to be popped
	bool isSurplus(StackOffset const _pos) const
	{
		return !destinationOf(_pos).has_value();
	}

	/// The destination of the slot at `_pos`: the target offset it is bound for, or none for a surplus slot.
	/// Whether absence is expected is the caller's business - after `removeSurplus` every slot has one.
	Destination const& destinationOf(StackOffset const _pos) const
	{
		return m_destination.destinationOf(_pos);
	}

	/// Whether the slot at `_pos` is bound for `_pos` itself, i.e., already is at its final target offset
	bool isFinal(StackOffset const _pos) const
	{
		return destinationOf(_pos) == _pos;
	}

	/// Depth of the slot at `_pos` below the top of the working stack
	StackDepth depthOf(StackOffset const _pos) const
	{
		return m_stack.offsetToDepth(_pos);
	}

	/// Whether a swap can reach the slot at `_pos`; trivially true for the top itself
	bool isSwapReachable(StackOffset const _pos) const
	{
		return !m_stack.isBeyondSwapRange(depthOf(_pos));
	}

	/// Whether a dup can reach the slot at `_pos`
	bool isDupReachable(StackOffset const _pos) const
	{
		return m_stack.dupReachable(depthOf(_pos));
	}

	std::optional<StackOffset> shallowestCopyPosition(StackSlot const& _slot) const
	{
		for (StackOffset const pos: stackOffsets() | ranges::views::reverse)
			if (m_data[pos.value] == _slot)
				return pos;
		return std::nullopt;
	}

	/// Swaps the top with the slot at `_pos`, the destinations traveling along
	void swapWith(StackOffset const _pos)
	{
		m_stack.swap(_pos);
		m_destination.swapDestinations(_pos, StackOffset{m_data.size() - 1});
	}

	void pop()
	{
		m_stack.pop();
		m_destination.pop();
	}

	void push(StackSlot const& _slot, StackOffset const _destination)
	{
		m_stack.push(_slot);
		m_destination.push(_destination);
	}

	void dup(StackOffset const _copy, StackOffset const _destination)
	{
		m_stack.dup(_copy);
		m_destination.push(_destination);
	}

	/// The slot at `_position` is out of reach by `_excess` slots
	[[nodiscard]] Blocked block(StackOffset const _position, std::size_t const _excess) const
	{
		yulAssert(_excess > 0);
		return Blocked{_position, _excess};
	}

	/// A swap cannot reach the slot at `_position`
	[[nodiscard]] Blocked blockSwapUnreachable(StackOffset const _position) const
	{
		return block(_position, depthOf(_position).value - m_maxSwapDepth);
	}

	/// A dup cannot reach the slot at `_position`
	[[nodiscard]] Blocked blockDupUnreachable(StackOffset const _position) const
	{
		return block(_position, depthOf(_position).value - m_maxDupDepth);
	}

	StackData const& m_target;
	Mapping const& m_mapping;
	spill::SpillSet const& m_spills;
	std::size_t const m_maxSwapDepth;
	std::size_t const m_maxDupDepth;

	/// The working stack and the destination of each of its slots - always of equal size
	StackData m_data;
	DestinationMap m_destination;
	/// Whether the slot for each target offset has been produced already
	std::vector<std::uint8_t> m_generated;
	/// Number of target offsets whose slot still has to be produced; decremented by `produce`
	std::size_t m_pendingGenerations = 0;
	ShuffleTrace m_trace;
	Stack m_stack{m_data, &m_trace, m_maxSwapDepth};
};

class Planner
{
public:
	Planner(
		StackData const& _source,
		StackData const& _target,
		spill::SpillSet _spills,
		bool const _spillingAllowed,
		Mapping::WildcardSlotsStrategy const _wildcardSlotsStrategy,
		std::size_t const _reachableStackDepth
	):
		m_maxSwapDepth(_reachableStackDepth),
		m_maxDupDepth(_reachableStackDepth - 1),
		m_source(_source),
		m_target(_target),
		m_spills(std::move(_spills)),
		m_spillingAllowed(_spillingAllowed),
		m_wildcardSlotsStrategy(_wildcardSlotsStrategy),
		m_dropped(_source.size(), false)
	{}

	bool run()
	{
		for (StackSlot const& slot: m_target)
			yulAssert(
				slot.isJunk() ||
				canBeFreelyGenerated(slot) ||
				ranges::contains(m_source, slot) ||
				isSpilled(slot, m_spills),
				"target slot neither on the stack nor generatable"
			);

		while (true)
		{
			Mapping mapping(m_source, m_target, m_dropped, m_wildcardSlotsStrategy);
			m_sawWildcardCopy |= mapping.sawWildcardCopy();
			Emission::Result result = Emission{m_source, m_target, mapping, m_spills, m_maxSwapDepth, m_maxDupDepth}.run();
			if (!result.blocked.has_value())
			{
				m_result = std::move(result);
				m_plan = std::move(mapping).release();
				return true;
			}
			if (!resolve(mapping, *result.blocked, result.destinations))
				return false;
		}
	}

	bool sawWildcardCopy() const { return m_sawWildcardCopy; }
	/// Spills the successful plan ends up with, including ones it planned itself
	std::size_t numSpills() const { return m_spills.numSpilled(); }
	/// Stack operations in the successful attempt's trace
	std::size_t numOps() const
	{
		yulAssert(m_result.has_value());
		return m_result->trace.size();
	}

	/// Moves the successful attempt into `_source` and `_spills` and hands out the plan
	[[nodiscard]] ShuffleResult apply(StackData& _source, spill::SpillSet& _spills) &&
	{
		yulAssert(m_result.has_value());
		_source = std::move(m_result->data);
		_spills = std::move(m_spills);
		return {
			.status = ShuffleResult::Status::Admissible,
			.trace = std::move(m_result->trace),
			.sourceOf = std::move(m_plan)
		};
	}

private:
	/// Drops slots above the blocked one until it is within reach, cheapest regeneration first: slots whose
	/// target is a wildcard, freely generatable ones, spilled ones, values with another retained copy, and
	/// finally values that get a planned spill. Function return labels cannot be regenerated and are never
	/// dropped.
	bool resolve(
		Mapping const& _mapping,
		Emission::Blocked const& _blocked,
		std::vector<Destination> const& _destinations
	)
	{
		std::vector<std::uint8_t> droppedNow(m_source.size(), false);

		auto const isRetained = [&](StackOffset const _sourceOffset) {
			return _mapping.targetOf(_sourceOffset.value).has_value();
		};

		auto const hasOtherRetainedCopy = [&](StackOffset const _offset) {
			for (StackOffset sourceOffset{0}; sourceOffset < m_source.size(); ++sourceOffset.value)
				if (
					sourceOffset != _offset &&
					m_source[sourceOffset.value] == m_source[_offset.value] &&  // same value
					isRetained(sourceOffset) &&
					!droppedNow[sourceOffset.value]  // not yet dropped
				)
					return true;
			return false;
		};

		// this slot is needed somewhere in the target
		auto const isDemanded = [&](StackSlot const& _slot) {
			return ranges::any_of(
				m_target,
				[&](StackSlot const& slot) { return !slot.isJunk() && slot == _slot; }
			);
		};

		/// How the slot at `_offset` can be regenerated after dropping it:
		///	- 0 = not needed or for free,
		///	- 1 = pushed,
		/// - 2 = reloaded from existing spill,
		/// - 3 = duplicated from another copy,
		/// - 4 = reloaded after a planned spill
		auto const classify = [&](StackOffset const _offset) -> std::optional<unsigned> {
			StackSlot const& slot = m_source[_offset.value];
			if (slot.isFunctionReturnLabel())
				return std::nullopt;
			bool const regenerable = slot.isJunk() || canBeFreelyGenerated(slot) || isSpilled(slot, m_spills) || hasOtherRetainedCopy(_offset);
			if (m_target[*_mapping.targetOf(_offset.value)].isJunk() && (regenerable || !isDemanded(slot)))
				return 0;
			if (canBeFreelyGenerated(slot))
				return 1;
			if (isSpilled(slot, m_spills))
				return 2;
			if (hasOtherRetainedCopy(_offset))
				return 3;
			if (m_spillingAllowed && slot.isValue() && !slot.isLiteralValue())
				return 4;
			return std::nullopt;
		};

		// retained slots above the blocked one; the source offset of a retained slot is the one its destination
		// maps back to, while generated and surplus slots have no source
		std::vector<StackOffset> candidates;
		for (StackOffset pos{_blocked.offset.value + 1}; pos < _destinations.size(); ++pos.value)
			if (_destinations[pos.value].has_value())
				if (std::optional<std::size_t> const source = _mapping.sourceOf(_destinations[pos.value]->value))
					candidates.emplace_back(StackOffset{*source});
		for (std::size_t dropped = 0; dropped < _blocked.excess; ++dropped)
		{
			// classes can shift as copies get dropped, so pick one victim at a time
			std::optional<StackOffset> best;
			unsigned bestClass = 0;
			for (StackOffset const offset: candidates)
			{
				if (droppedNow[offset.value])
					continue;
				auto const cls = classify(offset);
				if (!cls.has_value())
					continue;
				// within a class prefer the slot whose target is highest
				if (
					!best.has_value() ||
					*cls < bestClass ||
					(*cls == bestClass && *_mapping.targetOf(offset.value) > *_mapping.targetOf(best->value))
				)
				{
					best = offset;
					bestClass = *cls;
				}
			}
			if (!best.has_value())
				return false;  // couldn't drop anything
			if (bestClass == 4)
				m_spills.add(m_source[best->value].value());
			droppedNow[best->value] = true;
			m_dropped[best->value] = true;
		}
		return true;
	}

	/// Deepest slot a swap may address: SWAP16 swaps with depth 16, SWAPN with up to `deepReachableStackDepth`
	std::size_t const m_maxSwapDepth;
	/// Deepest slot a dup may address: DUP16 duplicates the slot at depth 15
	std::size_t const m_maxDupDepth;

	StackData const& m_source;
	StackData const& m_target;
	spill::SpillSet m_spills;
	bool const m_spillingAllowed;
	Mapping::WildcardSlotsStrategy const m_wildcardSlotsStrategy;
	bool m_sawWildcardCopy = false;
	std::vector<std::uint8_t> m_dropped;

	/// The successful attempt's outputs; set when `run` returns true
	std::optional<Emission::Result> m_result;
	/// The successful attempt's mapping of target offsets to the source offsets serving them
	std::vector<std::optional<std::size_t>> m_plan;
};

/// Plans the shuffle under both wildcard policies when the choice matters: copies sitting in wildcard slots are
/// first left there and, if any demand was answered by such a copy, planned again taking them. The cheaper
/// successful plan (fewest spills, then fewest operations) is committed into the source stack and spill set; on
/// `StackTooDeep` neither policy admitted a plan and both are left untouched.
class Shuffle
{
public:
	Shuffle(
		StackData& _source,
		StackData const& _target,
		spill::SpillSet& _spills,
		bool const _spillingAllowed,
		std::size_t const _reachableStackDepth
	):
		m_source(_source),
		m_target(_target),
		m_spills(_spills),
		m_spillingAllowed(_spillingAllowed),
		m_reachableStackDepth(_reachableStackDepth)
	{}

	Shuffle(Shuffle const&) = delete;
	Shuffle(Shuffle&&) = delete;
	Shuffle& operator=(Shuffle const&) = delete;
	Shuffle& operator=(Shuffle&&) = delete;

	[[nodiscard]] ShuffleResult run() const&&
	{
		Planner leave{m_source, m_target, m_spills, m_spillingAllowed, Mapping::WildcardSlotsStrategy::Leave, m_reachableStackDepth};
		bool const leaveOk = leave.run();
		if (!leave.sawWildcardCopy())
			return leaveOk ? std::move(leave).apply(m_source, m_spills) : stackTooDeep();
		Planner take{m_source, m_target, m_spills, m_spillingAllowed, Mapping::WildcardSlotsStrategy::Take, m_reachableStackDepth};
		bool const takeOk = take.run();
		if (!leaveOk && !takeOk)
			return stackTooDeep();
		bool const useTake =
			!leaveOk ||
			(takeOk && std::make_pair(take.numSpills(), take.numOps()) < std::make_pair(leave.numSpills(), leave.numOps()));
		return std::move(useTake ? take : leave).apply(m_source, m_spills);
	}

private:
	static ShuffleResult stackTooDeep() { return {.status = ShuffleResult::Status::StackTooDeep}; }

	StackData& m_source;
	StackData const& m_target;
	spill::SpillSet& m_spills;
	bool const m_spillingAllowed;
	std::size_t const m_reachableStackDepth;
};

}

ShuffleResult stack::shuffle(
	StackData& _source,
	StackData const& _target,
	spill::SpillSet& _spills,
	bool const _spillingAllowed,
	std::size_t const _reachableStackDepth
)
{
	yulAssert(2 <= _reachableStackDepth);
	return Shuffle{_source, _target, _spills, _spillingAllowed, _reachableStackDepth}.run();
}
