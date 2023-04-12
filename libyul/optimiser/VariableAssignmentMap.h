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
#include <unordered_map>

namespace solidity::yul
{

/**
 * Class that implements a reverse lookup for an ``unordered_map<YulString, set<YulString>>`` by wrapping the
 * two such maps - one ordered, and one reversed, e.g.
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
	 * This method will erase all references of ``_variable`` from both sets before performing the insertion,
	 * practically making this an insert with overwrite method, akin to container assignment with subscript operator,
	 * i.e. container[index] = value.
	 * For example, if ``_variable`` is ``x`` and ``_references`` is ``{"y", "z"}``, the following would be added to ``Ref 1``:
	 *
	 * m_ordered        m_reversed
	 * x -> (y, z,)     y -> (x,)
	 *                  y -> (z,)
	 *
	 * @param _variable current expression variable
	 * @param _references all referenced variables in the expression assigned to ``_variable``
	 */
	void insert(YulString const& _variable, std::set<YulString> const& _references);

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
	void erase(YulString const& _variable);
	std::set<YulString> const* getOrderedOrNullptr(YulString const& _variable) const;
	std::set<YulString> const* getReversedOrNullptr(YulString const& _variable) const;

private:
	/// m_ordered[a].contains[b] <=> the current expression assigned to ``a`` references ``b``
	std::unordered_map<YulString, std::set<YulString>> m_ordered;
	/// m_reversed[b].contains[a] <=> the current expression assigned to ``a`` references ``b``
	std::unordered_map<YulString, std::set<YulString>> m_reversed;
};

} // solidity::yul
