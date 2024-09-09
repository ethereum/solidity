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

#include <libyul/backends/evm/SSACFGLoopNestingForest.h>

using namespace solidity::yul;

SSACFGLoopNestingForest::SSACFGLoopNestingForest(ForwardSSACFGTopologicalSort const& _sort):
	m_sort(_sort),
	m_cfg(_sort.cfg()),
	m_vertexPartition(m_cfg.numBlocks()),
	m_loopParents(m_cfg.numBlocks(), std::numeric_limits<size_t>::max())
{
	auto dfsOrder = m_sort.preOrder();
	// we go from innermost to outermost
	std::reverse(dfsOrder.begin(), dfsOrder.end());

	for (auto const& blockId: dfsOrder)
		findLoop(blockId);

	// get the root nodes
	for (auto loopHeader: m_loopNodes)
	{
		while (m_loopParents[loopHeader] != std::numeric_limits<size_t>::max())
			loopHeader = m_loopParents[loopHeader];
		m_loopRootNodes.insert(loopHeader);
	}
}

void SSACFGLoopNestingForest::findLoop(size_t const _potentialHeader)
{
	if (m_sort.backEdgeTargets().count(_potentialHeader) > 0)
	{
		std::set<size_t> loopBody;
		std::set<size_t> workList;
		for (auto const pred: m_cfg.block(SSACFG::BlockId{_potentialHeader}).entries)
		{
			auto const representative = m_vertexPartition.find(pred.value);
			if (
				representative != _potentialHeader &&
				m_sort.backEdge(SSACFG::BlockId{pred}, SSACFG::BlockId{_potentialHeader})
			)
				workList.insert(representative);
		}

		while (!workList.empty())
		{
			auto const y = workList.extract(workList.begin()).value();
			loopBody.insert(y);

			for (auto const& predecessor: m_cfg.block(SSACFG::BlockId{y}).entries)
			{
				if (!m_sort.backEdge(SSACFG::BlockId{predecessor}, SSACFG::BlockId{y}))
				{
					auto const predecessorHeader = m_vertexPartition.find(predecessor.value);
					if (predecessorHeader != _potentialHeader && loopBody.count(predecessorHeader) == 0)
						workList.insert(predecessorHeader);
				}
			}
		}

		if (!loopBody.empty())
			collapse(loopBody, _potentialHeader);
	}
}
void SSACFGLoopNestingForest::collapse(std::set<size_t> const& _loopBody, size_t _loopHeader)
{
	for (auto const z: _loopBody)
	{
		m_loopParents[z] = _loopHeader;
		m_vertexPartition.merge(_loopHeader, z, false);  // don't merge by size, loop header should be representative
	}
	yulAssert(m_vertexPartition.find(_loopHeader) == _loopHeader);  // representative was preserved
	m_loopNodes.insert(_loopHeader);
}
