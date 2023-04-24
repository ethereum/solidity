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

#pragma once

#include <libyul/YulString.h>

#include <set>

namespace solidity::yul
{

/**
 * Class that implements a forward and a reverse lookup for YulStrings by storing them in custom multimaps
 * - one ordered, and one reversed, e.g.
 *
 *  m_ordered           m_reversed
 *  f -> (g,)           g -> (f,)
 *  c -> (b,d,e,)       d -> (c,)
 *  a -> (b,c,)         c -> (a,)
 *                      e -> (c,)
 *                      b -> (a,c,)
 *
 * The above example will from here onwards be referenced as ```Ref 1```.
 *
 * This allows us to have simultaneously managed insertion and deletion via a single interface, instead of manually
 * managing this at the point of usage (see ``DataFlowAnalyzer``).
 */
class VariableAssignmentMap
{
public:
	VariableAssignmentMap() = default;
	/**
	 * Insert a set of values for the provided key ``_variable`` into ``m_ordered`` and ``m_reversed``.
	 * For example, if ``_variable`` is ``x`` and ``_references`` is ``{"y", "z"}``, the following would be added to ``Ref 1``:
	 *
	 * m_ordered        m_reversed
	 * x -> (y, z,)     y -> (x,)
	 *                  y -> (z,)
	 *
	 * @param _variable current expression variable
	 * @param _references all referenced variables in the expression assigned to ``_variable``
	 */
	void insert(YulString const& _variable, std::set<YulString> const& _references)
	{
		erase(_variable);
		for (auto const& reference: _references)
		{
			m_ordered.emplace(_variable, reference);
			m_reversed.emplace(reference, _variable);
		}
	}

	/**
	 * Erase entries in both maps based on provided ``_variable``.
	 * For example, after deleting ``c`` for ``Ref 1``, ``Ref 1`` would contain the following:
	 *
	 *  m_ordered           m_reversed
	 *  f -> (g,)           g -> (f,)
	 *  a -> (b,c,)         c -> (a,)
	 *                      b -> (a,)
	 *
	 * @param _variable variable to erase
	 */
	void erase(YulString const& _variable)
	{
		m_ordered.walkIndices(
			_variable,
			[&](size_t i)
			{
				m_reversed.remove(m_ordered.value(i), _variable);
				m_ordered.remove(i);
			}
		);
	}

	std::vector<YulString> getOrdered(YulString const& _variable) const { return m_ordered.values(_variable); }
	std::vector<YulString> getReversed(YulString const& _variable) const { return m_reversed.values(_variable); }
	template <class Function>
	void walkOrdered(YulString const& _variable, Function _f) const { m_ordered.walkValues(_variable, _f); }
	template <class Function>
	void walkReversed(YulString const& _variable, Function _f) const { m_reversed.walkValues(_variable, _f); }

private:
	class Multimap
	{
	public:
		Multimap(): m_capacity(initialCapacity), m_load(0), m_size(0), m_elements(initialCapacity) {}

		void emplace(YulString const& _key, YulString const& _value)
		{
			size_t i = hash(_key);
			for (; isOccupied(m_elements[i]); i = next(i))
				if (m_elements[i].first == _key && m_elements[i].second == _value)
					return;
			// Only increment the load if the slot has never been used.
			if (isZero(m_elements[i]))
				++m_load;
			m_elements[i] = Element(_key, _value);
			++m_size;
			// Use a 50% load factor for simplicity and performance.
			if (m_load > m_capacity / 2)
			{
				// Only grow if there are not enough vacant slots.
				if (m_size > m_capacity / 4)
					m_capacity *= 2;
				m_newElements.resize(m_capacity);
				m_load = m_size;
				for (auto const& element: m_elements)
					if (isOccupied(element))
					{
						size_t i = hash(element.first);
						while (!isZero(m_newElements[i]))
							i = next(i);
						m_newElements[i] = element;
					}
				std::swap(m_elements, m_newElements); // To avoid reallocation.
				m_newElements.clear(); // So that a future resize will reset the elements.
			}
		}

		void remove(size_t _i)
		{
			m_elements[_i].first.setId(maxSize);
			--m_size;
		}

		void remove(YulString const& _key, YulString const& _value)
		{
			size_t i = find(_key);
			if (i != maxSize)
				for (; !isZero(m_elements[i]); i = next(i))
					if (m_elements[i].first == _key && m_elements[i].second == _value)
					{
						remove(i);
						return;
					}
		}

		std::vector<YulString> values(YulString const& _key) const
		{
			std::vector<YulString> v;
			size_t i = find(_key);
			if (i != maxSize)
				for (; !isZero(m_elements[i]); i = next(i))
					if (m_elements[i].first == _key)
						v.emplace_back(m_elements[i].second);
			return v;
		}

		template <class Function>
		void walkValues(YulString const& _key, Function _f) const
		{
			size_t i = find(_key);
			if (i != maxSize)
				for (; !isZero(m_elements[i]); i = next(i))
					if (m_elements[i].first == _key)
						_f(m_elements[i].second);
		}

		template <class Function>
		void walkIndices(YulString const& _key, Function _f) const
		{
			size_t i = find(_key);
			if (i != maxSize)
				for (; !isZero(m_elements[i]); i = next(i))
					if (m_elements[i].first == _key)
						_f(i);
		}

		YulString const& value(size_t _i) const { return m_elements[_i].second; }

	private:
		using Element = std::pair<YulString, YulString>;

		/// Must be a power of 2.
		constexpr static size_t initialCapacity = 512;
		/// To denote that a slot has been cleared, or to represent an index for a key not found.
		constexpr static size_t maxSize = (size_t) -1;

		/// Number of slots.
		size_t m_capacity;
		/// Number of slots that are occupied or previously cleared.
		size_t m_load;
		/// Number of occupied slots.
		size_t m_size;

		/// For holding the elements.
		std::vector<Element> m_elements;
		/// For rebuilding the table.
		std::vector<Element> m_newElements;

		size_t find(YulString const& _key) const
		{
			for (size_t i = hash(_key); ; i = next(i))
			{
				if (isZero(m_elements[i]))
					return maxSize;
				if (m_elements[i].first == _key)
					return i;
			}
		}

		bool isZero(Element const& _e) const { return _e.first.empty(); }
		bool isOccupied(Element const& _e) const { return !isZero(_e) && _e.first.id() != maxSize; }
		size_t next(size_t _i) const { return (_i + 1) & (m_capacity - 1); }
		size_t hash(YulString const& _s) const { return _s.hash() & (m_capacity - 1); }
	};

	/// m_ordered[a].contains[b] <=> the current expression assigned to ``a`` references ``b``
	Multimap m_ordered;
	/// m_reversed[b].contains[a] <=> the current expression assigned to ``a`` references ``b``
	Multimap m_reversed;
};

} // solidity::yul
