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

namespace solidity::yul
{

class ReducedTopologicalSort
{
public:
	static ReducedTopologicalSort run(SSACFG const& _cfg);

	std::vector<size_t> const& sortedBlocks() const { return m_reversedPostOrder; }
	std::vector<size_t> const& preOrder() const { return m_preOrder; }
	std::vector<size_t> const& maxSubtreePreOrder() const { return m_maxSubtreePreOrder; }
	std::set<size_t> const& backEdgeTargets() const { return m_backEdgeTargets; }
	bool ancestor(size_t _block1, size_t _block2) const;
	bool backEdge(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2) const { return ancestor(_block2.value, _block1.value); }

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
};

class SSACFGLiveness
{
public:
	using ReducedReachableNodes = std::map<SSACFG::BlockId, std::vector<SSACFG::BlockId>>;

	explicit SSACFGLiveness(SSACFG const& _cfg);

	std::vector<std::set<SSACFG::ValueId>> const& liveIns() const { return m_liveIns; }
	std::vector<std::set<SSACFG::ValueId>> const& liveOuts() const { return m_liveOuts; }

private:

	static ReducedReachableNodes computeReducedReachableNodes(SSACFG const& _cfg);

	void runDagDfs(
		SSACFG::BlockId v,
		std::vector<char>& _processed,
		std::vector<std::set<SSACFG::ValueId>>& _liveIns,
		std::vector<std::set<SSACFG::ValueId>>& _liveOuts
	);

	void runLoopTreeDfs(
		SSACFG::BlockId v,
		std::vector<std::set<SSACFG::ValueId>>& _liveIns,
		std::vector<std::set<SSACFG::ValueId>>& _liveOuts,
		std::set<SSACFG::BlockId> const& _loopNodes
	);

	SSACFG const& m_cfg;
	ReducedReachableNodes m_reducedReachableNodes;
	ReducedTopologicalSort m_topologicalSort;
	std::vector<std::set<SSACFG::ValueId>> m_liveIns;
	std::vector<std::set<SSACFG::ValueId>> m_liveOuts;
};

}
