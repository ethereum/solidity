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

#include <libyul/backends/evm/SSACFGLoopNestingForest.h>
#include <libyul/backends/evm/SSACFGTopologicalSort.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>

#include <cstddef>
#include <set>
#include <vector>

namespace solidity::yul
{

/// Performs liveness analysis on a reducible SSA CFG following Algorithm 9.1 in [1].
///
/// [1] Rastello, Fabrice, and Florent Bouchez Tichadou, eds. SSA-based Compiler Design. Springer, 2022.
class SSACFGLiveness
{
public:
	using LivenessData = std::set<SSACFG::ValueId>;
	explicit SSACFGLiveness(SSACFG const& _cfg);

	LivenessData const& liveIn(SSACFG::BlockId _blockId) const { return m_liveIns[_blockId.value]; }
	LivenessData const& liveOut(SSACFG::BlockId _blockId) const { return m_liveOuts[_blockId.value]; }
	std::vector<LivenessData> const& operationsLiveOut(SSACFG::BlockId _blockId) const { return m_operationLiveOuts[_blockId.value]; }
	ForwardSSACFGTopologicalSort const& topologicalSort() const { return m_topologicalSort; }
private:

	void runDagDfs();
	void runLoopTreeDfs(size_t _loopHeader);
	void fillOperationsLiveOut();

	SSACFG const& m_cfg;
	ForwardSSACFGTopologicalSort m_topologicalSort;
	SSACFGLoopNestingForest m_loopNestingForest;
	std::vector<LivenessData> m_liveIns;
	std::vector<LivenessData> m_liveOuts;
	std::vector<std::vector<LivenessData>> m_operationLiveOuts;
};

}
