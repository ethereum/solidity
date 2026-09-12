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
 * @file BlockDeduplicator.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unifies basic blocks that share content.
 */

#include <libevmasm/BlockDeduplicator.h>

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

#include <boost/container_hash/hash.hpp>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/equal.hpp>

#include <unordered_set>

using namespace solidity;
using namespace solidity::evmasm;


bool BlockDeduplicator::deduplicate()
{
	// Group basic blocks by a content hash and dedup within each bucket.
	// The hash and equality both walk a BlockIterator that ignores tags and stops at
	// opcodes that terminate control flow, replacing the block's own self-push by a
	// virtual tag so that recursive loops match.

	// Virtual tag that signifies "the current block" and which is used to optimize loops.
	// We abort if this virtual tag actually exists.
	AssemblyItem const pushSelf{PushTag, u256(-4)};
	{
		AssemblyItem const selfTag = pushSelf.tag();
		if (ranges::any_of(m_items, [&](AssemblyItem const& _item) { return _item == selfTag || _item == pushSelf; }))
			return false;
	}

	BlockIterator const end{m_items.end(), m_items.end()};

	// yields a block iterator into the body of a block (skips `Tag` typed assembly items at `_blockBegin`)
	auto const blockBodyBegin = [&](std::size_t const _blockBegin, AssemblyItem const& _selfTagPush)
	{
		BlockIterator it{
			m_items.begin() + static_cast<BlockIterator::difference_type>(_blockBegin),
			m_items.end(),
			&_selfTagPush,
			&pushSelf
		};
		if (it != end && (*it).type() == Tag)
			++it;
		return it;
	};

	auto const hashBlockAt = [&](std::size_t const _i)
	{
		std::size_t seed = boost::hash_range(blockBodyBegin(_i, m_items[_i].pushTag()), end);
		boost::hash_combine(seed, m_items[_i].isSubroutineEntry());
		return seed;
	};
	// EIP-7979: blocks entered by CALLSUB (CALLDEST labels) and blocks entered by
	// JUMP (JUMPDEST labels) are never merged, so that no CALLSUB is ever redirected
	// onto a JUMPDEST.
	auto const blocksAtEqual = [&](std::size_t const _i, std::size_t const _j)
	{
		return m_items[_i].isSubroutineEntry() == m_items[_j].isSubroutineEntry() && ranges::equal(
			blockBodyBegin(_i, m_items[_i].pushTag()), end,
			blockBodyBegin(_j, m_items[_j].pushTag()), end
		);
	};

	std::size_t iterations = 0;
	for (; ; ++iterations)
	{
		std::unordered_set<std::size_t, decltype(hashBlockAt), decltype(blocksAtEqual)> seen(0, hashBlockAt, blocksAtEqual);
		std::vector<std::size_t> retained;
		for (std::size_t i = 0; i < m_items.size(); ++i)
		{
			if (m_items[i].type() != Tag)
				continue;
			auto const [it, inserted] = seen.insert(i);
			if (!inserted)
			{
				m_replacedTags[m_items[i].data()] = m_items[*it].data();
				retained.push_back(*it);
			}
		}
		// Marking changes the block's kind, which the set's hash and equality depend
		// on, so it happens only after the set is no longer used.
		if (m_sharedBlocksAreSubroutineEntries)
			for (std::size_t const k: retained)
				m_items[k].setSubroutineEntry();

		if (!applyTagReplacement(m_items, m_replacedTags))
			break;
	}
	return iterations > 0;
}

bool BlockDeduplicator::applyTagReplacement(
	AssemblyItems& _items,
	std::map<u256, u256> const& _replacements,
	SubAssemblyID _subId
)
{
	bool changed = false;
	for (AssemblyItem& item: _items)
		if (item.type() == PushTag)
		{
			SubAssemblyID subId;
			size_t tagId;
			std::tie(subId, tagId) = item.splitForeignPushTag();
			if (subId != _subId)
				continue;
			auto it = _replacements.find(tagId);
			// Recursively look for the element replaced by tagId
			for (auto _it = it; _it != _replacements.end(); _it = _replacements.find(_it->second))
				it = _it;

			if (it != _replacements.end())
			{
				changed = true;
				item.setPushTagSubIdAndTag(subId, static_cast<size_t>(it->second));
			}
		}
	return changed;
}

BlockDeduplicator::BlockIterator& BlockDeduplicator::BlockIterator::operator++()
{
	if (it == end)
		return *this;
	if (SemanticInformation::altersControlFlow(*it) && *it != AssemblyItem{Instruction::JUMPI})
		it = end;
	else
	{
		++it;
		while (it != end && it->type() == Tag)
			++it;
	}
	return *this;
}

AssemblyItem const& BlockDeduplicator::BlockIterator::operator*() const
{
	if (replaceItem && replaceWith && *it == *replaceItem)
		return *replaceWith;
	else
		return *it;
}
