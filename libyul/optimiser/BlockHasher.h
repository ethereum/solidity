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
 * Optimiser component that calculates hash values for block prefixes.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>
#include <libyul/AsmData.h>

namespace yul
{

struct BlockPart {
	Block const* block;
	size_t length;
	bool operator<(BlockPart const& _rhs) const
	{
		return std::make_tuple(_rhs.length, _rhs.block) < std::make_tuple(length, block);
	}
};

/**
 * Optimiser component that calculates hash values for block prefixes.
 *
 * Prerequisite: Disambiguator
 */
class BlockHasher: public ASTWalker
{
public:
	struct State {
		std::map<BlockPart, uint64_t> blockPrefixHashes;
		std::map<uint64_t, std::vector<BlockPart>> hashClasses;
	};

	using ASTWalker::operator();

	void operator()(Literal const&) override;
	void operator()(Identifier const&) override;
	void operator()(FunctionalInstruction const& _instr) override;
	void operator()(FunctionCall const& _funCall) override;
	void operator()(ExpressionStatement const& _statement) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;
	void operator()(Break const&) override;
	void operator()(Continue const&) override;
	void operator()(Block const& _block) override;

	static State run(Block const& _block);

private:
	State& m_state;

	BlockHasher(State& _state): m_state(_state) {}

	void hash(uint8_t _value)
	{
		m_hash *= 1099511628211u;
		m_hash ^= _value;
	}
	void hash(uint16_t _value)
	{
		hash(static_cast<uint8_t>(_value & 0xFF));
		hash(static_cast<uint8_t>(_value >> 8));
	}
	void hash(uint32_t _value)
	{
		hash(static_cast<uint8_t>(_value & 0xFFFF));
		hash(static_cast<uint8_t>(_value >> 16));
	}
	void hash(uint64_t _value)
	{
		hash(static_cast<uint8_t>(_value & 0xFFFFFFFF));
		hash(static_cast<uint8_t>(_value >> 32));
	}

	uint64_t m_hash = 14695981039346656037u;
	struct VariableReference {
		size_t id = 0;
		bool isExternal = false;
	};
	std::map<YulString, VariableReference> m_variableReferences;
	size_t m_externalIdentifierCount = 0;
	size_t m_internalIdentifierCount = 0;
};


}
