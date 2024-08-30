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

	std::map<Vertex, std::set<Vertex>> ancestors;
};

class SSACFGLiveness
{
public:
	using ReducedReachableNodes = std::map<SSACFG::BlockId, std::vector<SSACFG::BlockId>>;

	SSACFGLiveness(SSACFG const& _cfg);

private:

	static ReducedReachableNodes computeReducedReachableNodes(SSACFG const& _cfg);
	static bool isConnectedInReducedGraph(SSACFG::BlockId v, SSACFG::BlockId w, SSACFG const& _cfg, std::set<SSACFGEdgeClassification::Edge> const& _backEdges);

	void runDagDfs(
		SSACFG::BlockId v,
		std::vector<char>& _processed,
		std::vector<std::set<SSACFG::ValueId>>& _liveIns,
		std::vector<std::set<SSACFG::ValueId>>& _liveOuts
	);

	SSACFG const& m_cfg;
	SSACFGEdgeClassification m_edgeClassification;
	ReducedReachableNodes m_reducedReachableNodes;
};

}
