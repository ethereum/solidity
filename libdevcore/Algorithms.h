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


#include <functional>
#include <set>

namespace dev
{

/**
 * Detector for cycles in directed graphs. It returns the first
 * vertex on the path towards a cycle or a nullptr if there is
 * no reachable cycle starting from a given vertex.
 */
template <typename V>
class CycleDetector
{
public:
	using Visitor = std::function<void(V const&, CycleDetector&, size_t)>;

	/// Initializes the cycle detector
	/// @param _visit function that is given the current vertex
	///               and is supposed to call @a run on all
	///               adjacent vertices.
	explicit CycleDetector(Visitor _visit):
		m_visit(std::move(_visit))
	{  }

	/// Recursively perform cycle detection starting
	/// (or continuing) with @param _vertex
	/// @returns the first vertex on the path towards a cycle from @a _vertex
	/// or nullptr if no cycle is reachable from @a _vertex.
	V const* run(V const& _vertex)
	{
		if (m_firstCycleVertex)
			return m_firstCycleVertex;
		if (m_processed.count(&_vertex))
			return nullptr;
		else if (m_processing.count(&_vertex))
			return m_firstCycleVertex = &_vertex;
		m_processing.insert(&_vertex);

		m_depth++;
		m_visit(_vertex, *this, m_depth);
		m_depth--;
		if (m_firstCycleVertex && m_depth == 1)
			m_firstCycleVertex = &_vertex;

		m_processing.erase(&_vertex);
		m_processed.insert(&_vertex);
		return m_firstCycleVertex;
	}

private:
	Visitor m_visit;
	std::set<V const*> m_processing;
	std::set<V const*> m_processed;
	size_t m_depth = 0;
	V const* m_firstCycleVertex = nullptr;
};

}
