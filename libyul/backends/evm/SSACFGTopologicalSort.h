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

#include <libyul/backends/evm/SSAControlFlowGraph.h>

#include <cstddef>
#include <set>
#include <vector>

namespace solidity::yul
{

/// Performs a topological sort on the forward CFG (no back/cross edges)
class ForwardSSACFGTopologicalSort
{
public:
	explicit ForwardSSACFGTopologicalSort(SSACFG const& _cfg);

	std::vector<size_t> const& preOrder() const { return m_preOrder; }
	std::vector<size_t> const& postOrder() const { return m_postOrder; }
	std::set<size_t> const& backEdgeTargets() const { return m_backEdgeTargets; }
	SSACFG const& cfg() const { return m_cfg; }
	bool backEdge(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2) const;
	size_t preOrderIndexOf(size_t _block) const { return m_blockWisePreOrder[_block]; }
	size_t maxSubtreePreOrderIndexOf(size_t _block) const { return m_blockWiseMaxSubtreePreOrder[_block]; }

private:
	void dfs(size_t _vertex);
	/// Checks if block1 is an ancestor of block2, ie there's a path from block1 to block2 in the dfs tree
	bool ancestor(size_t _block1, size_t _block2) const;

	SSACFG const& m_cfg;
	std::vector<char> m_explored{};
	std::vector<size_t> m_postOrder{};
	std::vector<size_t> m_preOrder{};
	std::vector<size_t> m_blockWisePreOrder{};
	std::vector<size_t> m_blockWiseMaxSubtreePreOrder{};
	std::vector<std::tuple<size_t, size_t>> m_potentialBackEdges{};
	std::set<size_t> m_backEdgeTargets{};
};
}
