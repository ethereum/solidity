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
/**
 * Dominator analysis of a control flow graph.
 * The implementation is based on the following paper:
 * https://www.cs.princeton.edu/courses/archive/spr03/cs423/download/dominators.pdf
 * See appendix B pg. 139.
 */
#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libsolutil/Visitor.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

#include <deque>
#include <map>
#include <vector>
#include <set>

namespace solidity::yul
{

template<typename V, typename ForEachSuccessor>
class DominatorFinder
{
public:

	DominatorFinder(V const& _entry, size_t _numVertices):
		m_vertices(_numVertices),
		m_immediateDominators(findDominators(_entry, _numVertices))
	{
		buildDominatorTree();
	}

	std::vector<V> const& vertices() const
	{
		return m_vertices;
	}

	std::map<V, size_t> const& vertexIndices() const
	{
		return m_vertexIndices;
	}

	std::vector<size_t> const& immediateDominators() const
	{
		return m_immediateDominators;
	}

	std::map<size_t, std::vector<size_t>> const& dominatorTree() const
	{
		return m_dominatorTree;
	}

	/// Checks whether ``_a`` dominates ``_b`` by going
	/// through the path from ``_b`` to the entry node.
	/// If ``_a`` is found, then it dominates ``_b``
	/// otherwise it doesn't.
	bool dominates(V const& _a, V const& _b) const
	{
		try {
			solAssert(!m_vertexIndices.empty());
			solAssert(!m_immediateDominators.empty());

			size_t aIdx = m_vertexIndices.at(_a);
			size_t bIdx = m_vertexIndices.at(_b);

			if (aIdx == bIdx)
				return true;

			size_t idomIdx = m_immediateDominators.at(bIdx);
			while (idomIdx != 0)
			{
				if (idomIdx == aIdx)
					return true;
				solAssert(m_immediateDominators.at(idomIdx) < idomIdx);
				idomIdx = m_immediateDominators[idomIdx];
			}
			// Now that we reached the entry node (i.e. idomIdx = 0),
			// either ``aIdx == 0`` or it does not dominate the other node.
			solAssert(idomIdx == 0, "");
			return aIdx == 0;
		}
		catch (std::out_of_range const&) {
			solThrow(util::ElementNotFound, "Vertex not found.");
		}
	}

	/// Find all dominators of a node _v
	/// This function returns the set of all dominators of a vertex in reverse order.
	/// @note for a vertex ``_v``, the _v’s inclusion in the set of dominators of ``_v`` is implicit.
	std::vector<V const*>  dominatorsOf(V const& _v) const
	{
		try
		{
			solAssert(!m_vertices.empty());
			solAssert(!m_vertexIndices.empty());
			solAssert(!m_immediateDominators.empty());

			std::vector<V const*> dominators{};
			// No one dominates the entry vertex and we consider self-dominance implicit
			// i.e. all nodes already dominates itself.
			if (m_vertexIndices.at(_v) == 0)
				return dominators;

			size_t idomIdx = m_immediateDominators.at(m_vertexIndices.at(_v));
			while (idomIdx != 0)
			{
				solAssert(m_immediateDominators.at(idomIdx) < idomIdx);
				dominators.emplace_back(&m_vertices.at(idomIdx));
				idomIdx = m_immediateDominators[idomIdx];
			}
			// The loop above discovers the dominators in the reverse order
			// i.e. from the given vertex upwards to the entry node (the root of the dominator tree).
			// And the entry vertex always dominates all other vertices.
			dominators.emplace_back(&m_vertices[0]);

			return dominators;
		}
		catch (std::out_of_range const&) {
			solThrow(util::ElementNotFound, "Vertex not found.");
		}
	}

	/// Path compression updates the ancestors of vertices along
	/// the path to the ancestor with the minimum label value.
	void compressPath(
		std::vector<size_t> &_ancestor,
		std::vector<size_t> &_label,
		std::vector<size_t> &_semi,
		size_t _vIdx
	) const
	{
		solAssert(_ancestor[_vIdx] != std::numeric_limits<size_t>::max());
		size_t uIdx = _ancestor[_vIdx];
		if (_ancestor[uIdx] != std::numeric_limits<size_t>::max())
		{
			compressPath(_ancestor, _label, _semi, uIdx);
			if (_semi[_label[uIdx]] < _semi[_label[_vIdx]])
				_label[_vIdx] = _label[uIdx];
			_ancestor[_vIdx] = _ancestor[uIdx];
		}
		solAssert(_label[uIdx] <= _label[_vIdx]);
	}

	std::vector<size_t> findDominators(V const& _entry, size_t numVertices)
	{
		solAssert(numVertices > 0);
		// semi(w): The DFS index of the semidominator of ``w``.
		std::vector<size_t> semi(numVertices, std::numeric_limits<size_t>::max());
		// parent(w): The index of the vertex which is the parent of ``w`` in the spanning
		// tree generated by the DFS.
		std::vector<size_t> parent(numVertices, std::numeric_limits<size_t>::max());
		// ancestor(w): The highest ancestor of a vertex ``w`` in the dominator tree used
		// for path compression.
		std::vector<size_t> ancestor(numVertices, std::numeric_limits<size_t>::max());
		// label(w): The index of the vertex ``w`` with the minimum semidominator in the path
		// to its parent.
		std::vector<size_t> label(numVertices, 0);

		// ``link`` adds an edge to the virtual forest.
		// It copies the ``parent`` index of the vertex ``w`` to the ancestor array to limit the search path upwards.
		// TODO: implement sophisticated link-eval algorithm as shown in pg 132
		// See: https://www.cs.princeton.edu/courses/archive/spr03/cs423/download/dominators.pdf
		auto link = [&](size_t _parentIdx, size_t _wIdx)
		{
			ancestor[_wIdx] = _parentIdx;
		};

		// ``eval`` computes the path compression.
		// Finds ancestor with lowest semi-dominator DFS number (i.e. index).
		auto eval = [&](size_t _vIdx) -> size_t
		{
			if (ancestor[_vIdx] != std::numeric_limits<size_t>::max())
			{
				compressPath(ancestor, label, semi, _vIdx);
				return label[_vIdx];
			}
			return _vIdx;
		};

		auto toIdx = [&](V const& v) { return m_vertexIndices[v]; };

		// step 1
		std::set<V> visited;
		// predecessors(w): The set of vertices ``v`` such that (``v``, ``w``) is an edge of the graph.
		std::vector<std::set<size_t>> predecessors(numVertices);
		// bucket(w): a set of vertices whose semidominator is ``w``
		// The index of the array represents the vertex's ``dfIdx``
		std::vector<std::deque<size_t>> bucket(numVertices);
		// idom(w): the index of the immediate dominator of ``w``
		std::vector<size_t> idom(numVertices, std::numeric_limits<size_t>::max());
		// The number of vertices reached during the DFS.
		// The vertices are indexed based on this number.
		size_t dfIdx = 0;
		auto dfs = [&](V const& _v, auto _dfs) -> void {
			auto [_, inserted] = visited.insert(_v);
			if (!inserted)
				return;
			m_vertices[dfIdx] = _v;
			m_vertexIndices[_v] = dfIdx;
			semi[dfIdx] = dfIdx;
			label[dfIdx] = dfIdx;
			dfIdx++;
			ForEachSuccessor{}(_v, [&](V const& w) {
				if (semi[dfIdx] == std::numeric_limits<size_t>::max())
				{
					parent[dfIdx] = m_vertexIndices[_v];
					_dfs(w, _dfs);
				}
				predecessors[m_vertexIndices[w]].insert(m_vertexIndices[_v]);
			});
		};
		dfs(_entry, dfs);

		// Process the vertices in decreasing order of the DFS number
		for (size_t wIdx: m_vertices | ranges::views::reverse | ranges::views::transform(toIdx))
		{
			// step 3
			// NOTE: this is an optimization, i.e. performing the step 3 before step 2.
			// The goal is to process the bucket in the beginning of the loop for the vertex ``w``
			// instead of ``parent[w]`` in the end of the loop as described in the original paper.
			// Inverting those steps ensures that a bucket is only processed once and
			// it does not need to be erased.
			// The optimization proposal is available here: https://jgaa.info/accepted/2006/GeorgiadisTarjanWerneck2006.10.1.pdf pg.77
			ranges::for_each(
				bucket[wIdx],
				[&](size_t vIdx)
				{
					size_t uIdx = eval(vIdx);
					solAssert(uIdx <= vIdx);
					idom[vIdx] = (semi[uIdx] < semi[vIdx]) ? uIdx : wIdx;
				}
			);

			// step 2
			for (size_t vIdx: predecessors[wIdx])
			{
				size_t uIdx = eval(vIdx);
				solAssert(uIdx <= vIdx);
				if (semi[uIdx] < semi[wIdx])
					semi[wIdx] = semi[uIdx];
			}
			bucket[semi[wIdx]].emplace_back(wIdx);
			link(parent[wIdx], wIdx);
			solAssert(ancestor[wIdx] == parent[wIdx]);
		}

		// step 4
		idom[0] = 0;
		for (size_t wIdx: m_vertices | ranges::views::drop(1) | ranges::views::transform(toIdx))
			if (idom[wIdx] != semi[wIdx])
				idom[wIdx] = idom[idom[wIdx]];

		return idom;
	}

private:
	/// Build dominator tree from the immediate dominators set.
	/// The function groups all the indices that are immediately dominated by a vertex.
	void buildDominatorTree()
	{
		solAssert(!m_immediateDominators.empty());

		//Ignoring the entry node since no one dominates it.
		for (size_t index = 1; index < m_immediateDominators.size(); ++index)
		{
			solAssert(m_immediateDominators[index] < index);
			m_dominatorTree[m_immediateDominators[index]].emplace_back(index);
		}
	}

	/// Keep the list of vertices in the DFS order.
	/// i.e. m_vertices[i] is the vertex whose DFS index is i.
	std::vector<V> m_vertices;

	/// Maps Vertex to its DFS index.
	std::map<V, size_t> m_vertexIndices;

	/// Immediate dominators by index.
	/// Maps a Vertex based on its DFS index (i.e. array index) to its immediate dominator DFS index.
	/// The entry vertex is the first element of the vector.
	///
	/// e.g. to get the immediate dominator of a Vertex w:
	/// idomIdx = m_immediateDominators[m_vertexIndices[w]]
	/// idomVertex = m_vertices[domIdx]
	std::vector<size_t> m_immediateDominators;

	/// Maps a Vertex to all vertices that it dominates.
	/// If the vertex does not dominate any other vertex it has no entry in the map.
	std::map<size_t, std::vector<size_t>> m_dominatorTree;
};
}
