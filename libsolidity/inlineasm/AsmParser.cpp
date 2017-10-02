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
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/ErrorReporter.h>

#include <boost/algorithm/string.hpp>

#include <ctype.h>
#include <algorithm>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

shared_ptr<assembly::Block> Parser::parse(std::shared_ptr<Scanner> const& _scanner)
{
	m_recursionDepth = 0;
	try
	{
		m_scanner = _scanner;
		return make_shared<Block>(parseBlock());
	}
	catch (FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
	}
	return nullptr;
}

assembly::Block Parser::parseBlock()
{
	RecursionGuard recursionGuard(*this);
	assembly::Block block = createWithLocation<Block>();
	expectToken(Token::LBrace);
	while (currentToken() != Token::RBrace)
		block.statements.emplace_back(parseStatement());
	block.location.end = endPosition();
	advance();
	return block;
}

assembly::Statement Parser::parseStatement()
{
	RecursionGuard recursionGuard(*this);
	switch (currentToken())
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
	case Token::For:
		return parseForLoop();
	case Token::Assign:
	{
		if (m_julia)
			break;
		assembly::StackAssignment assignment = createWithLocation<assembly::StackAssignment>();
		advance();
		expectToken(Token::Colon);
		assignment.variableName.location = location();
		assignment.variableName.name = currentLiteral();
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
	switch (currentToken())
	{
	case Token::LParen:
		return parseCall(std::move(statement));
	case Token::Comma:
	{
		// if a comma follows, a multiple assignment is assumed

		if (statement.type() != typeid(assembly::Identifier))
			fatalParserError("Label name / variable name must precede \",\" (multiple assignment).");
		assembly::Identifier const& identifier = boost::get<assembly::Identifier>(statement);

		Assignment assignment = createWithLocation<Assignment>(identifier.location);
		assignment.variableNames.emplace_back(identifier);

		do
		{
			expectToken(Token::Comma);
			statement = parseElementaryOperation(false);
			if (statement.type() != typeid(assembly::Identifier))
				fatalParserError("Variable name expected in multiple assignemnt.");
			assignment.variableNames.emplace_back(boost::get<assembly::Identifier>(statement));
		}
		while (currentToken() == Token::Comma);

		expectToken(Token::Colon);
		expectToken(Token::Assign);

		assignment.value.reset(new Statement(parseExpression()));
		assignment.location.end = locationOf(*assignment.value).end;
		return assignment;
	}
	case Token::Colon:
	{
		if (statement.type() != typeid(assembly::Identifier))
			fatalParserError("Label name / variable name must precede \":\".");
		assembly::Identifier const& identifier = boost::get<assembly::Identifier>(statement);
		advance();
		// identifier:=: should be parsed as identifier: =: (i.e. a label),
		// while identifier:= (being followed by a non-colon) as identifier := (assignment).
		if (currentToken() == Token::Assign && peekNextToken() != Token::Colon)
		{
			assembly::Assignment assignment = createWithLocation<assembly::Assignment>(identifier.location);
			if (!m_julia && instructions().count(identifier.name))
				fatalParserError("Cannot use instruction names for identifier names.");
			advance();
			assignment.variableNames.emplace_back(identifier);
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
	RecursionGuard recursionGuard(*this);
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

assembly::ForLoop Parser::parseForLoop()
{
	RecursionGuard recursionGuard(*this);
	ForLoop forLoop = createWithLocation<ForLoop>();
	expectToken(Token::For);
	forLoop.pre = parseBlock();
	forLoop.condition = make_shared<Statement>(parseExpression());
	if (forLoop.condition->type() == typeid(assembly::Instruction))
		fatalParserError("Instructions are not supported as conditions for the for statement.");
	forLoop.post = parseBlock();
	forLoop.body = parseBlock();
	forLoop.location.end = forLoop.body.location.end;
	return forLoop;
}

assembly::Statement Parser::parseExpression()
{
	RecursionGuard recursionGuard(*this);
	Statement operation = parseElementaryOperation(true);
	if (operation.type() == typeid(Instruction))
	{
		Instruction const& instr = boost::get<Instruction>(operation);
		int args = instructionInfo(instr.instruction).args;
		if (args > 0 && currentToken() != Token::LParen)
			fatalParserError(string(
				"Expected token \"(\" (\"" +
				instructionNames().at(instr.instruction) +
				"\" expects " +
				boost::lexical_cast<string>(args) +
				" arguments)"
			));
	}
	if (currentToken() == Token::LParen)
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
				solidity::isPushInstruction(instruction.second)
			)
				continue;
			string name = instruction.first;
			transform(name.begin(), name.end(), name.begin(), [](unsigned char _c) { return tolower(_c); });
			s_instructions[name] = instruction.second;
		}

		// add alias for suicide
		s_instructions["suicide"] = solidity::Instruction::SELFDESTRUCT;
		// add alis for sha3
		s_instructions["sha3"] = solidity::Instruction::KECCAK256;
	}
	return s_instructions;
}

std::map<dev::solidity::Instruction, string> const& Parser::instructionNames()
{
	static map<dev::solidity::Instruction, string> s_instructionNames;
	if (s_instructionNames.empty())
	{
		for (auto const& instr: instructions())
			s_instructionNames[instr.second] = instr.first;
		// set the ambiguous instructions to a clear default
		s_instructionNames[solidity::Instruction::SELFDESTRUCT] = "selfdestruct";
		s_instructionNames[solidity::Instruction::KECCAK256] = "keccak256";
	}
	return s_instructionNames;
}

assembly::Statement Parser::parseElementaryOperation(bool _onlySinglePusher)
{
	RecursionGuard recursionGuard(*this);
	Statement ret;
	switch (currentToken())
	{
	case Token::Identifier:
	case Token::Return:
	case Token::Byte:
	case Token::Address:
	{
		string literal;
		if (currentToken() == Token::Return)
			literal = "return";
		else if (currentToken() == Token::Byte)
			literal = "byte";
		else if (currentToken() == Token::Address)
			literal = "address";
		else
			literal = currentLiteral();
		// first search the set of instructions.
		if (!m_julia && instructions().count(literal))
		{
			dev::solidity::Instruction const& instr = instructions().at(literal);
			if (_onlySinglePusher)
			{
				InstructionInfo info = dev::solidity::instructionInfo(instr);
				if (info.ret != 1)
					fatalParserError("Instruction \"" + literal + "\" not allowed in this context.");
			}
			ret = Instruction{location(), instr};
		}
		else
			ret = Identifier{location(), literal};
		advance();
		break;
	}
	case Token::StringLiteral:
	case Token::Number:
	case Token::TrueLiteral:
	case Token::FalseLiteral:
	{
		LiteralKind kind = LiteralKind::Number;
		switch (currentToken())
		{
		case Token::StringLiteral:
			kind = LiteralKind::String;
			break;
		case Token::Number:
			if (!isValidNumberLiteral(currentLiteral()))
				fatalParserError("Invalid number literal.");
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
			currentLiteral(),
			""
		};
		advance();
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
	RecursionGuard recursionGuard(*this);
	VariableDeclaration varDecl = createWithLocation<VariableDeclaration>();
	expectToken(Token::Let);
	while (true)
	{
		varDecl.variables.emplace_back(parseTypedName());
		if (currentToken() == Token::Comma)
			expectToken(Token::Comma);
		else
			break;
	}
	if (currentToken() == Token::Colon)
	{
		expectToken(Token::Colon);
		expectToken(Token::Assign);
		varDecl.value.reset(new Statement(parseExpression()));
		varDecl.location.end = locationOf(*varDecl.value).end;
	}
	else
		varDecl.location.end = varDecl.variables.back().location.end;
	return varDecl;
}

assembly::FunctionDefinition Parser::parseFunctionDefinition()
{
	RecursionGuard recursionGuard(*this);
	FunctionDefinition funDef = createWithLocation<FunctionDefinition>();
	expectToken(Token::Function);
	funDef.name = expectAsmIdentifier();
	expectToken(Token::LParen);
	while (currentToken() != Token::RParen)
	{
		funDef.arguments.emplace_back(parseTypedName());
		if (currentToken() == Token::RParen)
			break;
		expectToken(Token::Comma);
	}
	expectToken(Token::RParen);
	if (currentToken() == Token::Sub)
	{
		expectToken(Token::Sub);
		expectToken(Token::GreaterThan);
		while (true)
		{
			funDef.returns.emplace_back(parseTypedName());
			if (currentToken() == Token::LBrace)
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
	RecursionGuard recursionGuard(*this);
	if (_instruction.type() == typeid(Instruction))
	{
		solAssert(!m_julia, "Instructions are invalid in JULIA");
		FunctionalInstruction ret;
		ret.instruction = std::move(boost::get<Instruction>(_instruction));
		ret.location = ret.instruction.location;
		solidity::Instruction instr = ret.instruction.instruction;
		InstructionInfo instrInfo = instructionInfo(instr);
		if (solidity::isDupInstruction(instr))
			fatalParserError("DUPi instructions not allowed for functional notation");
		if (solidity::isSwapInstruction(instr))
			fatalParserError("SWAPi instructions not allowed for functional notation");
		expectToken(Token::LParen);
		unsigned args = unsigned(instrInfo.args);
		for (unsigned i = 0; i < args; ++i)
		{
			/// check for premature closing parentheses
			if (currentToken() == Token::RParen)
				fatalParserError(string(
					"Expected expression (\"" +
					instructionNames().at(instr) +
					"\" expects " +
					boost::lexical_cast<string>(args) +
					" arguments)"
				));

			ret.arguments.emplace_back(parseExpression());
			if (i != args - 1)
			{
				if (currentToken() != Token::Comma)
					fatalParserError(string(
						"Expected comma (\"" +
						instructionNames().at(instr) +
						"\" expects " +
						boost::lexical_cast<string>(args) +
						" arguments)"
					));
				else
					advance();
			}
		}
		ret.location.end = endPosition();
		if (currentToken() == Token::Comma)
			fatalParserError(string(
				"Expected ')' (\"" +
				instructionNames().at(instr) +
				"\" expects " +
				boost::lexical_cast<string>(args) +
				" arguments)"
			));
		expectToken(Token::RParen);
		return ret;
	}
	else if (_instruction.type() == typeid(Identifier))
	{
		FunctionCall ret;
		ret.functionName = std::move(boost::get<Identifier>(_instruction));
		ret.location = ret.functionName.location;
		expectToken(Token::LParen);
		while (currentToken() != Token::RParen)
		{
			ret.arguments.emplace_back(parseExpression());
			if (currentToken() == Token::RParen)
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
	RecursionGuard recursionGuard(*this);
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
	string name = currentLiteral();
	if (m_julia)
	{
		if (currentToken() == Token::Bool)
		{
			advance();
			return name;
		}
	}
	else if (instructions().count(name))
		fatalParserError("Cannot use instruction names for identifier names.");
	expectToken(Token::Identifier);
	return name;
}

bool Parser::isValidNumberLiteral(string const& _literal)
{
	try
	{
		u256(_literal);
	}
	catch (...)
	{
		return false;
	}
	if (boost::starts_with(_literal, "0x"))
		return true;
	else
		return _literal.find_first_not_of("0123456789") == string::npos;
}
