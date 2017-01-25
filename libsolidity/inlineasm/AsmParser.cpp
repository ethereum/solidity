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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity inline assembly parser.
 */

#include <libsolidity/inlineasm/AsmParser.h>
#include <ctype.h>
#include <algorithm>
#include <libsolidity/parsing/Scanner.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

shared_ptr<assembly::Block> Parser::parse(std::shared_ptr<Scanner> const& _scanner)
{
	try
	{
		m_scanner = _scanner;
		return make_shared<Block>(parseBlock());
	}
	catch (FatalError const&)
	{
		if (m_errors.empty())
			throw; // Something is weird here, rather throw again.
	}
	return nullptr;
}

assembly::Block Parser::parseBlock()
{
	assembly::Block block = createWithLocation<Block>();
	expectToken(Token::LBrace);
	while (m_scanner->currentToken() != Token::RBrace)
		block.statements.emplace_back(parseStatement());
	block.location.end = endPosition();
	m_scanner->next();
	return block;
}

assembly::Statement Parser::parseStatement()
{
	switch (m_scanner->currentToken())
	{
	case Token::Let:
		return parseVariableDeclaration();
	case Token::LBrace:
		return parseBlock();
	case Token::Assign:
	{
		assembly::Assignment assignment = createWithLocation<assembly::Assignment>();
		m_scanner->next();
		expectToken(Token::Colon);
		assignment.variableName.location = location();
		assignment.variableName.name = m_scanner->currentLiteral();
		assignment.location.end = endPosition();
		expectToken(Token::Identifier);
		return assignment;
	}
	case Token::Return: // opcode
	case Token::Byte: // opcode
	default:
		break;
	}
	// Options left:
	// Simple instruction (might turn into functional),
	// literal,
	// identifier (might turn into label or functional assignment)
	Statement statement(parseElementaryOperation());
	switch (m_scanner->currentToken())
	{
	case Token::LParen:
		return parseFunctionalInstruction(std::move(statement));
	case Token::Colon:
	{
		if (statement.type() != typeid(assembly::Identifier))
			fatalParserError("Label name / variable name must precede \":\".");
		assembly::Identifier const& identifier = boost::get<assembly::Identifier>(statement);
		m_scanner->next();
		// identifier:=: should be parsed as identifier: =: (i.e. a label),
		// while identifier:= (being followed by a non-colon) as identifier := (assignment).
		if (m_scanner->currentToken() == Token::Assign && m_scanner->peekNextToken() != Token::Colon)
		{
			// functional assignment
			FunctionalAssignment funAss = createWithLocation<FunctionalAssignment>(identifier.location);
			m_scanner->next();
			funAss.variableName = identifier;
			funAss.value.reset(new Statement(parseExpression()));
			funAss.location.end = locationOf(*funAss.value).end;
			return funAss;
		}
		else
		{
			// label
			Label label = createWithLocation<Label>(identifier.location);
			label.name = identifier.name;
			return label;
		}
	}
	default:
		break;
	}
	return statement;
}

assembly::Statement Parser::parseExpression()
{
	Statement operation = parseElementaryOperation(true);
	if (m_scanner->currentToken() == Token::LParen)
		return parseFunctionalInstruction(std::move(operation));
	else
		return operation;
}

std::map<string, dev::solidity::Instruction> Parser::getInstructions()
{
	// Allowed instructions, lowercase names.
	static map<string, dev::solidity::Instruction> s_instructions;
	if (s_instructions.empty())
	{
		for (auto const& instruction: solidity::c_instructions)
		{
			if (
				instruction.second == solidity::Instruction::JUMPDEST ||
				(solidity::Instruction::PUSH1 <= instruction.second && instruction.second <= solidity::Instruction::PUSH32)
			)
				continue;
			string name = instruction.first;
			transform(name.begin(), name.end(), name.begin(), [](unsigned char _c) { return tolower(_c); });
			s_instructions[name] = instruction.second;
		}

		// add alias for selfdestruct
		s_instructions["selfdestruct"] = solidity::Instruction::SUICIDE;
	}
	return s_instructions;
}

assembly::Statement Parser::parseElementaryOperation(bool _onlySinglePusher)
{
	map<string, dev::solidity::Instruction> s_instructions = getInstructions();

	Statement ret;
	switch (m_scanner->currentToken())
	{
	case Token::Identifier:
	case Token::Return:
	case Token::Byte:
	case Token::Address:
	{
		string literal;
		if (m_scanner->currentToken() == Token::Return)
			literal = "return";
		else if (m_scanner->currentToken() == Token::Byte)
			literal = "byte";
		else if (m_scanner->currentToken() == Token::Address)
			literal = "address";
		else
			literal = m_scanner->currentLiteral();
		// first search the set of instructions.
		if (s_instructions.count(literal))
		{
			dev::solidity::Instruction const& instr = s_instructions[literal];
			if (_onlySinglePusher)
			{
				InstructionInfo info = dev::solidity::instructionInfo(instr);
				if (info.ret != 1)
					fatalParserError("Instruction " + info.name + " not allowed in this context.");
			}
			ret = Instruction{location(), instr};
		}
		else
			ret = Identifier{location(), literal};
		break;
	}
	case Token::StringLiteral:
	case Token::Number:
	{
		ret = Literal{
			location(),
			m_scanner->currentToken() == Token::Number,
			m_scanner->currentLiteral()
		};
		break;
	}
	default:
		fatalParserError("Expected elementary inline assembly operation.");
	}
	m_scanner->next();
	return ret;
}

assembly::VariableDeclaration Parser::parseVariableDeclaration()
{
	map<string, dev::solidity::Instruction> s_instructions = getInstructions();
	VariableDeclaration varDecl = createWithLocation<VariableDeclaration>();
	expectToken(Token::Let);
	varDecl.name = m_scanner->currentLiteral();
	if (s_instructions.count(varDecl.name))
		fatalParserError("Cannot use instruction names for identifier names.");
	expectToken(Token::Identifier);
	expectToken(Token::Colon);
	expectToken(Token::Assign);
	varDecl.value.reset(new Statement(parseExpression()));
	varDecl.location.end = locationOf(*varDecl.value).end;
	return varDecl;
}

FunctionalInstruction Parser::parseFunctionalInstruction(assembly::Statement&& _instruction)
{
	if (_instruction.type() != typeid(Instruction))
		fatalParserError("Assembly instruction required in front of \"(\")");
	FunctionalInstruction ret;
	ret.instruction = std::move(boost::get<Instruction>(_instruction));
	ret.location = ret.instruction.location;
	solidity::Instruction instr = ret.instruction.instruction;
	InstructionInfo instrInfo = instructionInfo(instr);
	if (solidity::Instruction::DUP1 <= instr && instr <= solidity::Instruction::DUP16)
		fatalParserError("DUPi instructions not allowed for functional notation");
	if (solidity::Instruction::SWAP1 <= instr && instr <= solidity::Instruction::SWAP16)
		fatalParserError("SWAPi instructions not allowed for functional notation");

	expectToken(Token::LParen);
	unsigned args = unsigned(instrInfo.args);
	for (unsigned i = 0; i < args; ++i)
	{
		ret.arguments.emplace_back(parseExpression());
		if (i != args - 1)
		{
			if (m_scanner->currentToken() != Token::Comma)
				fatalParserError(string(
					"Expected comma (" +
					instrInfo.name +
					" expects " +
					boost::lexical_cast<string>(args) +
					" arguments)"
				));
			else
				m_scanner->next();
		}
	}
	ret.location.end = endPosition();
	if (m_scanner->currentToken() == Token::Comma)
		fatalParserError(
			string("Expected ')' (" + instrInfo.name + " expects " + boost::lexical_cast<string>(args) + " arguments)")
		);
	expectToken(Token::RParen);
	return ret;
}
