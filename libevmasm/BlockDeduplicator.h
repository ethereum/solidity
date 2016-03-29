/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file BlockDeduplicator.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unifies basic blocks that share content.
 */

#pragma once

#include <cstddef>
#include <vector>
#include <functional>

namespace dev
{
namespace eth
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
	BlockDeduplicator(AssemblyItems& _items): m_items(_items) {}
	/// @returns true if something was changed
	bool deduplicate();

private:
	/// Iterator that skips tags and skips to the end if (all branches of) the control
	/// flow does not continue to the next instruction.
	/// If the arguments are supplied to the constructor, replaces items on the fly.
	struct BlockIterator: std::iterator<std::forward_iterator_tag, AssemblyItem const>
	{
	public:
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

	AssemblyItems& m_items;
};

}
}
