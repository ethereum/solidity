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

#include <libyul/backends/evm/ssa/SSACFG.h>

#include <vector>

namespace solidity::yul::ssa::analysis
{

class DepthFirstSpanningTree;

/// Computes the dominator tree of an SSACFG using the SEMI-NCA algorithm in its SNCA variant.
///
/// See in particular Section 2.3 of
/// [GTW06] L. Georgiadis, R. E. Tarjan, R. F. Werneck. Finding Dominators in Practice. Journal of Graph Algorithms and Applications, 10(1):69-94, 2006.
/// https://doi.org/10.7155/jgaa.00119
/// and
/// [LT79] T. Lengauer, R. E. Tarjan. A Fast Algorithm for Finding Dominators in a Flowgraph. ACM Transactions on Programming Languages and Systems, 1(1):121-141, 1979.
class DominatorTree
{
public:
	explicit DominatorTree(DepthFirstSpanningTree const& _dfsTree);

	/// Returns true if `_dominator` dominates `_dominated`. Both must be reachable.
	bool dominates(SSACFG::BlockId _dominator, SSACFG::BlockId _dominated) const;

	SSACFG::BlockId immediateDominator(SSACFG::BlockId _block) const;

private:
	bool reachable(SSACFG::BlockId _block) const;

	/// indexed by block ID value, stores the immediate dominator; no value for unreachable blocks
	std::vector<SSACFG::BlockId> m_idom;
	/// indexed by block ID value, preorder numbering of the dominator tree and the largest number within each subtree
	std::vector<SSACFG::BlockId::ValueType> m_domTreePreOrder;
	std::vector<SSACFG::BlockId::ValueType> m_domTreeMaxSubtreePreOrder;
};

}
