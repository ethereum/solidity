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
#include <libyul/Utilities.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/Common.h>
#include <libsolutil/Common.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <regex>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace
{

std::optional<int> toInt(std::string const& _value)
{
	try
	{
		return stoi(_value);
	}
	catch (std::out_of_range const&)
	{
		return std::nullopt;
	}
}

}

langutil::DebugData::ConstPtr Parser::createDebugData() const
{
	switch (m_useSourceLocationFrom)
	{
		case UseSourceLocationFrom::Scanner:
			return DebugData::create(ParserBase::currentLocation(), ParserBase::currentLocation());
		case UseSourceLocationFrom::LocationOverride:
			return DebugData::create(m_locationOverride, m_locationOverride);
		case UseSourceLocationFrom::Comments:
			return DebugData::create(ParserBase::currentLocation(), m_locationFromComment, m_astIDFromComment);
	}
	solAssert(false, "");
}

void Parser::updateLocationEndFrom(
	langutil::DebugData::ConstPtr& _debugData,
	SourceLocation const& _location
) const
{
	solAssert(_debugData, "");

	switch (m_useSourceLocationFrom)
	{
		case UseSourceLocationFrom::Scanner:
		{
			DebugData updatedDebugData = *_debugData;
			updatedDebugData.nativeLocation.end = _location.end;
			updatedDebugData.originLocation.end = _location.end;
			_debugData = std::make_shared<DebugData const>(std::move(updatedDebugData));
			break;
		}
		case UseSourceLocationFrom::LocationOverride:
			// Ignore the update. The location we're overriding with is not supposed to change
			break;
		case UseSourceLocationFrom::Comments:
		{
			DebugData updatedDebugData = *_debugData;
			updatedDebugData.nativeLocation.end = _location.end;
			_debugData = std::make_shared<DebugData const>(std::move(updatedDebugData));
			break;
		}
	}
}

std::unique_ptr<AST> Parser::parse(CharStream& _charStream)
{
	m_scanner = std::make_shared<Scanner>(_charStream);
	std::unique_ptr<AST> ast = parseInline(m_scanner);
	expectToken(Token::EOS);
	return ast;
}

std::unique_ptr<AST> Parser::parseInline(std::shared_ptr<Scanner> const& _scanner)
{
	m_recursionDepth = 0;

	auto previousScannerKind = _scanner->scannerKind();
	_scanner->setScannerMode(ScannerKind::Yul);
	ScopeGuard resetScanner([&]{ _scanner->setScannerMode(previousScannerKind); });

	try
	{
		m_scanner = _scanner;
		if (m_useSourceLocationFrom == UseSourceLocationFrom::Comments)
			fetchDebugDataFromComment();
		return std::make_unique<AST>(parseBlock());
	}
	catch (FatalError const& error)
	{
		yulAssert(m_errorReporter.hasErrors(), "Unreported fatal error: "s + error.what());
	}

	return nullptr;
}

langutil::Token Parser::advance()
{
	auto const token = ParserBase::advance();
	if (m_useSourceLocationFrom == UseSourceLocationFrom::Comments)
		fetchDebugDataFromComment();
	return token;
}

void Parser::fetchDebugDataFromComment()
{
	solAssert(m_sourceNames.has_value(), "");

	static std::regex const tagRegex = std::regex(
		R"~~((?:^|\s+)(@[a-zA-Z0-9\-_]+)(?:\s+|$))~~", // tag, e.g: @src
		std::regex_constants::ECMAScript | std::regex_constants::optimize
	);

	std::string_view commentLiteral = m_scanner->currentCommentLiteral();
	std::match_results<std::string_view::const_iterator> match;

	langutil::SourceLocation originLocation = m_locationFromComment;
	// Empty for each new node.
	std::optional<int> astID;

	while (regex_search(commentLiteral.cbegin(), commentLiteral.cend(), match, tagRegex))
	{
		solAssert(match.size() == 2, "");
		commentLiteral = commentLiteral.substr(static_cast<size_t>(match.position() + match.length()));

		if (match[1] == "@src")
		{
			if (auto parseResult = parseSrcComment(commentLiteral, m_scanner->currentCommentLocation()))
				tie(commentLiteral, originLocation) = *parseResult;
			else
				break;
		}
		else if (match[1] == "@ast-id")
		{
			if (auto parseResult = parseASTIDComment(commentLiteral, m_scanner->currentCommentLocation()))
				tie(commentLiteral, astID) = *parseResult;
			else
				break;
		}
		else
			// Ignore unrecognized tags.
			continue;
	}

	m_locationFromComment = originLocation;
	m_astIDFromComment = astID;
}

std::optional<std::pair<std::string_view, SourceLocation>> Parser::parseSrcComment(
	std::string_view const _arguments,
	langutil::SourceLocation const& _commentLocation
)
{
	CharStream argumentStream(std::string(_arguments), "");
	Scanner scanner(argumentStream, ScannerKind::SpecialComment);

	std::string_view tail{_arguments.substr(_arguments.size())};
	auto const parseLocationComponent = [](Scanner& _scanner, bool expectTrailingColon) -> std::optional<std::string>
	{
		bool negative = false;
		if (_scanner.currentToken() == Token::Sub)
		{
			negative = true;
			_scanner.next();
		}
		if (_scanner.currentToken() != Token::Number)
			return std::nullopt;
		if (expectTrailingColon && _scanner.peekNextToken() != Token::Colon)
			return std::nullopt;
		if (!isValidDecimal(_scanner.currentLiteral()))
			return std::nullopt;
		std::string decimal = (negative ? "-" : "") + _scanner.currentLiteral();
		_scanner.next();
		if (expectTrailingColon)
			_scanner.next();
		return decimal;
	};
	std::optional<std::string> rawSourceIndex = parseLocationComponent(scanner, true);
	std::optional<std::string> rawStart = parseLocationComponent(scanner, true);
	std::optional<std::string> rawEnd = parseLocationComponent(scanner, false);

	size_t const snippetStart = static_cast<size_t>(scanner.currentLocation().start);
	bool const locationScannedSuccessfully = rawSourceIndex && rawStart && rawEnd;
	bool const locationIsWhitespaceSeparated =
		scanner.peekNextToken() == Token::EOS ||
		(snippetStart > 0 && langutil::isWhiteSpace(_arguments[snippetStart - 1]));

	if (!locationScannedSuccessfully || !locationIsWhitespaceSeparated)
	{
		m_errorReporter.syntaxError(
			8387_error,
			_commentLocation,
			"Invalid values in source location mapping. Could not parse location specification."
		);
		return std::nullopt;
	}

	// captures error cases `"test` (illegal end quote) and `"test\` (illegal escape sequence / dangling backslash)
	bool const illegalLiteral = scanner.currentToken() == Token::Illegal && (scanner.currentError() == ScannerError::IllegalStringEndQuote || scanner.currentError() == ScannerError::IllegalEscapeSequence);
	if (scanner.currentToken() == Token::StringLiteral || illegalLiteral)
		tail = _arguments.substr(static_cast<size_t>(scanner.currentLocation().end));
	else
		tail = _arguments.substr(static_cast<size_t>(scanner.currentLocation().start));

	// Other scanner errors may occur if there is no string literal which follows
	// (f.ex. IllegalHexDigit, IllegalCommentTerminator), but these are ignored
	if (illegalLiteral)
	{
		m_errorReporter.syntaxError(
			1544_error,
			_commentLocation,
			"Invalid code snippet in source location mapping. Quote is not terminated."
		);
		return {{tail, SourceLocation{}}};
	}

	std::optional<int> const sourceIndex = toInt(*rawSourceIndex);
	std::optional<int> const start = toInt(*rawStart);
	std::optional<int> const end = toInt(*rawEnd);

	if (
		!sourceIndex.has_value() || *sourceIndex < -1 ||
		!start.has_value() || *start < -1 ||
		!end.has_value() || *end < -1
	)
		m_errorReporter.syntaxError(
			6367_error,
			_commentLocation,
			"Invalid value in source location mapping. "
			"Expected non-negative integer values or -1 for source index and location."
		);
	else if (sourceIndex == -1)
		return {{tail, SourceLocation{*start, *end, nullptr}}};
	else if (!(sourceIndex >= 0 && m_sourceNames->count(static_cast<unsigned>(*sourceIndex))))
		m_errorReporter.syntaxError(
			2674_error,
			_commentLocation,
			"Invalid source mapping. Source index not defined via @use-src."
		);
	else
	{
		std::shared_ptr<std::string const> sourceName = m_sourceNames->at(static_cast<unsigned>(*sourceIndex));
		solAssert(sourceName, "");
		return {{tail, SourceLocation{*start, *end, std::move(sourceName)}}};
	}
	return {{tail, SourceLocation{}}};
}

std::optional<std::pair<std::string_view, std::optional<int>>> Parser::parseASTIDComment(
	std::string_view _arguments,
	langutil::SourceLocation const& _commentLocation
)
{
	static std::regex const argRegex = std::regex(
		R"~~(^(\d+)(?:\s|$))~~",
		std::regex_constants::ECMAScript | std::regex_constants::optimize
	);
	std::match_results<std::string_view::const_iterator> match;
	std::optional<int> astID;
	bool matched = regex_search(_arguments.cbegin(), _arguments.cend(), match, argRegex);
	std::string_view tail = _arguments;
	if (matched)
	{
		solAssert(match.size() == 2, "");
		tail = _arguments.substr(static_cast<size_t>(match.position() + match.length()));

		astID = toInt(match[1].str());
	}

	if (!matched || !astID || *astID < 0 || static_cast<int64_t>(*astID) != *astID)
	{
		m_errorReporter.syntaxError(1749_error, _commentLocation, "Invalid argument for @ast-id.");
		astID = std::nullopt;
	}
	if (matched)
		return {{_arguments, astID}};
	else
		return std::nullopt;
}

Block Parser::parseBlock()
{
	RecursionGuard recursionGuard(*this);
	Block block = createWithDebugData<Block>();
	expectToken(Token::LBrace);
	while (currentToken() != Token::RBrace)
		block.statements.emplace_back(parseStatement());
	updateLocationEndFrom(block.debugData, currentLocation());
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
		If _if = createWithDebugData<If>();
		advance();
		_if.condition = std::make_unique<Expression>(parseExpression());
		_if.body = parseBlock();
		updateLocationEndFrom(_if.debugData, nativeLocationOf(_if.body));
		return Statement{std::move(_if)};
	}
	case Token::Switch:
	{
		Switch _switch = createWithDebugData<Switch>();
		advance();
		_switch.expression = std::make_unique<Expression>(parseExpression());
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
		updateLocationEndFrom(_switch.debugData, nativeLocationOf(_switch.cases.back().body));
		return Statement{std::move(_switch)};
	}
	case Token::For:
		return parseForLoop();
	case Token::Break:
	{
		Statement stmt{createWithDebugData<Break>()};
		checkBreakContinuePosition("break");
		advance();
		return stmt;
	}
	case Token::Continue:
	{
		Statement stmt{createWithDebugData<Continue>()};
		checkBreakContinuePosition("continue");
		advance();
		return stmt;
	}
	case Token::Leave:
	{
		Statement stmt{createWithDebugData<Leave>()};
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
	std::variant<Literal, Identifier> elementary(parseLiteralOrIdentifier());

	switch (currentToken())
	{
	case Token::LParen:
	{
		Expression expr = parseCall(std::move(elementary));
		return ExpressionStatement{debugDataOf(expr), std::move(expr)};
	}
	case Token::Comma:
	case Token::AssemblyAssign:
	{
		Assignment assignment;
		assignment.debugData = debugDataOf(elementary);

		while (true)
		{
			if (!std::holds_alternative<Identifier>(elementary))
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

		assignment.value = std::make_unique<Expression>(parseExpression());
		updateLocationEndFrom(assignment.debugData, nativeLocationOf(*assignment.value));

		return Statement{std::move(assignment)};
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
	Case _case = createWithDebugData<Case>();
	if (currentToken() == Token::Default)
		advance();
	else if (currentToken() == Token::Case)
	{
		advance();
		std::variant<Literal, Identifier> literal = parseLiteralOrIdentifier();
		if (!std::holds_alternative<Literal>(literal))
			fatalParserError(4805_error, "Literal expected.");
		_case.value = std::make_unique<Literal>(std::get<Literal>(std::move(literal)));
	}
	else
		yulAssert(false, "Case or default case expected.");
	_case.body = parseBlock();
	updateLocationEndFrom(_case.debugData, nativeLocationOf(_case.body));
	return _case;
}

ForLoop Parser::parseForLoop()
{
	RecursionGuard recursionGuard(*this);

	ForLoopComponent outerForLoopComponent = m_currentForLoopComponent;

	ForLoop forLoop = createWithDebugData<ForLoop>();
	expectToken(Token::For);
	m_currentForLoopComponent = ForLoopComponent::ForLoopPre;
	forLoop.pre = parseBlock();
	m_currentForLoopComponent = ForLoopComponent::None;
	forLoop.condition = std::make_unique<Expression>(parseExpression());
	m_currentForLoopComponent = ForLoopComponent::ForLoopPost;
	forLoop.post = parseBlock();
	m_currentForLoopComponent = ForLoopComponent::ForLoopBody;
	forLoop.body = parseBlock();
	updateLocationEndFrom(forLoop.debugData, nativeLocationOf(forLoop.body));

	m_currentForLoopComponent = outerForLoopComponent;

	return forLoop;
}

Expression Parser::parseExpression(bool _unlimitedLiteralArgument)
{
	RecursionGuard recursionGuard(*this);

	std::variant<Literal, Identifier> operation = parseLiteralOrIdentifier(_unlimitedLiteralArgument);
	return visit(GenericVisitor{
		[&](Identifier& _identifier) -> Expression
		{
			if (currentToken() == Token::LParen)
				return parseCall(std::move(operation));
			if (m_dialect.builtin(_identifier.name))
				fatalParserError(
					7104_error,
					nativeLocationOf(_identifier),
					"Builtin function \"" + _identifier.name.str() + "\" must be called."
				);
			return std::move(_identifier);
		},
		[&](Literal& _literal) -> Expression
		{
			return std::move(_literal);
		}
	}, operation);
}

std::variant<Literal, Identifier> Parser::parseLiteralOrIdentifier(bool _unlimitedLiteralArgument)
{
	RecursionGuard recursionGuard(*this);
	switch (currentToken())
	{
	case Token::Identifier:
	{
		Identifier identifier{createDebugData(), YulName{currentLiteral()}};
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

		auto const beginOfLiteralLoc = currentLocation();
		Literal literal{
			createDebugData(),
			kind,
			valueOfLiteral(currentLiteral(), kind, _unlimitedLiteralArgument && kind == LiteralKind::String)
		};
		advance();
		if (currentToken() == Token::Colon)
		{
			expectToken(Token::Colon);
			updateLocationEndFrom(literal.debugData, currentLocation());
			auto const loc = SourceLocation::smallestCovering(beginOfLiteralLoc, currentLocation());
			std::ignore = expectAsmIdentifier();
			expectUntyped(loc);
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
	VariableDeclaration varDecl = createWithDebugData<VariableDeclaration>();
	expectToken(Token::Let);
	while (true)
	{
		varDecl.variables.emplace_back(parseNameWithDebugData());
		if (currentToken() == Token::Comma)
			expectToken(Token::Comma);
		else
			break;
	}
	if (currentToken() == Token::AssemblyAssign)
	{
		expectToken(Token::AssemblyAssign);
		varDecl.value = std::make_unique<Expression>(parseExpression());
		updateLocationEndFrom(varDecl.debugData, nativeLocationOf(*varDecl.value));
	}
	else
		updateLocationEndFrom(varDecl.debugData, nativeLocationOf(varDecl.variables.back()));

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

	FunctionDefinition funDef = createWithDebugData<FunctionDefinition>();
	expectToken(Token::Function);
	funDef.name = expectAsmIdentifier();
	expectToken(Token::LParen);
	while (currentToken() != Token::RParen)
	{
		funDef.parameters.emplace_back(parseNameWithDebugData());
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
			funDef.returnVariables.emplace_back(parseNameWithDebugData());
			if (currentToken() == Token::LBrace)
				break;
			expectToken(Token::Comma);
		}
	}
	bool preInsideFunction = m_insideFunction;
	m_insideFunction = true;
	funDef.body = parseBlock();
	m_insideFunction = preInsideFunction;
	updateLocationEndFrom(funDef.debugData, nativeLocationOf(funDef.body));

	m_currentForLoopComponent = outerForLoopComponent;
	return funDef;
}

FunctionCall Parser::parseCall(std::variant<Literal, Identifier>&& _initialOp)
{
	RecursionGuard recursionGuard(*this);

	if (!std::holds_alternative<Identifier>(_initialOp))
		fatalParserError(9980_error, "Function name expected.");

	FunctionCall ret;
	ret.functionName = std::move(std::get<Identifier>(_initialOp));
	ret.debugData = ret.functionName.debugData;
	auto const isUnlimitedLiteralArgument = [f=m_dialect.builtin(ret.functionName.name)](size_t const index) {
		if (f && index < f->literalArguments.size())
			return f->literalArgument(index).has_value();
		return false;
	};
	size_t argumentIndex {0};
	expectToken(Token::LParen);
	if (currentToken() != Token::RParen)
	{
		ret.arguments.emplace_back(parseExpression(isUnlimitedLiteralArgument(argumentIndex++)));
		while (currentToken() != Token::RParen)
		{
			expectToken(Token::Comma);
			ret.arguments.emplace_back(parseExpression(isUnlimitedLiteralArgument(argumentIndex++)));
		}
	}
	updateLocationEndFrom(ret.debugData, currentLocation());
	expectToken(Token::RParen);
	return ret;
}

NameWithDebugData Parser::parseNameWithDebugData()
{
	RecursionGuard recursionGuard(*this);
	NameWithDebugData typedName = createWithDebugData<NameWithDebugData>();
	auto const locNameStart = currentLocation();
	typedName.name = expectAsmIdentifier();
	if (currentToken() == Token::Colon)
	{
		expectToken(Token::Colon);
		updateLocationEndFrom(typedName.debugData, currentLocation());
		auto const loc = SourceLocation::smallestCovering(locNameStart, currentLocation());
		std::ignore = expectAsmIdentifier();
		expectUntyped(loc);
	}

	return typedName;
}

YulName Parser::expectAsmIdentifier()
{
	YulName name{currentLiteral()};
	if (currentToken() == Token::Identifier && m_dialect.builtin(name))
		fatalParserError(5568_error, "Cannot use builtin function name \"" + name.str() + "\" as identifier name.");
	// NOTE: We keep the expectation here to ensure the correct source location for the error above.
	expectToken(Token::Identifier);
	return name;
}

void Parser::checkBreakContinuePosition(std::string const& _which)
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

bool Parser::isValidNumberLiteral(std::string const& _literal)
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
		return _literal.find_first_not_of("0123456789") == std::string::npos;
}

void Parser::expectUntyped(SourceLocation const& loc)
{
	m_errorReporter.parserError(5473_error, loc, "Types are not valid in untyped Yul.");
}
