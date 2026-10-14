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

#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>
#include <libyul/backends/evm/ssa/SSACFG.h>
#include <libyul/backends/evm/ssa/SSACFGLoopNestingForest.h>
#include <libyul/backends/evm/ssa/util/UseCountSet.h>

#include <boost/container/flat_map.hpp>

#include <vector>

namespace solidity::yul::ssa
{

/// Performs liveness analysis on a reducible SSA CFG following Algorithm 9.1 in [1].
///
/// [1] Rastello, Fabrice, and Florent Bouchez Tichadou, eds. SSA-based Compiler Design. Springer, 2022.
class LivenessAnalysis
{
public:
	/// Per-program-point liveness, each value's use count is the max number of times the value will be read along
	/// all paths downstream of that point
	using LivenessData = util::UseCountSet<InstId>;

	explicit LivenessAnalysis(SSACFG const& _cfg);

	LivenessData const& liveIn(SSACFG::BlockId const _blockId) const { return m_liveIns[_blockId.value]; }
	LivenessData const& liveOut(SSACFG::BlockId const _blockId) const { return m_liveOuts[_blockId.value]; }
	LivenessData used(SSACFG::BlockId _blockId) const;
	LivenessData const& operationLiveOut(InstId const _id) const { return m_operationLiveOutByInst.at(_id.value); }
	analysis::DepthFirstSpanningTree const& dfsTree() const { return m_dfsTree; }
	SSACFG const& cfg() const { return m_cfg; }

private:
	void runDagDfs();
	void runLoopTreeDfs(SSACFG::BlockId _loopHeader);
	void fillOperationsLiveOut();
	LivenessData blockExitValues(SSACFG::BlockId const& _blockId) const;

	auto excludingLiteralsFilter() const
	{
		return [this](InstId _v) { return !m_cfg.isLiteral(_v); };
	}

	SSACFG const& m_cfg;
	analysis::DepthFirstSpanningTree m_dfsTree;
	SSACFGLoopNestingForest m_loopNestingForest;
	std::vector<LivenessData> m_liveIns;
	std::vector<LivenessData> m_liveOuts;
	boost::container::flat_map<InstId::ValueType, LivenessData> m_operationLiveOutByInst;
};

}
