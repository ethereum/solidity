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
#include <libyul/Utilities.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{
static constexpr uint64_t compileTimeLiteralHash(char const* _literal, size_t _N)
{
	return (_N == 0) ? BlockHasher::fnvEmptyHash : (static_cast<uint64_t>(_literal[0]) * BlockHasher::fnvPrime) ^ compileTimeLiteralHash(_literal + 1, _N - 1);
}

template<size_t N>
static constexpr uint64_t compileTimeLiteralHash(char const (&_literal)[N])
{
	return compileTimeLiteralHash(_literal, N);
}
}

std::map<Block const*, uint64_t> BlockHasher::run(Block const& _block)
{
	std::map<Block const*, uint64_t> result;
	BlockHasher blockHasher(result);
	blockHasher(_block);
	return result;
}

void BlockHasher::operator()(Literal const& _literal)
{
	hash64(compileTimeLiteralHash("Literal"));
	hash64(_literal.value.hash());
	hash64(_literal.type.hash());
	hash8(static_cast<uint8_t>(_literal.kind));
}

void BlockHasher::operator()(Identifier const& _identifier)
{
	hash64(compileTimeLiteralHash("Identifier"));
	auto it = m_variableReferences.find(_identifier.name);
	if (it == m_variableReferences.end())
	{
		it = m_variableReferences.emplace(_identifier.name, VariableReference {
			m_externalIdentifierCount++,
			true
		}).first;
		m_externalReferences.emplace_back(_identifier.name);
	}
	if (it->second.isExternal)
		hash64(compileTimeLiteralHash("external"));
	else
		hash64(compileTimeLiteralHash("internal"));
	hash64(it->second.id);
}

void BlockHasher::operator()(FunctionalInstruction const& _instr)
{
	hash64(compileTimeLiteralHash("FunctionalInstruction"));
	hash8(static_cast<std::underlying_type_t<eth::Instruction>>(_instr.instruction));
	hash64(_instr.arguments.size());
	ASTWalker::operator()(_instr);
}

void BlockHasher::operator()(FunctionCall const& _funCall)
{
	hash64(compileTimeLiteralHash("FunctionCall"));
	hash64(_funCall.functionName.name.hash());
	hash64(_funCall.arguments.size());
	ASTWalker::operator()(_funCall);
}

void BlockHasher::operator()(ExpressionStatement const& _statement)
{
	hash64(compileTimeLiteralHash("ExpressionStatement"));
	ASTWalker::operator()(_statement);
}

void BlockHasher::operator()(Assignment const& _assignment)
{
	hash64(compileTimeLiteralHash("Assignment"));
	hash64(_assignment.variableNames.size());
	for (auto const& name: _assignment.variableNames)
		(*this)(name);
	visit(*_assignment.value);
}

void BlockHasher::operator()(VariableDeclaration const& _varDecl)
{
	hash64(compileTimeLiteralHash("VariableDeclaration"));
	hash64(_varDecl.variables.size());
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
	hash64(compileTimeLiteralHash("If"));
	ASTWalker::operator()(_if);
}

void BlockHasher::operator()(Switch const& _switch)
{
	hash64(compileTimeLiteralHash("Switch"));
	hash64(_switch.cases.size());
	// Instead of sorting we could consider to combine
	// the case hashes using a commutative operation here.
	std::set<Case const*, SwitchCaseCompareByLiteralValue> cases;
	for (auto const& _case: _switch.cases)
		cases.insert(&_case);

	visit(*_switch.expression);
	for (auto const& _case: cases)
	{
		if (_case->value)
			(*this)(*_case->value);
		(*this)(_case->body);
	}
}

void BlockHasher::operator()(FunctionDefinition const& _funDef)
{
	hash64(compileTimeLiteralHash("FunctionDefinition"));
	ASTWalker::operator()(_funDef);
}

void BlockHasher::operator()(ForLoop const& _loop)
{
	yulAssert(_loop.pre.statements.empty(), "");

	hash64(compileTimeLiteralHash("ForLoop"));
	ASTWalker::operator()(_loop);
}

void BlockHasher::operator()(Break const& _break)
{
	hash64(compileTimeLiteralHash("Break"));
	ASTWalker::operator()(_break);
}

void BlockHasher::operator()(Continue const& _continue)
{
	hash64(compileTimeLiteralHash("Continue"));
	ASTWalker::operator()(_continue);
}


void BlockHasher::operator()(Block const& _block)
{
	hash64(compileTimeLiteralHash("Block"));
	hash64(_block.statements.size());
	if (_block.statements.empty())
		return;

	BlockHasher subBlockHasher(m_blockHashes);
	for (auto const& statement: _block.statements)
		subBlockHasher.visit(statement);

	m_blockHashes[&_block] = subBlockHasher.m_hash;

	hash64(subBlockHasher.m_hash);
	hash64(subBlockHasher.m_externalReferences.size());

	for (auto& externalReference: subBlockHasher.m_externalReferences)
		(*this)(Identifier{{}, externalReference});
}
