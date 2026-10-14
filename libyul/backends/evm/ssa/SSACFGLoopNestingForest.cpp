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

#include <libyul/backends/evm/ssa/SSACFGLoopNestingForest.h>

#include <range/v3/algorithm/reverse.hpp>

using namespace solidity::yul::ssa;

SSACFGLoopNestingForest::SSACFGLoopNestingForest(analysis::DepthFirstSpanningTree const& _dfsTree):
	m_dfsTree(_dfsTree),
	m_cfg(_dfsTree.cfg()),
	m_vertexPartition(m_cfg.numBlocks()),
	m_loopParents(m_cfg.numBlocks())
{
	auto dfsOrder = m_dfsTree.preOrder();
	// we go from innermost to outermost
	ranges::reverse(dfsOrder);

	for (auto const& blockId: dfsOrder)
		findLoop(blockId);

	// get the root nodes
	for (auto loopHeader: m_loopNodes)
	{
		while (m_loopParents[loopHeader.value].hasValue())
			loopHeader = m_loopParents[loopHeader.value];
		m_loopRootNodes.insert(loopHeader);
	}
}

void SSACFGLoopNestingForest::findLoop(BlockId const _potentialHeader)
{
	if (m_dfsTree.backEdgeTargets().contains(_potentialHeader))
	{
		std::set<BlockId::ValueType> loopBody;
		std::set<BlockId::ValueType> workList;
		for (auto const pred: m_cfg.block(_potentialHeader).entries)
		{
			auto const representative = m_vertexPartition.find(pred.value);
			if (
				representative != _potentialHeader.value &&
				m_dfsTree.backEdge(pred, _potentialHeader)
			)
				workList.insert(representative);
		}

		while (!workList.empty())
		{
			auto const y = workList.extract(workList.begin()).value();
			loopBody.insert(y);

			for (auto const& predecessor: m_cfg.block(SSACFG::BlockId{y}).entries)
			{
				if (!m_dfsTree.backEdge(predecessor, SSACFG::BlockId{y}))
				{
					auto const predecessorHeader = m_vertexPartition.find(predecessor.value);
					if (predecessorHeader != _potentialHeader.value && !loopBody.contains(predecessorHeader))
						workList.insert(predecessorHeader);
				}
			}
		}

		if (!loopBody.empty())
			collapse(loopBody, _potentialHeader.value);
	}
}
void SSACFGLoopNestingForest::collapse(std::set<BlockId::ValueType> const& _loopBody, BlockId::ValueType _loopHeader)
{
	for (auto const z: _loopBody)
	{
		m_loopParents[z] = BlockId{_loopHeader};
		m_vertexPartition.merge(_loopHeader, z, false);  // don't merge by size, loop header should be representative
	}
	yulAssert(m_vertexPartition.find(_loopHeader) == _loopHeader);  // representative was preserved
	m_loopNodes.insert(BlockId{_loopHeader});
}
