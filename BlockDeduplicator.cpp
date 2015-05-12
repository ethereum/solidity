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
 * @file BlockDeduplicator.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unifies basic blocks that share content.
 */

#include <libevmasm/BlockDeduplicator.h>
#include <functional>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace dev;
using namespace dev::eth;


bool BlockDeduplicator::deduplicate()
{
	// Compares indices based on the suffix that starts there, ignoring tags and stopping at
	// opcodes that stop the control flow.
	function<bool(size_t, size_t)> comparator = [&](size_t _i, size_t _j)
	{
		if (_i == _j)
			return false;

		BlockIterator first(m_items.begin() + _i, m_items.end());
		BlockIterator second(m_items.begin() + _j, m_items.end());
		BlockIterator end(m_items.end(), m_items.end());

		if (first != end && (*first).type() == Tag)
			++first;
		if (second != end && (*second).type() == Tag)
			++second;

		return std::lexicographical_compare(first, end, second, end);
	};

	set<size_t, function<bool(size_t, size_t)>> blocksSeen(comparator);
	map<u256, u256> tagReplacement;
	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (m_items.at(i).type() != Tag)
			continue;
		auto it = blocksSeen.find(i);
		if (it == blocksSeen.end())
			blocksSeen.insert(i);
		else
			tagReplacement[m_items.at(i).data()] = m_items.at(*it).data();
	}

	bool ret = false;
	for (AssemblyItem& item: m_items)
		if (item.type() == PushTag && tagReplacement.count(item.data()))
		{
			ret = true;
			item.setData(tagReplacement.at(item.data()));
		}
	return ret;
}

BlockDeduplicator::BlockIterator& BlockDeduplicator::BlockIterator::operator++()
{
	if (it == end)
		return *this;
	if (SemanticInformation::altersControlFlow(*it) && *it != AssemblyItem(eth::Instruction::JUMPI))
		it = end;
	else
	{
		++it;
		while (it != end && it->type() == Tag)
			++it;
	}
	return *this;
}
