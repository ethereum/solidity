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
/**
 * @file Inliner.h
 * Inlines small code snippets by replacing JUMP with a copy of the code jumped to.
 */
#pragma once

#include <libsolutil/Common.h>
#include <range/v3/view/span.hpp>
#include <map>
#include <vector>

namespace solidity::evmasm
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

class Inliner
{
public:
	explicit Inliner(AssemblyItems& _items, size_t _inlineMaxOpcodes): m_items(_items), m_inlineMaxOpcodes(_inlineMaxOpcodes) {}
	virtual ~Inliner() = default;

	void optimise();

private:
	struct InlinableBlock
	{
		ranges::span<AssemblyItem const> items;
		uint64_t pushTagCount = 0;
	};

	bool isInlineCandidate(u256 const& _tag, InlinableBlock const& _block) const;
	/// @returns the exit jump for the block to be inlined, if a particular jump to it should be inlined, otherwise nullopt.
	std::optional<AssemblyItem> shouldInline(u256 const& _tag, AssemblyItem const& _jump, InlinableBlock const& _block) const;
	std::map<u256, InlinableBlock> determineInlinableBlocks(AssemblyItems const& _items) const;

	AssemblyItems& m_items;
	size_t const m_inlineMaxOpcodes;
};

}
