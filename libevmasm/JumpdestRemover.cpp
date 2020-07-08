// SPDX-License-Identifier: GPL-3.0
/**
 * @author Alex Beregszaszi
 * Removes unused JUMPDESTs.
 */

#include <libevmasm/JumpdestRemover.h>

#include <libevmasm/AssemblyItem.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;

bool JumpdestRemover::optimise(set<size_t> const& _tagsReferencedFromOutside)
{
	set<size_t> references{referencedTags(m_items, numeric_limits<size_t>::max())};
	references.insert(_tagsReferencedFromOutside.begin(), _tagsReferencedFromOutside.end());

	size_t initialSize = m_items.size();
	/// Remove tags which are never referenced.
	auto pend = remove_if(
		m_items.begin(),
		m_items.end(),
		[&](AssemblyItem const& _item)
		{
			if (_item.type() != Tag)
				return false;
			auto asmIdAndTag = _item.splitForeignPushTag();
			assertThrow(asmIdAndTag.first == numeric_limits<size_t>::max(), OptimizerException, "Sub-assembly tag used as label.");
			size_t tag = asmIdAndTag.second;
			return !references.count(tag);
		}
	);
	m_items.erase(pend, m_items.end());
	return m_items.size() != initialSize;
}

set<size_t> JumpdestRemover::referencedTags(AssemblyItems const& _items, size_t _subId)
{
	set<size_t> ret;
	for (auto const& item: _items)
		if (item.type() == PushTag)
		{
			auto subAndTag = item.splitForeignPushTag();
			if (subAndTag.first == _subId)
				ret.insert(subAndTag.second);
		}
	return ret;
}
