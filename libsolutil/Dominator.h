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
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

#include <deque>
#include <vector>
#include <set>

namespace
{
	template <typename, typename = void>
	struct has_id : std::false_type {};

	template <typename T>
	struct has_id<T, std::void_t<decltype(std::declval<T>().id)>> : std::true_type {};
}

namespace solidity::util
{

/// DominatorFinder computes the dominator tree of a directed graph.
/// V is the type of the vertex and it is assumed to have a unique identifier.
/// ForEachSuccessor is a visitor that visits the successors of a vertex.
template<typename V, typename ForEachSuccessor>
class DominatorFinder
{
static_assert(has_id<V>::value, "vertex must define id member");
using VId = typename V::Id;
using DfsIndex = size_t;

public:

	DominatorFinder(V const& _entry, size_t _numVertices):
		m_verticesInDFSOrder(_numVertices),
		m_immediateDominators(findDominators(_entry, _numVertices))
	{
		buildDominatorTree();
	}

	std::vector<VId> const& verticesIdsInDFSOrder() const
	{
		return m_verticesInDFSOrder;
	}

	std::map<VId, DfsIndex> const& dfsIndexById() const
	{
		return m_dfsIndexByVertexId;
	}

	std::map<VId, VId> const idom() const
	{
		return m_immediateDominators
			| ranges::views::enumerate
			| ranges::views::transform([&](auto const& _v) {
				return std::make_pair(m_verticesInDFSOrder[_v.first], m_verticesInDFSOrder[_v.second]);
			})
			| ranges::to<std::map<VId, VId>>;
	}

	std::vector<DfsIndex> const& idomByDfsIndex() const
	{
		return m_immediateDominators;
	}

	std::map<VId, std::set<VId>> const& dominatorTree() const
	{
		return m_dominatorTree;
	}

	bool dominates(VId const& _aId, VId const& _bId) const
	{
		solAssert(!m_dfsIndexByVertexId.empty());
		solAssert(!m_immediateDominators.empty());

		solAssert(m_dfsIndexByVertexId.count(_aId) && m_dfsIndexByVertexId.count(_bId));
		DfsIndex aIdx = m_dfsIndexByVertexId.at(_aId);
		DfsIndex bIdx = m_dfsIndexByVertexId.at(_bId);

		if (aIdx == bIdx)
			return true;

		DfsIndex idomIdx = m_immediateDominators.at(bIdx);
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

	/// Checks whether vertex ``_a`` dominates ``_b`` by going
	/// through the path from ``_b`` to the entry node.
	/// If ``_a`` is found, then it dominates ``_b``
	/// otherwise it doesn't.
	bool dominates(V const& _a, V const& _b) const
	{
		return dominates(_a.id, _b.id);
	}

	std::vector<VId> dominatorsOf(VId const& _vId) const
	{
		solAssert(!m_verticesInDFSOrder.empty());
		solAssert(!m_dfsIndexByVertexId.empty());
		solAssert(!m_immediateDominators.empty());

		std::vector<VId> dominators{};
		solAssert(m_dfsIndexByVertexId.count(_vId));
		// No one dominates the entry vertex and we consider self-dominance implicit
		// i.e. all nodes already dominates itself.
		if (m_dfsIndexByVertexId.at(_vId) == 0)
			return dominators;

		solAssert(m_dfsIndexByVertexId.count(_vId));
		solAssert(m_dfsIndexByVertexId.at(_vId) < m_immediateDominators.size());
		DfsIndex idomIdx = m_immediateDominators.at(m_dfsIndexByVertexId.at(_vId));
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

	/// Find all dominators of a node _v
	/// @note for a vertex ``_v``, the _v’s inclusion in the set of dominators of ``_v`` is implicit.
	std::vector<VId> const& dominatorsOf(V const& _v) const
	{
		return dominatorsOf(_v.id);
	}

private:
	std::vector<DfsIndex> findDominators(V const& _entry, size_t _numVertices)
	{
		solAssert(_numVertices > 0);

		// semi(w): The DFS index of the semidominator of ``w``.
		std::vector<DfsIndex> semi(_numVertices, std::numeric_limits<size_t>::max());
		// parent(w): The index of the vertex which is the parent of ``w`` in the spanning
		// tree generated by the DFS.
		std::vector<DfsIndex> parent(_numVertices, std::numeric_limits<size_t>::max());
		// ancestor(w): The highest ancestor of a vertex ``w`` in the dominator tree used
		// for path compression.
		std::vector<DfsIndex> ancestor(_numVertices, std::numeric_limits<size_t>::max());
		// label(w): The index of the vertex ``w`` with the minimum semidominator in the path
		// to its parent.
		std::vector<DfsIndex> label(_numVertices, 0);

		// ``link`` adds an edge to the virtual forest.
		// It copies the ``parent`` index of the vertex ``w`` to the ancestor array to limit the search path upwards.
		// TODO: implement sophisticated link-eval algorithm as shown in pg 132
		// See: https://www.cs.princeton.edu/courses/archive/spr03/cs423/download/dominators.pdf
		auto link = [&](DfsIndex _parentIdx, DfsIndex _wIdx)
		{
			ancestor[_wIdx] = _parentIdx;
		};

		// ``eval`` computes the path compression.
		// Finds ancestor with lowest semi-dominator DFS number (i.e. index).
		auto eval = [&](DfsIndex _vIdx) -> DfsIndex
		{
			if (ancestor[_vIdx] != std::numeric_limits<size_t>::max())
			{
				compressPath(ancestor, label, semi, _vIdx);
				return label[_vIdx];
			}
			return _vIdx;
		};

		auto toDfsIndex = [&](VId const& _vId) { return m_dfsIndexByVertexId[_vId]; };

		// step 1
		std::set<VId> visited;
		// predecessors(w): The set of vertices ``v`` such that (``v``, ``w``) is an edge of the graph.
		std::vector<std::set<DfsIndex>> predecessors(_numVertices);
		// bucket(w): a set of vertices whose semidominator is ``w``
		// The index of the array represents the vertex's ``dfIdx``
		std::vector<std::deque<DfsIndex>> bucket(_numVertices);
		// idom(w): the vertex Id of the immediate dominator of ``w``
		std::vector<DfsIndex> idom(_numVertices, std::numeric_limits<size_t>::max());
		// The number of vertices reached during the DFS.
		// The vertices are indexed based on this number.
		DfsIndex dfIdx = 0;
		auto dfs = [&](V const& _v, auto _dfs) -> void {
			auto [_, inserted] = visited.insert(_v.id);
			if (!inserted)
				return;
			m_verticesInDFSOrder[dfIdx] = _v.id;
			m_dfsIndexByVertexId[_v.id] = dfIdx;
			semi[dfIdx] = dfIdx;
			label[dfIdx] = dfIdx;
			// NOTE: Since each vertex is only visited once, incrementing dfIdx before visiting successors ensures
			// it points to the next vertex to be visited (i.e., w).
			// This is because we do not know the index of a vertex before visiting them.
			dfIdx++;
			ForEachSuccessor{}(_v, [&](V const& w) {
				if (!visited.count(w.id))
				{
					// Here, ``dfIdx`` is the index of the vertex ``w`` in the DFS order.
					parent[dfIdx] = m_dfsIndexByVertexId[_v.id];
					_dfs(w, _dfs);
				}
				predecessors[m_dfsIndexByVertexId[w.id]].insert(m_dfsIndexByVertexId[_v.id]);
			});
		};
		dfs(_entry, dfs);

		// Process the vertices in decreasing order of the DFS number
		for (DfsIndex wIdx: m_verticesInDFSOrder | ranges::views::reverse | ranges::views::transform(toDfsIndex))
		{
			// step 3
			// NOTE: this is an optimization, i.e. performing the step 3 before step 2.
			// The goal is to process the bucket in the beginning of the loop for the vertex ``w``
			// instead of ``parent[w]`` in the end of the loop as described in the original paper.
			// Inverting those steps ensures that a bucket is only processed once and
			// it does not need to be erased.
			// The optimization proposal is available here: https://jgaa.info/index.php/jgaa/article/view/paper119/2847 pg.77
			for (DfsIndex vIdx: bucket[wIdx])
			{
				DfsIndex uIdx = eval(vIdx);
				solAssert(uIdx <= vIdx);
				idom[vIdx] = (semi[uIdx] < semi[vIdx]) ? uIdx : wIdx;
			}

			// step 2
			for (DfsIndex vIdx: predecessors[wIdx])
			{
				DfsIndex uIdx = eval(vIdx);
				solAssert(uIdx <= vIdx);
				if (semi[uIdx] < semi[wIdx])
					semi[wIdx] = semi[uIdx];
			}
			bucket[semi[wIdx]].emplace_back(wIdx);
			link(parent[wIdx], wIdx);
			solAssert(ancestor[wIdx] == parent[wIdx]);
		}

		// step 4
		// Compute idom in dfs order
		idom[0] = 0;
		for (DfsIndex wIdx: m_verticesInDFSOrder | ranges::views::drop(1) | ranges::views::transform(toDfsIndex))
			if (idom[wIdx] != semi[wIdx])
				idom[wIdx] = idom[idom[wIdx]];

		return idom;
	}

	/// Path compression updates the ancestors of vertices along
	/// the path to the ancestor with the minimum label value.
	void compressPath(
		std::vector<DfsIndex>& _ancestor,
		std::vector<DfsIndex>& _label,
		std::vector<DfsIndex>& _semi,
		DfsIndex _vIdx
	) const
	{
		solAssert(_ancestor[_vIdx] != std::numeric_limits<size_t>::max());
		DfsIndex uIdx = _ancestor[_vIdx];
		if (_ancestor[uIdx] != std::numeric_limits<size_t>::max())
		{
			compressPath(_ancestor, _label, _semi, uIdx);
			if (_semi[_label[uIdx]] < _semi[_label[_vIdx]])
				_label[_vIdx] = _label[uIdx];
			_ancestor[_vIdx] = _ancestor[uIdx];
		}
		solAssert(_label[uIdx] <= _label[_vIdx]);
	}

	/// Build dominator tree from the immediate dominators set.
	/// The function groups all the vertices ids that are immediately dominated by a vertex.
	void buildDominatorTree()
	{
		// m_immediateDominators is guaranteed to have at least one element after findingDominators() is executed.
		solAssert(m_immediateDominators.size() > 0);
		solAssert(m_immediateDominators.size() == m_verticesInDFSOrder.size());
		solAssert(m_immediateDominators[0] == 0);

		// Ignoring the entry node since no one dominates it.
		for (DfsIndex index = 1; index < m_verticesInDFSOrder.size(); ++index)
		{
			VId vId = m_verticesInDFSOrder[index];
			solAssert(m_dfsIndexByVertexId.count(vId));
			solAssert(index == m_dfsIndexByVertexId.at(vId));
			VId idomId = m_verticesInDFSOrder[m_immediateDominators[index]];
			solAssert(m_immediateDominators[index] < index);
			m_dominatorTree[idomId].insert(vId);
		}
	}

	/// Keep the list of vertices ids in the DFS order.
	/// The entry vertex is the first element of the vector.
	/// The index of the other vertices is based on the order they are visited during the DFS.
	/// i.e. m_verticesInDFSOrder[i] is the vertex ID whose DFS index is i.
	///
	/// DFS index -> Vertex id
	std::vector<VId> m_verticesInDFSOrder;

	/// Maps a Vertex id to its DFS order index.
	/// NOTE: The CFG can be disconnected, so the vertex Ids are not necessarily contiguous.
	///
	/// Vertex id -> DFS index
	std::map<VId, DfsIndex> m_dfsIndexByVertexId;

	/// Maps a Vertex to all vertices that it dominates.
	/// If the vertex does not dominate any other vertex it has no entry in the map.
	/// The value is a set of vertex Ids dominated by the vertex whose ID is the map key.
	///
	/// Vertex id -> dominates set {Vertex id}
	std::map<VId, std::set<VId>> m_dominatorTree;

	/// Immediate dominators by DFS index.
	/// Maps a Vertex based on its DFS index (i.e. array index) to its immediate dominator DFS index.
	/// The entry vertex is the first element of the vector.
	///
	/// e.g. to get the immediate dominator of a Vertex w:
	/// idomIdx = m_immediateDominators[m_dfsIndexByVertexId[w.id]]
	/// idomVertexId = m_verticesInDFSOrder[domIdx]
	///
	/// DFS index -> dominates DFS index
	std::vector<DfsIndex> m_immediateDominators;
};
}
