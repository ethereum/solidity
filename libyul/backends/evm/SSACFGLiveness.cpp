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

using namespace solidity::yul;

SSACFGLiveness::SSACFGLiveness(SSACFG const& _cfg):
	m_cfg(_cfg),
	m_topologicalSort(_cfg),
	m_loopNestingForest(m_topologicalSort),
	m_liveIns(_cfg.numBlocks()),
	m_liveOuts(_cfg.numBlocks())
{
	runDagDfs();
	for (auto const loopRootNode: m_loopNestingForest.loopRootNodes())
		runLoopTreeDfs(loopRootNode);
}
void SSACFGLiveness::runDagDfs()
{
	// SSA Book, Algorithm 9.2
	for(auto const blockIdValue: m_topologicalSort.postOrder())
	{
		// post-order traversal
		SSACFG::BlockId blockId{blockIdValue};
		auto const filterLiterals = [this](auto const& valueId){
			return !std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(valueId));
		};
		auto const& block = m_cfg.block(blockId);

		// live <- PhiUses(B)
		std::set<SSACFG::ValueId> live{};
		block.forEachExit(
			[&](SSACFG::BlockId const& _successor)
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
					if (argIndex < std::get<SSACFG::PhiValue>(info).arguments.size())
					{
						// todo not sure why arg index would exceed phi arg length but it does happen
						auto const arg = std::get<SSACFG::PhiValue>(info).arguments.at(argIndex);
						if (!std::holds_alternative<SSACFG::LiteralValue>(m_cfg.valueInfo(arg)))
							live.insert(arg);
					}
					else
						yulAssert(false);
				}
			});

		// for each S \in succs(B) s.t. (B, S) not a back edge: live <- live \cup (LiveIn(S) - PhiDefs(S))
		block.forEachExit(
			[&](SSACFG::BlockId const& _successor) {
				if (!m_topologicalSort.backEdge(blockId, _successor))
					live += m_liveIns[_successor.value] - m_cfg.block(_successor).phis;
			});
		if (std::holds_alternative<SSACFG::BasicBlock::FunctionReturn>(block.exit))
			live += std::get<SSACFG::BasicBlock::FunctionReturn>(block.exit).returnValues
					| ranges::views::filter(filterLiterals);

		// clean out unreachables
		live = live
			   | ranges::views::filter(
				   [&](auto const& valueId)
				   { return !std::holds_alternative<SSACFG::UnreachableValue>(m_cfg.valueInfo(valueId)); })
			   | ranges::to<std::set>;

		// LiveOut(B) <- live
		m_liveOuts[blockId.value] = live;

		// for each program point p in B, backwards, do:
		for (auto const& op: block.operations | ranges::views::reverse)
		{
			// remove variables defined at p from live
			live -= op.outputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
			// add uses at p to live
			live += op.inputs | ranges::views::filter(filterLiterals) | ranges::to<std::vector>;
		}

		// livein(b) <- live \cup PhiDefs(B)
		m_liveIns[blockId.value] = live + block.phis;
	}
}
void SSACFGLiveness::runLoopTreeDfs(size_t const _loopHeader)
{
	// SSA Book, Algorithm 9.3
	if (m_loopNestingForest.loopNodes().count(_loopHeader) > 0)
	{
		// the loop header block id
		auto const& block = m_cfg.block(SSACFG::BlockId{_loopHeader});
		// LiveLoop <- LiveIn(B_N) - PhiDefs(B_N)
		auto liveLoop = m_liveIns[_loopHeader] - block.phis;
		// must be live out of header if live in of children
		m_liveOuts[_loopHeader] += liveLoop;
		// for each blockId \in children(loopHeader)
		for (size_t blockId = 0; blockId < m_cfg.numBlocks(); ++blockId)
			if (m_loopNestingForest.loopParents()[blockId] == _loopHeader)
			{
				// propagate loop liveness information down to the loop header's children
				m_liveIns[blockId] += liveLoop;
				m_liveOuts[blockId] += liveLoop;

				runLoopTreeDfs(blockId);
			}
	}
}
