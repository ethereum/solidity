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

#include <cstdint>
#include <vector>

namespace solidity::yul::ssa
{

/// Identifies blocks where stack balance constraints can be relaxed.
/// These are blocks that inevitably terminate down the line (i.e., there is no path to a function return exit) and
/// which are "bridge vertices". For a bridge, the graph decomposes into `G1` and `G2` with a singular edge `e=(v1->v2)`
/// between them. Therefore, traversal into `G2` cannot escape back into `G1` and in particular there cannot be a
/// parallel path into G2 that has relaxed constraints with respect to introducing junk. Consequently, there cannot be
/// a situation in which a junk-bloated stack has to be unified with a slimmer stack layout stemming from another path
/// into `G2`.
class JunkAdmittingBlocksFinder
{
public:
	explicit JunkAdmittingBlocksFinder(SSACFG const& _cfg, analysis::DepthFirstSpanningTree const& _dfsTree);
	bool allowsAdditionOfJunk(SSACFG::BlockId const& _blockId) const;
private:
	std::vector<std::uint8_t> m_blockAllowsJunk;
};

}
