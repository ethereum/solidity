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
 * Optimiser component that calculates hash values for blocks.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

namespace solidity::yul
{

class HasherBase
{
public:
	static constexpr uint64_t fnvPrime = 1099511628211u;
	static constexpr uint64_t fnvEmptyHash = 14695981039346656037u;

protected:
	void hash8(uint8_t _value)
	{
		m_hash *= fnvPrime;
		m_hash ^= _value;
	}
	void hash16(uint16_t _value)
	{
		hash8(static_cast<uint8_t>(_value & 0xFF));
		hash8(static_cast<uint8_t>(_value >> 8));
	}
	void hash32(uint32_t _value)
	{
		hash16(static_cast<uint16_t>(_value & 0xFFFF));
		hash16(static_cast<uint16_t>(_value >> 16));
	}
	void hash64(uint64_t _value)
	{
		hash32(static_cast<uint32_t>(_value & 0xFFFFFFFF));
		hash32(static_cast<uint32_t>(_value >> 32));
	}


	uint64_t m_hash = fnvEmptyHash;
};

/**
 * Optimiser component that calculates hash values for blocks.
 * Syntactically equal blocks will have identical hashes and
 * blocks with equal hashes will likely be syntactically equal.
 *
 * The names of internally declared variables are replaced by
 * a simple counter, so differing names are not taken into account,
 * but only the order of references to declared variables.
 *
 * Similarly, the names of referenced external variables are not considered,
 * but replaced by a (distinct) counter as well.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter
 */
class BlockHasher: public ASTWalker, public HasherBase
{
public:

	using ASTWalker::operator();

	void operator()(Literal const&) override;
	void operator()(Identifier const&) override;
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
	void operator()(Leave const&) override;
	void operator()(Block const& _block) override;

	static std::map<Block const*, uint64_t> run(Block const& _block);


private:
	BlockHasher(std::map<Block const*, uint64_t>& _blockHashes): m_blockHashes(_blockHashes) {}

	std::map<Block const*, uint64_t>& m_blockHashes;

	struct VariableReference
	{
		size_t id = 0;
		bool isExternal = false;
	};
	std::map<YulString, VariableReference> m_variableReferences;
	std::vector<YulString> m_externalReferences;
	size_t m_externalIdentifierCount = 0;
	size_t m_internalIdentifierCount = 0;
};


/**
 * Computes hashes of expressions that are likely different for syntactically different expressions.
 * In contrast to the BlockHasher, hashes of identifiers are likely different if the identifiers
 * have a different name and the same if the name matches.
 * This means this hasher should only be used on disambiguated sources.
 */
class ExpressionHasher: public ASTWalker, public HasherBase
{
public:
	/// Computes a hash of an expression that (in contrast to the behaviour of the class)
	/// distinguishes (up to hash collisions) variables with different names.
	static uint64_t run(Expression const& _e);

	using ASTWalker::operator();

	void operator()(Literal const&) override;
	void operator()(Identifier const&) override;
	void operator()(FunctionCall const& _funCall) override;
};

struct ExpressionHash
{
	uint64_t operator()(Expression const& _expression) const
	{
		return ExpressionHasher::run(_expression);
	}
};

}
