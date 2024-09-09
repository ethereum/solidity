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

/// Performs liveness analysis on an SSA CFG following Algorithm 9.1 in [1].
///
/// [1] Rastello, Fabrice, and Florent Bouchez Tichadou, eds. SSA-based Compiler Design. Springer, 2022.
class SSACFGLiveness
{
public:
	explicit SSACFGLiveness(SSACFG const& _cfg);

	std::vector<std::set<SSACFG::ValueId>> const& liveIns() const { return m_liveIns; }
	std::vector<std::set<SSACFG::ValueId>> const& liveOuts() const { return m_liveOuts; }

private:

	void runDagDfs();
	void runLoopTreeDfs(size_t _loopHeader);

	SSACFG const& m_cfg;
	ForwardSSACFGTopologicalSort m_topologicalSort;
	SSACFGLoopNestingForest m_loopNestingForest;
	std::vector<std::set<SSACFG::ValueId>> m_liveIns;
	std::vector<std::set<SSACFG::ValueId>> m_liveOuts;
};

}
