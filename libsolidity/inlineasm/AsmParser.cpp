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
#include <libsolidity/interface/Exceptions.h>

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
	case Token::Function:
		return parseFunctionDefinition();
	case Token::LBrace:
		return parseBlock();
	case Token::Switch:
	{
		assembly::Switch _switch = createWithLocation<assembly::Switch>();
		m_scanner->next();
		_switch.expression = make_shared<Statement>(parseExpression());
		if (_switch.expression->type() == typeid(assembly::Instruction))
			fatalParserError("Instructions are not supported as expressions for switch.");
		while (m_scanner->currentToken() == Token::Case)
			_switch.cases.emplace_back(parseCase());
		if (m_scanner->currentToken() == Token::Default)
			_switch.cases.emplace_back(parseCase());
		if (m_scanner->currentToken() == Token::Default)
			fatalParserError("Only one default case allowed.");
		else if (m_scanner->currentToken() == Token::Case)
			fatalParserError("Case not allowed after default case.");
		if (_switch.cases.size() == 0)
			fatalParserError("Switch statement without any cases.");
		_switch.location.end = _switch.cases.back().body.location.end;
		return _switch;
	}
	case Token::Assign:
	{
		if (m_julia)
			break;
		assembly::StackAssignment assignment = createWithLocation<assembly::StackAssignment>();
		m_scanner->next();
		expectToken(Token::Colon);
		assignment.variableName.location = location();
		assignment.variableName.name = m_scanner->currentLiteral();
		if (!m_julia && instructions().count(assignment.variableName.name))
			fatalParserError("Identifier expected, got instruction name.");
		assignment.location.end = endPosition();
		expectToken(Token::Identifier);
		return assignment;
	}
	default:
		break;
	}
	// Options left:
	// Simple instruction (might turn into functional),
	// literal,
	// identifier (might turn into label or functional assignment)
	Statement statement(parseElementaryOperation(false));
	switch (m_scanner->currentToken())
	{
	case Token::LParen:
		return parseCall(std::move(statement));
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
			assembly::Assignment assignment = createWithLocation<assembly::Assignment>(identifier.location);
			if (!m_julia && instructions().count(identifier.name))
				fatalParserError("Cannot use instruction names for identifier names.");
			m_scanner->next();
			assignment.variableName = identifier;
			assignment.value.reset(new Statement(parseExpression()));
			assignment.location.end = locationOf(*assignment.value).end;
			return assignment;
		}
		else
		{
			// label
			if (m_julia)
				fatalParserError("Labels are not supported.");
			Label label = createWithLocation<Label>(identifier.location);
			label.name = identifier.name;
			return label;
		}
	}
	default:
		if (m_julia)
			fatalParserError("Call or assignment expected.");
		break;
	}
	return statement;
}

assembly::Case Parser::parseCase()
{
	assembly::Case _case = createWithLocation<assembly::Case>();
	if (m_scanner->currentToken() == Token::Default)
		m_scanner->next();
	else if (m_scanner->currentToken() == Token::Case)
	{
		m_scanner->next();
		assembly::Statement statement = parseElementaryOperation();
		if (statement.type() != typeid(assembly::Literal))
			fatalParserError("Literal expected.");
		_case.value = make_shared<Literal>(std::move(boost::get<assembly::Literal>(statement)));
	}
	else
		fatalParserError("Case or default case expected.");
	_case.body = parseBlock();
	_case.location.end = _case.body.location.end;
	return _case;
}

assembly::Statement Parser::parseExpression()
{
	Statement operation = parseElementaryOperation(true);
	if (m_scanner->currentToken() == Token::LParen)
		return parseCall(std::move(operation));
	else
		return operation;
}

std::map<string, dev::solidity::Instruction> const& Parser::instructions()
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

		// add alias for suicide
		s_instructions["suicide"] = solidity::Instruction::SELFDESTRUCT;
	}
	return s_instructions;
}

assembly::Statement Parser::parseElementaryOperation(bool _onlySinglePusher)
{
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
		if (!m_julia && instructions().count(literal))
		{
			dev::solidity::Instruction const& instr = instructions().at(literal);
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
		m_scanner->next();
		break;
	}
	case Token::StringLiteral:
	case Token::Number:
	case Token::TrueLiteral:
	case Token::FalseLiteral:
	{
		LiteralKind kind = LiteralKind::Number;
		switch (m_scanner->currentToken())
		{
		case Token::StringLiteral:
			kind = LiteralKind::String;
			break;
		case Token::Number:
			kind = LiteralKind::Number;
			break;
		case Token::TrueLiteral:
		case Token::FalseLiteral:
			kind = LiteralKind::Boolean;
			break;
		default:
			break;
		}

		Literal literal{
			location(),
			kind,
			m_scanner->currentLiteral(),
			""
		};
		m_scanner->next();
		if (m_julia)
		{
			expectToken(Token::Colon);
			literal.location.end = endPosition();
			literal.type = expectAsmIdentifier();
		}
		else if (kind == LiteralKind::Boolean)
			fatalParserError("True and false are not valid literals.");
		ret = std::move(literal);
		break;
	}
	default:
		fatalParserError(
			m_julia ?
			"Literal or identifier expected." :
			"Literal, identifier or instruction expected."
		);
	}
	return ret;
}

assembly::VariableDeclaration Parser::parseVariableDeclaration()
{
	VariableDeclaration varDecl = createWithLocation<VariableDeclaration>();
	expectToken(Token::Let);
	while (true)
	{
		varDecl.variables.emplace_back(parseTypedName());
		if (m_scanner->currentToken() == Token::Comma)
			expectToken(Token::Comma);
		else
			break;
	}
	expectToken(Token::Colon);
	expectToken(Token::Assign);
	varDecl.value.reset(new Statement(parseExpression()));
	varDecl.location.end = locationOf(*varDecl.value).end;
	return varDecl;
}

assembly::FunctionDefinition Parser::parseFunctionDefinition()
{
	FunctionDefinition funDef = createWithLocation<FunctionDefinition>();
	expectToken(Token::Function);
	funDef.name = expectAsmIdentifier();
	expectToken(Token::LParen);
	while (m_scanner->currentToken() != Token::RParen)
	{
		funDef.arguments.emplace_back(parseTypedName());
		if (m_scanner->currentToken() == Token::RParen)
			break;
		expectToken(Token::Comma);
	}
	expectToken(Token::RParen);
	if (m_scanner->currentToken() == Token::Sub)
	{
		expectToken(Token::Sub);
		expectToken(Token::GreaterThan);
		while (true)
		{
			funDef.returns.emplace_back(parseTypedName());
			if (m_scanner->currentToken() == Token::LBrace)
				break;
			expectToken(Token::Comma);
		}
	}
	funDef.body = parseBlock();
	funDef.location.end = funDef.body.location.end;
	return funDef;
}

assembly::Statement Parser::parseCall(assembly::Statement&& _instruction)
{
	if (_instruction.type() == typeid(Instruction))
	{
		solAssert(!m_julia, "Instructions are invalid in JULIA");
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
			/// check for premature closing parentheses
			if (m_scanner->currentToken() == Token::RParen)
				fatalParserError(string(
					"Expected expression (" +
					instrInfo.name +
					" expects " +
					boost::lexical_cast<string>(args) +
					" arguments)"
				));

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
	else if (_instruction.type() == typeid(Identifier))
	{
		FunctionCall ret;
		ret.functionName = std::move(boost::get<Identifier>(_instruction));
		ret.location = ret.functionName.location;
		expectToken(Token::LParen);
		while (m_scanner->currentToken() != Token::RParen)
		{
			ret.arguments.emplace_back(parseExpression());
			if (m_scanner->currentToken() == Token::RParen)
				break;
			expectToken(Token::Comma);
		}
		ret.location.end = endPosition();
		expectToken(Token::RParen);
		return ret;
	}
	else
		fatalParserError(
			m_julia ?
			"Function name expected." :
			"Assembly instruction or function name required in front of \"(\")"
		);

	return {};
}

TypedName Parser::parseTypedName()
{
	TypedName typedName = createWithLocation<TypedName>();
	typedName.name = expectAsmIdentifier();
	if (m_julia)
	{
		expectToken(Token::Colon);
		typedName.location.end = endPosition();
		typedName.type = expectAsmIdentifier();
	}
	return typedName;
}

string Parser::expectAsmIdentifier()
{
	string name = m_scanner->currentLiteral();
	if (m_julia)
	{
		if (m_scanner->currentToken() == Token::Bool)
		{
			m_scanner->next();
			return name;
		}
	}
	else if (instructions().count(name))
		fatalParserError("Cannot use instruction names for identifier names.");
	expectToken(Token::Identifier);
	return name;
}
