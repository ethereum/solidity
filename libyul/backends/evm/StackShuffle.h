
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

#include <libsolutil/Numeric.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/enumerate.hpp>

namespace solidity::yul
{

class IndexingMap
{
public:
	size_t operator[](StackSlot const& _slot)
	{
		if (auto* p = std::get_if<FunctionCallReturnLabelSlot>(&_slot))
			return getIndex(m_functionCallReturnLabelSlotIndex, *p);
		if (std::holds_alternative<FunctionReturnLabelSlot>(_slot))
		{
			m_indexedSlots[1] = _slot;
			return 1;
		}
		if (auto* p = std::get_if<VariableSlot>(&_slot))
			return getIndex(m_variableSlotIndex, *p);
		if (auto* p = std::get_if<LiteralSlot>(&_slot))
			return getIndex(m_literalSlotIndex, *p);
		if (auto* p = std::get_if<TemporarySlot>(&_slot))
			return getIndex(m_temporarySlotIndex, *p);
		m_indexedSlots[0] = _slot;
		return 0;
	}
	std::vector<StackSlot> indexedSlots()
	{
		return std::move(m_indexedSlots);
	}
private:
	template<typename MapType, typename ElementType>
	size_t getIndex(MapType&& _map, ElementType&& _element)
	{
		auto [element, newlyInserted] = _map.emplace(std::make_pair(_element, size_t(0u)));
		if (newlyInserted)
		{
			element->second = m_indexedSlots.size();
			m_indexedSlots.emplace_back(_element);
		}
		return element->second;
	}
	std::map<FunctionCallReturnLabelSlot, size_t> m_functionCallReturnLabelSlotIndex;
	std::map<VariableSlot, size_t> m_variableSlotIndex;
	std::map<LiteralSlot, size_t> m_literalSlotIndex;
	std::map<TemporarySlot, size_t> m_temporarySlotIndex;
	std::vector<StackSlot> m_indexedSlots{JunkSlot{}, JunkSlot{}};
};

using IndexedStack = std::vector<size_t>;

// ShuffleOperations interface
// Abstraction of stack shuffling operations. Used as an interface for the stack shuffler.
// The shuffle operation class is expected to internally keep track of a current stack layout (the "source layout")
// that the shuffler is supposed to shuffle to a fixed target stack layout.
// The shuffler works iteratively. At each iteration it calls the shuffle operations implementation and
// queries it for various information about the current source stack layout and the target layout, as described
// in the interface below.
// Based on that information the shuffler decides which is the next optimal operation to perform on the stack
// and calls the corresponding entry point in the shuffling operations (swap, pushOrDupTarget or pop).
class ShuffleOperations {
public:
	virtual ~ShuffleOperations() {}
	virtual void updateMultiplicity() = 0;
	// Returns true, iff the current slot at sourceOffset in source layout is a suitable slot at targetOffset.
	virtual bool isCompatible(size_t _source, size_t _target) = 0;
	// Returns true, iff the slots at the two given source offsets are identical.
	virtual bool sourceIsSame(size_t _lhs, size_t _rhs) = 0;
	// Returns a positive integer n, if the slot at the given source offset needs n more copies.
	// Returns a negative integer -n, if the slot at the given source offsets occurs n times too many.
	// Returns zero if the amount of occurrences, in the current source layout, of the slot at the given source offset
	// matches the desired amount of occurrences in the target.
	virtual int sourceMultiplicity(size_t _offset) = 0;
	// Returns a positive integer n, if the slot at the given target offset needs n more copies.
	// Returns a negative integer -n, if the slot at the given target offsets occurs n times too many.
	// Returns zero if the amount of occurrences, in the current source layout, of the slot at the given target offset
	// matches the desired amount of occurrences in the target.
	virtual int targetMultiplicity(size_t _offset) = 0;
	// Returns true, iff any slot is compatible with the given target offset.
	virtual bool targetIsArbitrary(size_t _offset) = 0;
	// Returns the number of slots in the source layout.
	virtual size_t sourceSize() = 0;
	// Returns the number of slots in the target layout.
	virtual size_t targetSize() = 0;
	// Swaps the top most slot in the source with the slot `depth` slots below the top.
	// In terms of EVM opcodes this is supposed to be a `SWAP<depth>`.
	// In terms of vectors this is supposed to be `std::swap(source.at(source.size() - depth - 1, source.top))`.
	virtual void swap(size_t _depth) = 0;
	// Pops the top most slot in the source, i.e. the slot at offset ops.sourceSize() - 1.
	// In terms of EVM opcodes this is `POP`.
	// In terms of vectors this is `source.pop();`.
	virtual void pop() = 0;
	// Dups or pushes the slot that is supposed to end up at the given target offset.
	virtual void pushOrDupTarget(size_t _offset) = 0;
};


struct PreviousSlot { size_t slot; };
using SymbolicStackLayout = std::vector<std::variant<PreviousSlot, size_t>>;

// Performs symbolic stack shuffling on top of the regular stack slots
class SymbolicStackShuffleOperations : public ShuffleOperations
{
public:
	SymbolicStackShuffleOperations(
		SymbolicStackLayout& _stackLayout,
		IndexedStack& _targetStack,
		std::function<bool(size_t)> _generateSlotOnTheFly,
		size_t _numSlots
	) :
		m_stackLayout(_stackLayout),
		m_targetStack(_targetStack),
		m_generateSlotOnTheFly(_generateSlotOnTheFly),
		m_numSlots(_numSlots)
	{}

	// TODO: get rid of multiplicity recalculation on every step and move it to IndexedStack
	void updateMultiplicity()
	{
		std::vector<int> multiplicity(m_numSlots, 0);
		for (auto const& layoutSlot: m_stackLayout)
			if (size_t const* slot = std::get_if<size_t>(&layoutSlot))
			{
				m_operationOutputs.insert(*slot);
				--multiplicity[*slot];
			}

		for (auto&& slot: m_targetStack)
			++multiplicity[slot];
		m_multiplicity = multiplicity;
	}

	bool isCompatible(size_t _source, size_t _target)
	{
		return
			_source < m_stackLayout.size() &&
			_target < m_targetStack.size() &&
			(
				m_junkIndex == m_targetStack.at(_target) ||
				std::visit(util::GenericVisitor{
					[&](PreviousSlot const&) {
						return !m_operationOutputs.count(m_targetStack.at(_target)) && !m_generateSlotOnTheFly(m_targetStack.at(_target));
					},
					[&](size_t const& _s) { return _s == m_targetStack.at(_target); }
				}, m_stackLayout.at(_source))
			);
	}

	bool sourceIsSame(size_t _lhs, size_t _rhs)
	{
		return std::visit(util::GenericVisitor{
			[&](PreviousSlot const&, PreviousSlot const&) { return true; },
			[&](size_t const& _lhs, size_t const& _rhs) { return _lhs == _rhs; },
			[&](auto const&, auto const&) { return false; }
		}, m_stackLayout.at(_lhs), m_stackLayout.at(_rhs));
	}

	int sourceMultiplicity(size_t _offset)
	{
		return std::visit(util::GenericVisitor{
			[&](PreviousSlot const&) { return 0; },
			[&](size_t _s) { return m_multiplicity.at(_s); }
		}, m_stackLayout.at(_offset));
	}

	int targetMultiplicity(size_t _offset)
	{
		if (!m_operationOutputs.count(m_targetStack.at(_offset)) && !m_generateSlotOnTheFly(m_targetStack.at(_offset)))
			return 0;
		return m_multiplicity.at(m_targetStack.at(_offset));
	}

	bool targetIsArbitrary(size_t _offset)
	{
		return _offset < m_targetStack.size() && m_junkIndex == m_targetStack.at(_offset);
	}

	void swap(size_t _i)
	{
		yulAssert(
			!std::holds_alternative<PreviousSlot>(m_stackLayout.at(m_stackLayout.size() - _i - 1)) ||
			!std::holds_alternative<PreviousSlot>(m_stackLayout.back()), ""
		);
		std::swap(m_stackLayout.at(m_stackLayout.size() -  _i - 1), m_stackLayout.back());
	}

	size_t sourceSize() { return m_stackLayout.size(); }

	size_t targetSize() { return m_targetStack.size(); }

	void pop() { m_stackLayout.pop_back(); }

	void pushOrDupTarget(size_t _offset)
	{
		m_stackLayout.push_back(m_targetStack.at(_offset));
	}

private:
	SymbolicStackLayout& m_stackLayout;
	IndexedStack& m_targetStack;
	std::function<bool(size_t)> m_generateSlotOnTheFly;
	size_t m_numSlots;
	std::set<size_t> m_operationOutputs{};
	std::vector<int> m_multiplicity;
	size_t m_junkIndex = std::numeric_limits<size_t>::max();
};

// Performs stack shuffling over stack slots
template<typename Swap, typename PushOrDup, typename Pop>
class StackShuffleOperations : public ShuffleOperations
{
public:
	StackShuffleOperations(
		IndexedStack& _currentStack,
		IndexedStack const& _targetStack,
		Swap _swap,
		PushOrDup _pushOrDup,
		Pop _pop,
		size_t _numSlots,
		size_t _junkIndex
	):
		m_currentStack(_currentStack),
		m_targetStack(_targetStack),
		m_swap(_swap),
		m_pushOrDup(_pushOrDup),
		m_pop(_pop),
		m_numSlots(_numSlots),
		m_junkIndex(_junkIndex)
	{}

	void updateMultiplicity()
	{
		std::vector<int> multiplicity(m_numSlots, 0);
		for (auto const& slot: m_currentStack)
			--multiplicity[slot];
		for (auto&& [offset, slot]: m_targetStack | ranges::views::enumerate)
			if (slot == m_junkIndex && offset < m_currentStack.size())
				++multiplicity[m_currentStack.at(offset)];
			else
				++multiplicity[slot];
		m_multiplicity = multiplicity;
	}

	bool isCompatible(size_t _source, size_t _target)
	{
		return
			_source < m_currentStack.size() &&
			_target < m_targetStack.size() &&
			(
				m_junkIndex == m_targetStack.at(_target) ||
				m_currentStack.at(_source) == m_targetStack.at(_target)
			);
	}

	bool sourceIsSame(size_t _lhs, size_t _rhs)
	{
		return m_currentStack.at(_lhs) == m_currentStack.at(_rhs);
	}

	int sourceMultiplicity(size_t _offset)
	{
		return m_multiplicity.at(m_currentStack.at(_offset));
	}

	int targetMultiplicity(size_t _offset)
	{
		return m_multiplicity.at(m_targetStack.at(_offset));
	}

	bool targetIsArbitrary(size_t offset)
	{
		return offset < m_targetStack.size() && m_junkIndex == m_targetStack.at(offset);
	}

	void swap(size_t _i)
	{
		m_swap(static_cast<unsigned>(_i));
		std::swap(m_currentStack.at(m_currentStack.size() - _i - 1), m_currentStack.back());
	}

	size_t sourceSize() { return m_currentStack.size(); }

	size_t targetSize() { return m_targetStack.size(); }

	void pop()
	{
		m_pop();
		m_currentStack.pop_back();
	}

	void pushOrDupTarget(size_t _offset)
	{
		auto const& targetSlot = m_targetStack.at(_offset);
		m_pushOrDup(targetSlot);
		m_currentStack.push_back(targetSlot);
	}

private:
	IndexedStack& m_currentStack;
	IndexedStack const& m_targetStack;
	std::vector<int> m_multiplicity;
	Swap m_swap;
	PushOrDup m_pushOrDup;
	Pop m_pop;
	size_t m_numSlots;
	size_t m_junkIndex = std::numeric_limits<size_t>::max();
};

}
