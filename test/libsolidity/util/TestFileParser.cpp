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
#include <liblangutil/Common.h>
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
using namespace soltest;

namespace
{
	enum class DeclaredAlignment
	{
		Left,
		Right,
		None,
	};

	inline bytes alignLeft(bytes _bytes)
	{
		return std::move(_bytes) + bytes(32 - _bytes.size(), 0);
	}

	inline bytes alignRight(bytes _bytes)
	{
		return bytes(32 - _bytes.size(), 0) + std::move(_bytes);
	}

	inline bytes applyAlign(DeclaredAlignment _alignment, ABIType& _abiType, bytes _converted)
	{
		if (_alignment != DeclaredAlignment::None)
			_abiType.alignDeclared = true;

		switch (_alignment)
		{
		case DeclaredAlignment::Left:
			_abiType.align = ABIType::AlignLeft;
			return alignLeft(std::move(_converted));
		case DeclaredAlignment::Right:
			_abiType.align = ABIType::AlignRight;
			return alignRight(std::move(_converted));
		default:
			_abiType.align = ABIType::AlignRight;
			return alignRight(std::move(_converted));
		}
	}
}

vector<dev::solidity::test::FunctionCall> TestFileParser::parseFunctionCalls()
{
	vector<FunctionCall> calls;
	if (!accept(Token::EOS))
	{
		assert(m_scanner.currentToken() == Token::Unknown);
		m_scanner.scanNextToken();

		while (!accept(Token::EOS))
		{
			if (!accept(Token::Whitespace))
			{
				FunctionCall call;

				/// If this is not the first call in the test,
				/// the last call to parseParameter could have eaten the
				/// new line already. This could only be fixed with a one
				/// token lookahead that checks parseParameter
				/// if the next token is an identifier.
				if (calls.empty())
					expect(Token::Newline);
				else
					accept(Token::Newline, true);

				call.signature = parseFunctionSignature();
				if (accept(Token::Comma, true))
					call.value = parseFunctionCallValue();
				if (accept(Token::Colon, true))
					call.arguments = parseFunctionCallArguments();

				if (accept(Token::Newline, true))
					call.displayMode = FunctionCall::DisplayMode::MultiLine;

				call.arguments.comment = parseComment();

				if (accept(Token::Newline, true))
					call.displayMode = FunctionCall::DisplayMode::MultiLine;

				expect(Token::Arrow);
				call.expectations = parseFunctionCallExpectations();

				accept(Token::Newline, true);
				call.expectations.comment = parseComment();

				calls.emplace_back(std::move(call));
			}
		}
	}
	return calls;
}

bool TestFileParser::accept(soltest::Token _token, bool const _expect)
{
	if (m_scanner.currentToken() != _token)
		return false;
	if (_expect)
		return expect(_token);
	return true;
}

bool TestFileParser::expect(soltest::Token _token, bool const _advance)
{
	if (m_scanner.currentToken() != _token || m_scanner.currentToken() == Token::Invalid)
		throw Error(
			Error::Type::ParserError,
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
	expect(Token::Identifier);

	signature += formatToken(Token::LParen);
	expect(Token::LParen);

	string parameters;
	if (!accept(Token::RParen, false))
		parameters = parseIdentifierOrTuple();

	while (accept(Token::Comma))
	{
		parameters += formatToken(Token::Comma);
		expect(Token::Comma);
		parameters += parseIdentifierOrTuple();
	}
	if (accept(Token::Arrow, true))
		throw Error(Error::Type::ParserError, "Invalid signature detected: " + signature);

	signature += parameters;

	expect(Token::RParen);
	signature += formatToken(Token::RParen);
	return signature;
}

u256 TestFileParser::parseFunctionCallValue()
{
	try
	{
		u256 value{parseDecimalNumber()};
		expect(Token::Ether);
		return value;
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Ether value encoding invalid.");
	}
}

FunctionCallArgs TestFileParser::parseFunctionCallArguments()
{
	FunctionCallArgs arguments;

	auto param = parseParameter();
	if (param.abiType.type == ABIType::None)
		throw Error(Error::Type::ParserError, "No argument provided.");
	arguments.parameters.emplace_back(param);

	while (accept(Token::Comma, true))
		arguments.parameters.emplace_back(parseParameter());
	return arguments;
}

FunctionCallExpectations TestFileParser::parseFunctionCallExpectations()
{
	FunctionCallExpectations expectations;

	auto param = parseParameter();
	if (param.abiType.type == ABIType::None)
	{
		expectations.failure = false;
		return expectations;
	}
	expectations.result.emplace_back(param);

	while (accept(Token::Comma, true))
		expectations.result.emplace_back(parseParameter());

	/// We have always one virtual parameter in the parameter list.
	/// If its type is FAILURE, the expected result is also a REVERT etc.
	if (expectations.result.at(0).abiType.type != ABIType::Failure)
		expectations.failure = false;
	return expectations;
}

Parameter TestFileParser::parseParameter()
{
	Parameter parameter;
	if (accept(Token::Newline, true))
		parameter.format.newline = true;
	auto literal = parseABITypeLiteral();
	parameter.rawBytes = get<0>(literal);
	parameter.abiType = get<1>(literal);
	parameter.rawString = get<2>(literal);
	return parameter;
}

tuple<bytes, ABIType, string> TestFileParser::parseABITypeLiteral()
{
	ABIType abiType{ABIType::None, ABIType::AlignNone, 0};
	DeclaredAlignment alignment{DeclaredAlignment::None};
	bytes result{toBigEndian(u256{0})};
	string rawString;
	bool isSigned = false;

	if (accept(Token::Left, true))
	{
		rawString += formatToken(Token::Left);
		expect(Token::LParen);
		rawString += formatToken(Token::LParen);
		alignment = DeclaredAlignment::Left;
	}
	if (accept(Token::Right, true))
	{
		rawString += formatToken(Token::Right);
		expect(Token::LParen);
		rawString += formatToken(Token::LParen);
		alignment = DeclaredAlignment::Right;
	}

	try
	{
		if (accept(Token::Sub, true))
		{
			rawString += formatToken(Token::Sub);
			isSigned = true;
		}
		if (accept(Token::Boolean))
		{
			if (isSigned)
				throw Error(Error::Type::ParserError, "Invalid boolean literal.");
			abiType = ABIType{ABIType::Boolean, ABIType::AlignRight, 32};
			string parsed = parseBoolean();
			rawString += parsed;
			result = applyAlign(alignment, abiType, convertBoolean(parsed));
		}
		else if (accept(Token::HexNumber))
		{
			if (isSigned)
				throw Error(Error::Type::ParserError, "Invalid hex number literal.");
			abiType = ABIType{ABIType::Hex, ABIType::AlignRight, 32};
			string parsed = parseHexNumber();
			rawString += parsed;
			result = applyAlign(alignment, abiType, convertHexNumber(parsed));
		}
		else if (accept(Token::Hex, true))
		{
			if (isSigned)
				throw Error(Error::Type::ParserError, "Invalid hex string literal.");
			if (alignment != DeclaredAlignment::None)
				throw Error(Error::Type::ParserError, "Hex string literals cannot be aligned or padded.");
			string parsed = parseString();
			rawString += "hex\"" + parsed + "\"";
			result = convertHexNumber(parsed);
			abiType = ABIType{ABIType::HexString, ABIType::AlignNone, result.size()};
		}
		else if (accept(Token::String))
		{
			if (isSigned)
				throw Error(Error::Type::ParserError, "Invalid string literal.");
			if (alignment != DeclaredAlignment::None)
				throw Error(Error::Type::ParserError, "String literals cannot be aligned or padded.");
			abiType = ABIType{ABIType::String, ABIType::AlignLeft, 32};
			string parsed = parseString();
			rawString += "\"" + parsed + "\"";
			result = applyAlign(DeclaredAlignment::Left, abiType, convertString(parsed));
		}
		else if (accept(Token::Number))
		{
			auto type = isSigned ? ABIType::SignedDec : ABIType::UnsignedDec;
			abiType = ABIType{type, ABIType::AlignRight, 32};
			string parsed = parseDecimalNumber();
			rawString += parsed;
			if (isSigned)
				parsed = "-" + parsed;
			result = applyAlign(alignment, abiType, convertNumber(parsed));
		}
		else if (accept(Token::Failure, true))
		{
			if (isSigned)
				throw Error(Error::Type::ParserError, "Invalid failure literal.");
			abiType = ABIType{ABIType::Failure, ABIType::AlignRight, 0};
			result = bytes{};
		}
		if (alignment != DeclaredAlignment::None)
		{
			expect(Token::RParen);
			rawString += formatToken(Token::RParen);
		}
		return make_tuple(result, abiType, rawString);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Literal encoding invalid.");
	}
}

string TestFileParser::parseIdentifierOrTuple()
{
	string identOrTuple;

	auto parseArrayDimensions = [&]()
	{
		while (accept(Token::LBrack))
		{
			identOrTuple += formatToken(Token::LBrack);
			expect(Token::LBrack);
			if (accept(Token::Number))
				identOrTuple += parseDecimalNumber();
			identOrTuple += formatToken(Token::RBrack);
			expect(Token::RBrack);
		}
	};

	if (accept(Token::Identifier))
	{
		identOrTuple = m_scanner.currentLiteral();
		expect(Token::Identifier);
		parseArrayDimensions();
		return identOrTuple;
	}
	expect(Token::LParen);
	identOrTuple += formatToken(Token::LParen);
	identOrTuple += parseIdentifierOrTuple();

	while (accept(Token::Comma))
	{
		identOrTuple += formatToken(Token::Comma);
		expect(Token::Comma);
		identOrTuple += parseIdentifierOrTuple();
	}
	expect(Token::RParen);
	identOrTuple += formatToken(Token::RParen);

	parseArrayDimensions();
	return identOrTuple;
}

string TestFileParser::parseBoolean()
{
	string literal = m_scanner.currentLiteral();
	expect(Token::Boolean);
	return literal;
}

string TestFileParser::parseComment()
{
	string comment = m_scanner.currentLiteral();
	if (accept(Token::Comment, true))
		return comment;
	return string{};
}

string TestFileParser::parseDecimalNumber()
{
	string literal = m_scanner.currentLiteral();
	expect(Token::Number);
	return literal;
}

string TestFileParser::parseHexNumber()
{
	string literal = m_scanner.currentLiteral();
	expect(Token::HexNumber);
	return literal;
}

string TestFileParser::parseString()
{
	string literal = m_scanner.currentLiteral();
	expect(Token::String);
	return literal;
}

bytes TestFileParser::convertBoolean(string const& _literal)
{
	if (_literal == "true")
		return bytes{true};
	else if (_literal == "false")
		return bytes{false};
	else
		throw Error(Error::Type::ParserError, "Boolean literal invalid.");
}

bytes TestFileParser::convertNumber(string const& _literal)
{
	try
	{
		return toCompactBigEndian(u256{_literal});
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Number encoding invalid.");
	}
}

bytes TestFileParser::convertHexNumber(string const& _literal)
{
	try
	{
		if (_literal.size() % 2)
			throw Error(Error::Type::ParserError, "Hex number encoding invalid.");
		else
			return fromHex(_literal);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "Hex number encoding invalid.");
	}
}

bytes TestFileParser::convertString(string const& _literal)
{
	try
	{
		return asBytes(_literal);
	}
	catch (std::exception const&)
	{
		throw Error(Error::Type::ParserError, "String encoding invalid.");
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
	using namespace langutil;

	// Make code coverage happy.
	assert(formatToken(Token::NUM_TOKENS) == "");

	auto detectKeyword = [](std::string const& _literal = "") -> TokenDesc {
		if (_literal == "true") return TokenDesc{Token::Boolean, _literal};
		if (_literal == "false") return TokenDesc{Token::Boolean, _literal};
		if (_literal == "ether") return TokenDesc{Token::Ether, _literal};
		if (_literal == "left") return TokenDesc{Token::Left, _literal};
		if (_literal == "right") return TokenDesc{Token::Right, _literal};
		if (_literal == "hex") return TokenDesc{Token::Hex, _literal};
		if (_literal == "FAILURE") return TokenDesc{Token::Failure, _literal};
		return TokenDesc{Token::Identifier, _literal};
	};

	auto selectToken = [this](Token _token, std::string const& _literal = "") -> TokenDesc {
		advance();
		return make_pair(_token, !_literal.empty() ? _literal : formatToken(_token));
	};

	TokenDesc token = make_pair(Token::Unknown, "");
	do
	{
		switch(current())
		{
		case '/':
			advance();
			if (current() == '/')
				token = selectToken(Token::Newline);
			else
				token = selectToken(Token::Invalid);
			break;
		case '-':
			if (peek() == '>')
			{
				advance();
				token = selectToken(Token::Arrow);
			}
			else
				token = selectToken(Token::Sub);
			break;
		case ':':
			token = selectToken(Token::Colon);
			break;
		case '#':
			token = selectToken(Token::Comment, scanComment());
			break;
		case ',':
			token = selectToken(Token::Comma);
			break;
		case '(':
			token = selectToken(Token::LParen);
			break;
		case ')':
			token = selectToken(Token::RParen);
			break;
		case '[':
			token = selectToken(Token::LBrack);
			break;
		case ']':
			token = selectToken(Token::RBrack);
			break;
		case '\"':
			token = selectToken(Token::String, scanString());
			break;
		default:
			if (isIdentifierStart(current()))
			{
				TokenDesc detectedToken = detectKeyword(scanIdentifierOrKeyword());
				token = selectToken(detectedToken.first, detectedToken.second);
			}
			else if (isDecimalDigit(current()))
			{
				if (current() == '0' && peek() == 'x')
				{
					advance();
					advance();
					token = selectToken(Token::HexNumber, "0x" + scanHexNumber());
				}
				else
					token = selectToken(Token::Number, scanDecimalNumber());
			}
			else if (isWhiteSpace(current()))
				token = selectToken(Token::Whitespace);
			else if (isEndOfLine())
				token = selectToken(Token::EOS);
			else
				throw Error(
					Error::Type::ParserError,
					"Unexpected character: '" + string{current()} + "'"
				);
			break;
		}
	}
	while (token.first == Token::Whitespace);
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
	while (langutil::isIdentifierPart(peek()))
	{
		advance();
		identifier += current();
	}
	return identifier;
}

string TestFileParser::Scanner::scanDecimalNumber()
{
	string number;
	number += current();
	while (langutil::isDecimalDigit(peek()))
	{
		advance();
		number += current();
	}
	return number;
}

string TestFileParser::Scanner::scanHexNumber()
{
	string number;
	number += current();
	while (langutil::isHexDigit(peek()))
	{
		advance();
		number += current();
	}
	return number;
}

string TestFileParser::Scanner::scanString()
{
	string str;
	advance();

	while (current() != '\"')
	{
		str += current();
		advance();
	}
	return str;
}
