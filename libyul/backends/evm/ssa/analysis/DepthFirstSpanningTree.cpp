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

#include <libyul/backends/evm/ssa/analysis/DepthFirstSpanningTree.h>

using namespace solidity::yul::ssa::analysis;

DepthFirstSpanningTree::DepthFirstSpanningTree(SSACFG const& _cfg):
	m_cfg(_cfg),
	m_explored(m_cfg.numBlocks(), false), m_blockWisePreOrder(m_cfg.numBlocks(), 0),
	m_blockWiseMaxSubtreePreOrder(m_cfg.numBlocks(), 0),
	m_parent(m_cfg.numBlocks())
{
	m_preOrder.reserve(m_cfg.numBlocks());
	m_postOrder.reserve(m_cfg.numBlocks());
	dfs(m_cfg.entry);

	for (SSACFG::BlockId const source: m_preOrder)
		m_cfg.block(source).forEachExit([&](SSACFG::BlockId const& _target) {
			if (ancestor(_target, source))
				m_backEdgeTargets.insert(_target);
		});
}

void DepthFirstSpanningTree::dfs(SSACFG::BlockId const _block)
{
	yulAssert(!m_explored[_block.value]);
	m_explored[_block.value] = true;
	m_blockWisePreOrder[_block.value] = static_cast<SSACFG::BlockId::ValueType>(m_preOrder.size());
	m_preOrder.push_back(_block);

	m_cfg.block(_block).forEachExit([&](SSACFG::BlockId const& _exitBlock) {
		if (!m_explored[_exitBlock.value])
		{
			m_parent[_exitBlock.value] = _block;
			dfs(_exitBlock);
		}
	});

	// the subtree has been visited completely and occupies the pre-order indices up to here
	m_blockWiseMaxSubtreePreOrder[_block.value] = static_cast<SSACFG::BlockId::ValueType>(m_preOrder.size() - 1);
	m_postOrder.push_back(_block);
}

bool DepthFirstSpanningTree::ancestor(SSACFG::BlockId const _ancestor, SSACFG::BlockId const _block) const
{
	yulAssert(_ancestor.value < m_blockWisePreOrder.size());
	yulAssert(_block.value < m_blockWisePreOrder.size());

	auto const preOrderIndexAncestor = m_blockWisePreOrder[_ancestor.value];
	auto const preOrderIndexBlock = m_blockWisePreOrder[_block.value];

	bool const ancestorVisitedBeforeBlock = preOrderIndexAncestor <= preOrderIndexBlock;
	bool const blockInSubtreeOfAncestor = preOrderIndexBlock <= m_blockWiseMaxSubtreePreOrder[_ancestor.value];
	return ancestorVisitedBeforeBlock && blockInSubtreeOfAncestor;
}

bool DepthFirstSpanningTree::backEdge(SSACFG::BlockId const _source, SSACFG::BlockId const _target) const
{
	if (ancestor(_target, _source))
	{
		// check that source -> target is indeed an edge in the cfg
		bool isEdge = false;
		m_cfg.block(_source).forEachExit([&](SSACFG::BlockId const& _exit) { isEdge |= _target == _exit; });
		return isEdge;
	}
	return false;
}
