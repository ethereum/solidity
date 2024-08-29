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

#include "SSAControlFlowGraphBuilder.h"


#include <libyul/backends/evm/SSACFGLiveness.h>
#include <stack>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{
class ReducedTopologicalSort
{
public:
	static std::vector<SSACFG::BlockId> run(SSACFG const& _cfg, SSACFGLiveness::BackEdges const& _backEdges)
	{
		ReducedTopologicalSort sort(_cfg, _backEdges);
		sort.perform();
		return sort.m_reversedPostOrder;
	}

private:
	ReducedTopologicalSort(SSACFG const& _cfg, SSACFGLiveness::BackEdges const& _backEdges): m_cfg(_cfg), m_backEdges(_backEdges)
	{
		m_reversedPostOrder.reserve(m_cfg.numBlocks());
	}

	void perform()
	{
		for (size_t id = 0; id < m_cfg.numBlocks(); ++id)
		{
			SSACFG::BlockId blockId{id};
			if (m_explored.find(blockId) == m_explored.end())
				dfs(blockId);
		}
		std::reverse(m_reversedPostOrder.begin(), m_reversedPostOrder.end());
	}

	void dfs(SSACFG::BlockId _vertex)
	{
		m_explored.insert(_vertex);
		auto const& block = m_cfg.block(_vertex);

		block.forEachExit([&](SSACFG::BlockId const& _exitBlock){
			// we have an edge e = id -> _exitBlock
			// check if it's an back-edge, ie, _exitBlock is in the toVisit block
			if (m_backEdges.find(std::make_tuple(_vertex, _exitBlock)) == m_backEdges.end())
			{
				dfs(_exitBlock);
			}
		});
		m_reversedPostOrder.push_back(_vertex);
	}

	SSACFG const& m_cfg;
	SSACFGLiveness::BackEdges const& m_backEdges;

	std::set<SSACFG::BlockId> m_explored{};
	std::vector<SSACFG::BlockId> m_reversedPostOrder{};
};
}

SSACFGLiveness::SSACFGLiveness(SSACFG const& _cfg):
	m_reducedReachableNodes(computeReducedReachableNodes(_cfg))
{ }

SSACFGLiveness::BackEdges SSACFGLiveness::findBackEdges(SSACFG const& _cfg)
{
	std::set<std::tuple<SSACFG::BlockId, SSACFG::BlockId>> backEdges;
	std::set<SSACFG::BlockId> explored{};
	SSACFG::BlockId blockId = _cfg.entry;;
	if (explored.find(blockId) == explored.end())
	{
		explored.insert(blockId);

		std::vector<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(blockId);

		while(!toVisit.empty())
		{
			auto const id = toVisit.back();
			toVisit.pop_back();
			auto const& block = _cfg.block(id);

			block.forEachExit([&](SSACFG::BlockId const& _exitBlock){
				// we have an edge e = id -> _exitBlock
				// check if it's an back-edge, ie, _exitBlock is in the toVisit block
				if (std::find(toVisit.begin(), toVisit.end(), _exitBlock) != toVisit.end())
				{
					backEdges.emplace(id, _exitBlock);
				}
				explored.insert(_exitBlock);
				toVisit.emplace_back(_exitBlock);
			});
		}
	}
	return backEdges;
}

bool SSACFGLiveness::isConnectedInReducedGraph(SSACFG::BlockId v, SSACFG::BlockId w, SSACFG const& _cfg, BackEdges const& _backEdges)
{
	std::set<SSACFG::BlockId> explored{};
	SSACFG::BlockId blockId = v;;
	if (explored.find(blockId) == explored.end())
	{
		explored.insert(blockId);

		std::vector<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(blockId);

		while(!toVisit.empty())
		{
			auto const id = toVisit.back();
			if (id == w)
				return true;
			toVisit.pop_back();
			auto const& block = _cfg.block(id);

			block.forEachExit([&](SSACFG::BlockId const& _exitBlock){
				// we have an edge e = id -> _exitBlock
				// check if it's an back-edge, ie, _exitBlock is in the toVisit block
				if (_backEdges.find(std::make_tuple(id, _exitBlock)) == _backEdges.end())
				{
					explored.insert(_exitBlock);
					toVisit.emplace_back(_exitBlock);
				}
			});
		}
	}
	return false;
}


SSACFGLiveness::ReducedReachableNodes SSACFGLiveness::computeReducedReachableNodes(SSACFG const& _cfg)
{
	ReducedReachableNodes result;

	if (_cfg.numBlocks() > 1)
	{
		auto const backEdges = findBackEdges(_cfg);
		auto const order = ReducedTopologicalSort::run(_cfg, backEdges);
		yulAssert(order.size() == _cfg.numBlocks(), "Invalid number of nodes in sort");
		// todo this can very likely be done more smartly by using bfs and marking on the way!
		// there can only be a path between v and w if v < w in topological sort
		for (auto it = order.begin(); it != order.end()-1; ++it)
		{
			for (auto it2 = it + 1; it2 != order.end(); ++it2)
			{
				if (isConnectedInReducedGraph(*it, *it2, _cfg, backEdges))
					result[*it].push_back(*it2);
			}
		}
	}

	return result;
}
