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

#include <libyul/backends/evm/SSACFGTopologicalSort.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>

#include <libsolutil/DisjointSet.h>

#include <cstddef>
#include <set>
#include <vector>

namespace solidity::yul
{

/// Constructs a loop nesting forest for an SSACFG using Tarjan's algorithm [1].
///
/// [1] Ramalingam, Ganesan. "Identifying loops in almost linear time."
///     ACM Transactions on Programming Languages and Systems (TOPLAS) 21.2 (1999): 175-188.
class SSACFGLoopNestingForest
{
public:
	explicit SSACFGLoopNestingForest(ForwardSSACFGTopologicalSort const& _sort);

	/// blocks which are not contained in a loop get assigned the loop parent numeric_limit<size_t>::max()
	std::vector<size_t> const& loopParents() const;
	/// all loop nodes (entry blocks for loops), also nested ones
	std::set<size_t> const& loopNodes() const;
	/// root loop nodes in the forest for outer-most loops
	std::set<size_t> const& loopRootNodes();
private:
	void findLoop(size_t _potentialHeader);
	void collapse(std::set<size_t> const& _loopBody, size_t _loopHeader);

	ForwardSSACFGTopologicalSort const& m_sort;
	SSACFG const& m_cfg;

	util::ContiguousDisjointSet m_vertexPartition;
	std::vector<size_t> m_loopParents;
	std::set<size_t> m_loopNodes;
	std::set<size_t> m_loopRootNodes;
};

}
