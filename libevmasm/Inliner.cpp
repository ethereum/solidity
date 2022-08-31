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
 * @file Inliner.cpp
 * Inlines small code snippets by replacing JUMP with a copy of the code jumped to.
 */

#include <libevmasm/Inliner.h>

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/GasMeter.h>
#include <libevmasm/KnownState.h>
#include <libevmasm/SemanticInformation.h>

#include <libsolutil/CommonData.h>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/transform.hpp>

#include <optional>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;


namespace
{
/// @returns an estimation of the runtime gas cost of the AsssemblyItems in @a _itemRange.
template<typename RangeType>
u256 executionCost(RangeType const& _itemRange, langutil::EVMVersion _evmVersion)
{
	GasMeter gasMeter{std::make_shared<KnownState>(), _evmVersion};
	auto gasConsumption = ranges::accumulate(_itemRange | ranges::views::transform(
		[&gasMeter](auto const& _item) { return gasMeter.estimateMax(_item, false); }
	), GasMeter::GasConsumption());
	if (gasConsumption.isInfinite)
		return numeric_limits<u256>::max();
	else
		return gasConsumption.value;
}
/// @returns an estimation of the code size in bytes needed for the AssemblyItems in @a _itemRange.
template<typename RangeType>
uint64_t codeSize(RangeType const& _itemRange)
{
	return ranges::accumulate(_itemRange | ranges::views::transform(
		[](auto const& _item) { return _item.bytesRequired(2, Precision::Approximate); }
	), 0u);
}
/// @returns the tag id, if @a _item is a PushTag or Tag into the current subassembly, nullopt otherwise.
optional<size_t> getLocalTag(AssemblyItem const& _item)
{
	if (_item.type() != PushTag && _item.type() != Tag)
		return nullopt;
	auto [subId, tag] = _item.splitForeignPushTag();
	if (subId != numeric_limits<size_t>::max())
		return nullopt;
	return tag;
}
}

bool Inliner::isInlineCandidate(size_t _tag, ranges::span<AssemblyItem const> _items) const
{
	assertThrow(_items.size() > 0, OptimizerException, "");

	if (_items.back().type() != Operation)
		return false;
	if (
		_items.back() != Instruction::JUMP &&
		!SemanticInformation::terminatesControlFlow(_items.back().instruction())
	)
		return false;

	// Never inline tags that reference themselves.
	for (AssemblyItem const& item: _items)
		if (item.type() == PushTag)
			if (getLocalTag(item) == _tag)
					return false;

	return true;
}

map<size_t, Inliner::InlinableBlock> Inliner::determineInlinableBlocks(AssemblyItems const& _items) const
{
	std::map<size_t, ranges::span<AssemblyItem const>> inlinableBlockItems;
	std::map<size_t, uint64_t> numPushTags;
	std::optional<size_t> lastTag;
	for (auto&& [index, item]: _items | ranges::views::enumerate)
	{
		// The number of PushTags approximates the number of calls to a block.
		if (item.type() == PushTag)
			if (optional<size_t> tag = getLocalTag(item))
				++numPushTags[*tag];

		// We can only inline blocks with straight control flow that end in a jump.
		// Using breaksCSEAnalysisBlock will hopefully allow the return jump to be optimized after inlining.
		if (lastTag && SemanticInformation::breaksCSEAnalysisBlock(item, false))
		{
			ranges::span<AssemblyItem const> block = _items | ranges::views::slice(*lastTag + 1, index + 1);
			if (optional<size_t> tag = getLocalTag(_items[*lastTag]))
				if (isInlineCandidate(*tag, block))
					inlinableBlockItems[*tag] = block;
			lastTag.reset();
		}

		if (item.type() == Tag)
		{
			assertThrow(getLocalTag(item), OptimizerException, "");
			lastTag = index;
		}
	}

	// Store the number of PushTags alongside the assembly items and discard tags that are never pushed.
	map<size_t, InlinableBlock> result;
	for (auto&& [tag, items]: inlinableBlockItems)
		if (uint64_t const* numPushes = util::valueOrNullptr(numPushTags, tag))
			result.emplace(tag, InlinableBlock{items, *numPushes});
	return result;
}

bool Inliner::shouldInlineFullFunctionBody(size_t _tag, ranges::span<AssemblyItem const> _block, uint64_t _pushTagCount) const
{
	// Accumulate size of the inline candidate block in bytes (without the return jump).
	uint64_t functionBodySize = codeSize(ranges::views::drop_last(_block, 1));

	// Use the number of push tags as approximation of the average number of calls to the function per run.
	uint64_t numberOfCalls = _pushTagCount;
	// Also use the number of push tags as approximation of the number of call sites to the function.
	uint64_t numberOfCallSites = _pushTagCount;

	static AssemblyItems const uninlinedCallSitePattern = {
		AssemblyItem{PushTag},
		AssemblyItem{PushTag},
		AssemblyItem{Instruction::JUMP},
		AssemblyItem{Tag}
	};
	static AssemblyItems const uninlinedFunctionPattern = {
		AssemblyItem{Tag},
		// Actual function body of size functionBodySize. Handled separately below.
		AssemblyItem{Instruction::JUMP}
	};

	// Both the call site and jump site pattern is executed for each call.
	// Since the function body has to be executed equally often both with and without inlining,
	// it can be ignored.
	bigint uninlinedExecutionCost = numberOfCalls * (
		executionCost(uninlinedCallSitePattern, m_evmVersion) +
		executionCost(uninlinedFunctionPattern, m_evmVersion)
	);
	// Each call site deposits the call site pattern, whereas the jump site pattern and the function itself are deposited once.
	bigint uninlinedDepositCost = GasMeter::dataGas(
		numberOfCallSites * codeSize(uninlinedCallSitePattern) +
		codeSize(uninlinedFunctionPattern) +
		functionBodySize,
		m_isCreation,
		m_evmVersion
	);
	// When inlining the execution cost beyond the actual function execution is zero,
	// but for each call site a copy of the function is deposited.
	bigint inlinedDepositCost = GasMeter::dataGas(
		numberOfCallSites * functionBodySize,
		m_isCreation,
		m_evmVersion
	);
	// If the block is referenced from outside the current subassembly, the original function cannot be removed.
	// Note that the function also cannot always be removed, if it is not referenced from outside, but in that case
	// the heuristics is optimistic.
	if (m_tagsReferencedFromOutside.count(_tag))
		inlinedDepositCost += GasMeter::dataGas(
			codeSize(uninlinedFunctionPattern) + functionBodySize,
			m_isCreation,
			m_evmVersion
		);

	// If the estimated runtime cost over the lifetime of the contract plus the deposit cost in the uninlined case
	// exceed the inlined deposit costs, it is beneficial to inline.
	if (bigint(m_runs) * uninlinedExecutionCost + uninlinedDepositCost > inlinedDepositCost)
		return true;

	return false;
}

optional<AssemblyItem> Inliner::shouldInline(size_t _tag, AssemblyItem const& _jump, InlinableBlock const& _block) const
{
	assertThrow(_jump == Instruction::JUMP, OptimizerException, "");
	AssemblyItem blockExit = _block.items.back();

	if (
		_jump.getJumpType() == AssemblyItem::JumpType::IntoFunction &&
		blockExit == Instruction::JUMP &&
		blockExit.getJumpType() == AssemblyItem::JumpType::OutOfFunction &&
		shouldInlineFullFunctionBody(_tag, _block.items, _block.pushTagCount)
	)
	{
		blockExit.setJumpType(AssemblyItem::JumpType::Ordinary);
		return blockExit;
	}

	// Inline small blocks, if the jump to it is ordinary or the blockExit is a terminating instruction.
	if (
		_jump.getJumpType() == AssemblyItem::JumpType::Ordinary ||
		SemanticInformation::terminatesControlFlow(blockExit.instruction())
	)
	{
		static AssemblyItems const jumpPattern = {
			AssemblyItem{PushTag},
			AssemblyItem{Instruction::JUMP},
		};
		if (
			GasMeter::dataGas(codeSize(_block.items), m_isCreation, m_evmVersion) <=
			GasMeter::dataGas(codeSize(jumpPattern), m_isCreation, m_evmVersion)
		)
			return blockExit;
	}

	return nullopt;
}


void Inliner::optimise()
{
	std::map<size_t, InlinableBlock> inlinableBlocks = determineInlinableBlocks(m_items);

	if (inlinableBlocks.empty())
		return;

	AssemblyItems newItems;
	for (auto it = m_items.begin(); it != m_items.end(); ++it)
	{
		AssemblyItem const& item = *it;
		if (next(it) != m_items.end())
		{
			AssemblyItem const& nextItem = *next(it);
			if (item.type() == PushTag && nextItem == Instruction::JUMP)
			{
				if (optional<size_t> tag = getLocalTag(item))
					if (auto* inlinableBlock = util::valueOrNullptr(inlinableBlocks, *tag))
						if (auto exitItem = shouldInline(*tag, nextItem, *inlinableBlock))
						{
							newItems += inlinableBlock->items | ranges::views::drop_last(1);
							newItems.emplace_back(std::move(*exitItem));

							// We are removing one push tag to the block we inline.
							--inlinableBlock->pushTagCount;
							// We might increase the number of push tags to other blocks.
							for (AssemblyItem const& inlinedItem: inlinableBlock->items)
								if (inlinedItem.type() == PushTag)
									if (optional<size_t> duplicatedTag = getLocalTag(inlinedItem))
										if (auto* block = util::valueOrNullptr(inlinableBlocks, *duplicatedTag))
											++block->pushTagCount;

							// Skip the original jump to the inlined tag and continue.
							++it;
							continue;
						}
			}
		}
		newItems.emplace_back(item);
	}

	m_items = std::move(newItems);
}
