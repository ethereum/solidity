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
 * Parser for Yul code and data object container.
 */

#include <libyul/AST.h>
#include <libyul/ObjectParser.h>

#include <libyul/AsmParser.h>
#include <libyul/Exceptions.h>

#include <liblangutil/Token.h>
#include <liblangutil/Scanner.h>

#include <libsolutil/StringUtils.h>

#include <regex>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

std::shared_ptr<Object> ObjectParser::parse(std::shared_ptr<Scanner> const& _scanner, bool _reuseScanner)
{
	m_recursionDepth = 0;
	try
	{
		std::shared_ptr<Object> object;
		m_scanner = _scanner;

		if (currentToken() == Token::LBrace)
		{
			// Special case: Code-only form.
			object = std::make_shared<Object>(m_dialect);
			object->name = "object";
			auto sourceNameMapping = tryParseSourceNameMapping();
			object->debugData = std::make_shared<ObjectDebugData>(ObjectDebugData{sourceNameMapping});
			object->setCode(parseBlock(sourceNameMapping));
			if (!object->hasCode())
				return nullptr;
		}
		else
			object = parseObject();
		if (!_reuseScanner)
			expectToken(Token::EOS);
		return object;
	}
	catch (FatalError const& error)
	{
		yulAssert(m_errorReporter.hasErrors(), "Unreported fatal error: "s + error.what());
	}
	return nullptr;
}

std::shared_ptr<Object> ObjectParser::parseObject(Object* _containingObject)
{
	RecursionGuard guard(*this);

	std::shared_ptr<Object> ret = std::make_shared<Object>(m_dialect);

	auto sourceNameMapping = tryParseSourceNameMapping();
	ret->debugData = std::make_shared<ObjectDebugData>(ObjectDebugData{sourceNameMapping});

	if (currentToken() != Token::Identifier || currentLiteral() != "object")
		fatalParserError(4294_error, "Expected keyword \"object\".");
	advance();

	ret->name = parseUniqueName(_containingObject);

	expectToken(Token::LBrace);

	ret->setCode(parseCode(std::move(sourceNameMapping)));

	while (currentToken() != Token::RBrace)
	{
		if (currentToken() == Token::Identifier && currentLiteral() == "object")
			parseObject(ret.get());
		else if (currentToken() == Token::Identifier && currentLiteral() == "data")
			parseData(*ret);
		else
			fatalParserError(8143_error, "Expected keyword \"data\" or \"object\" or \"}\".");
	}
	if (_containingObject)
		addNamedSubObject(*_containingObject, ret->name, ret);

	expectToken(Token::RBrace);

	return ret;
}

std::shared_ptr<AST> ObjectParser::parseCode(std::optional<SourceNameMap> _sourceNames)
{
	if (currentToken() != Token::Identifier || currentLiteral() != "code")
		fatalParserError(4846_error, "Expected keyword \"code\".");
	advance();

	return parseBlock(std::move(_sourceNames));
}

std::optional<SourceNameMap> ObjectParser::tryParseSourceNameMapping() const
{
	// @use-src 0:"abc.sol", 1:"foo.sol", 2:"bar.sol"
	//
	// UseSrcList := UseSrc (',' UseSrc)*
	// UseSrc     := [0-9]+ ':' FileName
	// FileName   := "(([^\"]|\.)*)"

	// Matches some "@use-src TEXT".
	static std::regex const lineRE = std::regex(
		"(^|\\s)@use-src\\b",
		std::regex_constants::ECMAScript | std::regex_constants::optimize
	);
	std::smatch sm;
	if (!std::regex_search(m_scanner->currentCommentLiteral(), sm, lineRE))
		return std::nullopt;

	solAssert(sm.size() == 2, "");
	auto text = m_scanner->currentCommentLiteral().substr(static_cast<size_t>(sm.position() + sm.length()));
	CharStream charStream(text, "");
	Scanner scanner(charStream);
	if (scanner.currentToken() == Token::EOS)
		return SourceNameMap{};
	SourceNameMap sourceNames;

	while (scanner.currentToken() != Token::EOS)
	{
		if (scanner.currentToken() != Token::Number)
			break;
		auto sourceIndex = toUnsignedInt(scanner.currentLiteral());
		if (!sourceIndex)
			break;
		if (scanner.next() != Token::Colon)
			break;
		if (scanner.next() != Token::StringLiteral)
			break;
		sourceNames[*sourceIndex] = std::make_shared<std::string const>(scanner.currentLiteral());

		Token const next = scanner.next();
		if (next == Token::EOS)
			return {std::move(sourceNames)};
		if (next != Token::Comma)
			break;
		scanner.next();
	}

	m_errorReporter.syntaxError(
		9804_error,
		m_scanner->currentCommentLocation(),
		"Error parsing arguments to @use-src. Expected: <number> \":\" \"<filename>\", ..."
	);
	return std::nullopt;
}

std::shared_ptr<AST> ObjectParser::parseBlock(std::optional<SourceNameMap> _sourceNames)
{
	Parser parser(m_errorReporter, m_dialect, std::move(_sourceNames));
	auto ast = parser.parseInline(m_scanner);
	yulAssert(ast || m_errorReporter.hasErrors(), "Invalid block but no error!");
	return ast;
}

void ObjectParser::parseData(Object& _containingObject)
{
	yulAssert(
		currentToken() == Token::Identifier && currentLiteral() == "data",
		"parseData called on wrong input."
	);
	advance();

	auto const name = parseUniqueName(&_containingObject);

	if (currentToken() == Token::HexStringLiteral)
		expectToken(Token::HexStringLiteral, false);
	else
		expectToken(Token::StringLiteral, false);
	addNamedSubObject(_containingObject, name, std::make_shared<Data>(name, asBytes(currentLiteral())));
	advance();
}

std::string ObjectParser::parseUniqueName(Object const* _containingObject)
{
	expectToken(Token::StringLiteral, false);
	auto const name = currentLiteral();
	if (name.empty())
		parserError(3287_error, "Object name cannot be empty.");
	else if (_containingObject && _containingObject->name == name)
		parserError(8311_error, "Object name cannot be the same as the name of the containing object.");
	else if (_containingObject && _containingObject->subIndexByName.count(name))
		parserError(8794_error, "Object name \"" + name + "\" already exists inside the containing object.");
	advance();
	return name;
}

void ObjectParser::addNamedSubObject(Object& _container, std::string_view const _name, std::shared_ptr<ObjectNode> _subObject)
{
	_container.subIndexByName.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(_name),
		std::forward_as_tuple(_container.subObjects.size())
	);
	_container.subObjects.emplace_back(std::move(_subObject));
}
