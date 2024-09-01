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

struct SSACFGEdgeClassification
{
	SSACFGEdgeClassification(SSACFG const& _cfg);

	using Vertex = SSACFG::BlockId;
	using Edge = std::tuple<Vertex, Vertex>;

	std::set<Edge> treeEdges;
	std::set<Edge> backEdges;
	std::set<Edge> forwardEdges;
	std::set<Edge> crossEdges;
};

class ReducedTopologicalSort
{
public:
	static ReducedTopologicalSort run(SSACFG const& _cfg);

	std::vector<SSACFG::BlockId> const& sortedBlocks() const { return m_reversedPostOrder; }
	std::vector<SSACFG::BlockId> const& preOrder() const { return m_preOrder; }
	std::set<SSACFG::BlockId> const& backEdgeTargets() const { return m_backEdgeTargets; }
	bool ancestor(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2, bool checked=true) const;
	bool backEdge(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2) const { return ancestor(_block2, _block1); }

private:
	explicit ReducedTopologicalSort(SSACFG const& _cfg);
	void perform();
	size_t dfs(SSACFG::BlockId _vertex);

	SSACFG const& m_cfg;

	size_t m_currentNode {0};
	std::set<SSACFG::BlockId> m_explored{};
	std::vector<SSACFG::BlockId> m_reversedPostOrder{};
	std::vector<SSACFG::BlockId> m_preOrder{};
	std::vector<size_t> m_maxSubtreePreOrder{};
	std::set<SSACFG::BlockId> m_backEdgeTargets{};
};

class SSACFGLiveness
{
public:
	using ReducedReachableNodes = std::map<SSACFG::BlockId, std::vector<SSACFG::BlockId>>;

	explicit SSACFGLiveness(SSACFG const& _cfg);

	std::vector<std::set<SSACFG::ValueId>> const& liveIns() const { return m_liveIns; }
	std::vector<std::set<SSACFG::ValueId>> const& liveOuts() const { return m_liveOuts; }
	SSACFGEdgeClassification const& edgeClassification() const { return m_edgeClassification; }

private:

	static ReducedReachableNodes computeReducedReachableNodes(SSACFG const& _cfg);
	static bool isConnectedInReducedGraph(SSACFG::BlockId v, SSACFG::BlockId w, SSACFG const& _cfg, std::set<SSACFGEdgeClassification::Edge> const& _backEdges);

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
	SSACFGEdgeClassification m_edgeClassification;
	ReducedReachableNodes m_reducedReachableNodes;
	ReducedTopologicalSort m_topologicalSort;
	std::vector<std::set<SSACFG::ValueId>> m_liveIns;
	std::vector<std::set<SSACFG::ValueId>> m_liveOuts;
};

}
