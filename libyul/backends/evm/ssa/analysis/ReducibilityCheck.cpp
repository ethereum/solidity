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

#include <libyul/backends/evm/ssa/analysis/ReducibilityCheck.h>

#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>
#include <libyul/backends/evm/ssa/analysis/DominatorTree.h>

using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::analysis;

bool analysis::isReducibleCFG(DepthFirstSpanningTree const& _dfsTree)
{
	DominatorTree const dominatorTree(_dfsTree);
	for (auto const src: _dfsTree.preOrder())
	{
		bool reducible = true;
		_dfsTree.cfg().block(src).forEachExit([&](SSACFG::BlockId const& _target) {
			if (
				_dfsTree.ancestor(_target, src) &&
				!dominatorTree.dominates(_target, src)
			)
				reducible = false;
		});
		if (!reducible)
			return false;
	}
	return true;
}
