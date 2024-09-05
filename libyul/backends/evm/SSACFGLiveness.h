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

#include <libyul/backends/evm/ControlFlow.h>

#include <libsolutil/DisjointSet.h>

namespace solidity::yul
{

class ReducedTopologicalSort
{
public:
	static ReducedTopologicalSort run(SSACFG const& _cfg);

	std::vector<size_t> const& sortedBlocks() const { return m_reversedPostOrder; }
	std::vector<size_t> const& preOrder() const { return m_preOrder; }
	std::vector<size_t> const& reversedPostOrder() const { return m_reversedPostOrder; }
	std::vector<size_t> const& maxSubtreePreOrder() const { return m_maxSubtreePreOrder; }
	std::set<size_t> const& backEdgeTargets() const { return m_backEdgeTargets; }
	std::vector<std::set<size_t>> const& predecessors() const { return m_predecessors; }
	bool ancestor(size_t _block1, size_t _block2) const;
	bool backEdge(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2) const {
		if (ancestor(_block2.value, _block1.value))
		{
			// check that block1 -> block2 is indeed an edge in the cfg
			bool isEdge = false;
			m_cfg.block(_block1).forEachExit([&_block2, &isEdge](SSACFG::BlockId const& _exit) { isEdge |= _block2 == _exit; });
			return isEdge;
		}

		return false;
	}

private:
	explicit ReducedTopologicalSort(SSACFG const& _cfg);
	void perform();
	void dfs(size_t _vertex);

	SSACFG const& m_cfg;

	size_t m_currentNode {0};
	std::vector<char> m_explored{};
	std::vector<size_t> m_reversedPostOrder{};
	std::vector<size_t> m_preOrder{};
	std::vector<size_t> m_maxSubtreePreOrder{};
	std::vector<std::tuple<size_t, size_t>> m_potentialBackEdges{};
	std::set<size_t> m_backEdgeTargets{};
	std::vector<std::set<size_t>> m_predecessors{};
};

class TarjansLoopNestingForest
{
	// Implementation according to
	// Ramalingam, Ganesan. "Identifying loops in almost linear time."
	// ACM Transactions on Programming Languages and Systems (TOPLAS) 21.2 (1999): 175-188.
public:
	TarjansLoopNestingForest(SSACFG const& _cfg, ReducedTopologicalSort const& _sort);

	/// blocks which are not contained in a loop get assigned the loop parent numeric_limit<size_t>::max()
	std::vector<size_t> const& loopParents() const { return m_loopParents; }
	std::set<size_t> const& loopNodes() const { return m_loopNodes; }
	std::set<size_t> const& loopRootNodes() const { return m_loopRootNodes; }
private:
	void build();
	void findLoop(size_t blockId);
	void collapse(std::set<size_t> const& _loopBody, size_t _loopHeader);

	SSACFG const& m_cfg;
	ReducedTopologicalSort const& m_sort;

	util::ContiguousDisjointSet m_vertexPartition;
	std::vector<size_t> m_loopParents;
	std::set<size_t> m_loopNodes;
	std::set<size_t> m_loopRootNodes;
};

class SSACFGLiveness
{
public:
	using ReducedReachableNodes = std::map<SSACFG::BlockId, std::vector<SSACFG::BlockId>>;

	explicit SSACFGLiveness(SSACFG const& _cfg);

	std::vector<std::set<SSACFG::ValueId>> const& liveIns() const { return m_liveIns; }
	std::vector<std::set<SSACFG::ValueId>> const& liveOuts() const { return m_liveOuts; }

private:

	void runDagDfs(
		SSACFG::BlockId v,
		std::vector<char>& _processed,
		std::vector<std::set<SSACFG::ValueId>>& _liveIns,
		std::vector<std::set<SSACFG::ValueId>>& _liveOuts
	);

	void runLoopTreeDfs(
		size_t v,
		std::vector<std::set<SSACFG::ValueId>>& _liveIns,
		std::vector<std::set<SSACFG::ValueId>>& _liveOuts
	);

	SSACFG const& m_cfg;
	ReducedReachableNodes m_reducedReachableNodes;
	ReducedTopologicalSort m_topologicalSort;
	TarjansLoopNestingForest m_loopNestingForest;
	std::vector<std::set<SSACFG::ValueId>> m_liveIns;
	std::vector<std::set<SSACFG::ValueId>> m_liveOuts;
};

}
