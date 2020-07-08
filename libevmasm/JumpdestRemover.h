// SPDX-License-Identifier: GPL-3.0
/**
 * @author Alex Beregszaszi
 * Removes unused JUMPDESTs.
 */
#pragma once

#include <vector>
#include <cstddef>
#include <set>

namespace solidity::evmasm
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
