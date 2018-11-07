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

#include <cctype>
#include <algorithm>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

shared_ptr<assembly::Block> Parser::parse(std::shared_ptr<Scanner> const& _scanner, bool _reuseScanner)
{
	m_recursionDepth = 0;
	try
	{
		m_scanner = _scanner;
		auto block = make_shared<Block>(parseBlock());
		if (!_reuseScanner)
			expectToken(Token::EOS);
		return block;
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
	case Token::If:
	{
		assembly::If _if = createWithLocation<assembly::If>();
		m_scanner->next();
		_if.condition = make_shared<Expression>(parseExpression());
		_if.body = parseBlock();
		return _if;
	}
	case Token::Switch:
	{
		assembly::Switch _switch = createWithLocation<assembly::Switch>();
		m_scanner->next();
		_switch.expression = make_shared<Expression>(parseExpression());
		while (m_scanner->currentToken() == Token::Case)
			_switch.cases.emplace_back(parseCase());
		if (m_scanner->currentToken() == Token::Default)
			_switch.cases.emplace_back(parseCase());
		if (m_scanner->currentToken() == Token::Default)
			fatalParserError("Only one default case allowed.");
		else if (m_scanner->currentToken() == Token::Case)
			fatalParserError("Case not allowed after default case.");
		if (_switch.cases.empty())
			fatalParserError("Switch statement without any cases.");
		_switch.location.end = _switch.cases.back().body.location.end;
		return _switch;
	}
	case Token::For:
		return parseForLoop();
	case Token::Assign:
	{
		if (m_flavour != AsmFlavour::Loose)
			break;
		assembly::StackAssignment assignment = createWithLocation<assembly::StackAssignment>();
		advance();
		expectToken(Token::Colon);
		assignment.variableName.location = location();
		assignment.variableName.name = YulString(currentLiteral());
		if (instructions().count(assignment.variableName.name.str()))
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
	ElementaryOperation elementary(parseElementaryOperation());
	switch (currentToken())
	{
	case Token::LParen:
	{
		Expression expr = parseCall(std::move(elementary));
		return ExpressionStatement{locationOf(expr), expr};
	}
	case Token::Comma:
	{
		// if a comma follows, a multiple assignment is assumed

		if (elementary.type() != typeid(assembly::Identifier))
			fatalParserError("Label name / variable name must precede \",\" (multiple assignment).");
		assembly::Identifier const& identifier = boost::get<assembly::Identifier>(elementary);

		Assignment assignment = createWithLocation<Assignment>(identifier.location);
		assignment.variableNames.emplace_back(identifier);

		do
		{
			expectToken(Token::Comma);
			elementary = parseElementaryOperation();
			if (elementary.type() != typeid(assembly::Identifier))
				fatalParserError("Variable name expected in multiple assignment.");
			assignment.variableNames.emplace_back(boost::get<assembly::Identifier>(elementary));
		}
		while (currentToken() == Token::Comma);

		expectToken(Token::Colon);
		expectToken(Token::Assign);

		assignment.value.reset(new Expression(parseExpression()));
		assignment.location.end = locationOf(*assignment.value).end;
		return assignment;
	}
	case Token::Colon:
	{
		if (elementary.type() != typeid(assembly::Identifier))
			fatalParserError("Label name / variable name must precede \":\".");
		assembly::Identifier const& identifier = boost::get<assembly::Identifier>(elementary);
		advance();
		// identifier:=: should be parsed as identifier: =: (i.e. a label),
		// while identifier:= (being followed by a non-colon) as identifier := (assignment).
		if (currentToken() == Token::Assign && peekNextToken() != Token::Colon)
		{
			assembly::Assignment assignment = createWithLocation<assembly::Assignment>(identifier.location);
			if (m_flavour != AsmFlavour::Yul && instructions().count(identifier.name.str()))
				fatalParserError("Cannot use instruction names for identifier names.");
			advance();
			assignment.variableNames.emplace_back(identifier);
			assignment.value.reset(new Expression(parseExpression()));
			assignment.location.end = locationOf(*assignment.value).end;
			return assignment;
		}
		else
		{
			// label
			if (m_flavour != AsmFlavour::Loose)
				fatalParserError("Labels are not supported.");
			Label label = createWithLocation<Label>(identifier.location);
			label.name = identifier.name;
			return label;
		}
	}
	default:
		if (m_flavour != AsmFlavour::Loose)
			fatalParserError("Call or assignment expected.");
		break;
	}
	if (elementary.type() == typeid(assembly::Identifier))
	{
		Expression expr = boost::get<assembly::Identifier>(elementary);
		return ExpressionStatement{locationOf(expr), expr};
	}
	else if (elementary.type() == typeid(assembly::Literal))
	{
		Expression expr = boost::get<assembly::Literal>(elementary);
		return ExpressionStatement{locationOf(expr), expr};
	}
	else
	{
		solAssert(elementary.type() == typeid(assembly::Instruction), "Invalid elementary operation.");
		return boost::get<assembly::Instruction>(elementary);
	}
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
		ElementaryOperation literal = parseElementaryOperation();
		if (literal.type() != typeid(assembly::Literal))
			fatalParserError("Literal expected.");
		_case.value = make_shared<Literal>(boost::get<assembly::Literal>(std::move(literal)));
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
	forLoop.condition = make_shared<Expression>(parseExpression());
	forLoop.post = parseBlock();
	forLoop.body = parseBlock();
	forLoop.location.end = forLoop.body.location.end;
	return forLoop;
}

assembly::Expression Parser::parseExpression()
{
	RecursionGuard recursionGuard(*this);
	// In strict mode, this might parse a plain Instruction, but
	// it will be converted to a FunctionalInstruction inside
	// parseCall below.
	ElementaryOperation operation = parseElementaryOperation();
	if (operation.type() == typeid(Instruction))
	{
		Instruction const& instr = boost::get<Instruction>(operation);
		// Disallow instructions returning multiple values (and DUP/SWAP) as expression.
		if (
			instructionInfo(instr.instruction).ret != 1 ||
			isDupInstruction(instr.instruction) ||
			isSwapInstruction(instr.instruction)
		)
			fatalParserError(
				"Instruction \"" +
				instructionNames().at(instr.instruction) +
				"\" not allowed in this context."
			);
		if (m_flavour != AsmFlavour::Loose && currentToken() != Token::LParen)
			fatalParserError(
				"Non-functional instructions are not allowed in this context."
			);
		// Enforce functional notation for instructions requiring multiple arguments.
		int args = instructionInfo(instr.instruction).args;
		if (args > 0 && currentToken() != Token::LParen)
			fatalParserError(string(
				"Expected '(' (instruction \"" +
				instructionNames().at(instr.instruction) +
				"\" expects " +
				to_string(args) +
				" arguments)"
			));
	}
	if (currentToken() == Token::LParen)
		return parseCall(std::move(operation));
	else if (operation.type() == typeid(Instruction))
	{
		// Instructions not taking arguments are allowed as expressions.
		solAssert(m_flavour == AsmFlavour::Loose, "");
		Instruction& instr = boost::get<Instruction>(operation);
		return FunctionalInstruction{std::move(instr.location), instr.instruction, {}};
	}
	else if (operation.type() == typeid(assembly::Identifier))
		return boost::get<assembly::Identifier>(operation);
	else
	{
		solAssert(operation.type() == typeid(assembly::Literal), "");
		return boost::get<assembly::Literal>(operation);
	}
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

Parser::ElementaryOperation Parser::parseElementaryOperation()
{
	RecursionGuard recursionGuard(*this);
	ElementaryOperation ret;
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
		if (m_flavour != AsmFlavour::Yul && instructions().count(literal))
		{
			dev::solidity::Instruction const& instr = instructions().at(literal);
			ret = Instruction{location(), instr};
		}
		else
			ret = Identifier{location(), YulString{literal}};
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
			YulString{currentLiteral()},
			{}
		};
		advance();
		if (m_flavour == AsmFlavour::Yul)
		{
			expectToken(Token::Colon);
			literal.location.end = endPosition();
			literal.type = YulString{expectAsmIdentifier()};
		}
		else if (kind == LiteralKind::Boolean)
			fatalParserError("True and false are not valid literals.");
		ret = std::move(literal);
		break;
	}
	default:
		fatalParserError(
			m_flavour == AsmFlavour::Yul ?
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
		varDecl.value.reset(new Expression(parseExpression()));
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
	funDef.name = YulString{expectAsmIdentifier()};
	expectToken(Token::LParen);
	while (currentToken() != Token::RParen)
	{
		funDef.parameters.emplace_back(parseTypedName());
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
			funDef.returnVariables.emplace_back(parseTypedName());
			if (currentToken() == Token::LBrace)
				break;
			expectToken(Token::Comma);
		}
	}
	funDef.body = parseBlock();
	funDef.location.end = funDef.body.location.end;
	return funDef;
}

assembly::Expression Parser::parseCall(Parser::ElementaryOperation&& _initialOp)
{
	RecursionGuard recursionGuard(*this);
	if (_initialOp.type() == typeid(Instruction))
	{
		solAssert(m_flavour != AsmFlavour::Yul, "Instructions are invalid in Yul");
		Instruction& instruction = boost::get<Instruction>(_initialOp);
		FunctionalInstruction ret;
		ret.instruction = instruction.instruction;
		ret.location = std::move(instruction.location);
		solidity::Instruction instr = ret.instruction;
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
					"Expected expression (instruction \"" +
					instructionNames().at(instr) +
					"\" expects " +
					to_string(args) +
					" arguments)"
				));

			ret.arguments.emplace_back(parseExpression());
			if (i != args - 1)
			{
				if (currentToken() != Token::Comma)
					fatalParserError(string(
						"Expected ',' (instruction \"" +
						instructionNames().at(instr) +
						"\" expects " +
						to_string(args) +
						" arguments)"
					));
				else
					advance();
			}
		}
		ret.location.end = endPosition();
		if (currentToken() == Token::Comma)
			fatalParserError(string(
				"Expected ')' (instruction \"" +
				instructionNames().at(instr) +
				"\" expects " +
				to_string(args) +
				" arguments)"
			));
		expectToken(Token::RParen);
		return ret;
	}
	else if (_initialOp.type() == typeid(Identifier))
	{
		FunctionCall ret;
		ret.functionName = std::move(boost::get<Identifier>(_initialOp));
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
			m_flavour == AsmFlavour::Yul ?
			"Function name expected." :
			"Assembly instruction or function name required in front of \"(\")"
		);

	return {};
}

TypedName Parser::parseTypedName()
{
	RecursionGuard recursionGuard(*this);
	TypedName typedName = createWithLocation<TypedName>();
	typedName.name = YulString{expectAsmIdentifier()};
	if (m_flavour == AsmFlavour::Yul)
	{
		expectToken(Token::Colon);
		typedName.location.end = endPosition();
		typedName.type = YulString{expectAsmIdentifier()};
	}
	return typedName;
}

string Parser::expectAsmIdentifier()
{
	string name = currentLiteral();
	if (m_flavour == AsmFlavour::Yul)
	{
		switch (currentToken())
		{
		case Token::Return:
		case Token::Byte:
		case Token::Address:
		case Token::Bool:
			advance();
			return name;
		default:
			break;
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
		// Try to convert _literal to u256.
		auto tmp = u256(_literal);
		(void) tmp;
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
