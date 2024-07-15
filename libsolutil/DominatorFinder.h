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
///
/// The graph must contain at least one vertex (the entry point) and is assumed to not be disconnected.
/// Only vertices reachable from the entry vertex are visited.
template<typename V, typename ForEachSuccessor>
class DominatorFinder
{
public:
	static_assert(has_id<V>::value, "vertex must define id member");
	using VId = typename V::Id;
	using DfsIndex = size_t;

	DominatorFinder(V const& _entry):
		m_immediateDominators(findDominators(_entry))
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

	std::map<VId, std::optional<VId>> const immediateDominators() const
	{
		return m_immediateDominators
			| ranges::views::enumerate
			| ranges::views::transform([&](auto const& _v) {
				std::optional<VId> idomId = (_v.second.has_value()) ? std::optional<VId>(m_verticesInDFSOrder[_v.second.value()]) : std::nullopt;
				return std::make_pair(m_verticesInDFSOrder[_v.first], idomId);
			})
			| ranges::to<std::map<VId, std::optional<VId>>>;
	}

	std::vector<std::optional<DfsIndex>> const& immediateDominatorsByDfsIndex() const
	{
		return m_immediateDominators;
	}

	std::map<VId, std::vector<VId>> const& dominatorTree() const
	{
		return m_dominatorTree;
	}

	bool dominates(VId const& _dominatorId, VId const& _dominatedId) const
	{
		solAssert(!m_dfsIndexByVertexId.empty());
		solAssert(!m_immediateDominators.empty());

		solAssert(m_dfsIndexByVertexId.count(_dominatorId) && m_dfsIndexByVertexId.count(_dominatedId));
		DfsIndex dominatorIdx = m_dfsIndexByVertexId.at(_dominatorId);
		DfsIndex dominatedIdx = m_dfsIndexByVertexId.at(_dominatedId);

		if (dominatorIdx == dominatedIdx)
			return true;

		DfsIndex idomIdx = m_immediateDominators.at(dominatedIdx).value_or(0);
		while (idomIdx != 0)
		{
			solAssert(idomIdx < m_immediateDominators.size());
			if (idomIdx == dominatorIdx)
				return true;
			// The index of the immediate dominator of a vertex is always less than the index of the vertex itself.
			solAssert(m_immediateDominators.at(idomIdx).has_value() && m_immediateDominators.at(idomIdx).value() < idomIdx);
			idomIdx = m_immediateDominators[idomIdx].value();
		}
		// Now that we reached the entry node (i.e. idomIdx = 0),
		// either ``dominatorIdx == 0`` or it does not dominate the other node.
		solAssert(idomIdx == 0);
		return dominatorIdx == 0;
	}

	/// Checks whether vertex ``_dominator`` dominates ``_dominated`` by going
	/// through the path from ``_dominated`` to the entry node.
	/// If ``_dominator`` is found, then it dominates ``_dominated``
	/// otherwise it doesn't.
	bool dominates(V const& _dominator, V const& _dominated) const
	{
		return dominates(_dominator.id, _dominated.id);
	}

	std::vector<VId> dominatorsOf(VId const& _vId) const
	{
		solAssert(!m_verticesInDFSOrder.empty());
		solAssert(!m_dfsIndexByVertexId.empty());
		solAssert(!m_immediateDominators.empty());

		solAssert(m_dfsIndexByVertexId.count(_vId));

		// No one dominates the entry vertex and we consider self-dominance implicit
		// i.e. every node already dominates itself.
		if (m_dfsIndexByVertexId.at(_vId) == 0)
			return {};

		solAssert(m_dfsIndexByVertexId.at(_vId) < m_immediateDominators.size());
		DfsIndex idomIdx = m_immediateDominators.at(m_dfsIndexByVertexId.at(_vId)).value_or(0);
		solAssert(idomIdx < m_immediateDominators.size());

		std::vector<VId> dominators;
		while (idomIdx != 0)
		{
			solAssert(m_immediateDominators.at(idomIdx).has_value() && m_immediateDominators.at(idomIdx).value() < idomIdx);
			dominators.emplace_back(m_verticesInDFSOrder.at(idomIdx));
			idomIdx = m_immediateDominators[idomIdx].value();
		}

		// The loop above discovers the dominators in the reverse order
		// i.e. from the given vertex upwards to the entry node (the root of the dominator tree).
		// And the entry vertex always dominates all other vertices.
		dominators.emplace_back(m_verticesInDFSOrder[0]);

		return dominators;
	}

	/// Find all dominators of a node _v
	/// @note for a vertex ``_v``, the _v’s inclusion in the set of dominators of ``_v`` is implicit.
	std::vector<VId> dominatorsOf(V const& _v) const
	{
		return dominatorsOf(_v.id);
	}

private:
	std::vector<std::optional<DfsIndex>> findDominators(V const& _entry)
	{
		solAssert(m_verticesInDFSOrder.empty());
		solAssert(m_dfsIndexByVertexId.empty());

		// parent(w): The index of the vertex which is the parent of ``w`` in the spanning
		// tree generated by the DFS.
		std::vector<DfsIndex> parent;

		// step 1
		// The vertices are assigned indices in DFS order.
		std::set<VId> visited;
		DfsIndex nextUnusedDFSIndex = 0;

		auto dfs = [&](V const& _v, auto _dfs) -> void {
			auto [_, inserted] = visited.insert(_v.id);
			if (!inserted)
				return;

			m_verticesInDFSOrder.emplace_back(_v.id);
			m_dfsIndexByVertexId[_v.id] = nextUnusedDFSIndex;
			nextUnusedDFSIndex++;

			ForEachSuccessor{}(_v, [&](V const& _successor) {
				if (visited.count(_successor.id) == 0)
				{
					parent.push_back(m_dfsIndexByVertexId[_v.id]);
					m_predecessors.push_back({m_dfsIndexByVertexId[_v.id]});
					_dfs(_successor, _dfs);
				}
				else
				{
					solAssert(m_dfsIndexByVertexId[_successor.id] < m_predecessors.size());
					m_predecessors[m_dfsIndexByVertexId[_successor.id]].insert(m_dfsIndexByVertexId[_v.id]);
				}
			});
		};

		parent.emplace_back(std::numeric_limits<DfsIndex>::max());
		m_predecessors.emplace_back();
		dfs(_entry, dfs);

		size_t numVertices = visited.size();
		solAssert(nextUnusedDFSIndex == numVertices);
		solAssert(m_verticesInDFSOrder.size() == numVertices);
		solAssert(visited.size() == numVertices);
		solAssert(m_predecessors.size() == numVertices);
		solAssert(parent.size() == numVertices);

		// ancestor(w): Parent of vertex ``w`` in the virtual forest traversed by eval().
		// The forest consists of disjoint subtrees of the spanning tree and the parent of ``w`` is
		// always one of its ancestors in that spanning tree.
		// Initially each subtree consists of a single vertex. As the algorithm iterates over the
		// graph, each processed vertex gets connected to its parent from the spanning tree using
		// link(). Later on, the path compression performed by eval() may move it up in the subtree.
		std::vector<DfsIndex> ancestor(numVertices, std::numeric_limits<DfsIndex>::max());
		// label(w): The index of a vertex with the smallest semidominator, on the path between ``w``
		// and the root of its subtree. The value is not updated immediately by link(), but
		// only during path compression performed by eval().
		std::vector<DfsIndex> label;
		// bucket(w): The set of all vertices having ``w`` as their semidominator.
		// The index of the array represents the vertex' DFS index.
		std::vector<std::deque<DfsIndex>> bucket(numVertices);

		// semi(w): The DFS index of the semidominator of ``w``.
		std::vector<DfsIndex> semi;
		// idom(w): The DFS index of the immediate dominator of ``w``.
		std::vector<std::optional<DfsIndex>> idom(numVertices, std::nullopt);

		// ``link(v, w)`` adds an edge from ``w`` to ``v`` in the virtual forest.
		// It is meant to initially attach vertex ``w`` to its parent from the spanning tree,
		// but path compression can later limit the search path upwards.
		// TODO: implement sophisticated link-eval algorithm as shown in pg 132
		// See: https://www.cs.princeton.edu/courses/archive/spr03/cs423/download/dominators.pdf
		auto link = [&](DfsIndex _parentIdx, DfsIndex _wIdx)
		{
			solAssert(ancestor[_wIdx] == std::numeric_limits<DfsIndex>::max());
			ancestor[_wIdx] = _parentIdx;
		};

		// ``eval(v)`` returns a vertex with the smallest semidominator index on the path between
		// vertex ``v`` and the root of its subtree in the virtual forest, i.e. the label of ``v``.
		// Performs path compression in the process, updating labels and ancestors on the path to
		// the subtree root.
		auto eval = [&](DfsIndex _vIdx) -> DfsIndex
		{
			if (ancestor[_vIdx] == std::numeric_limits<DfsIndex>::max())
				return _vIdx;

			compressPath(ancestor, label, semi, _vIdx);
			return label[_vIdx];
		};

		auto toDfsIndex = [&](VId const& _vId) { return m_dfsIndexByVertexId[_vId]; };

		for (DfsIndex wIdx = 0; wIdx < m_verticesInDFSOrder.size(); ++wIdx)
		{
			semi.push_back(wIdx);
			label.push_back(wIdx);
		}

		// Process the vertices in decreasing order of the DFS number
		for (DfsIndex wIdx: m_verticesInDFSOrder | ranges::views::reverse | ranges::views::transform(toDfsIndex))
		{
			// step 3
			// NOTE: this is an optimization, i.e. performing the step 3 before step 2.
			// The goal is to process the bucket at the beginning of the loop for the vertex ``w``
			// instead of ``parent[w]`` at the end of the loop as described in the original paper.
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
			for (DfsIndex vIdx: m_predecessors[wIdx])
			{
				DfsIndex uIdx = eval(vIdx);
				solAssert(uIdx <= vIdx);
				if (semi[uIdx] < semi[wIdx])
					semi[wIdx] = semi[uIdx];
			}
			solAssert(semi[wIdx] < wIdx || wIdx == 0);
			bucket[semi[wIdx]].emplace_back(wIdx);
			link(parent[wIdx], wIdx);
			solAssert(ancestor[wIdx] == parent[wIdx]);
		}

		// step 4
		// Compute idom in DFS order
		// The entry vertex does not have an immediate dominator.
		solAssert(idom[0] == std::nullopt);
		for (DfsIndex wIdx: m_verticesInDFSOrder | ranges::views::drop(1) | ranges::views::transform(toDfsIndex))
		{
			// All the other vertices must have an immediate dominator.
			solAssert(idom[wIdx].has_value());
			if (idom[wIdx].value() != semi[wIdx])
				idom[wIdx] = idom[idom[wIdx].value()];
		}

		return idom;
	}

	/// Path compression updates the ancestors of vertices along
	/// the path to the ancestor with the minimum label value.
	static void compressPath(
		std::vector<DfsIndex>& _ancestor,
		std::vector<DfsIndex>& _label,
		std::vector<DfsIndex> const& _semi,
		DfsIndex _vIdx
	)
	{
		solAssert(_vIdx < _ancestor.size() && _vIdx < _semi.size() && _vIdx < _label.size());
		solAssert(_ancestor[_vIdx] != std::numeric_limits<size_t>::max());
		solAssert(_ancestor[_vIdx] < _vIdx);
		if (_ancestor[_ancestor[_vIdx]] != std::numeric_limits<DfsIndex>::max())
		{
			solAssert(_ancestor[_ancestor[_vIdx]] < _ancestor[_vIdx]);
			compressPath(_ancestor, _label, _semi, _ancestor[_vIdx]);
			if (_semi[_label[_ancestor[_vIdx]]] < _semi[_label[_vIdx]])
				_label[_vIdx] = _label[_ancestor[_vIdx]];
			_ancestor[_vIdx] = _ancestor[_ancestor[_vIdx]];
		}
		solAssert(_label[_ancestor[_vIdx]] <= _label[_vIdx]);
	}

	/// Build dominator tree from the immediate dominators set.
	/// The function groups all the vertex IDs that are immediately dominated by a vertex.
	void buildDominatorTree()
	{
		// m_immediateDominators is guaranteed to have at least one element after findingDominators() is executed.
		solAssert(m_immediateDominators.size() > 0);
		solAssert(m_immediateDominators.size() == m_verticesInDFSOrder.size());
		solAssert(m_immediateDominators[0] == std::nullopt);

		// Ignoring the entry node since no one dominates it.
		for (DfsIndex dominatedIdx = 1; dominatedIdx < m_verticesInDFSOrder.size(); ++dominatedIdx)
		{
			VId dominatedId = m_verticesInDFSOrder[dominatedIdx];
			solAssert(m_dfsIndexByVertexId.count(dominatedId));
			solAssert(dominatedIdx == m_dfsIndexByVertexId.at(dominatedId));

			// If the vertex does not have an immediate dominator, it is the entry vertex (i.e. index 0).
			// NOTE: `dominatedIdx` will never be 0 since the loop starts from 1.
			solAssert(m_immediateDominators[dominatedIdx].has_value());
			DfsIndex dominatorIdx = m_immediateDominators[dominatedIdx].value();

			solAssert(dominatorIdx < dominatedIdx);
			VId dominatorId = m_verticesInDFSOrder[dominatorIdx];
			m_dominatorTree[dominatorId].emplace_back(dominatedId);
		}
	}

	// predecessors(w): The set of vertices ``v`` such that (``v``, ``w``) is an edge of the graph.
	std::vector<std::set<DfsIndex>> m_predecessors;

	/// Keeps the list of vertex IDs in the DFS order.
	/// The entry vertex is the first element of the vector.
	/// The indices of the other vertices are assigned in the order they are visited during the DFS.
	/// I.e. m_verticesInDFSOrder[i] is the ID of the vertex whose DFS index is i.
	///
	/// DFS index -> vertex ID
	std::vector<VId> m_verticesInDFSOrder;

	/// Maps a vertex ID to its DFS order index.
	///
	/// Vertex ID -> DFS index
	std::map<VId, DfsIndex> m_dfsIndexByVertexId;

	/// Maps a vertex to all vertices that it dominates.
	/// If the vertex does not dominate any other vertex it has no entry in the map.
	/// The value is a set of IDs of vertices dominated by the vertex whose ID is the map key.
	///
	/// Vertex id -> dominates set {vertex ID}
	std::map<VId, std::vector<VId>> m_dominatorTree;

	/// Immediate dominators by DFS index.
	/// Maps a vertex' DFS index (i.e. array index) to its immediate dominator DFS index.
	/// As the entry vertex does not have immediate dominator, its idom is always set to `std::nullopt`.
	/// However note that the DFS index of the entry vertex is 0, since it is the first element of the vector.
	///
	/// E.g. to get the immediate dominator of a Vertex w:
	/// idomIdx = m_immediateDominators[m_dfsIndexByVertexId[w.id]]
	/// idomVertexId = m_verticesInDFSOrder[domIdx]
	///
	/// DFS index -> dominates DFS index
	std::vector<std::optional<DfsIndex>> m_immediateDominators;
};
}
