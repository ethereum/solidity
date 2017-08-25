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
/**
 * @author Alex Beregszaszi
 * Removes unused JUMPDESTs.
 */
#pragma once

#include <vector>
#include <cstddef>
#include <set>

namespace dev
{
namespace eth
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

class JumpdestRemover
{
public:
	explicit JumpdestRemover(AssemblyItems& _items): m_items(_items) {}

	bool optimise(std::set<size_t> const& _tagsReferencedFromOutside);

	/// @returns a set of all tags from the given sub-assembly that are referenced
	/// from the given list of items.
	static std::set<size_t> referencedTags(AssemblyItems const& _items, size_t _subId);

private:
	AssemblyItems& m_items;
};

}
}
