/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file RangeMask.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <map>
#include <utility>
#include <vector>
#include <iterator>
#include <iostream>
#include <assert.h>

namespace dev
{

class RLPStream;

using UnsignedRange = std::pair<unsigned, unsigned>;
using UnsignedRanges = std::vector<UnsignedRange>;

/**
 * Set of elements of a certain "ground range" representable by unions of ranges inside this
 * ground range.
 * Ranges are given as pairs (begin, end), denoting the interval [begin, end), i.e. end is excluded.
 * Supports set-theoretic operators, size and iteration.
 */
template <class T>
class RangeMask
{
	template <class U> friend std::ostream& operator<<(std::ostream& _out, RangeMask<U> const& _r);

public:
	using Range = std::pair<T, T>;
	using Ranges = std::vector<Range>;

	/// Constructs an empty range mask with empty ground range.
	RangeMask(): m_all(0, 0) {}
	/// Constructs an empty range mask with ground range [_begin, _end).
	RangeMask(T _begin, T _end): m_all(_begin, _end) {}
	/// Constructs an empty range mask with ground range _c.
	RangeMask(Range const& _c): m_all(_c) {}

	/// @returns the union with the range mask _m, taking also the union of the ground ranges.
	RangeMask unionedWith(RangeMask const& _m) const { return operator+(_m); }
	RangeMask operator+(RangeMask const& _m) const { return RangeMask(*this) += _m; }

	/// @returns a new range mask containing the smallest _items elements (not ranges).
	RangeMask lowest(decltype(T{} - T{}) _items) const
	{
		RangeMask ret(m_all);
		for (auto i = m_ranges.begin(); i != m_ranges.end() && _items; ++i)
			_items -= (ret.m_ranges[i->first] = std::min(i->first + _items, i->second)) - i->first;
		return ret;
	}

	/// @returns the complement of the range mask relative to the ground range.
	RangeMask operator~() const { return inverted(); }

	/// @returns a copy of this range mask representing the complement relative to the ground range.
	RangeMask inverted() const
	{
		RangeMask ret(m_all);
		T last = m_all.first;
		for (auto i: m_ranges)
		{
			if (i.first != last)
				ret.m_ranges[last] = i.first;
			last = i.second;
		}
		if (last != m_all.second)
			ret.m_ranges[last] = m_all.second;
		return ret;
	}

	/// Changes the range mask to its complement relative to the ground range and returns a
	/// reference to itself.
	RangeMask& invert() { return *this = inverted(); }

	template <class S> RangeMask operator-(S const& _m) const { auto ret = *this; return ret -= _m; }
	template <class S> RangeMask& operator-=(S const& _m) { return invert().unionWith(_m).invert(); }

	RangeMask& operator+=(RangeMask const& _m) { return unionWith(_m); }

	RangeMask& unionWith(RangeMask const& _m)
	{
		m_all.first = std::min(_m.m_all.first, m_all.first);
		m_all.second = std::max(_m.m_all.second, m_all.second);
		for (auto const& i: _m.m_ranges)
			unionWith(i);
		return *this;
	}
	RangeMask& operator+=(Range const& _m) { return unionWith(_m); }
	/// Modifies this range mask to also include the range _m, which has to be a subset of
	/// the ground range.
	RangeMask& unionWith(Range const& _m);

	/// Adds the single element _i to the range mask.
	RangeMask& operator+=(T _m) { return unionWith(_m); }
	/// Adds the single element _i to the range mask.
	RangeMask& unionWith(T _i)
	{
		return operator+=(Range(_i, _i + 1));
	}

	bool contains(T _i) const
	{
		auto it = m_ranges.upper_bound(_i);
		if (it == m_ranges.begin())
			return false;
		return (--it)->second > _i;
	}

	bool empty() const
	{
		return m_ranges.empty();
	}

	bool full() const
	{
		return m_all.first == m_all.second || (m_ranges.size() == 1 && m_ranges.begin()->first == m_all.first && m_ranges.begin()->second == m_all.second);
	}

	void clear()
	{
		m_ranges.clear();
	}

	void reset()
	{
		m_ranges.clear();
		m_all = std::make_pair(0, 0);
	}

	/// @returns the ground range.
	std::pair<T, T> const& all() const { return m_all; }
	/// Extends the ground range to include _i.
	void extendAll(T _i) { m_all = std::make_pair(std::min(m_all.first, _i), std::max(m_all.second, _i + 1)); }

	class const_iterator: public std::iterator<std::forward_iterator_tag, T>
	{
		friend class RangeMask;

	public:
		const_iterator() {}

		T operator*() const { return m_value; }
		const_iterator& operator++() { if (m_owner) m_value = m_owner->next(m_value); return *this; }
		const_iterator operator++(int) { auto ret = *this; if (m_owner) m_value = m_owner->next(m_value); return ret; }

		bool operator==(const_iterator const& _i) const { return m_owner == _i.m_owner && m_value == _i.m_value; }
		bool operator!=(const_iterator const& _i) const { return !operator==(_i); }
		bool operator<(const_iterator const& _i) const { return m_value < _i.m_value; }

	private:
		const_iterator(RangeMask const& _m, bool _end): m_owner(&_m), m_value(_m.m_ranges.empty() || _end ? _m.m_all.second : _m.m_ranges.begin()->first) {}

		RangeMask const* m_owner = nullptr;
		T m_value = 0;
	};

	const_iterator begin() const { return const_iterator(*this, false); }
	const_iterator end() const { return const_iterator(*this, true); }
	/// @returns the smallest element in the range mask that is larger than _t or the end of the
	/// base range if such an element does not exist.
	T next(T _t) const
	{
		_t++;
		// find next item in range at least _t
		auto uit = m_ranges.upper_bound(_t);	 // > _t
		auto it = uit == m_ranges.begin() ? m_ranges.end() : std::prev(uit);
		if (it != m_ranges.end() && it->first <= _t && it->second > _t)
			return _t;
		return uit == m_ranges.end() ? m_all.second : uit->first;
	}

	/// @returns the number of elements (not ranges) in the range mask.
	size_t size() const
	{
		size_t c = 0;
		for (auto const& r: this->m_ranges)
			c += r.second - r.first;
		return c;
	}

	size_t firstOut() const
	{
		if (m_ranges.empty() || !m_ranges.count(m_all.first))
			return m_all.first;
		return m_ranges.at(m_all.first);
	}

	size_t lastIn() const
	{
		if (m_ranges.empty())
			return m_all.first;
		return m_ranges.rbegin()->second - 1;
	}

private:
	/// The ground range.
	UnsignedRange m_all;
	/// Mapping begin -> end containing the ranges.
	std::map<T, T> m_ranges;
};

template <class T> inline std::ostream& operator<<(std::ostream& _out, RangeMask<T> const& _r)
{
	_out << _r.m_all.first << "{ ";
	for (auto const& i: _r.m_ranges)
		_out << "[" << i.first << ", " << i.second << ") ";
	_out << "}" << _r.m_all.second;
	return _out;
}

template <class T>
RangeMask<T>& RangeMask<T>::unionWith(typename RangeMask<T>::Range const& _m)
{
	for (auto i = _m.first; i < _m.second;)
	{
		assert(i >= m_all.first);
		assert(i < m_all.second);
		// For each number, we find the element equal or next lower. this, if any, must contain the value.
		// First range that starts after i.
		auto rangeAfter = m_ranges.upper_bound(i);
		// Range before rangeAfter or "end" if the rangeAfter is the first ever...
		auto it = rangeAfter == m_ranges.begin() ? m_ranges.end() : std::prev(rangeAfter);
		if (it == m_ranges.end() || it->second < i)
		{
			// i is either before the first range or between two ranges (with some distance
			// so that we cannot merge it onto "it").
			// lower range is too low to merge.
			// if the next higher range is too high.
			if (rangeAfter == m_ranges.end() || rangeAfter->first > _m.second)
			{
				// just create a new range
				m_ranges[i] = _m.second;
				break;
			}
			else
			{
				if (rangeAfter->first == i)
					// move i to end of range
					i = rangeAfter->second;
				else
				{
					// merge with the next higher range
					// move i to end of range
					i = m_ranges[i] = rangeAfter->second;
					m_ranges.erase(rangeAfter);
				}
			}
		}
		else if (it->second == i)
		{
			// The range before i ends with i.
			// if the next higher range is too high.
			if (rangeAfter == m_ranges.end() || rangeAfter->first > _m.second)
			{
				// merge with the next lower range
				m_ranges[it->first] = _m.second;
				break;
			}
			else
			{
				// merge with both next lower & next higher.
				i = m_ranges[it->first] = rangeAfter->second;
				m_ranges.erase(rangeAfter);
			}
		}
		else
			i = it->second;
	}
	return *this;
}

}
