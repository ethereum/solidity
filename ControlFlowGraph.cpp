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
 * @file ControlFlowGraph.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Control flow analysis for the optimizer.
 */

#include <libevmasm/ControlFlowGraph.h>
#include <map>
#include <memory>
#include <libevmasm/Exceptions.h>
#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>
#include <libevmasm/KnownState.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

BlockId::BlockId(u256 const& _id): m_id(_id)
{
	assertThrow( _id < initial().m_id, OptimizerException, "Tag number too large.");
}

BasicBlocks ControlFlowGraph::optimisedBlocks()
{
	if (m_items.empty())
		return BasicBlocks();

	findLargestTag();
	splitBlocks();
	resolveNextLinks();
	removeUnusedBlocks();
	setPrevLinks();
	gatherKnowledge();

	return rebuildCode();
}

void ControlFlowGraph::findLargestTag()
{
	m_lastUsedId = 0;
	for (auto const& item: m_items)
		if (item.type() == Tag || item.type() == PushTag)
		{
			// Assert that it can be converted.
			BlockId(item.data());
			m_lastUsedId = max(unsigned(item.data()), m_lastUsedId);
		}
}

void ControlFlowGraph::splitBlocks()
{
	m_blocks.clear();
	BlockId id = BlockId::initial();
	m_blocks[id].begin = 0;
	for (size_t index = 0; index < m_items.size(); ++index)
	{
		AssemblyItem const& item = m_items.at(index);
		if (item.type() == Tag)
		{
			if (id)
				m_blocks[id].end = index;
			id = BlockId::invalid();
		}
		if (!id)
		{
			id = item.type() == Tag ? BlockId(item.data()) : generateNewId();
			m_blocks[id].begin = index;
		}
		if (item.type() == PushTag)
			m_blocks[id].pushedTags.push_back(BlockId(item.data()));
		if (SemanticInformation::altersControlFlow(item))
		{
			m_blocks[id].end = index + 1;
			if (item == Instruction::JUMP)
				m_blocks[id].endType = BasicBlock::EndType::JUMP;
			else if (item == Instruction::JUMPI)
				m_blocks[id].endType = BasicBlock::EndType::JUMPI;
			else
				m_blocks[id].endType = BasicBlock::EndType::STOP;
			id = BlockId::invalid();
		}
	}
	if (id)
	{
		m_blocks[id].end = m_items.size();
		if (m_blocks[id].endType == BasicBlock::EndType::HANDOVER)
			m_blocks[id].endType = BasicBlock::EndType::STOP;
	}
}

void ControlFlowGraph::resolveNextLinks()
{
	map<unsigned, BlockId> blockByBeginPos;
	for (auto const& idAndBlock: m_blocks)
		if (idAndBlock.second.begin != idAndBlock.second.end)
			blockByBeginPos[idAndBlock.second.begin] = idAndBlock.first;

	for (auto& idAndBlock: m_blocks)
	{
		BasicBlock& block = idAndBlock.second;
		switch (block.endType)
		{
		case BasicBlock::EndType::JUMPI:
		case BasicBlock::EndType::HANDOVER:
			assertThrow(
				blockByBeginPos.count(block.end),
				OptimizerException,
				"Successor block not found."
			);
			block.next = blockByBeginPos.at(block.end);
			break;
		default:
			break;
		}
	}
}

void ControlFlowGraph::removeUnusedBlocks()
{
	vector<BlockId> blocksToProcess{BlockId::initial()};
	set<BlockId> neededBlocks{BlockId::initial()};
	while (!blocksToProcess.empty())
	{
		BasicBlock const& block = m_blocks.at(blocksToProcess.back());
		blocksToProcess.pop_back();
		for (BlockId tag: block.pushedTags)
			if (!neededBlocks.count(tag))
			{
				neededBlocks.insert(tag);
				blocksToProcess.push_back(tag);
			}
		if (block.next && !neededBlocks.count(block.next))
		{
			neededBlocks.insert(block.next);
			blocksToProcess.push_back(block.next);
		}
	}
	for (auto it = m_blocks.begin(); it != m_blocks.end();)
		if (neededBlocks.count(it->first))
			++it;
		else
			m_blocks.erase(it++);
}

void ControlFlowGraph::setPrevLinks()
{
	for (auto& idAndBlock: m_blocks)
	{
		BasicBlock& block = idAndBlock.second;
		switch (block.endType)
		{
		case BasicBlock::EndType::JUMPI:
		case BasicBlock::EndType::HANDOVER:
			assertThrow(
				!m_blocks.at(block.next).prev,
				OptimizerException,
				"Successor already has predecessor."
			);
			m_blocks[block.next].prev = idAndBlock.first;
			break;
		default:
			break;
		}
	}
	// If block ends with jump to not yet linked block, link them removing the jump
	for (auto& idAndBlock: m_blocks)
	{
		BlockId blockId = idAndBlock.first;
		BasicBlock& block = idAndBlock.second;
		if (block.endType != BasicBlock::EndType::JUMP || block.end - block.begin < 2)
			continue;
		AssemblyItem const& push = m_items.at(block.end - 2);
		if (push.type() != PushTag)
			continue;
		BlockId nextId(push.data());
		if (m_blocks.at(nextId).prev)
			continue;
		bool hasLoop = false;
		for (BlockId id = nextId; id && !hasLoop; id = m_blocks.at(id).next)
			hasLoop = (id == blockId);
		if (hasLoop)
			continue;

		m_blocks[nextId].prev = blockId;
		block.next = nextId;
		block.end -= 2;
		assertThrow(
			!block.pushedTags.empty() && block.pushedTags.back() == nextId,
			OptimizerException,
			"Last pushed tag not at end of pushed list."
		);
		block.pushedTags.pop_back();
		block.endType = BasicBlock::EndType::HANDOVER;
	}
}

void ControlFlowGraph::gatherKnowledge()
{
	// @todo actually we know that memory is filled with zeros at the beginning,
	// we could make use of that.
	KnownStatePointer emptyState = make_shared<KnownState>();
	ExpressionClasses& expr = emptyState->expressionClasses();
	bool unknownJumpEncountered = false;

	vector<pair<BlockId, KnownStatePointer>> workQueue({make_pair(BlockId::initial(), emptyState->copy())});
	while (!workQueue.empty())
	{
		//@todo we might have to do something like incrementing the sequence number for each JUMPDEST
		assertThrow(!!workQueue.back().first, OptimizerException, "");
		BasicBlock& block = m_blocks.at(workQueue.back().first);
		KnownStatePointer state = workQueue.back().second;
		workQueue.pop_back();
		if (block.startState)
		{
			state->reduceToCommonKnowledge(*block.startState);
			if (*state == *block.startState)
				continue;
		}

		block.startState = state->copy();
		//@todo we might know the return address for the first pass, but not anymore for the second,
		// -> store knowledge about tags as a union.

		// Feed all items except for the final jump yet because it will erase the target tag.
		unsigned pc = block.begin;
		while (pc < block.end && !SemanticInformation::altersControlFlow(m_items.at(pc)))
			state->feedItem(m_items.at(pc++));

		if (
			block.endType == BasicBlock::EndType::JUMP ||
			block.endType == BasicBlock::EndType::JUMPI
		)
		{
			assertThrow(block.begin <= pc && pc == block.end - 1, OptimizerException, "");
			//@todo in the case of JUMPI, add knowledge about the condition to the state
			// (for both values of the condition)
			BlockId nextBlock = expressionClassToBlockId(
				state->stackElement(state->stackHeight(), SourceLocation()),
				expr
			);
			state->feedItem(m_items.at(pc++));
			if (nextBlock)
				workQueue.push_back(make_pair(nextBlock, state->copy()));
			else if (!unknownJumpEncountered)
			{
				// We do not know where this jump goes, so we have to reset the states of all
				// JUMPDESTs.
				unknownJumpEncountered = true;
				for (auto const& it: m_blocks)
					if (it.second.begin < it.second.end && m_items[it.second.begin].type() == Tag)
						workQueue.push_back(make_pair(it.first, emptyState->copy()));
			}
		}
		else if (block.begin <= pc && pc < block.end)
			state->feedItem(m_items.at(pc++));
		assertThrow(block.end <= block.begin || pc == block.end, OptimizerException, "");

		block.endState = state;

		if (
			block.endType == BasicBlock::EndType::HANDOVER ||
			block.endType == BasicBlock::EndType::JUMPI
		)
			workQueue.push_back(make_pair(block.next, state->copy()));
	}
}

BasicBlocks ControlFlowGraph::rebuildCode()
{
	map<BlockId, unsigned> pushes;
	for (auto& idAndBlock: m_blocks)
		for (BlockId ref: idAndBlock.second.pushedTags)
			pushes[ref]++;

	set<BlockId> blocksToAdd;
	for (auto it: m_blocks)
		blocksToAdd.insert(it.first);
	set<BlockId> blocksAdded;
	BasicBlocks blocks;

	for (
		BlockId blockId = BlockId::initial();
		blockId;
		blockId = blocksToAdd.empty() ? BlockId::invalid() : *blocksToAdd.begin()
	)
	{
		bool previousHandedOver = (blockId == BlockId::initial());
		while (m_blocks.at(blockId).prev)
			blockId = m_blocks.at(blockId).prev;
		for (; blockId; blockId = m_blocks.at(blockId).next)
		{
			BasicBlock& block = m_blocks.at(blockId);
			blocksToAdd.erase(blockId);
			blocksAdded.insert(blockId);

			if (block.begin == block.end)
				continue;
			// If block starts with unused tag, skip it.
			if (previousHandedOver && !pushes[blockId] && m_items[block.begin].type() == Tag)
				++block.begin;
			if (block.begin < block.end)
				blocks.push_back(block);
			previousHandedOver = (block.endType == BasicBlock::EndType::HANDOVER);
		}
	}

	return blocks;
}

BlockId ControlFlowGraph::expressionClassToBlockId(
	ExpressionClasses::Id _id,
	ExpressionClasses& _exprClasses
)
{
	ExpressionClasses::Expression expr = _exprClasses.representative(_id);
	if (expr.item && expr.item->type() == PushTag)
		return BlockId(expr.item->data());
	else
		return BlockId::invalid();
}

BlockId ControlFlowGraph::generateNewId()
{
	BlockId id = BlockId(++m_lastUsedId);
	assertThrow(id < BlockId::initial(), OptimizerException, "Out of block IDs.");
	return id;
}
