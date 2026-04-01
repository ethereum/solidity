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

#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libyul/Exceptions.h>

#include <set>
#include <vector>

namespace solidity::yul::ssa::analysis
{

/// Depth-first spanning tree of the CFG, rooted at the entry block.
class DepthFirstSpanningTree
{
public:
	explicit DepthFirstSpanningTree(SSACFG const& _cfg);

	std::vector<SSACFG::BlockId> const& preOrder() const { return m_preOrder; }

	/// Reversed, this is a topological order of the CFG without its back edges
	std::vector<SSACFG::BlockId> const& postOrder() const { return m_postOrder; }

	std::set<SSACFG::BlockId> const& backEdgeTargets() const { return m_backEdgeTargets; }

	SSACFG const& cfg() const { return m_cfg; }

	/// Whether `_source -> _target` is an edge of the CFG whose target is a DFS-tree ancestor of its source
	/// (a retreating edge). In a reducible CFG, these are exactly the back edges, i.e., the target dominates the source.
	bool backEdge(SSACFG::BlockId _source, SSACFG::BlockId _target) const;

	SSACFG::BlockId::ValueType preOrderIndexOf(SSACFG::BlockId _block) const
	{
		yulAssert(reachable(_block));
		return m_blockWisePreOrder[_block.value];
	}

	SSACFG::BlockId::ValueType maxSubtreePreOrderIndexOf(SSACFG::BlockId _block) const
	{
		yulAssert(reachable(_block));
		return m_blockWiseMaxSubtreePreOrder[_block.value];
	}

	/// Parent of a reachable non-entry block in the DFS tree.
	SSACFG::BlockId parentOf(SSACFG::BlockId _block) const
	{
		yulAssert(reachable(_block) && _block != m_cfg.entry, "Parent only defined for reachable non-entry blocks.");
		return m_parent[_block.value];
	}

	/// Whether the block is reachable from the entry, i.e., was visited by the DFS.
	bool reachable(SSACFG::BlockId _block) const
	{
		yulAssert(_block.value < m_explored.size());
		return m_explored[_block.value];
	}

	/// Checks if block1 is an ancestor of block2, ie there's a path from block1 to block2 in the dfs tree
	bool ancestor(SSACFG::BlockId _ancestor, SSACFG::BlockId _block) const;

private:
	void dfs(SSACFG::BlockId _block);

	SSACFG const& m_cfg;
	std::vector<char> m_explored{};
	std::vector<SSACFG::BlockId> m_postOrder{};
	std::vector<SSACFG::BlockId> m_preOrder{};
	std::vector<SSACFG::BlockId::ValueType> m_blockWisePreOrder{};
	std::vector<SSACFG::BlockId::ValueType> m_blockWiseMaxSubtreePreOrder{};
	std::vector<SSACFG::BlockId> m_parent{};
	std::set<SSACFG::BlockId> m_backEdgeTargets{};
};
}
