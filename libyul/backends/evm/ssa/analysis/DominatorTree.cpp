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

#include <libyul/backends/evm/ssa/analysis/DominatorTree.h>

#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>

#include <libyul/Exceptions.h>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/to_container.hpp>

#include <algorithm>
#include <limits>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::analysis;

DominatorTree::DominatorTree(DepthFirstSpanningTree const& _dfsTree)
{
	SSACFG const& cfg = _dfsTree.cfg();
	auto const& preOrder = _dfsTree.preOrder();
	std::size_t const numConnectedBlocks = preOrder.size();
	std::size_t const numBlocks = cfg.numBlocks();

	m_idom.assign(numBlocks, SSACFG::BlockId{});
	m_domTreePreOrder.assign(numBlocks, 0);
	m_domTreeMaxSubtreePreOrder.assign(numBlocks, 0);

	// SEMI-NCA algorithm [GTW06, Section 2.3]
	// link and eval follow the simple version (path compression only) of [LT79]
	// All arrays are indexed by DFS index (0 = entry)
	std::vector<std::size_t> parent(numConnectedBlocks, 0);
	for (std::size_t i = 1; i < numConnectedBlocks; ++i)
		parent[i] = _dfsTree.preOrderIndexOf(_dfsTree.parentOf(preOrder[i]));

	std::vector semi = ranges::views::iota(0u, numConnectedBlocks) | ranges::to<std::vector<std::size_t>>();
	// label[v] = minimal semidominator value on the compressed forest path ending in a linked vertex v.
	// Unlike [LT79], where label holds a vertex attaining that minimum, it holds the value itself: SEMI-NCA
	// only needs the minimal value [GTW06, Section 2.3]
	std::vector label = semi;
	static std::size_t constexpr NO_FOREST_ANCESTOR = std::numeric_limits<std::size_t>::max();
	std::vector forestAncestor(numConnectedBlocks, NO_FOREST_ANCESTOR);

	// The `eval` operation of the link-eval structure [GTW06, Section 2.2], [LT79] with path
	// compression only: returns the semidominator value of `_v` if it is a forest root, and otherwise the minimal
	// semidominator value on the forest path from `_v` up to, but excluding, its root.
	// The recursive `compress` of [LT79] is unrolled: collect the path, then compress it top-down
	auto eval = [&, compressionPath = std::vector<std::size_t>{}](std::size_t const _v) mutable -> std::size_t {
		if (forestAncestor[_v] == NO_FOREST_ANCESTOR)
			return semi[_v];
		compressionPath.clear();
		for (std::size_t u = _v; forestAncestor[forestAncestor[u]] != NO_FOREST_ANCESTOR; u = forestAncestor[u])
			compressionPath.push_back(u);
		for (std::size_t const u: compressionPath | ranges::views::reverse)
		{
			label[u] = std::min(label[u], label[forestAncestor[u]]);
			forestAncestor[u] = forestAncestor[forestAncestor[u]];
		}
		return label[_v];
	};

	// GTW06, Section 2.3 (a): compute semidominators in reverse DFS preorder as sdom(w) = min{s_w(v) | v in pred(w)}
	// [GTW06, Section 2.2]. When w is processed, exactly the vertices > w are linked, so the forest root of a
	// predecessor v is NCA(D, {v, w}) and eval(v) = s_w(v)
	for (std::size_t w = numConnectedBlocks - 1; w >= 1; --w)
	{
		for (auto const& predBlock: cfg.block(preOrder[w]).entries)
			if (_dfsTree.reachable(predBlock))
				semi[w] = std::min(semi[w], eval(_dfsTree.preOrderIndexOf(predBlock)));
		// link(w, sdom(w)) [GTW06, Section 2.2]
		{
			forestAncestor[w] = parent[w];
			label[w] = semi[w];
		}
	}

	// GTW06, Section 2.3 (b): compute immediate dominators
	//
	// For each node in preorder, walk from its DFS-tree parent up toward the root (via already-computed idoms) until
	// reaching a node whose preorder index is at most that of its semidominator, i.e., the semidominator itself or
	// a DFS tree ancestor of it
	std::vector<std::size_t> idom(numConnectedBlocks, 0);
	for (std::size_t w = 1; w < numConnectedBlocks; ++w)
	{
		std::size_t runner = parent[w];
		while (runner > semi[w])
			runner = idom[runner];
		idom[w] = runner;
	}

	// Number the dominator tree in preorder, so that `dominates` becomes an interval check. The idom of a vertex
	// precedes it in DFS preorder, i.e., subtree sizes accumulate in reverse DFS preorder, and in DFS preorder each
	// vertex claims the next free part of its idom's interval
	std::vector<std::size_t> subtreeSize(numConnectedBlocks, 1);
	for (std::size_t w = numConnectedBlocks - 1; w >= 1; --w)
		subtreeSize[idom[w]] += subtreeSize[w];
	std::vector<std::size_t> domTreePreOrder(numConnectedBlocks, 0);
	std::vector<std::size_t> nextFreePreOrder(numConnectedBlocks, 0);
	nextFreePreOrder[0] = 1;
	for (std::size_t w = 1; w < numConnectedBlocks; ++w)
	{
		domTreePreOrder[w] = nextFreePreOrder[idom[w]];
		nextFreePreOrder[idom[w]] += subtreeSize[w];
		nextFreePreOrder[w] = domTreePreOrder[w] + 1;
	}

	// DFS indices -> block id values
	for (std::size_t i = 0; i < numConnectedBlocks; ++i)
	{
		auto const block = preOrder[i].value;
		m_idom[block] = preOrder[idom[i]];
		m_domTreePreOrder[block] = static_cast<SSACFG::BlockId::ValueType>(domTreePreOrder[i]);
		m_domTreeMaxSubtreePreOrder[block] = static_cast<SSACFG::BlockId::ValueType>(domTreePreOrder[i] + subtreeSize[i] - 1);
	}
}

bool DominatorTree::dominates(SSACFG::BlockId _dominator, SSACFG::BlockId _dominated) const
{
	yulAssert(
		reachable(_dominator) && reachable(_dominated),
		"Dominance is only defined for reachable blocks."
	);
	return
		m_domTreePreOrder[_dominator.value] <= m_domTreePreOrder[_dominated.value] &&
		m_domTreePreOrder[_dominated.value] <= m_domTreeMaxSubtreePreOrder[_dominator.value];
}

SSACFG::BlockId DominatorTree::immediateDominator(SSACFG::BlockId _block) const
{
	yulAssert(reachable(_block), "Immediate dominator is only defined for reachable blocks.");
	return m_idom[_block.value];
}

bool DominatorTree::reachable(SSACFG::BlockId _block) const
{
	yulAssert(_block.value < m_idom.size());
	return m_idom[_block.value].hasValue();
}
