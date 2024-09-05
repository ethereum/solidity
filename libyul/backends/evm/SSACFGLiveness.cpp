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
	m_loopNodes.insert(std::numeric_limits<size_t>::max());
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
	m_loopNodes.insert(_loopHeader);
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
	m_topologicalSort(ReducedTopologicalSort::run(_cfg)),
	m_loopNestingForest(_cfg, m_topologicalSort),
	m_liveIns(_cfg.numBlocks()),
	m_liveOuts(_cfg.numBlocks())
{
	std::vector<char> processed(_cfg.numBlocks(), false);

	runDagDfs(_cfg.entry, processed, m_liveIns, m_liveOuts);
	runLoopTreeDfs(std::numeric_limits<size_t>::max(), m_liveIns, m_liveOuts);
}

void SSACFGLiveness::runLoopTreeDfs
(
	size_t v,
	std::vector<std::set<SSACFG::ValueId>>& _liveIns,
	std::vector<std::set<SSACFG::ValueId>>& _liveOuts
)
{
	// if v is a loop node of the nesting forest (or the very root, ie max(size_t))
	if (m_loopNestingForest.loopNodes().count(v) > 0)
	{
		// the loop header block id
		size_t loopHeader = v == std::numeric_limits<size_t>::max() ? 0 : v;
		auto const& block = m_cfg.block(SSACFG::BlockId{loopHeader});
		// LiveLoop <- LiveIn(B_N) - PhiDefs(B_N)
		auto liveLoop = _liveIns[loopHeader] - block.phis;
		// todo this could be done smarter by doing the mapping first
		for (size_t blockId = 0; blockId < m_cfg.numBlocks(); ++blockId)
		{
			if (m_loopNestingForest.loopParents()[blockId] == v)
			{
				_liveIns[blockId] += liveLoop;
				_liveOuts[blockId] += liveLoop;
				runLoopTreeDfs(blockId, _liveIns, _liveOuts);
			}
		}
	}
}


void SSACFGLiveness::runDagDfs(SSACFG::BlockId blockId, std::vector<char>& _processed, std::vector<std::set<SSACFG::ValueId>>& _liveIns, std::vector<std::set<SSACFG::ValueId>>& _liveOuts)
{
	// SSA Book, Algorithm 9.2
	auto const filterLiterals = [this](auto const& valueId) {
		return !std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(valueId));
	};
	auto const& block = m_cfg.block(blockId);
	// for each S \in succs(B) s.t. (B, S) not back edge and S unprocessed: DAG_DFS(S)
	// todo functionreturn has to be respected still
	block.forEachExit([&](SSACFG::BlockId const& _successor){
		if (!m_topologicalSort.backEdge(blockId, _successor))
			if (!_processed[_successor.value])
				runDagDfs(_successor, _processed, _liveIns, _liveOuts);
	});

	// live <- PhiUses(B)
	std::set<SSACFG::ValueId> live{};
	block.forEachExit([&](SSACFG::BlockId const& _successor)
	{
		for (auto const& phi: m_cfg.block(_successor).phis)
		{
			auto const& info = m_cfg.valueInfo(phi);
			yulAssert(std::holds_alternative<SSACFG::PhiValue>(info), "value info of phi wasn't PhiValue");
			auto const& entries = m_cfg.block(std::get<SSACFG::PhiValue>(info).block).entries;
			// this is getting the argument index of the phi function corresponding to the path going
			// through "blockId", ie, the currently handled block
			auto const it = entries.find(blockId);
			yulAssert(it != entries.end());
			auto const argIndex = static_cast<size_t>(std::distance(entries.begin(), it));
			auto const arg = std::get<SSACFG::PhiValue>(info).arguments.at(argIndex);
			if (!std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(arg)))
				live.insert(arg);
		}
	});

	// for each S \in succs(B) s.t. (B, S) not a back edge: live <- live \cup (LiveIn(S) - PhiDefs(S))
	block.forEachExit([&](SSACFG::BlockId const& _successor){
		if (!m_topologicalSort.backEdge(blockId, _successor))
			live += _liveIns[_successor.value] - m_cfg.block(_successor).phis;
	});

	// LiveOut(B) <- live
	_liveOuts[blockId.value] = live;

	// for each program point p in B, backwards, do:
	for (auto const& op: block.operations | ranges::views::reverse)
	{
		// remove variables defined at p from live
		live -= op.outputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
		// add uses at p to live
		live += op.inputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
	}

	// livein(b) <- live \cup PhiDefs(B)
	_liveIns[blockId.value] = live + block.phis;

	// mark processed
	_processed[blockId.value] = true;
}
