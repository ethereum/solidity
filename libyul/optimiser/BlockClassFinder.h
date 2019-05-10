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
 * Optimiser component that finds classes of equivalent blocks.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmDataForward.h>
#include <libyul/YulString.h>
#include <libyul/AsmData.h>

namespace yul
{

struct BlockClassMember
{
	Block const* block = nullptr;
	std::vector<YulString> externalReferences;
	std::set<YulString> externalAssignments;
	std::set<YulString> externalReads;
};

struct BlockClass
{
	std::vector<BlockClassMember> members;
	YulString nameHint;
	bool hasFreeBreakOrContinue = false;
};

/**
 * Optimiser component that finds classes of equivalent blocks.
 *
 * Prerequisite: Disambiguator
 *
 * Works best after running the FunctionHoister and FunctionGrouper
 */
class BlockClassFinder: public ASTWalker
{
public:

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

	static std::vector<BlockClass> run(Block const& _block);

private:
	struct GlobalState
	{
		struct BlockID
		{
			size_t blockClass = 0;
			size_t indexInClass = 0;
		};
		std::map<uint64_t, std::vector<BlockID>> hashToBlockIDs;
		std::map<Block const*, size_t> blockToClassID;
		std::vector<BlockClass> blockClasses;
		BlockClassMember const& block(BlockID const& id)
		{
			return blockClasses.at(id.blockClass).members.at(id.indexInClass);
		}
	};

	BlockClassFinder(GlobalState& _globalState): m_globalState(_globalState) {}

	void hash(uint64_t _value)
	{
		m_hash *= 1099511628211u;
		m_hash ^= _value;
	}

	GlobalState& m_globalState;

	bool isExternal(YulString _identifier) const;

	uint64_t m_hash = 14695981039346656037u;
	std::map<YulString, size_t> m_identifierMapping;
	std::vector<YulString> m_externalIdentifiers;
	std::set<YulString> m_externalAssignments;
	std::set<YulString> m_externalReads;
	size_t m_externalIdentifierCount = 0;
	size_t m_internalIdentifierCount = 0;
	bool m_isAssignmentLHS = false;
	size_t m_loopDepth = 0;
	bool m_hasFreeBreakOrContinue = false;
	YulString m_functionName;
};


}
