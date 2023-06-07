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

#include <liblangutil/Exceptions.h>
#include <libsolutil/Visitor.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

#include <deque>
#include <map>
#include <vector>
#include <set>

namespace solidity::util
{

/// DominatorFinder computes the dominator tree of a directed graph.
/// V is the type of the vertex.
/// VId is the type of the vertex identifier. VIds should be unique and comparable.
/// ForEachSuccessor is a visitor that visits the successors of a vertex.
/// TODO: pass the graph or an adjacency list as a parameter to the constructor.
template<typename V, typename VId, typename ForEachSuccessor>
class DominatorFinder
{
public:

	DominatorFinder(V const& _entry, VId const& _entryId, size_t _numVertices):
		m_verticesInDFSOrder(_numVertices),
		m_immediateDominators(findDominators(_entry, _entryId, _numVertices))
	{
		buildDominatorTree();
	}

	std::vector<VId> const& verticesIdsInDFSOrder() const
	{
		return m_verticesInDFSOrder;
	}

	std::map<VId, size_t> const& dfsIndexById() const
	{
		return m_dfsIndicesMap;
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
	bool dominates(VId const& _a, VId const& _b) const
	{
		solAssert(!m_dfsIndicesMap.empty());
		solAssert(!m_immediateDominators.empty());

		solAssert(m_dfsIndicesMap.count(_a) && m_dfsIndicesMap.count(_b));
		size_t aIdx = m_dfsIndicesMap.at(_a);
		size_t bIdx = m_dfsIndicesMap.at(_b);

		if (aIdx == bIdx)
			return true;

		solAssert(aIdx < m_immediateDominators.size() && bIdx < m_immediateDominators.size());
		size_t idomIdx = m_immediateDominators.at(bIdx);
		while (idomIdx != 0)
		{
			solAssert(idomIdx < m_immediateDominators.size());
			if (idomIdx == aIdx)
				return true;
			// The index of the immediate dominator of a vertex is always less than the index of the vertex itself.
			solAssert(m_immediateDominators.at(idomIdx) < idomIdx);
			idomIdx = m_immediateDominators[idomIdx];
		}
		// Now that we reached the entry node (i.e. idomIdx = 0),
		// either ``aIdx == 0`` or it does not dominate the other node.
		solAssert(idomIdx == 0, "");
		return aIdx == 0;
	}

	/// Find all dominators of a node _v
	/// This function returns the set of all dominators of a vertex in reverse order.
	/// @note for a vertex ``_v``, the _v’s inclusion in the set of dominators of ``_v`` is implicit.
	std::vector<VId>  dominatorsOf(VId const& _v) const
	{
		solAssert(!m_verticesInDFSOrder.empty());
		solAssert(!m_dfsIndicesMap.empty());
		solAssert(!m_immediateDominators.empty());

		std::vector<VId> dominators{};
		solAssert(m_dfsIndicesMap.count(_v));
		// No one dominates the entry vertex and we consider self-dominance implicit
		// i.e. all nodes already dominates itself.
		if (m_dfsIndicesMap.at(_v) == 0)
			return dominators;

		solAssert(m_dfsIndicesMap.at(_v) < m_immediateDominators.size());
		size_t idomIdx = m_immediateDominators.at(m_dfsIndicesMap.at(_v));
		solAssert(idomIdx < m_immediateDominators.size());
		while (idomIdx != 0)
		{
			solAssert(m_immediateDominators.at(idomIdx) < idomIdx);
			dominators.emplace_back(m_verticesInDFSOrder.at(idomIdx));
			idomIdx = m_immediateDominators[idomIdx];
		}
		// The loop above discovers the dominators in the reverse order
		// i.e. from the given vertex upwards to the entry node (the root of the dominator tree).
		// And the entry vertex always dominates all other vertices.
		dominators.emplace_back(m_verticesInDFSOrder[0]);

		return dominators;
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

private:
	std::vector<size_t> findDominators(V const& _entry, VId const& _entryId, size_t _numVertices)
	{
		solAssert(_numVertices > 0);
		// semi(w): The DFS index of the semidominator of ``w``.
		std::vector<size_t> semi(_numVertices, std::numeric_limits<size_t>::max());
		// parent(w): The index of the vertex which is the parent of ``w`` in the spanning
		// tree generated by the DFS.
		std::vector<size_t> parent(_numVertices, std::numeric_limits<size_t>::max());
		// ancestor(w): The highest ancestor of a vertex ``w`` in the dominator tree used
		// for path compression.
		std::vector<size_t> ancestor(_numVertices, std::numeric_limits<size_t>::max());
		// label(w): The index of the vertex ``w`` with the minimum semidominator in the path
		// to its parent.
		std::vector<size_t> label(_numVertices, 0);

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

		auto toIdx = [&](VId const& v) { return m_dfsIndicesMap[v]; };

		// step 1
		std::set<VId> visited;
		// predecessors(w): The set of vertices ``v`` such that (``v``, ``w``) is an edge of the graph.
		std::vector<std::set<size_t>> predecessors(_numVertices);
		// bucket(w): a set of vertices whose semidominator is ``w``
		// The index of the array represents the vertex's ``dfIdx``
		std::vector<std::deque<size_t>> bucket(_numVertices);
		// idom(w): the index of the immediate dominator of ``w``
		std::vector<size_t> idom(_numVertices, std::numeric_limits<size_t>::max());
		// The number of vertices reached during the DFS.
		// The vertices are indexed based on this number.
		size_t dfIdx = 0;
		auto dfs = [&](V const& _v, VId const& _vId, auto _dfs) -> void {
			auto [_, inserted] = visited.insert(_vId);
			if (!inserted)
				return;
			m_verticesInDFSOrder[dfIdx] = _vId;
			m_dfsIndicesMap[_vId] = dfIdx;
			semi[dfIdx] = dfIdx;
			label[dfIdx] = dfIdx;
			// NOTE: Since each vertex is only visited once, incrementing dfIdx before visiting successors ensures
			// it points to the next vertex to be visited (i.e., w).
			// This is because we do not know the index of a vertex before visiting them.
			dfIdx++;
			ForEachSuccessor{}(_v, _vId, [&](V const& w, VId wId) {
				if (!visited.count(wId))
				{
					// Here, ``dfIdx`` is the index of the vertex ``w`` in the DFS order.
					parent[dfIdx] = m_dfsIndicesMap[_vId];
					_dfs(w, wId, _dfs);
				}
				predecessors[m_dfsIndicesMap[wId]].insert(m_dfsIndicesMap[_vId]);
			});
		};
		dfs(_entry, _entryId, dfs);

		// Process the vertices in decreasing order of the DFS number
		for (size_t wIdx: m_verticesInDFSOrder | ranges::views::reverse | ranges::views::transform(toIdx))
		{
			// step 3
			// NOTE: this is an optimization, i.e. performing the step 3 before step 2.
			// The goal is to process the bucket in the beginning of the loop for the vertex ``w``
			// instead of ``parent[w]`` in the end of the loop as described in the original paper.
			// Inverting those steps ensures that a bucket is only processed once and
			// it does not need to be erased.
			// The optimization proposal is available here: https://jgaa.info/index.php/jgaa/article/view/paper119/2847 pg.77
			for (size_t vIdx: bucket[wIdx])
			{
				size_t uIdx = eval(vIdx);
				solAssert(uIdx <= vIdx);
				idom[vIdx] = (semi[uIdx] < semi[vIdx]) ? uIdx : wIdx;
			}

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
		for (size_t wIdx: m_verticesInDFSOrder | ranges::views::drop(1) | ranges::views::transform(toIdx))
			if (idom[wIdx] != semi[wIdx])
				idom[wIdx] = idom[idom[wIdx]];

		return idom;
	}

	/// Build dominator tree from the immediate dominators set.
	/// The function groups all the indices that are immediately dominated by a vertex.
	void buildDominatorTree()
	{
		solAssert(m_immediateDominators.size() == m_verticesInDFSOrder.size());

		// Ignoring the entry node since no one dominates it.
		solAssert(m_immediateDominators[0] == 0);
		for (size_t index = 1; index < m_verticesInDFSOrder.size(); ++index)
		{
			solAssert(m_immediateDominators[index] < index);
			m_dominatorTree[m_immediateDominators[index]].emplace_back(index);
		}
	}

	/// Keep the list of vertices ids in the DFS order.
	/// The entry vertex is the first element of the vector.
	/// The index of the other vertices is based on the order they are visited during the DFS.
	/// i.e. m_verticesInDFSOrder[i] is the vertex whose DFS index is i.
	std::vector<VId> m_verticesInDFSOrder;

	/// Maps a Vertex by id to its index in the DFS order.
	std::map<VId, size_t> m_dfsIndicesMap;

	/// Immediate dominators by index.
	/// Maps a Vertex based on its DFS index (i.e. array index) to its immediate dominator DFS index.
	/// The entry vertex is the first element of the vector.
	///
	/// e.g. to get the immediate dominator of a Vertex w:
	/// idomIdx = m_immediateDominators[m_dfsIndicesMap[wId]]
	/// idomVertexId = m_verticesInDFSOrder[domIdx]
	std::vector<size_t> m_immediateDominators;

	/// Maps a Vertex to all vertices that it dominates.
	/// If the vertex does not dominate any other vertex it has no entry in the map.
	/// The key is the index of the vertex in the DFS order.
	/// The value is a vector of DFS indices of the vertices that are dominated by the key vertex.
	std::map<size_t, std::vector<size_t>> m_dominatorTree;
};
}
