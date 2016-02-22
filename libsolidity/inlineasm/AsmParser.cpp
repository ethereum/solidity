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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity inline assembly parser.
 */

#include <libsolidity/inlineasm/AsmParser.h>
#include <ctype.h>
#include <algorithm>
#include <libevmcore/Instruction.h>
#include <libsolidity/parsing/Scanner.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

shared_ptr<AsmData> InlineAssemblyParser::parse(std::shared_ptr<Scanner> const& _scanner)
{
	try
	{
		m_scanner = _scanner;
		return make_shared<AsmData>(parseBlock());
	}
	catch (FatalError const&)
	{
		if (m_errors.empty())
			throw; // Something is weird here, rather throw again.
	}
	return nullptr;
}

AsmData::Block InlineAssemblyParser::parseBlock()
{
	expectToken(Token::LBrace);
	AsmData::Block block;
	while (m_scanner->currentToken() != Token::RBrace)
		block.statements.emplace_back(parseStatement());
	m_scanner->next();
	return block;
}

AsmData::Statement InlineAssemblyParser::parseStatement()
{
	switch (m_scanner->currentToken())
	{
	case Token::Let:
		return parseVariableDeclaration();
	case Token::LBrace:
		return parseBlock();
	case Token::Assign:
	{
		m_scanner->next();
		expectToken(Token::Colon);
		string name = m_scanner->currentLiteral();
		expectToken(Token::Identifier);
		return AsmData::Assignment{AsmData::Identifier{name}};
	}
	default:
		break;
	}
	// Options left:
	// Simple instruction (might turn into functional),
	// literal,
	// identifier (might turn into label or functional assignment)
	AsmData::Statement statement(parseElementaryOperation());
	switch (m_scanner->currentToken())
	{
	case Token::LParen:
		return parseFunctionalInstruction(statement);
	case Token::Colon:
	{
		if (statement.type() != typeid(AsmData::Identifier))
			fatalParserError("Label name / variable name must precede \":\".");
		string const& name = boost::get<AsmData::Identifier>(statement).name;
		m_scanner->next();
		if (m_scanner->currentToken() == Token::Assign)
		{
			// functional assignment
			m_scanner->next();
			unique_ptr<AsmData::Statement> value;
			value.reset(new AsmData::Statement(parseExpression()));
			return AsmData::FunctionalAssignment{{move(name)}, move(value)};
		}
		else
			// label
			return AsmData::Label{name};
	}
	default:
		break;
	}
	return statement;
}

AsmData::Statement InlineAssemblyParser::parseExpression()
{
	AsmData::Statement operation = parseElementaryOperation(true);
	if (m_scanner->currentToken() == Token::LParen)
		return parseFunctionalInstruction(operation);
	else
		return operation;
}

AsmData::Statement InlineAssemblyParser::parseElementaryOperation(bool _onlySinglePusher)
{
	// Allowed instructions, lowercase names.
	static map<string, eth::Instruction> s_instructions;
	if (s_instructions.empty())
		for (auto const& instruction: eth::c_instructions)
		{
			if (
				instruction.second == eth::Instruction::JUMPDEST ||
				(eth::Instruction::PUSH1 <= instruction.second && instruction.second <= eth::Instruction::PUSH32)
			)
				continue;
			string name = instruction.first;
			transform(name.begin(), name.end(), name.begin(), [](unsigned char _c) { return tolower(_c); });
			s_instructions[name] = instruction.second;
		}

	//@TODO track location

	switch (m_scanner->currentToken())
	{
	case Token::Identifier:
	{
		string literal = m_scanner->currentLiteral();
		// first search the set of instructions.
		if (s_instructions.count(literal))
		{
			eth::Instruction const& instr = s_instructions[literal];
			if (_onlySinglePusher)
			{
				eth::InstructionInfo info = eth::instructionInfo(instr);
				if (info.ret != 1)
					fatalParserError("Instruction " + info.name + " not allowed in this context.");
			}
			m_scanner->next();
			return AsmData::Instruction{instr};
		}
		else
			m_scanner->next();
			return AsmData::Identifier{literal};
		break;
	}
	case Token::StringLiteral:
	case Token::Number:
	{
		AsmData::Literal literal{
			m_scanner->currentToken() == Token::Number,
			m_scanner->currentLiteral()
		};
		m_scanner->next();
		return literal;
	}
	default:
		break;
	}
	fatalParserError("Expected elementary inline assembly operation.");
	return {};
}

AsmData::VariableDeclaration InlineAssemblyParser::parseVariableDeclaration()
{
	expectToken(Token::Let);
	string name = m_scanner->currentLiteral();
	expectToken(Token::Identifier);
	expectToken(Token::Colon);
	expectToken(Token::Assign);
	unique_ptr<AsmData::Statement> value;
	value.reset(new AsmData::Statement(parseExpression()));
	return AsmData::VariableDeclaration{name, move(value)};
}

AsmData::FunctionalInstruction InlineAssemblyParser::parseFunctionalInstruction(AsmData::Statement const& _instruction)
{
	if (_instruction.type() != typeid(AsmData::Instruction))
		fatalParserError("Assembly instruction required in front of \"(\")");
	eth::Instruction instr = boost::get<AsmData::Instruction>(_instruction).instruction;
	eth::InstructionInfo instrInfo = eth::instructionInfo(instr);
	if (eth::Instruction::DUP1 <= instr && instr <= eth::Instruction::DUP16)
		fatalParserError("DUPi instructions not allowed for functional notation");
	if (eth::Instruction::SWAP1 <= instr && instr <= eth::Instruction::SWAP16)
		fatalParserError("SWAPi instructions not allowed for functional notation");

	expectToken(Token::LParen);
	vector<AsmData::Statement> arguments;
	unsigned args = unsigned(instrInfo.args);
	for (unsigned i = 0; i < args; ++i)
	{
		arguments.push_back(parseExpression());
		if (i != args - 1)
			expectToken(Token::Comma);
	}
	expectToken(Token::RParen);
	return AsmData::FunctionalInstruction{{instr}, move(arguments)};
}
