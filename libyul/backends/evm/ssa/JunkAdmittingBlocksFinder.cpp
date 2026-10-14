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

#include <libyul/backends/evm/ssa/JunkAdmittingBlocksFinder.h>

#include <libyul/backends/evm/ssa/BridgeFinder.h>

namespace solidity::yul::ssa
{

JunkAdmittingBlocksFinder::JunkAdmittingBlocksFinder(SSACFG const& _cfg, analysis::DepthFirstSpanningTree const& _dfsTree):
	m_blockAllowsJunk(_cfg.numBlocks(), false)
{
	// special case: only one block here, we mark it as junkable in case it's not a function return
	if (_dfsTree.preOrder().size() == 1)
	{
		SSACFG::BlockId const id = _dfsTree.preOrder().front();
		m_blockAllowsJunk[id.value] = !_cfg.block(id).isFunctionReturnBlock();
		return;
	}

	// Find all bridges, i.e., vertices, which upon removal increase the number of connected components.
	// Translated to SSA CFGs this means:
	//   - control flow that enters a bridge vertex never returns to a previously visited block
	//   - there is no parallel path to a child of the vertex, ie, adding junk is fine in terms of stack balance
	BridgeFinder const bridgeFinder(_cfg);

	// of the bridge vertices, we have the exclude the ones that can lead to a function return
	std::vector<SSACFG::BlockId> toVisit;
	for (SSACFG::BlockId const blockId: _dfsTree.preOrder())
	{
		bool const isLoopHead = _dfsTree.backEdgeTargets().contains(blockId);
		m_blockAllowsJunk[blockId.value] = (bridgeFinder.bridgeVertex(blockId) && !isLoopHead) || _cfg.block(blockId).isTerminationBlock();
		if (_cfg.block(blockId).isFunctionReturnBlock())
			toVisit.emplace_back(blockId);
	}

	std::vector<uint8_t> visited(_cfg.numBlocks(), false);
	while (!toVisit.empty())
	{
		auto const blockId = toVisit.back();
		auto const& block = _cfg.block(blockId);
		toVisit.pop_back();

		m_blockAllowsJunk[blockId.value] = false;
		visited[blockId.value] = true;
		for (auto const& entry: block.entries)
			if (!visited[entry.value])
				toVisit.emplace_back(entry);
	}
}

bool JunkAdmittingBlocksFinder::allowsAdditionOfJunk(SSACFG::BlockId const& _blockId) const
{
	return m_blockAllowsJunk[_blockId.value];
}

}
