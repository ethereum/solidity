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
 * @file ControlFlowGraph.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Control flow analysis for the optimizer.
 */

#pragma once

#include <vector>
#include <memory>
#include <libdevcore/Common.h>
#include <libdevcore/Assertions.h>
#include <libevmasm/ExpressionClasses.h>

namespace dev
{
namespace eth
{

class KnownState;
using KnownStatePointer = std::shared_ptr<KnownState>;

/**
 * Identifier for a block, coincides with the tag number of an AssemblyItem but adds a special
 * ID for the inital block.
 */
class BlockId
{
public:
	BlockId() { *this = invalid(); }
	explicit BlockId(unsigned _id): m_id(_id) {}
	explicit BlockId(u256 const& _id);
	static BlockId initial() { return BlockId(-2); }
	static BlockId invalid() { return BlockId(-1); }

	bool operator==(BlockId const& _other) const { return m_id == _other.m_id; }
	bool operator!=(BlockId const& _other) const { return m_id != _other.m_id; }
	bool operator<(BlockId const& _other) const { return m_id < _other.m_id; }
	explicit operator bool() const { return *this != invalid(); }

private:
	unsigned m_id;
};

/**
 * Control flow block inside which instruction counter is always incremented by one
 * (except for possibly the last instruction).
 */
struct BasicBlock
{
	/// Start index into assembly item list.
	unsigned begin = 0;
	/// End index (excluded) inte assembly item list.
	unsigned end = 0;
	/// Tags pushed inside this block, with multiplicity.
	std::vector<BlockId> pushedTags;
	/// ID of the block that always follows this one (either non-branching part of JUMPI or flow
	/// into new block), or BlockId::invalid() otherwise
	BlockId next = BlockId::invalid();
	/// ID of the block that has to precede this one (because control flows into it).
	BlockId prev = BlockId::invalid();

	enum class EndType { JUMP, JUMPI, STOP, HANDOVER };
	EndType endType = EndType::HANDOVER;

	/// Knowledge about the state when this block is entered. Intersection of all possible ways
	/// to enter this block.
	KnownStatePointer startState;
	/// Knowledge about the state at the end of this block.
	KnownStatePointer endState;
};

using BasicBlocks = std::vector<BasicBlock>;

/**
 * Control flow graph optimizer.
 * ASSUMES THAT WE ONLY JUMP TO TAGS THAT WERE PREVIOUSLY PUSHED. THIS IS NOT TRUE ANYMORE
 * NOW THAT FUNCTION TAGS CAN BE STORED IN STORAGE.
 */
class ControlFlowGraph
{
public:
	/// Initializes the control flow graph.
	/// @a _items has to persist across the usage of this class.
	/// @a _joinKnowledge if true, reduces state knowledge to common base at the join of two paths
	explicit ControlFlowGraph(AssemblyItems const& _items, bool _joinKnowledge = true):
		m_items(_items),
		m_joinKnowledge(_joinKnowledge)
	{}
	/// @returns vector of basic blocks in the order they should be used in the final code.
	/// Should be called only once.
	BasicBlocks optimisedBlocks();

private:
	void findLargestTag();
	void splitBlocks();
	void resolveNextLinks();
	void removeUnusedBlocks();
	void gatherKnowledge();
	void setPrevLinks();
	BasicBlocks rebuildCode();

	BlockId generateNewId();

	unsigned m_lastUsedId = 0;
	AssemblyItems const& m_items;
	bool m_joinKnowledge = true;
	std::map<BlockId, BasicBlock> m_blocks;
};


}
}
