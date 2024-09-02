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

bool ReducedTopologicalSort::ancestor(SSACFG::BlockId const& _block1, SSACFG::BlockId const& _block2, bool checked) const
{
	// todo if we assume iota block indices we can reduce this to just size_t everywhere
	//		or we remap it to iota before doing anything further; then no find lookup is required
	auto it1 = std::find(m_preOrder.begin(), m_preOrder.end(), _block1);
	yulAssert(!checked || it1 != m_preOrder.end(), "Preorder didn't contain block");
	if (it1 == m_preOrder.end()) return false;
	auto it2 = std::find(m_preOrder.begin(), m_preOrder.end(), _block2);
	yulAssert(!checked || it2 != m_preOrder.end(), "Preorder didn't contain block");
	if (it2 == m_preOrder.end()) return false;

	auto preOrderIndex1 = static_cast<size_t>(std::distance(m_preOrder.begin(), it1));
	auto preOrderIndex2 = static_cast<size_t>(std::distance(m_preOrder.begin(), it2));

	bool node1VisitedBeforeNode2 = preOrderIndex1 <= preOrderIndex2;
	bool node2InSubtreeOfNode1 = preOrderIndex2 <= m_maxSubtreePreOrder[preOrderIndex1];
	return node1VisitedBeforeNode2 && node2InSubtreeOfNode1;
}

ReducedTopologicalSort::ReducedTopologicalSort(SSACFG const& _cfg): m_cfg(_cfg)
{
	m_reversedPostOrder.reserve(m_cfg.numBlocks());
	m_preOrder.reserve(m_cfg.numBlocks());
	m_maxSubtreePreOrder.reserve(m_cfg.numBlocks());
}

void ReducedTopologicalSort::perform()
{
	for (size_t id = 0; id < m_cfg.numBlocks(); ++id)
	{
		SSACFG::BlockId blockId{id};
		if (m_explored.find(blockId) == m_explored.end())
			dfs(blockId);
	}
	std::reverse(m_reversedPostOrder.begin(), m_reversedPostOrder.end());
}

size_t ReducedTopologicalSort::dfs(SSACFG::BlockId _vertex)
{
	m_preOrder.emplace_back(_vertex);
	size_t preOrderNumber = m_currentNode;
	++m_currentNode;
	m_explored.insert(_vertex);
	auto const& block = m_cfg.block(_vertex);

	block.forEachExit([&](SSACFG::BlockId const& _exitBlock){
		// we have an edge e = id -> _exitBlock
		// check if it's an back-edge, ie, _exitBlock is in the toVisit block
		//if (m_edgeClassification.backEdges.find(std::make_tuple(_vertex, _exitBlock)) == m_edgeClassification.backEdges.end())
		bool const isBackEdge = ancestor(_exitBlock, _vertex, false);
		if (!isBackEdge)
			preOrderNumber = std::max(preOrderNumber, dfs(_exitBlock));
		else
			m_backEdgeTargets.emplace(_vertex);
	});
	m_reversedPostOrder.push_back(_vertex);
	m_maxSubtreePreOrder.emplace_back(preOrderNumber);
	return preOrderNumber;
}

SSACFGEdgeClassification::SSACFGEdgeClassification(SSACFG const& _cfg)
{
	std::set<Vertex> explored{};
	Vertex blockId = _cfg.entry;
	explored.insert(blockId);

	std::vector<Vertex> toVisit{};
	toVisit.emplace_back(blockId);
	std::vector<Vertex> path{};

	while (!toVisit.empty())
	{
		auto const id = toVisit.back();
		toVisit.pop_back();
		auto const& block = _cfg.block(id);

		path.emplace_back(id);
		block.forEachExit([&](Vertex const& _exitBlock){
			if (explored.find(_exitBlock) == explored.end())
			{
				explored.insert(_exitBlock);
				toVisit.emplace_back(_exitBlock);
				treeEdges.emplace(id, _exitBlock);
			}
			else
			{
				// todo we have to checkpoint the path or maybe not. let's try with topological sort
				bool const isBackEdge = std::find(path.begin(), path.end(), _exitBlock) != path.end();
				if (isBackEdge)
					backEdges.emplace(id, _exitBlock);
				if (!isBackEdge)
					forwardEdges.emplace(id, _exitBlock);
				else
					crossEdges.emplace(id, _exitBlock);
				path.clear();
			}
		});
	}
}

SSACFGLiveness::SSACFGLiveness(SSACFG const& _cfg):
	m_cfg(_cfg),
	m_edgeClassification(_cfg),
	m_reducedReachableNodes(computeReducedReachableNodes(_cfg)),
	m_topologicalSort(ReducedTopologicalSort::run(_cfg)),
	m_liveIns(_cfg.numBlocks()),
	m_liveOuts(_cfg.numBlocks())
{
	std::vector<char> processed(_cfg.numBlocks(), false);

	runDagDfs(_cfg.entry, processed, m_liveIns, m_liveOuts);

	// m_edgeClassification.backEdges
	// 	| ranges::views::transform([](auto const& edge) { return std::get<1>(edge); })
	// 	| ranges::to<std::set>();
	std::set<SSACFG::BlockId> loopNestingForestRootNodes = m_topologicalSort.backEdgeTargets();

	for (auto const& rootNode: loopNestingForestRootNodes)
		runLoopTreeDfs(SSACFG::BlockId{rootNode}, m_liveIns, m_liveOuts, loopNestingForestRootNodes);
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


bool SSACFGLiveness::isConnectedInReducedGraph(SSACFG::BlockId v, SSACFG::BlockId w, SSACFG const& _cfg, std::set<SSACFGEdgeClassification::Edge> const& _backEdges)
{
	std::set<SSACFG::BlockId> explored{};
	SSACFG::BlockId blockId = v;
	if (explored.find(blockId) == explored.end())
	{
		explored.insert(blockId);

		std::vector<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(blockId);

		while (!toVisit.empty())
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
	for (auto const& op: block.operations | ranges::views::reverse)
	{
		live -= op.outputs;
		live += op.inputs;
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
