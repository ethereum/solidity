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

#include <libyul/backends/evm/SSACFGLiveness.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/reverse.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

ReducedTopologicalSort ReducedTopologicalSort::run(SSACFG const& _cfg)
{
	ReducedTopologicalSort sort(_cfg);
	sort.perform();
	return sort;
}

bool ReducedTopologicalSort::ancestor(size_t _block1, size_t _block2) const
{
	yulAssert(_block1 < m_preOrder.size());
	yulAssert(_block2 < m_preOrder.size());

	auto const preOrderIndex1 = m_preOrder[_block1];
	auto const preOrderIndex2 = m_preOrder[_block2];

	bool const node1VisitedBeforeNode2 = preOrderIndex1 <= preOrderIndex2;
	bool const node2InSubtreeOfNode1 = preOrderIndex2 <= m_maxSubtreePreOrder[_block1];
	return node1VisitedBeforeNode2 && node2InSubtreeOfNode1;
}

ReducedTopologicalSort::ReducedTopologicalSort(SSACFG const& _cfg):
	m_cfg(_cfg),
	m_explored(m_cfg.numBlocks(), false),
	m_reversedPostOrder(m_cfg.numBlocks(), std::numeric_limits<size_t>::max()),
	m_preOrder(m_cfg.numBlocks(), std::numeric_limits<size_t>::max()),
	m_maxSubtreePreOrder(m_cfg.numBlocks(), 0),
	m_predecessors(m_cfg.numBlocks())
{
	perform();
}

void ReducedTopologicalSort::perform()
{
	for (size_t id = 0; id < m_cfg.numBlocks(); ++id)
	{
		if (!m_explored[id])
			dfs(id);
	}
	std::reverse(m_reversedPostOrder.begin(), m_reversedPostOrder.end());

	for (auto const& [v1, v2]: m_potentialBackEdges)
		if (ancestor(v2, v1))
			m_backEdgeTargets.insert(v2);
}

void ReducedTopologicalSort::dfs(size_t _vertex)
{
	yulAssert(!m_explored[_vertex]);
	m_explored[_vertex] = true;
	m_preOrder[_vertex] = m_currentNode++;
	m_maxSubtreePreOrder[_vertex] = m_preOrder[_vertex];

	m_cfg.block(SSACFG::BlockId{_vertex}).forEachExit([&](SSACFG::BlockId const& _exitBlock){
		m_predecessors[_exitBlock.value].insert(_vertex);
		// we have an edge e = id -> _exitBlock
		// check if it's an back-edge, ie, _exitBlock is in the toVisit block
		//if (m_edgeClassification.backEdges.find(std::make_tuple(_vertex, _exitBlock)) == m_edgeClassification.backEdges.end())
		if (!m_explored[_exitBlock.value])
		{
			dfs(_exitBlock.value);
			m_maxSubtreePreOrder[_vertex] = std::max(m_maxSubtreePreOrder[_vertex], m_maxSubtreePreOrder[_exitBlock.value]);
		}
		else
			m_potentialBackEdges.emplace_back(_vertex, _exitBlock.value);
	});

	m_reversedPostOrder[m_preOrder[_vertex]] = _vertex;
}

void TarjansLoopNestingForest::build()
{
	auto dfsOrder = m_sort.reversedPostOrder();
	// we go from inner-most to outer-most
	std::reverse(dfsOrder.begin(), dfsOrder.end());

	for (auto const& blockId : dfsOrder)
		findLoop(blockId);
}

TarjansLoopNestingForest::TarjansLoopNestingForest(SSACFG const& _cfg, ReducedTopologicalSort const& _sort):
	m_cfg(_cfg),
	m_sort(_sort),
	m_vertexPartition(m_cfg.numBlocks()),
	m_loopParents(m_cfg.numBlocks(), std::numeric_limits<size_t>::max())
{
	build();
	for (size_t i = 0; i < m_loopParents.size(); ++i)
	{
		fmt::print("Block {} has loop parent {}\n", i, m_loopParents[i]);
	}
}

void TarjansLoopNestingForest::collapse(std::set<size_t> const& _loopBody, size_t _loopHeader)
{
	for (auto const z : _loopBody)
	{
		m_loopParents[z] = _loopHeader;
		m_vertexPartition.merge(_loopHeader, z, false);  // don't merge by size, loop header should be representative
	}
	yulAssert(m_vertexPartition.find(_loopHeader) == _loopHeader);  // representative was preserved
}

void TarjansLoopNestingForest::findLoop(size_t potentialHeader)
{
	if(m_sort.backEdgeTargets().count(potentialHeader) > 0)
	{
		std::set<size_t> loopBody;
		std::set<size_t> workList;
		for (auto const pred : m_sort.predecessors()[potentialHeader])
		{
			auto const representative = m_vertexPartition.find(pred);
			if (representative != potentialHeader && m_sort.backEdge(SSACFG::BlockId{pred}, SSACFG::BlockId{potentialHeader}))
				workList.insert(representative);
		}

		while (!workList.empty())
		{
			auto const y = workList.extract(workList.begin()).value();
			loopBody.insert(y);

			for (auto const& predecessor : m_sort.predecessors()[y])
			{
				if (!m_sort.backEdge(SSACFG::BlockId{predecessor}, SSACFG::BlockId{y}))
				{
					auto const predecessorHeader = m_vertexPartition.find(predecessor);
					if (predecessorHeader != potentialHeader && loopBody.count(predecessorHeader) == 0)
						workList.insert(predecessorHeader);
				}
			}
		}

		if (!loopBody.empty())
			collapse(loopBody, potentialHeader);
	}
}

SSACFGLiveness::SSACFGLiveness(SSACFG const& _cfg):
	m_cfg(_cfg),
	m_reducedReachableNodes(computeReducedReachableNodes(_cfg)),
	m_topologicalSort(ReducedTopologicalSort::run(_cfg)),
	m_loopNestingForest(_cfg, m_topologicalSort),
	m_liveIns(_cfg.numBlocks()),
	m_liveOuts(_cfg.numBlocks())
{
	std::vector<char> processed(_cfg.numBlocks(), false);

	runDagDfs(_cfg.entry, processed, m_liveIns, m_liveOuts);

	std::set<size_t> loopNestingForestRootNodes = m_topologicalSort.backEdgeTargets();

	for (auto const& rootNode: loopNestingForestRootNodes)
		runLoopTreeDfs(SSACFG::BlockId{rootNode}, m_liveIns, m_liveOuts, {} /* todo loopNestingForestRootNodes */);
}

void SSACFGLiveness::runLoopTreeDfs
(
	SSACFG::BlockId v,
	std::vector<std::set<SSACFG::ValueId>>& _liveIns,
	std::vector<std::set<SSACFG::ValueId>>& _liveOuts,
	std::set<SSACFG::BlockId> const& _loopNodes
)
{
	if (_loopNodes.count(v) > 0)
	{
		auto const& block = m_cfg.block(v);
		auto liveLoop = _liveIns[v.value] - block.phis;
		block.forEachExit([&](auto const& _exitBlockId) {
			_liveIns[_exitBlockId.value] += liveLoop;
			_liveOuts[_exitBlockId.value] += liveLoop;
			runLoopTreeDfs(_exitBlockId, _liveIns, _liveOuts, _loopNodes);
		});
	}
}


void SSACFGLiveness::runDagDfs(SSACFG::BlockId blockId, std::vector<char>& _processed, std::vector<std::set<SSACFG::ValueId>>& _liveIns, std::vector<std::set<SSACFG::ValueId>>& _liveOuts)
{
	// SSA Book, Algorithm 9.2
	auto const& block = m_cfg.block(blockId);
	std::set<SSACFG::ValueId> live{};
	block.forEachExit([&](SSACFG::BlockId const& _successor){
		for (auto const& phi: m_cfg.block(_successor).phis)
		{
			auto const& info = m_cfg.valueInfo(phi);
			yulAssert(std::holds_alternative<SSACFG::PhiValue>(info), "value info of phi wasn't PhiValue");
			live += std::get<SSACFG::PhiValue>(info).arguments;
		}
		if (!m_topologicalSort.backEdge(blockId, _successor))
			if (!_processed[_successor.value])
				runDagDfs(_successor, _processed, _liveIns, _liveOuts);
	});
	block.forEachExit([&](SSACFG::BlockId const& _successor){
		if (!m_topologicalSort.backEdge(blockId, _successor))
			live += _liveIns[_successor.value] - m_cfg.block(_successor).phis;
	});
	_liveOuts[blockId.value] = live;
	auto const filterLiterals = [this](auto const& valueId) {
		return !std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(valueId));
	};
	for (auto const& op: block.operations | ranges::views::reverse)
	{
		live -= op.outputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
		live += op.inputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
	}
	_liveIns[blockId.value] = live + block.phis;
	_processed[blockId.value] = true;
}


SSACFGLiveness::ReducedReachableNodes SSACFGLiveness::computeReducedReachableNodes(SSACFG const& _cfg)
{
	ReducedReachableNodes result;

	std::vector<char> processed(_cfg.numBlocks(), false);
	std::vector<std::set<SSACFG::ValueId>> liveIns (_cfg.numBlocks());
	std::vector<std::set<SSACFG::ValueId>> liveOuts (_cfg.numBlocks());

	/*if (_cfg.numBlocks() > 1)
	{
		SSACFGEdgeClassification edgeClassification(_cfg);
		auto const order = ReducedTopologicalSort::run(_cfg, edgeClassification);
		yulAssert(order.sortedBlocks().size() == _cfg.numBlocks(), "Invalid number of nodes in sort");
		// todo this can very likely be done more smartly by using bfs and marking on the way!
		// there can only be a path between v and w if v < w in topological sort
		for (auto it = order.sortedBlocks().begin(); it != order.sortedBlocks().end()-1; ++it)
			for (auto it2 = it + 1; it2 != order.sortedBlocks().end(); ++it2)
				if (order.ancestor(*it, *it2))  // isConnectedInReducedGraph(*it, *it2, _cfg, edgeClassification.backEdges)
					result[*it].push_back(*it2);
	}*/

	return result;
}
