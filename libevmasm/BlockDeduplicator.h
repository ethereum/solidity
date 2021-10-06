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
 * @file BlockDeduplicator.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unifies basic blocks that share content.
 */

#pragma once

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>


#include <cstddef>
#include <vector>
#include <functional>
#include <map>

namespace solidity::evmasm
{

class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

/**
 * Optimizer class to be used to unify blocks that share content.
 * Modifies the passed vector in place.
 */
class BlockDeduplicator
{
public:
	explicit BlockDeduplicator(AssemblyItems& _items): m_items(_items) {}
	/// @returns true if something was changed
	bool deduplicate();
	/// @returns the tags that were replaced.
	std::map<u256, u256> const& replacedTags() const { return m_replacedTags; }

	/// Replaces all PushTag operations insied @a _items that match a key in
	/// @a _replacements by the respective value. If @a _subID is not -1, only
	/// apply the replacement for foreign tags from this sub id.
	/// @returns true iff a replacement was performed.
	static bool applyTagReplacement(
		AssemblyItems& _items,
		std::map<u256, u256> const& _replacements,
		size_t _subID = size_t(-1)
	);

private:
	/// Iterator that skips tags and skips to the end if (all branches of) the control
	/// flow does not continue to the next instruction.
	/// If the arguments are supplied to the constructor, replaces items on the fly.
	struct BlockIterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = AssemblyItem const;
		using difference_type = std::ptrdiff_t;
		using pointer = AssemblyItem const*;
		using reference = AssemblyItem const&;
		BlockIterator(
			AssemblyItems::const_iterator _it,
			AssemblyItems::const_iterator _end,
			AssemblyItem const* _replaceItem = nullptr,
			AssemblyItem const* _replaceWith = nullptr
		):
			it(_it), end(_end), replaceItem(_replaceItem), replaceWith(_replaceWith) {}
		BlockIterator& operator++();
		bool operator==(BlockIterator const& _other) const { return it == _other.it; }
		bool operator!=(BlockIterator const& _other) const { return it != _other.it; }
		AssemblyItem const& operator*() const;
		AssemblyItems::const_iterator it;
		AssemblyItems::const_iterator end;
		AssemblyItem const* replaceItem;
		AssemblyItem const* replaceWith;
	};

	std::map<u256, u256> m_replacedTags;
	AssemblyItems& m_items;
};

}
