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

#include <liblangutil/Exceptions.h>

#include <cstddef>
#include <numeric>
#include <vector>

namespace solidity::util
{

/// Assumes that ids are contiguous and start from 0
class ContiguousDisjointSet
{
public:
	using size_type = size_t;
	using value_type = size_t;

	explicit ContiguousDisjointSet(size_t const _numNodes):
		m_parents(_numNodes),
		m_neighbors(_numNodes),
		m_sizes(_numNodes, static_cast<value_type>(1)),
		m_numSets(_numNodes)
	{
		// each is their own neighbor and parent
		std::iota(m_parents.begin(), m_parents.end(), 0);
		std::iota(m_neighbors.begin(), m_neighbors.end(), 0);
	}

	size_type numSets() const { return m_numSets; }

	value_type find(value_type const _representative) const
	{
		solAssert(_representative < m_parents.size());
		// path halving
		value_type rootElement = _representative;
		while (rootElement != m_parents[rootElement])
		{
			m_parents[rootElement] = m_parents[m_parents[rootElement]];
			rootElement = m_parents[rootElement];
		}
		return rootElement;
	}

	bool merge(value_type x, value_type y, bool mergeBySize=true)
	{
		solAssert(x < m_parents.size() && y < m_parents.size());
		auto xRoot = find(x);
		auto yRoot = find(y);

		if (xRoot == yRoot)
			return false;  // we're done, nothing to merge here

		// merge smaller (yRoot) into larger (xRoot) subset
		if (mergeBySize && m_sizes[xRoot] < m_sizes[yRoot])
			std::swap(xRoot, yRoot);

		m_parents[yRoot] = xRoot;
		m_sizes[xRoot] += m_sizes[yRoot];
		std::swap(m_neighbors[xRoot], m_neighbors[yRoot]);
		--m_numSets;
		return true;
	}

	bool sameSubset(value_type x, value_type y) const
	{
		return find(x) == find(y);
	}

	size_type sizeOfSubset(value_type x) const
	{
		return m_sizes[find(x)];
	}

	std::set<value_type> subset(value_type x) const
	{
		solAssert(x < m_parents.size());
		std::set<value_type> result{x};
		value_type neighbor = m_neighbors[x];
		while (neighbor != x)
		{
			result.insert(neighbor);
			neighbor = m_neighbors[neighbor];
		}
		return result;
	}

	std::vector<std::set<value_type>> subsets() const
	{
		std::vector<std::set<value_type>> result;
		std::vector<std::uint8_t> visited(m_parents.size(), false);
		for (value_type x = 0; x < m_parents.size(); ++x)
		{
			auto xRoot = find(x);
			if (!visited[xRoot])
			{
				result.push_back(subset(xRoot));
				visited[xRoot] = true;
			}
		}
		return result;
	}

private:
	std::vector<value_type> mutable m_parents;
	std::vector<value_type> m_neighbors;
	std::vector<value_type> m_sizes;
	size_type m_numSets;
};

}
