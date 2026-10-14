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

#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>
#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libsolutil/DisjointSet.h>

#include <cstddef>
#include <set>
#include <vector>

namespace solidity::yul::ssa
{

/// Constructs a loop nesting forest for an SSACFG using Tarjan's algorithm [1].
///
/// [1] Ramalingam, Ganesan. "Identifying loops in almost linear time."
///     ACM Transactions on Programming Languages and Systems (TOPLAS) 21.2 (1999): 175-188.
class SSACFGLoopNestingForest
{
public:
	explicit SSACFGLoopNestingForest(analysis::DepthFirstSpanningTree const& _dfsTree);

	/// parent of `_block` in the loop nesting forest, empty if `_block` is not contained in a loop
	BlockId loopParent(BlockId _block) const { return m_loopParents[_block.value]; }
	/// all loop nodes (entry blocks for loops), also nested ones
	std::set<BlockId> const& loopNodes() const { return m_loopNodes; }
	/// root loop nodes in the forest for outer-most loops
	std::set<BlockId> const& loopRootNodes() const { return m_loopRootNodes; }
private:
	void findLoop(BlockId _potentialHeader);
	void collapse(std::set<BlockId::ValueType> const& _loopBody, BlockId::ValueType _loopHeader);

	analysis::DepthFirstSpanningTree const& m_dfsTree;
	SSACFG const& m_cfg;

	solidity::util::ContiguousDisjointSet<BlockId::ValueType> m_vertexPartition;
	std::vector<BlockId> m_loopParents;
	std::set<BlockId> m_loopNodes;
	std::set<BlockId> m_loopRootNodes;
};

}
