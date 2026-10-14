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

#include <libyul/backends/evm/ssa/ShuffleTrace.h>

#include <utility>
#include <vector>

namespace solidity::yul::ssa
{

struct BlockLayout
{
	// stack layout required to enter the block
	StackData stackIn;

	/// One trace per Inst of the block
	std::vector<ShuffleTrace> operationShuffles;
	/// Transforms the stack after the last operation into the block's exit state (for conditional jumps: condition on top, pre-JUMPI)
	ShuffleTrace exitShuffle;
	/// Per predecessor edge: transforms the predecessor's post-exit stack (for conditional jumps: after
	/// popping the condition) into the phi preimage of `stackIn` under that edge
	std::vector<std::pair<SSACFG::BlockId, ShuffleTrace>> tracesForStackIn;

	/// The recorded shuffle for the edge from `_predecessor` into this block
	ShuffleTrace const& traceForStackIn(SSACFG::BlockId const& _predecessor) const
	{
		for (auto const& [parent, trace]: tracesForStackIn)
			if (parent == _predecessor)
				return trace;
		yulAssert(false, fmt::format("no recorded shuffle for predecessor edge from block {}", _predecessor));
		solidity::util::unreachable();
	}

	/// Records the shuffle for the edge from `_predecessor` into this block
	void addTraceForStackIn(SSACFG::BlockId const& _predecessor, ShuffleTrace&& _trace)
	{
		for (auto const& [parent, trace]: tracesForStackIn)
			if (parent == _predecessor)
			{
				yulAssert(
					trace == _trace,
					fmt::format("conflicting shuffles recorded for the predecessor edge from block {}", _predecessor)
				);
				return;
			}
		tracesForStackIn.emplace_back(_predecessor, std::move(_trace));
	}
};

/// For each (reachable) block in the SSACFG one block layout
class SSACFGStackLayout
{
public:
	SSACFGStackLayout(std::size_t const _numBlocks): m_blockLayouts(_numBlocks) {}

	std::optional<BlockLayout>& operator[](SSACFG::BlockId const& _blockId)
	{
		yulAssert(_blockId.hasValue() && _blockId.value < m_blockLayouts.size());
		return m_blockLayouts[_blockId.value];
	}

	std::optional<BlockLayout> const& operator[](SSACFG::BlockId const& _blockId) const
	{
		yulAssert(_blockId.hasValue() && _blockId.value < m_blockLayouts.size());
		return m_blockLayouts[_blockId.value];
	}

private:
	std::vector<std::optional<BlockLayout>> m_blockLayouts;
};

}
