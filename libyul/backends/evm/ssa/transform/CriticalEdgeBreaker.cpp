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

#include <libyul/backends/evm/ssa/transform/CriticalEdgeBreaker.h>

#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libyul/Exceptions.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/copy_if.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/range/conversion.hpp>

#include <iterator>
#include <variant>
#include <vector>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::ssa;

namespace
{

/// yields whether the target has several predecessors and carries phis
bool isCriticalPhiTarget(SSACFG const& _cfg, BlockId const _target)
{
	auto const& target = _cfg.block(_target);
	return
		target.entries.size() > 1 &&
		ranges::any_of(target.instructions, [&](InstId const _id) { return _cfg.isPhi(_id); });
}

/// Inserts a block on the edge from `_predecessor` to `_successor`, moves the predecessor's upsilons of the
/// successor's phis into it and redirects the predecessor's exit and the successor's entry to it
void splitEdge(SSACFG& _cfg, BlockId const _predecessor, BlockId const _successor)
{
	langutil::DebugData::ConstPtr const debugData = _cfg.debugInfo ? _cfg.debugInfo->exitDebugData(_predecessor) : nullptr;
	BlockId const edgeBlockId = _cfg.makeBlock(debugData);
	if (_cfg.debugInfo)
		_cfg.debugInfo->setExitDebugData(edgeBlockId, debugData);

	auto& edgeBlock = _cfg.block(edgeBlockId);
	auto& predecessor = _cfg.block(_predecessor);
	auto& successor = _cfg.block(_successor);

	edgeBlock.entries = {_predecessor};
	edgeBlock.exit = SSACFG::BasicBlock::Jump{_successor};

	// The upsilons of the successor's phis execute on the edge only
	auto const feedsSuccessor = [&](InstId const _id) {
		return _cfg.isUpsilon(_id) && _cfg.inst(_cfg.upsilonPhi(_id)).block == _successor;
	};
	ranges::copy_if(predecessor.instructions, std::back_inserter(edgeBlock.instructions), feedsSuccessor);
	std::erase_if(predecessor.instructions, feedsSuccessor);
	for (InstId const id: edgeBlock.instructions)
		_cfg.inst(id).block = edgeBlockId;

	auto& conditionalJump = std::get<SSACFG::BasicBlock::ConditionalJump>(predecessor.exit);
	yulAssert(conditionalJump.zero != conditionalJump.nonZero);
	(conditionalJump.zero == _successor ? conditionalJump.zero : conditionalJump.nonZero) = edgeBlockId;

	auto const entry = ranges::find(successor.entries, _predecessor);
	yulAssert(entry != successor.entries.end(), "edge target does not list the predecessor as entry");
	*entry = edgeBlockId;
}

}

void transform::breakCriticalEdges(SSACFG& _cfg)
{
	// we might add new blocks, so we work on a copy of the blocks
	std::vector<BlockId> const blocks = _cfg.liveBlocks() | ranges::to<std::vector>;
	for (BlockId const blockId: blocks)
	{
		auto const* conditionalJump = std::get_if<SSACFG::BasicBlock::ConditionalJump>(&_cfg.block(blockId).exit);
		if (!conditionalJump)
			continue;

		// splitting in this case would move all of the block's upsilons into the first edge block and leave the second without
		if (conditionalJump->zero == conditionalJump->nonZero)
			continue;

		BlockId const zero = conditionalJump->zero;
		BlockId const nonZero = conditionalJump->nonZero;

		if (zero != blockId && isCriticalPhiTarget(_cfg, zero))
			splitEdge(_cfg, blockId, zero);
		if (nonZero != blockId && isCriticalPhiTarget(_cfg, nonZero))
			splitEdge(_cfg, blockId, nonZero);
	}
}
