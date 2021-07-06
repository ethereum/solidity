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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity inline assembly parser.
 */

#include <libyul/AST.h>
#include <libyul/AsmParser.h>
#include <libyul/Exceptions.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>
#include <libsolutil/Common.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace
{

[[nodiscard]]
shared_ptr<DebugData const> updateLocationEndFrom(
	shared_ptr<DebugData const> const& _debugData,
	langutil::SourceLocation const& _location
)
{
	SourceLocation updatedLocation = _debugData->location;
	updatedLocation.end = _location.end;
	return make_shared<DebugData const>(updatedLocation);
}

}

unique_ptr<Block> Parser::parse(std::shared_ptr<Scanner> const& _scanner, bool _reuseScanner)
{
	m_recursionDepth = 0;

	_scanner->setScannerMode(ScannerKind::Yul);
	ScopeGuard resetScanner([&]{ _scanner->setScannerMode(ScannerKind::Solidity); });

	try
	{
		m_scanner = _scanner;
		auto block = make_unique<Block>(parseBlock());
		if (!_reuseScanner)
			expectToken(Token::EOS);
		return block;
	}
	catch (FatalError const&)
	{
		yulAssert(!m_errorReporter.errors().empty(), "Fatal error detected, but no error is reported.");
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
	block.debugData = updateLocationEndFrom(block.debugData, currentLocation());
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
		advance();
		_if.condition = make_unique<Expression>(parseExpression());
		_if.body = parseBlock();
		return Statement{move(_if)};
	}
	case Token::Switch:
	{
		Switch _switch = createWithLocation<Switch>();
		advance();
		_switch.expression = make_unique<Expression>(parseExpression());
		while (currentToken() == Token::Case)
			_switch.cases.emplace_back(parseCase());
		if (currentToken() == Token::Default)
			_switch.cases.emplace_back(parseCase());
		if (currentToken() == Token::Default)
			fatalParserError(6931_error, "Only one default case allowed.");
		else if (currentToken() == Token::Case)
			fatalParserError(4904_error, "Case not allowed after default case.");
		if (_switch.cases.empty())
			fatalParserError(2418_error, "Switch statement without any cases.");
		_switch.debugData = updateLocationEndFrom(_switch.debugData, _switch.cases.back().body.debugData->location);
		return Statement{move(_switch)};
	}
	case Token::For:
		return parseForLoop();
	case Token::Break:
	{
		Statement stmt{createWithLocation<Break>()};
		checkBreakContinuePosition("break");
		advance();
		return stmt;
	}
	case Token::Continue:
	{
		Statement stmt{createWithLocation<Continue>()};
		checkBreakContinuePosition("continue");
		advance();
		return stmt;
	}
	case Token::Leave:
	{
		Statement stmt{createWithLocation<Leave>()};
		if (!m_insideFunction)
			m_errorReporter.syntaxError(8149_error, currentLocation(), "Keyword \"leave\" can only be used inside a function.");
		advance();
		return stmt;
	}
	default:
		break;
	}

	// Options left:
	// Expression/FunctionCall
	// Assignment
	variant<Literal, Identifier> elementary(parseLiteralOrIdentifier());

	switch (currentToken())
	{
	case Token::LParen:
	{
		Expression expr = parseCall(std::move(elementary));
		return ExpressionStatement{debugDataOf(expr), move(expr)};
	}
	case Token::Comma:
	case Token::AssemblyAssign:
	{
		Assignment assignment;
		assignment.debugData = debugDataOf(elementary);

		while (true)
		{
			if (!holds_alternative<Identifier>(elementary))
			{
				auto const token = currentToken() == Token::Comma ? "," : ":=";

				fatalParserError(
					2856_error,
					std::string("Variable name must precede \"") +
					token +
					"\"" +
					(currentToken() == Token::Comma ? " in multiple assignment." : " in assignment.")
				);
			}

			auto const& identifier = std::get<Identifier>(elementary);

			if (m_dialect.builtin(identifier.name))
				fatalParserError(6272_error, "Cannot assign to builtin function \"" + identifier.name.str() + "\".");

			assignment.variableNames.emplace_back(identifier);

			if (currentToken() != Token::Comma)
				break;

			expectToken(Token::Comma);

			elementary = parseLiteralOrIdentifier();
		}

		expectToken(Token::AssemblyAssign);

		assignment.value = make_unique<Expression>(parseExpression());
		assignment.debugData = updateLocationEndFrom(assignment.debugData, locationOf(*assignment.value));

		return Statement{move(assignment)};
	}
	default:
		fatalParserError(6913_error, "Call or assignment expected.");
		break;
	}

	yulAssert(false, "");
	return {};
}

Case Parser::parseCase()
{
	RecursionGuard recursionGuard(*this);
	Case _case = createWithLocation<Case>();
	if (currentToken() == Token::Default)
		advance();
	else if (currentToken() == Token::Case)
	{
		advance();
		variant<Literal, Identifier> literal = parseLiteralOrIdentifier();
		if (!holds_alternative<Literal>(literal))
			fatalParserError(4805_error, "Literal expected.");
		_case.value = make_unique<Literal>(std::get<Literal>(std::move(literal)));
	}
	else
		yulAssert(false, "Case or default case expected.");
	_case.body = parseBlock();
	_case.debugData = updateLocationEndFrom(_case.debugData, _case.body.debugData->location);
	return _case;
}

ForLoop Parser::parseForLoop()
{
	RecursionGuard recursionGuard(*this);

	ForLoopComponent outerForLoopComponent = m_currentForLoopComponent;

	ForLoop forLoop = createWithLocation<ForLoop>();
	expectToken(Token::For);
	m_currentForLoopComponent = ForLoopComponent::ForLoopPre;
	forLoop.pre = parseBlock();
	m_currentForLoopComponent = ForLoopComponent::None;
	forLoop.condition = make_unique<Expression>(parseExpression());
	m_currentForLoopComponent = ForLoopComponent::ForLoopPost;
	forLoop.post = parseBlock();
	m_currentForLoopComponent = ForLoopComponent::ForLoopBody;
	forLoop.body = parseBlock();
	forLoop.debugData = updateLocationEndFrom(forLoop.debugData, forLoop.body.debugData->location);

	m_currentForLoopComponent = outerForLoopComponent;

	return forLoop;
}

Expression Parser::parseExpression()
{
	RecursionGuard recursionGuard(*this);

	variant<Literal, Identifier> operation = parseLiteralOrIdentifier();
	return visit(GenericVisitor{
		[&](Identifier& _identifier) -> Expression
		{
			if (currentToken() == Token::LParen)
				return parseCall(std::move(operation));
			if (m_dialect.builtin(_identifier.name))
				fatalParserError(
					7104_error,
					_identifier.debugData->location,
					"Builtin function \"" + _identifier.name.str() + "\" must be called."
				);
			return move(_identifier);
		},
		[&](Literal& _literal) -> Expression
		{
			return move(_literal);
		}
	}, operation);
}

variant<Literal, Identifier> Parser::parseLiteralOrIdentifier()
{
	RecursionGuard recursionGuard(*this);
	switch (currentToken())
	{
	case Token::Identifier:
	{
		Identifier identifier{DebugData::create(currentLocation()), YulString{currentLiteral()}};
		advance();
		return identifier;
	}
	case Token::StringLiteral:
	case Token::HexStringLiteral:
	case Token::Number:
	case Token::TrueLiteral:
	case Token::FalseLiteral:
	{
		LiteralKind kind = LiteralKind::Number;
		switch (currentToken())
		{
		case Token::StringLiteral:
		case Token::HexStringLiteral:
			kind = LiteralKind::String;
			break;
		case Token::Number:
			if (!isValidNumberLiteral(currentLiteral()))
				fatalParserError(4828_error, "Invalid number literal.");
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
			DebugData::create(currentLocation()),
			kind,
			YulString{currentLiteral()},
			kind == LiteralKind::Boolean ? m_dialect.boolType : m_dialect.defaultType
		};
		advance();
		if (currentToken() == Token::Colon)
		{
			expectToken(Token::Colon);
			literal.debugData = updateLocationEndFrom(literal.debugData, currentLocation());
			literal.type = expectAsmIdentifier();
		}

		return literal;
	}
	case Token::Illegal:
		fatalParserError(1465_error, "Illegal token: " + to_string(m_scanner->currentError()));
		break;
	default:
		fatalParserError(1856_error, "Literal or identifier expected.");
	}
	return {};
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
		varDecl.debugData = updateLocationEndFrom(varDecl.debugData, locationOf(*varDecl.value));
	}
	else
		varDecl.debugData = updateLocationEndFrom(varDecl.debugData, varDecl.variables.back().debugData->location);

	return varDecl;
}

FunctionDefinition Parser::parseFunctionDefinition()
{
	RecursionGuard recursionGuard(*this);

	if (m_currentForLoopComponent == ForLoopComponent::ForLoopPre)
		m_errorReporter.syntaxError(
			3441_error,
			currentLocation(),
			"Functions cannot be defined inside a for-loop init block."
		);

	ForLoopComponent outerForLoopComponent = m_currentForLoopComponent;
	m_currentForLoopComponent = ForLoopComponent::None;

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
	if (currentToken() == Token::RightArrow)
	{
		expectToken(Token::RightArrow);
		while (true)
		{
			funDef.returnVariables.emplace_back(parseTypedName());
			if (currentToken() == Token::LBrace)
				break;
			expectToken(Token::Comma);
		}
	}
	bool preInsideFunction = m_insideFunction;
	m_insideFunction = true;
	funDef.body = parseBlock();
	m_insideFunction = preInsideFunction;
	funDef.debugData = updateLocationEndFrom(funDef.debugData, funDef.body.debugData->location);

	m_currentForLoopComponent = outerForLoopComponent;
	return funDef;
}

FunctionCall Parser::parseCall(variant<Literal, Identifier>&& _initialOp)
{
	RecursionGuard recursionGuard(*this);

	if (!holds_alternative<Identifier>(_initialOp))
		fatalParserError(9980_error, "Function name expected.");

	FunctionCall ret;
	ret.functionName = std::move(std::get<Identifier>(_initialOp));
	ret.debugData = ret.functionName.debugData;

	expectToken(Token::LParen);
	if (currentToken() != Token::RParen)
	{
		ret.arguments.emplace_back(parseExpression());
		while (currentToken() != Token::RParen)
		{
			expectToken(Token::Comma);
			ret.arguments.emplace_back(parseExpression());
		}
	}
	ret.debugData = updateLocationEndFrom(ret.debugData, currentLocation());
	expectToken(Token::RParen);
	return ret;
}

TypedName Parser::parseTypedName()
{
	RecursionGuard recursionGuard(*this);
	TypedName typedName = createWithLocation<TypedName>();
	typedName.name = expectAsmIdentifier();
	if (currentToken() == Token::Colon)
	{
		expectToken(Token::Colon);
		typedName.debugData = updateLocationEndFrom(typedName.debugData, currentLocation());
		typedName.type = expectAsmIdentifier();
	}
	else
		typedName.type = m_dialect.defaultType;

	return typedName;
}

YulString Parser::expectAsmIdentifier()
{
	YulString name{currentLiteral()};
	if (currentToken() == Token::Identifier && m_dialect.builtin(name))
		fatalParserError(5568_error, "Cannot use builtin function name \"" + name.str() + "\" as identifier name.");
	// NOTE: We keep the expectation here to ensure the correct source location for the error above.
	expectToken(Token::Identifier);
	return name;
}

void Parser::checkBreakContinuePosition(string const& _which)
{
	switch (m_currentForLoopComponent)
	{
	case ForLoopComponent::None:
		m_errorReporter.syntaxError(2592_error, currentLocation(), "Keyword \"" + _which + "\" needs to be inside a for-loop body.");
		break;
	case ForLoopComponent::ForLoopPre:
		m_errorReporter.syntaxError(9615_error, currentLocation(), "Keyword \"" + _which + "\" in for-loop init block is not allowed.");
		break;
	case ForLoopComponent::ForLoopPost:
		m_errorReporter.syntaxError(2461_error, currentLocation(), "Keyword \"" + _which + "\" in for-loop post block is not allowed.");
		break;
	case ForLoopComponent::ForLoopBody:
		break;
	}
}

bool Parser::isValidNumberLiteral(string const& _literal)
{
	try
	{
		// Try to convert _literal to u256.
		[[maybe_unused]] auto tmp = u256(_literal);
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
