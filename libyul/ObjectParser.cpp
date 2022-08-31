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

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

shared_ptr<Object> ObjectParser::parse(shared_ptr<Scanner> const& _scanner, bool _reuseScanner)
{
	m_recursionDepth = 0;
	try
	{
		shared_ptr<Object> object;
		m_scanner = _scanner;

		if (currentToken() == Token::LBrace)
		{
			// Special case: Code-only form.
			object = make_shared<Object>();
			object->name = "object"_yulstring;
			auto sourceNameMapping = tryParseSourceNameMapping();
			object->debugData = make_shared<ObjectDebugData>(ObjectDebugData{sourceNameMapping});
			object->code = parseBlock(sourceNameMapping);
			if (!object->code)
				return nullptr;
		}
		else
			object = parseObject();
		if (!_reuseScanner)
			expectToken(Token::EOS);
		return object;
	}
	catch (FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
	}
	return nullptr;
}

shared_ptr<Object> ObjectParser::parseObject(Object* _containingObject)
{
	RecursionGuard guard(*this);

	shared_ptr<Object> ret = make_shared<Object>();

	auto sourceNameMapping = tryParseSourceNameMapping();
	ret->debugData = make_shared<ObjectDebugData>(ObjectDebugData{sourceNameMapping});

	if (currentToken() != Token::Identifier || currentLiteral() != "object")
		fatalParserError(4294_error, "Expected keyword \"object\".");
	advance();

	ret->name = parseUniqueName(_containingObject);

	expectToken(Token::LBrace);

	ret->code = parseCode(std::move(sourceNameMapping));

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

shared_ptr<Block> ObjectParser::parseCode(optional<SourceNameMap> _sourceNames)
{
	if (currentToken() != Token::Identifier || currentLiteral() != "code")
		fatalParserError(4846_error, "Expected keyword \"code\".");
	advance();

	return parseBlock(std::move(_sourceNames));
}

optional<SourceNameMap> ObjectParser::tryParseSourceNameMapping() const
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
		return nullopt;

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
		sourceNames[*sourceIndex] = make_shared<string const>(scanner.currentLiteral());

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
	return nullopt;
}

shared_ptr<Block> ObjectParser::parseBlock(optional<SourceNameMap> _sourceNames)
{
	Parser parser(m_errorReporter, m_dialect, std::move(_sourceNames));
	shared_ptr<Block> block = parser.parseInline(m_scanner);
	yulAssert(block || m_errorReporter.hasErrors(), "Invalid block but no error!");
	return block;
}

void ObjectParser::parseData(Object& _containingObject)
{
	yulAssert(
		currentToken() == Token::Identifier && currentLiteral() == "data",
		"parseData called on wrong input."
	);
	advance();

	YulString name = parseUniqueName(&_containingObject);

	if (currentToken() == Token::HexStringLiteral)
		expectToken(Token::HexStringLiteral, false);
	else
		expectToken(Token::StringLiteral, false);
	addNamedSubObject(_containingObject, name, make_shared<Data>(name, asBytes(currentLiteral())));
	advance();
}

YulString ObjectParser::parseUniqueName(Object const* _containingObject)
{
	expectToken(Token::StringLiteral, false);
	YulString name{currentLiteral()};
	if (name.empty())
		parserError(3287_error, "Object name cannot be empty.");
	else if (_containingObject && _containingObject->name == name)
		parserError(8311_error, "Object name cannot be the same as the name of the containing object.");
	else if (_containingObject && _containingObject->subIndexByName.count(name))
		parserError(8794_error, "Object name \"" + name.str() + "\" already exists inside the containing object.");
	advance();
	return name;
}

void ObjectParser::addNamedSubObject(Object& _container, YulString _name, shared_ptr<ObjectNode> _subObject)
{
	_container.subIndexByName[_name] = _container.subObjects.size();
	_container.subObjects.emplace_back(std::move(_subObject));
}
