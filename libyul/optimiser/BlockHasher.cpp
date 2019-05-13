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

#include <libyul/optimiser/BlockHasher.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace {
static constexpr uint64_t compileTimeLiteralHash(char const* _literal, size_t _N)
{
	return (_N == 0) ? 14695981039346656037u : compileTimeLiteralHash(_literal + 1, _N - 1) ^ 1099511628211u;
}

template<size_t N>
static constexpr uint64_t compileTimeLiteralHash(char const (&_literal)[N])
{
	return compileTimeLiteralHash(_literal, N);
}
}

BlockHasher::State BlockHasher::run(Block const& _block)
{
	BlockHasher::State result;
	BlockHasher blockHasher(result);
	blockHasher(_block);
	return result;
}

void BlockHasher::operator()(Literal const& _literal)
{
	hash(compileTimeLiteralHash("Literal"));
	hash(_literal.value.hash());
	hash(_literal.type.hash());
	hash(static_cast<uint8_t>(static_cast<std::underlying_type_t<LiteralKind>>(_literal.kind)));
}

void BlockHasher::operator()(Identifier const& _identifier)
{
	hash(compileTimeLiteralHash("Identifier"));
	auto it = m_variableReferences.find(_identifier.name);
	if (it == m_variableReferences.end())
		it = m_variableReferences.emplace(_identifier.name, VariableReference {
			m_externalIdentifierCount++,
			true
		}).first;

	if (it->second.isExternal)
		hash(compileTimeLiteralHash("external"));
	else
		hash(compileTimeLiteralHash("internal"));
	hash(it->second.id);
}

void BlockHasher::operator()(FunctionalInstruction const& _instr)
{
	hash(compileTimeLiteralHash("FunctionalInstruction"));
	hash(static_cast<std::underlying_type_t<eth::Instruction>>(_instr.instruction));
	hash(_instr.arguments.size());
	ASTWalker::operator()(_instr);
}

void BlockHasher::operator()(FunctionCall const& _funCall)
{
	hash(compileTimeLiteralHash("FunctionCall"));
	hash(_funCall.arguments.size());
	hash(_funCall.functionName.name.hash());
	ASTWalker::operator()(_funCall);
}

void BlockHasher::operator()(ExpressionStatement const& _statement)
{
	hash(compileTimeLiteralHash("ExpressionStatement"));
	ASTWalker::operator()(_statement);
}

void BlockHasher::operator()(Assignment const& _assignment)
{
	hash(compileTimeLiteralHash("Assignment"));
	hash(_assignment.variableNames.size());
	for (auto const& name: _assignment.variableNames)
		(*this)(name);
	visit(*_assignment.value);
}

void BlockHasher::operator()(VariableDeclaration const& _varDecl)
{
	hash(compileTimeLiteralHash("VariableDeclaration"));
	hash(_varDecl.variables.size());
	for (auto const& var: _varDecl.variables)
	{
		yulAssert(!m_variableReferences.count(var.name), "");
		m_variableReferences[var.name] = VariableReference{
			m_internalIdentifierCount++,
			false
		};
	}
	ASTWalker::operator()(_varDecl);
}

void BlockHasher::operator()(If const& _if)
{
	hash(compileTimeLiteralHash("If"));
	ASTWalker::operator()(_if);
}

void BlockHasher::operator()(Switch const& _switch)
{
	hash(compileTimeLiteralHash("Switch"));
	hash(_switch.cases.size());
	ASTWalker::operator()(_switch);
}

void BlockHasher::operator()(FunctionDefinition const& _funDef)
{
	hash(compileTimeLiteralHash("FunctionDefinition"));
	ASTWalker::operator()(_funDef);
}

void BlockHasher::operator()(ForLoop const& _loop)
{
	hash(compileTimeLiteralHash("ForLoop"));
	ASTWalker::operator()(_loop);
}

void BlockHasher::operator()(Break const& _break)
{
	hash(compileTimeLiteralHash("Break"));
	ASTWalker::operator()(_break);
}

void BlockHasher::operator()(Continue const& _continue)
{
	hash(compileTimeLiteralHash("Continue"));
	ASTWalker::operator()(_continue);
}


void BlockHasher::operator()(Block const& _block)
{
	hash(compileTimeLiteralHash("Block"));
	hash(_block.statements.size());
	if (_block.statements.empty())
		return;

	BlockHasher subBlockPrefixHasher(m_state);
	BlockPart blockPart { &_block, 0 };
	for (auto const& statement: _block.statements)
	{
		subBlockPrefixHasher.visit(statement);
		++blockPart.length;
		m_state.blockPrefixHashes[blockPart] = subBlockPrefixHasher.m_hash;
		m_state.hashClasses[subBlockPrefixHasher.m_hash].emplace_back(blockPart);
	}
	// propagate external references in subblock
	hash(subBlockPrefixHasher.m_hash);
	std::vector<YulString> externalReferences;
	for (auto const& variableReference: subBlockPrefixHasher.m_variableReferences)
		if (variableReference.second.isExternal)
			externalReferences.emplace_back(variableReference.first);
	hash(externalReferences.size());
	for (auto& externalReference: externalReferences)
		(*this)(Identifier{{}, externalReference});
}
