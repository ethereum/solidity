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

#include <test/libsolidity/util/TestFileParser.h>

#include <test/libsolidity/util/BytesUtils.h>
#include <test/libsolidity/util/SoltestErrors.h>
#include <test/Common.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

using Token = soltest::Token;

char TestFileParser::Scanner::peek() const noexcept
{
	if (std::distance(m_char, m_source.end()) < 2)
		return '\0';

	auto next = m_char;
	std::advance(next, 1);
	return *next;
}

vector<solidity::frontend::test::FunctionCall> TestFileParser::parseFunctionCalls(size_t _lineOffset)
{
	vector<FunctionCall> calls;
	if (!accept(Token::EOS))
	{
		soltestAssert(m_scanner.currentToken() == Token::Unknown, "");
		m_scanner.scanNextToken();

		while (!accept(Token::EOS))
		{
			if (!accept(Token::Whitespace))
			{
				/// If this is not the first call in the test,
				/// the last call to parseParameter could have eaten the
				/// new line already. This could only be fixed with a one
				/// token lookahead that checks parseParameter
				/// if the next token is an identifier.
				if (calls.empty())
					expect(Token::Newline);
				else
					if (accept(Token::Newline, true))
						m_lineNumber++;

				try
				{
					if (accept(Token::Gas, true))
					{
						if (calls.empty())
							BOOST_THROW_EXCEPTION(TestParserError("Expected function call before gas usage filter."));

						string runType = m_scanner.currentLiteral();
						if (set<string>{"ir", "irOptimized", "legacy", "legacyOptimized"}.count(runType) > 0)
						{
							m_scanner.scanNextToken();
							expect(Token::Colon);
							if (calls.back().expectations.gasUsed.count(runType) > 0)
								throw TestParserError("Gas usage expectation set multiple times.");
							calls.back().expectations.gasUsed[runType] = u256(parseDecimalNumber());
						}
						else
							BOOST_THROW_EXCEPTION(TestParserError(
								"Expected \"ir\", \"irOptimized\", \"legacy\", or \"legacyOptimized\"."
							));
					}
					else
					{
						FunctionCall call;
						if (accept(Token::Library, true))
						{
							expect(Token::Colon);
							string libraryName;
							if (accept(Token::String))
							{
								libraryName = m_scanner.currentLiteral();
								expect(Token::String);
								expect(Token::Colon);
								libraryName += ':' + m_scanner.currentLiteral();
								expect(Token::Identifier);
							}
							else if (accept(Token::Colon, true))
							{
								libraryName = ':' + m_scanner.currentLiteral();
								expect(Token::Identifier);
							}
							else
							{
								libraryName = m_scanner.currentLiteral();
								expect(Token::Identifier);
							}
							call.signature = libraryName;
							call.kind = FunctionCall::Kind::Library;
							call.expectations.failure = false;
						}
						else
						{
							bool lowLevelCall = false;
							tie(call.signature, lowLevelCall) = parseFunctionSignature();
							if (lowLevelCall)
								call.kind = FunctionCall::Kind::LowLevel;
							else if (isBuiltinFunction(call.signature))
								call.kind = FunctionCall::Kind::Builtin;

							if (accept(Token::Comma, true))
								call.value = parseFunctionCallValue();

							if (accept(Token::Colon, true))
								call.arguments = parseFunctionCallArguments();

							if (accept(Token::Newline, true))
							{
								call.displayMode = FunctionCall::DisplayMode::MultiLine;
								m_lineNumber++;
							}

							call.arguments.comment = parseComment();

							if (accept(Token::Newline, true))
							{
								call.displayMode = FunctionCall::DisplayMode::MultiLine;
								m_lineNumber++;
							}

							if (accept(Token::Arrow, true))
							{
								call.omitsArrow = false;
								call.expectations = parseFunctionCallExpectations();
								if (accept(Token::Newline, true))
									m_lineNumber++;
							}
							else
							{
								call.expectations.failure = false;
								call.displayMode = FunctionCall::DisplayMode::SingleLine;
							}

							call.expectations.comment = parseComment();

							if (call.signature == "constructor()")
								call.kind = FunctionCall::Kind::Constructor;
						}

						accept(Token::Newline, true);
						call.expectedSideEffects = parseFunctionCallSideEffects();

						calls.emplace_back(move(call));
					}
				}
				catch (TestParserError const& _e)
				{
					BOOST_THROW_EXCEPTION(
						TestParserError("Line " + to_string(_lineOffset + m_lineNumber) + ": " + _e.what())
					);
				}
			}
		}
	}
	return calls;
}

vector<string> TestFileParser::parseFunctionCallSideEffects()
{
	vector<string> result;
	while (accept(Token::Tilde, false))
	{
		string effect = m_scanner.currentLiteral();
		result.emplace_back(effect);
		soltestAssert(m_scanner.currentToken() == Token::Tilde, "");
		m_scanner.scanNextToken();
		if (m_scanner.currentToken() == Token::Newline)
			m_scanner.scanNextToken();
	}

	return result;
}

bool TestFileParser::accept(Token _token, bool const _expect)
{
	if (m_scanner.currentToken() != _token)
		return false;
	if (_expect)
		return expect(_token);
	return true;
}

bool TestFileParser::expect(Token _token, bool const _advance)
{
	if (m_scanner.currentToken() != _token || m_scanner.currentToken() == Token::Invalid)
		BOOST_THROW_EXCEPTION(TestParserError(
			"Unexpected " + formatToken(m_scanner.currentToken()) + ": \"" +
			m_scanner.currentLiteral() + "\". " +
			"Expected \"" + formatToken(_token) + "\"."
			)
		);
	if (_advance)
		m_scanner.scanNextToken();
	return true;
}

pair<string, bool> TestFileParser::parseFunctionSignature()
{
	string signature;
	bool hasName = false;

	if (accept(Token::Identifier, false))
	{
		hasName = true;
		signature = m_scanner.currentLiteral();
		expect(Token::Identifier);
	}

	if (isBuiltinFunction(signature) && m_scanner.currentToken() != Token::LParen)
		return {signature, false};

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
		BOOST_THROW_EXCEPTION(TestParserError("Invalid signature detected: " + signature));

	if (!hasName && !parameters.empty())
		BOOST_THROW_EXCEPTION(TestParserError("Signatures without a name cannot have parameters: " + signature));
	else
		signature += parameters;

	expect(Token::RParen);
	signature += formatToken(Token::RParen);

	return {signature, !hasName};
}

FunctionValue TestFileParser::parseFunctionCallValue()
{
	try
	{
		u256 value{ parseDecimalNumber() };
		Token token = m_scanner.currentToken();
		if (token != Token::Ether && token != Token::Wei)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid value unit provided. Coins can be wei or ether."));

		m_scanner.scanNextToken();

		FunctionValueUnit unit = token == Token::Wei ? FunctionValueUnit::Wei : FunctionValueUnit::Ether;
		return { (unit == FunctionValueUnit::Wei ? u256(1) : exp256(u256(10), u256(18))) * value, unit };
	}
	catch (std::exception const&)
	{
		BOOST_THROW_EXCEPTION(TestParserError("Ether value encoding invalid."));
	}
}

FunctionCallArgs TestFileParser::parseFunctionCallArguments()
{
	FunctionCallArgs arguments;

	auto param = parseParameter();
	if (param.abiType.type == ABIType::None)
		BOOST_THROW_EXCEPTION(TestParserError("No argument provided."));
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
	{
		parameter.format.newline = true;
		m_lineNumber++;
	}
	parameter.abiType = ABIType{ABIType::None, ABIType::AlignNone, 0};

	bool isSigned = false;
	if (accept(Token::Left, true))
	{
		parameter.rawString += formatToken(Token::Left);
		expect(Token::LParen);
		parameter.rawString += formatToken(Token::LParen);
		parameter.alignment = Parameter::Alignment::Left;
	}
	if (accept(Token::Right, true))
	{
		parameter.rawString += formatToken(Token::Right);
		expect(Token::LParen);
		parameter.rawString += formatToken(Token::LParen);
		parameter.alignment = Parameter::Alignment::Right;
	}

	if (accept(Token::Sub, true))
	{
		parameter.rawString += formatToken(Token::Sub);
		isSigned = true;
	}
	if (accept(Token::Boolean))
	{
		if (isSigned)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid boolean literal."));

		parameter.abiType = ABIType{ABIType::Boolean, ABIType::AlignRight, 32};
		string parsed = parseBoolean();
		parameter.rawString += parsed;
		parameter.rawBytes = BytesUtils::applyAlign(
			parameter.alignment,
			parameter.abiType,
			BytesUtils::convertBoolean(parsed)
		);
	}
	else if (accept(Token::HexNumber))
	{
		if (isSigned)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid hex number literal."));

		parameter.abiType = ABIType{ABIType::Hex, ABIType::AlignRight, 32};
		string parsed = parseHexNumber();
		parameter.rawString += parsed;
		parameter.rawBytes = BytesUtils::applyAlign(
			parameter.alignment,
			parameter.abiType,
			BytesUtils::convertHexNumber(parsed)
		);
	}
	else if (accept(Token::Hex, true))
	{
		if (isSigned)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid hex string literal."));
		if (parameter.alignment != Parameter::Alignment::None)
			BOOST_THROW_EXCEPTION(TestParserError("Hex string literals cannot be aligned or padded."));

		string parsed = parseString();
		parameter.rawString += "hex\"" + parsed + "\"";
		parameter.rawBytes = BytesUtils::convertHexNumber(parsed);
		parameter.abiType = ABIType{
			ABIType::HexString, ABIType::AlignNone, parameter.rawBytes.size()
		};
	}
	else if (accept(Token::String))
	{
		if (isSigned)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid string literal."));
		if (parameter.alignment != Parameter::Alignment::None)
			BOOST_THROW_EXCEPTION(TestParserError("String literals cannot be aligned or padded."));

		string parsed = parseString();
		parameter.abiType = ABIType{ABIType::String, ABIType::AlignLeft, parsed.size()};
		parameter.rawString += "\"" + parsed + "\"";
		parameter.rawBytes = BytesUtils::applyAlign(
			Parameter::Alignment::Left,
			parameter.abiType,
			BytesUtils::convertString(parsed)
		);
	}
	else if (accept(Token::Number))
	{
		auto type = isSigned ? ABIType::SignedDec : ABIType::UnsignedDec;

		parameter.abiType = ABIType{type, ABIType::AlignRight, 32};
		string parsed = parseDecimalNumber();
		parameter.rawString += parsed;
		if (isSigned)
			parsed = "-" + parsed;

		if (parsed.find('.') == string::npos)
			parameter.rawBytes = BytesUtils::applyAlign(
				parameter.alignment,
				parameter.abiType,
				BytesUtils::convertNumber(parsed)
			);
		else
		{
			parameter.abiType.type = isSigned ? ABIType::SignedFixedPoint : ABIType::UnsignedFixedPoint;
			parameter.rawBytes = BytesUtils::convertFixedPoint(parsed, parameter.abiType.fractionalDigits);
		}
	}
	else if (accept(Token::Failure, true))
	{
		if (isSigned)
			BOOST_THROW_EXCEPTION(TestParserError("Invalid failure literal."));

		parameter.abiType = ABIType{ABIType::Failure, ABIType::AlignRight, 0};
		parameter.rawBytes = bytes{};
	}
	if (parameter.alignment != Parameter::Alignment::None)
	{
		expect(Token::RParen);
		parameter.rawString += formatToken(Token::RParen);
	}

	return parameter;
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

void TestFileParser::Scanner::readStream(istream& _stream)
{
	std::string line;
	// TODO: std::getline(..) removes newlines '\n', if present. This could be improved.
	while (std::getline(_stream, line))
		m_source += line;
	m_char = m_source.begin();
}

void TestFileParser::Scanner::scanNextToken()
{
	// Make code coverage happy.
	soltestAssert(formatToken(Token::NUM_TOKENS).empty(), "");

	auto detectKeyword = [](std::string const& _literal = "") -> std::pair<Token, std::string> {
		if (_literal == "true") return {Token::Boolean, "true"};
		if (_literal == "false") return {Token::Boolean, "false"};
		if (_literal == "ether") return {Token::Ether, ""};
		if (_literal == "wei") return {Token::Wei, ""};
		if (_literal == "left") return {Token::Left, ""};
		if (_literal == "library") return {Token::Library, ""};
		if (_literal == "right") return {Token::Right, ""};
		if (_literal == "hex") return {Token::Hex, ""};
		if (_literal == "FAILURE") return {Token::Failure, ""};
		if (_literal == "gas") return {Token::Gas, ""};
		return {Token::Identifier, _literal};
	};

	auto selectToken = [this](Token _token, std::string const& _literal = "") {
		advance();
		m_currentToken = _token;
		m_currentLiteral = _literal;
	};

	m_currentToken = Token::Unknown;
	m_currentLiteral = "";
	do
	{
		switch(current())
		{
		case '/':
			advance();
			if (current() == '/')
				selectToken(Token::Newline);
			else
				selectToken(Token::Invalid);
			break;
		case '-':
			if (peek() == '>')
			{
				advance();
				selectToken(Token::Arrow);
			}
			else
				selectToken(Token::Sub);
			break;
		case '~':
			advance();
			selectToken(Token::Tilde, readLine());
			break;
		case ':':
			selectToken(Token::Colon);
			break;
		case '#':
			selectToken(Token::Comment, scanComment());
			break;
		case ',':
			selectToken(Token::Comma);
			break;
		case '(':
			selectToken(Token::LParen);
			break;
		case ')':
			selectToken(Token::RParen);
			break;
		case '[':
			selectToken(Token::LBrack);
			break;
		case ']':
			selectToken(Token::RBrack);
			break;
		case '\"':
			selectToken(Token::String, scanString());
			break;
		default:
			if (langutil::isIdentifierStart(current()))
			{
				std::tie(m_currentToken, m_currentLiteral) = detectKeyword(scanIdentifierOrKeyword());
				advance();
			}
			else if (langutil::isDecimalDigit(current()))
			{
				if (current() == '0' && peek() == 'x')
				{
					advance();
					advance();
					selectToken(Token::HexNumber, "0x" + scanHexNumber());
				}
				else
					selectToken(Token::Number, scanDecimalNumber());
			}
			else if (langutil::isWhiteSpace(current()))
				selectToken(Token::Whitespace);
			else if (isEndOfFile())
			{
				m_currentToken = Token::EOS;
				m_currentLiteral = "";
			}
			else
				BOOST_THROW_EXCEPTION(TestParserError("Unexpected character: '" + string{current()} + "'"));
			break;
		}
	}
	while (m_currentToken == Token::Whitespace);
}

string TestFileParser::Scanner::readLine()
{
	string line;
	// Right now the scanner discards all (real) new-lines '\n' in TestFileParser::Scanner::readStream(..).
	// Token::NewLine is defined as `//`, and NOT '\n'. We are just searching here for the next `/`.
	// Note that `/` anywhere else than at the beginning of a line is currently forbidden (TODO: until we fix newline handling).
	// Once the end of the file would be reached (or beyond), peek() will return '\0'.
	while (peek() != '\0' && peek() != '/')
	{
		advance();
		line += current();
	}
	return line;
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
	while (langutil::isDecimalDigit(peek()) || '.' == peek())
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
		if (current() == '\\')
		{
			advance();
			switch (current())
			{
				case '\\':
					str += current();
					advance();
					break;
				case 'n':
					str += '\n';
					advance();
					break;
				case 'r':
					str += '\r';
					advance();
					break;
				case 't':
					str += '\t';
					advance();
					break;
				case '0':
					str += '\0';
					advance();
					break;
				case 'x':
					str += scanHexPart();
					break;
				default:
					BOOST_THROW_EXCEPTION(TestParserError("Invalid or escape sequence found in string literal."));
			}
		}
		else
		{
			str += current();
			advance();
		}
	}
	return str;
}

// TODO: use fromHex() from CommonData
char TestFileParser::Scanner::scanHexPart()
{
	advance(); // skip 'x'

	int value{};
	if (isdigit(current()))
		value = current() - '0';
	else if (tolower(current()) >= 'a' && tolower(current()) <= 'f')
		value = tolower(current()) - 'a' + 10;
	else
		BOOST_THROW_EXCEPTION(TestParserError("\\x used with no following hex digits."));

	advance();
	if (current() == '"')
		return static_cast<char>(value);

	value <<= 4;
	if (isdigit(current()))
		value |= current() - '0';
	else if (tolower(current()) >= 'a' && tolower(current()) <= 'f')
		value |= tolower(current()) - 'a' + 10;

	advance();

	return static_cast<char>(value);
}

bool TestFileParser::isBuiltinFunction(std::string const& _signature)
{
	return m_builtins.count(_signature) > 0;
}
