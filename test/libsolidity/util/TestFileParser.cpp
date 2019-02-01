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

#include <test/libsolidity/util/TestFileParser.h>

#include <test/Options.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

namespace
{
	bool isDecimalDigit(char c)
	{
		return '0' <= c && c <= '9';
	}
	bool isWhiteSpace(char c)
	{
		return c == ' ' || c == '\n' || c == '\t' || c == '\r';
	}
	bool isIdentifierStart(char c)
	{
		return c == '_' || c == '$' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
	}
	bool isIdentifierPart(char c)
	{
		return isIdentifierStart(c) || isDecimalDigit(c);
	}
}

vector<dev::solidity::test::FunctionCall> TestFileParser::parseFunctionCalls()
{
	vector<FunctionCall> calls;
	if (!accept(SoltToken::EOS))
	{
		// TODO: check initial token state
		expect(SoltToken::Unknown);
		while (!accept(SoltToken::EOS))
		{
			if (!accept(SoltToken::Whitespace))
			{
				FunctionCall call;

				expect(SoltToken::Newline);
				call.signature = parseFunctionSignature();

				if (accept(SoltToken::Comma, true))
					call.value = parseFunctionCallValue();
				if (accept(SoltToken::Colon, true))
					call.arguments = parseFunctionCallArguments();

				call.displayMode = parseNewline();
				call.arguments.comment = parseComment();

				if (accept(SoltToken::Newline, true))
					call.displayMode = FunctionCall::DisplayMode::MultiLine;
				expect(SoltToken::Arrow);

				call.expectations = parseFunctionCallExpectations();

				if (accept(SoltToken::Newline, false))
					call.displayMode = parseNewline();
				call.expectations.comment = parseComment();

				calls.emplace_back(std::move(call));
			}
			else
				m_scanner.scanNextToken();
		}
	}
	return calls;
}

string TestFileParser::formatToken(SoltToken _token)
{
	switch (_token)
	{
#define T(name, string, precedence) case SoltToken::name: return string;
		SOLT_TOKEN_LIST(T, T)
#undef T
		default: // Token::NUM_TOKENS:
			return "";
	}
}

bool TestFileParser::accept(SoltToken _token, bool const _expect)
{
	if (m_scanner.currentToken() == _token)
	{
		if (_expect)
			expect(_token);
		return true;
	}
	return false;
}

bool TestFileParser::expect(SoltToken _token, bool const _advance)
{
	if (m_scanner.currentToken() != _token)
		throw Error
			(Error::Type::ParserError,
					"Unexpected " + formatToken(m_scanner.currentToken()) + ": \"" +
					m_scanner.currentLiteral() + "\". " +
					"Expected \"" + formatToken(_token) + "\"."
					);
	if (_advance)
		m_scanner.scanNextToken();
	return true;
}

string TestFileParser::parseFunctionSignature()
{
	string signature = m_scanner.currentLiteral();
	expect(SoltToken::Identifier);

	signature += formatToken(SoltToken::LParen);
	expect(SoltToken::LParen);

	while (!accept(SoltToken::RParen))
	{
		signature += m_scanner.currentLiteral();
		expect(SoltToken::UInt);
		while (accept(SoltToken::Comma))
		{
			signature += m_scanner.currentLiteral();
			expect(SoltToken::Comma);
			signature += m_scanner.currentLiteral();
			expect(SoltToken::UInt);
		}
	}
	signature += formatToken(SoltToken::RParen);
	expect(SoltToken::RParen);
	return signature;
}

u256 TestFileParser::parseFunctionCallValue()
{
	u256 value = convertNumber(parseNumber());
	expect(SoltToken::Ether);
	return value;
}

FunctionCallArgs TestFileParser::parseFunctionCallArguments()
{
	FunctionCallArgs arguments;

	auto param = parseParameter();
	if (param.abiType.type == ABIType::None)
		throw Error(Error::Type::ParserError, "No argument provided.");
	arguments.parameters.emplace_back(param);

	while (accept(SoltToken::Comma, true))
		arguments.parameters.emplace_back(parseParameter());
	return arguments;
}

FunctionCallExpectations TestFileParser::parseFunctionCallExpectations()
{
	FunctionCallExpectations expectations;

	auto param = parseParameter();
	if (param.abiType.type == ABIType::None)
		return expectations;
	expectations.parameters.emplace_back(param);

	while (accept(SoltToken::Comma, true))
		expectations.parameters.emplace_back(parseParameter());

	/// We have always one virtual parameter in the parameter list.
	/// If its type is FAILURE, the expected result is also a REVERT etc.
	if (expectations.parameters.at(0).abiType.type != ABIType::Failure)
		expectations.failure = false;
	return expectations;
}

Parameter TestFileParser::parseParameter()
{
	Parameter parameter;
	if (accept(SoltToken::Newline, true))
		parameter.format.newline = true;
	auto literal = parseABITypeLiteral();
	parameter.rawBytes = literal.first;
	parameter.abiType = literal.second;
	return parameter;
}

pair<bytes, ABIType> TestFileParser::parseABITypeLiteral()
{
	try
	{
		u256 number;
		ABIType abiType;
		if (accept(SoltToken::Sub))
		{
			abiType = ABIType{ABIType::SignedDec, 32};
			expect(SoltToken::Sub);
			number = convertNumber(parseNumber()) * -1;
		}
		else
		{
			if (accept(SoltToken::Number))
			{
				abiType = ABIType{ABIType::UnsignedDec, 32};
				number = convertNumber(parseNumber());
			}
			if (accept(SoltToken::Failure, true))
			{
				abiType = ABIType{ABIType::Failure, 0};
				return make_pair(bytes{}, abiType);
			}
		}
		return make_pair(toBigEndian(number), abiType);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Number encoding invalid.");
	}
}

solidity::test::FunctionCall::DisplayMode TestFileParser::parseNewline()
{
	if (accept(SoltToken::Newline, true))
		return FunctionCall::DisplayMode::MultiLine;
	return FunctionCall::DisplayMode::SingleLine;
}

string TestFileParser::parseComment()
{
	string comment = m_scanner.currentLiteral();
	if (accept(SoltToken::Comment, true))
		return comment;
	return string{};
}

string TestFileParser::parseNumber()
{
	string literal = m_scanner.currentLiteral();
	expect(SoltToken::Number);
	return literal;
}

u256 TestFileParser::convertNumber(string const& _literal)
{
	try {
		return u256{_literal};
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Number encoding invalid.");
	}
}

void TestFileParser::Scanner::readStream(istream& _stream)
{
	std::string line;
	while (std::getline(_stream, line))
		m_line += line;
	m_char = m_line.begin();
}

void TestFileParser::Scanner::scanNextToken()
{
	auto detectToken = [](std::string const& _literal = "") -> TokenDesc {
		if (_literal == "ether") return TokenDesc{SoltToken::Ether, _literal};
		if (_literal == "uint256") return TokenDesc{SoltToken::UInt, _literal};
		if (_literal == "FAILURE") return TokenDesc{SoltToken::Failure, _literal};
		return TokenDesc{SoltToken::Identifier, _literal};
	};

	auto selectToken = [this](SoltToken _token, std::string const& _literal = "") -> TokenDesc {
		advance();
		return make_pair(_token, !_literal.empty() ? _literal : formatToken(_token));
	};

	TokenDesc token = make_pair(SoltToken::Unknown, "");
	do
	{
		switch(current())
		{
		case '/':
			advance();
			if (current() == '/')
				token = selectToken(SoltToken::Newline);
			break;
		case '-':
			if (peek() == '>')
			{
				advance();
				token = selectToken(SoltToken::Arrow);
			}
			else
				token = selectToken(SoltToken::Sub);
			break;
		case ':':
			token = selectToken(SoltToken::Colon);
			break;
		case '#':
			token = selectToken(SoltToken::Comment, scanComment());
			break;
		case ',':
			token = selectToken(SoltToken::Comma);
			break;
		case '(':
			token = selectToken(SoltToken::LParen);
			break;
		case ')':
			token = selectToken(SoltToken::RParen);
			break;
		default:
			if (isIdentifierStart(current()))
			{
				TokenDesc detectedToken = detectToken(scanIdentifierOrKeyword());
				token = selectToken(detectedToken.first, detectedToken.second);
			}
			else if (isDecimalDigit(current()))
				token = selectToken(SoltToken::Number, scanNumber());
			else if (isWhiteSpace(current()))
				token = selectToken(SoltToken::Whitespace);
			else if (isEndOfLine())
				token = selectToken(SoltToken::EOS);
			else
				token = selectToken(SoltToken::Invalid);
			break;
		}
	}
	while (token.first == SoltToken::Whitespace);

	m_nextToken = token;
	m_currentToken = token;
}

string TestFileParser::Scanner::scanComment()
{
	string comment;
	advance();

	while (current() != '#')
	{
		comment += current();
		advance();
	}
	return comment;
}

string TestFileParser::Scanner::scanIdentifierOrKeyword()
{
	string identifier;
	identifier += current();
	while (isIdentifierPart(peek()))
	{
		advance();
		identifier += current();
	}
	return identifier;
}

string TestFileParser::Scanner::scanNumber()
{
	string number;
	number += current();
	while (isDecimalDigit(peek()))
	{
		advance();
		number += current();
	}
	return number;
}
