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

#include <libyul/AsmParser.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>

#include <boost/algorithm/string.hpp>

#include <cctype>
#include <algorithm>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;
using namespace dev::solidity;

shared_ptr<Block> Parser::parse(std::shared_ptr<Scanner> const& _scanner, bool _reuseScanner)
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
		solAssert(!m_errorReporter.errors().empty(), "Fatal error detected, but no error is reported.");
	}
	return nullptr;
}

Block Parser::parseBlock()
{
	RecursionGuard recursionGuard(*this);
	Block block = createWithLocation<Block>();
	expectToken(Token::LBrace);
	while (currentToken() != Token::RBrace)
		block.statements.emplace_back(parseStatement());
	block.location.end = endPosition();
	advance();
	return block;
}

Statement Parser::parseStatement()
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
		If _if = createWithLocation<If>();
		m_scanner->next();
		_if.condition = make_unique<Expression>(parseExpression());
		_if.body = parseBlock();
		return Statement{move(_if)};
	}
	case Token::Switch:
	{
		Switch _switch = createWithLocation<Switch>();
		m_scanner->next();
		_switch.expression = make_unique<Expression>(parseExpression());
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
		return Statement{move(_switch)};
	}
	case Token::For:
		return parseForLoop();
	case Token::Assign:
	{
		if (m_dialect->flavour != AsmFlavour::Loose)
			break;
		StackAssignment assignment = createWithLocation<StackAssignment>();
		advance();
		expectToken(Token::Colon);
		assignment.variableName.location = location();
		assignment.variableName.name = YulString(currentLiteral());
		if (m_dialect->builtin(assignment.variableName.name))
			fatalParserError("Identifier expected, got builtin symbol.");
		else if (instructions().count(assignment.variableName.name.str()))
			fatalParserError("Identifier expected, got instruction name.");
		assignment.location.end = endPosition();
		expectToken(Token::Identifier);
		return Statement{move(assignment)};
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
	case Token::AssemblyAssign:
	{
		std::vector<Identifier> variableNames;

		while (true)
		{
			if (elementary.type() != typeid(Identifier))
			{
				auto const token = currentToken() == Token::Comma ? "," : ":=";

				fatalParserError(
					std::string("Variable name must precede \"") +
					token +
					"\"" +
					(currentToken() == Token::Comma ? " in multiple assignment." : " in assignment.")
				);
			}

			auto const& identifier = boost::get<Identifier>(elementary);

			if (m_dialect->builtin(identifier.name))
				fatalParserError("Cannot assign to builtin function \"" + identifier.name.str() + "\".");

			variableNames.emplace_back(identifier);

			if (currentToken() != Token::Comma)
				break;

			expectToken(Token::Comma);

			elementary = parseElementaryOperation();
		}

		Assignment assignment =
			createWithLocation<Assignment>(boost::get<Identifier>(elementary).location);
		assignment.variableNames = std::move(variableNames);

		expectToken(Token::AssemblyAssign);

		assignment.value.reset(new Expression(parseExpression()));
		assignment.location.end = locationOf(*assignment.value).end;

		return Statement{std::move(assignment)};
	}
	case Token::Colon:
	{
		if (elementary.type() != typeid(Identifier))
			fatalParserError("Label name must precede \":\".");

		Identifier const& identifier = boost::get<Identifier>(elementary);

		advance();

		// label
		if (m_dialect->flavour != AsmFlavour::Loose)
			fatalParserError("Labels are not supported.");

		Label label = createWithLocation<Label>(identifier.location);
		label.name = identifier.name;
		return label;
	}
	default:
		if (m_dialect->flavour != AsmFlavour::Loose)
			fatalParserError("Call or assignment expected.");
		break;
	}
	if (elementary.type() == typeid(Identifier))
	{
		Expression expr = boost::get<Identifier>(elementary);
		return ExpressionStatement{locationOf(expr), expr};
	}
	else if (elementary.type() == typeid(Literal))
	{
		Expression expr = boost::get<Literal>(elementary);
		return ExpressionStatement{locationOf(expr), expr};
	}
	else
	{
		solAssert(elementary.type() == typeid(Instruction), "Invalid elementary operation.");
		return boost::get<Instruction>(elementary);
	}
}

Case Parser::parseCase()
{
	RecursionGuard recursionGuard(*this);
	Case _case = createWithLocation<Case>();
	if (m_scanner->currentToken() == Token::Default)
		m_scanner->next();
	else if (m_scanner->currentToken() == Token::Case)
	{
		m_scanner->next();
		ElementaryOperation literal = parseElementaryOperation();
		if (literal.type() != typeid(Literal))
			fatalParserError("Literal expected.");
		_case.value = make_unique<Literal>(boost::get<Literal>(std::move(literal)));
	}
	else
		solAssert(false, "Case or default case expected.");
	_case.body = parseBlock();
	_case.location.end = _case.body.location.end;
	return _case;
}

ForLoop Parser::parseForLoop()
{
	RecursionGuard recursionGuard(*this);
	ForLoop forLoop = createWithLocation<ForLoop>();
	expectToken(Token::For);
	forLoop.pre = parseBlock();
	forLoop.condition = make_unique<Expression>(parseExpression());
	forLoop.post = parseBlock();
	forLoop.body = parseBlock();
	forLoop.location.end = forLoop.body.location.end;
	return forLoop;
}

Expression Parser::parseExpression()
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
		if (m_dialect->flavour != AsmFlavour::Loose && currentToken() != Token::LParen)
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
		solAssert(m_dialect->flavour == AsmFlavour::Loose, "");
		Instruction& instr = boost::get<Instruction>(operation);
		return FunctionalInstruction{std::move(instr.location), instr.instruction, {}};
	}
	else if (operation.type() == typeid(Identifier))
		return boost::get<Identifier>(operation);
	else
	{
		solAssert(operation.type() == typeid(Literal), "");
		return boost::get<Literal>(operation);
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
		YulString literal;
		if (currentToken() == Token::Return)
			literal = "return"_yulstring;
		else if (currentToken() == Token::Byte)
			literal = "byte"_yulstring;
		else if (currentToken() == Token::Address)
			literal = "address"_yulstring;
		else
			literal = YulString{currentLiteral()};
		// first search the set of builtins, then the instructions.
		if (m_dialect->builtin(literal))
			ret = Identifier{location(), literal};
		else if (m_dialect->flavour != AsmFlavour::Yul && instructions().count(literal.str()))
		{
			dev::solidity::Instruction const& instr = instructions().at(literal.str());
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
			YulString{currentLiteral()},
			{}
		};
		advance();
		if (m_dialect->flavour == AsmFlavour::Yul)
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
			m_dialect->flavour == AsmFlavour::Yul ?
			"Literal or identifier expected." :
			"Literal, identifier or instruction expected."
		);
	}
	return ret;
}

VariableDeclaration Parser::parseVariableDeclaration()
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
	if (currentToken() == Token::AssemblyAssign)
	{
		expectToken(Token::AssemblyAssign);
		varDecl.value = make_unique<Expression>(parseExpression());
		varDecl.location.end = locationOf(*varDecl.value).end;
	}
	else
		varDecl.location.end = varDecl.variables.back().location.end;
	return varDecl;
}

FunctionDefinition Parser::parseFunctionDefinition()
{
	RecursionGuard recursionGuard(*this);
	FunctionDefinition funDef = createWithLocation<FunctionDefinition>();
	expectToken(Token::Function);
	funDef.name = expectAsmIdentifier();
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

Expression Parser::parseCall(Parser::ElementaryOperation&& _initialOp)
{
	RecursionGuard recursionGuard(*this);
	if (_initialOp.type() == typeid(Instruction))
	{
		solAssert(m_dialect->flavour != AsmFlavour::Yul, "Instructions are invalid in Yul");
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
			m_dialect->flavour == AsmFlavour::Yul ?
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
	if (m_dialect->flavour == AsmFlavour::Yul)
	{
		expectToken(Token::Colon);
		typedName.location.end = endPosition();
		typedName.type = expectAsmIdentifier();
	}
	return typedName;
}

YulString Parser::expectAsmIdentifier()
{
	YulString name = YulString{currentLiteral()};
	if (m_dialect->flavour == AsmFlavour::Yul)
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
	else if (m_dialect->builtin(name))
		fatalParserError("Cannot use builtin function name \"" + name.str() + "\" as identifier name.");
	else if (instructions().count(name.str()))
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
