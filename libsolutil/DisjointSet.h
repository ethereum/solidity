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

#include <cstddef>
#include <set>
#include <vector>

namespace solidity::util
{

/// Implementation of the Disjoint set data structure [1], also called union-set, with path-halving [2].
/// This implementation assumes that each set element is identified with an element in a iota range (hence contiguous).
///
/// [1] https://en.wikipedia.org/wiki/Disjoint-set_data_structure
/// [2] Tarjan, Robert E., and Jan Van Leeuwen. "Worst-case analysis of set union algorithms."
///     Journal of the ACM (JACM) 31.2 (1984): 245-281.
class ContiguousDisjointSet
{
public:
	using size_type = size_t;
	using value_type = size_t;

	/// Constructs a new disjoint set datastructure with `_numNodes` elements and each element in its own individual set
	explicit ContiguousDisjointSet(size_t _numNodes);

	size_type numSets() const;

	/// finds one representative for a set that contains `_element`
	value_type find(value_type _element) const;

	/// joins the two sets containing `_x` and `_y`, returns true if the sets were disjoint, otherwise false
	void merge(value_type _x, value_type _y, bool _mergeBySize=true);

	bool sameSubset(value_type _x, value_type _y) const;

	size_type sizeOfSubset(value_type _x) const;

	std::set<value_type> subset(value_type _x) const;

	std::vector<std::set<value_type>> subsets() const;

private:
	std::vector<value_type> mutable m_parents;  // mutable for path compression, doesn't change semantic state
	std::vector<value_type> m_neighbors;
	std::vector<value_type> m_sizes;
	size_type m_numSets;
};

}
