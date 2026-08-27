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

#include <libyul/backends/evm/ssa/ShuffleTrace.h>
#include <libyul/backends/evm/ssa/StackSlot.h>

#include <range/v3/algorithm/find.hpp>
#include <range/v3/view/reverse.hpp>

#include <cstddef>

namespace solidity::yul::ssa
{

/// A view over a `StackData` mimicking EVM stack semantics. When constructed with a trace, every stack
/// manipulation is recorded as a `ShuffleOp`; an untraced view mutates the data silently.
/// `_reachableStackDepth` is the deepest slot a swap may address
class Stack
{
public:
	using Slot = StackSlot;
	using Data = StackData;
	using Depth = StackDepth;
	using Offset = StackOffset;

	explicit Stack(
		Data& _data,
		ShuffleTrace* _trace = nullptr,
		std::size_t const _reachableStackDepth = reachableStackDepth
	):
		m_data(&_data),
		m_trace(_trace),
		m_reachableStackDepth(_reachableStackDepth)
	{}

	Slot const& top() const
	{
		yulAssert(!m_data->empty());
		return m_data->back();
	}

	void swap(Depth const& _depth) { swap(depthToOffset(_depth)); }
	void swap(Offset const& _offset)
	{
		yulAssert(isValidSwapTarget(_offset), "Stack too deep");
		std::swap((*m_data)[_offset.value], m_data->back());
		if (m_trace)
			m_trace->push_back(ShuffleOp::swap(offsetToDepth(_offset)));
	}

	void pop()
	{
		yulAssert(!m_data->empty());
		m_data->pop_back();
		if (m_trace)
			m_trace->push_back(ShuffleOp::pop());
	}

	void push(Slot const& _slot)
	{
		yulAssert(!_slot.isFunctionReturnLabel(), "Cannot push function return label");
		m_data->emplace_back(_slot);
		if (m_trace)
			m_trace->push_back(
				// a pushed non-literal value can only be a spill reload
				_slot.isValue() && !_slot.isLiteralValue() ? ShuffleOp::load(_slot) : ShuffleOp::push(_slot)
			);
	}

	void dup(Depth const& _depth) { dup(depthToOffset(_depth)); }
	void dup(Offset const& _offset)
	{
		auto const depth = offsetToDepth(_offset);
		yulAssert(dupReachable(depth), "Stack too deep");
		auto const slot = (*m_data)[_offset.value];
		yulAssert(!slot.isFunctionReturnLabel(), "Cannot dup function return label");
		m_data->push_back(slot);
		if (m_trace)
			m_trace->push_back(ShuffleOp::dup(Depth{depth.value + 1}));
	}

	bool dupReachable(Offset const& _offset) const noexcept { return dupReachable(offsetToDepth(_offset)); }
	bool dupReachable(Depth const& _depth) const noexcept { return _depth < size() && _depth.value + 1 <= m_reachableStackDepth; }
	bool isValidSwapTarget(Offset const& _offset) const noexcept { return isValidSwapTarget(offsetToDepth(_offset)); }
	bool isValidSwapTarget(Depth const& _depth) const noexcept { return _depth < size() && 1 <= _depth.value && _depth.value <= m_reachableStackDepth; }
	bool isBeyondSwapRange(Offset const& _offset) const noexcept { return isBeyondSwapRange(offsetToDepth(_offset)); }
	bool isBeyondSwapRange(Depth const& _depth) const noexcept { return _depth > m_reachableStackDepth; }

	void declareJunk(Offset const& _offset) { (*m_data)[_offset.value] = Slot::makeJunk(); }
	void declareJunk(Depth const& _depth) { declareJunk(depthToOffset(_depth)); }

	Slot const& slot(Depth const& _depth) const { return (*m_data)[depthToOffset(_depth).value]; }
	Slot const& slot(Offset const& _offset) const { return slot(offsetToDepth(_offset)); }
	bool empty() const noexcept { return size() == 0; }
	size_t size() const noexcept { return m_data->size(); }

	std::optional<Depth> findSlotDepth(Slot const& _value) const
	{
		auto rview = *this | ranges::views::reverse;
		auto it = ranges::find(rview, _value);

		if (it == ranges::end(rview))
			return std::nullopt;

		return Depth{static_cast<size_t>(std::distance(ranges::begin(rview), it))};
	}

	Slot const& operator[](Offset const& _index) const noexcept { return (*m_data)[_index.value]; }
	Data::const_iterator begin() const { return m_data->begin(); }
	Data::const_iterator end() const { return m_data->end(); }

	Data const& data() const
	{
		return *m_data;
	}

	/// index scheme conversion offset -> depth
	Depth offsetToDepth(Offset const& _offset) const
	{
		yulAssert(_offset < size(), "Offset out of range");
		return Depth{size() - _offset.value - 1};
	}
	/// index scheme conversion depth -> offset
	Offset depthToOffset(Depth const& _depth) const
	{
		yulAssert(_depth < size(), "Depth out of range");
		return Offset{size() - _depth.value - 1};
	}

private:
	Data* m_data;
	ShuffleTrace* m_trace;
	std::size_t m_reachableStackDepth;
};

}
