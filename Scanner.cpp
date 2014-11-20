/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.

	This file is derived from the file "scanner.cc", which was part of the
	V8 project. The original copyright header follows:

	Copyright 2006-2012, the V8 project authors. All rights reserved.
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above
	  copyright notice, this list of conditions and the following
	  disclaimer in the documentation and/or other materials provided
	  with the distribution.
	* Neither the name of Google Inc. nor the names of its
	  contributors may be used to endorse or promote products derived
	  from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity scanner.
 */

#include <algorithm>
#include <tuple>
#include <libsolidity/Scanner.h>

using namespace std;

namespace dev
{
namespace solidity
{

namespace
{
bool isDecimalDigit(char c)
{
	return '0' <= c && c <= '9';
}
bool isHexDigit(char c)
{
	return isDecimalDigit(c)
		   || ('a' <= c && c <= 'f')
		   || ('A' <= c && c <= 'F');
}
bool isLineTerminator(char c)
{
	return c == '\n';
}
bool isWhiteSpace(char c)
{
	return c == ' ' || c == '\n' || c == '\t';
}
bool isIdentifierStart(char c)
{
	return c == '_' || c == '$' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}
bool isIdentifierPart(char c)
{
	return isIdentifierStart(c) || isDecimalDigit(c);
}

int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else return -1;
}
} // end anonymous namespace

void Scanner::reset(CharStream const& _source)
{
	bool foundDocComment;
	m_source = _source;
	m_char = m_source.get();
	skipWhitespace();
	foundDocComment = scanToken();

	// special version of Scanner:next() taking the previous scanToken() result into account
	m_currentToken = m_nextToken;
	if (scanToken() || foundDocComment)
		m_skippedComment = m_nextSkippedComment;
}


bool Scanner::scanHexByte(char& o_scannedByte)
{
	char x = 0;
	for (int i = 0; i < 2; i++)
	{
		int d = hexValue(m_char);
		if (d < 0)
		{
			rollback(i);
			return false;
		}
		x = x * 16 + d;
		advance();
	}
	o_scannedByte = x;
	return true;
}


// Ensure that tokens can be stored in a byte.
BOOST_STATIC_ASSERT(Token::NUM_TOKENS <= 0x100);

Token::Value Scanner::next()
{
	m_currentToken = m_nextToken;
	if (scanToken())
		m_skippedComment = m_nextSkippedComment;
	return m_currentToken.token;
}

Token::Value Scanner::selectToken(char _next, Token::Value _then, Token::Value _else)
{
	advance();
	if (m_char == _next)
		return selectToken(_then);
	else
		return _else;
}


bool Scanner::skipWhitespace()
{
	int const startPosition = getSourcePos();
	while (isWhiteSpace(m_char))
		advance();
	// Return whether or not we skipped any characters.
	return getSourcePos() != startPosition;
}


Token::Value Scanner::skipSingleLineComment()
{
	// The line terminator at the end of the line is not considered
	// to be part of the single-line comment; it is recognized
	// separately by the lexical grammar and becomes part of the
	// stream of input elements for the syntactic grammar
	while (advance() && !isLineTerminator(m_char)) { };
	return Token::WHITESPACE;
}

/// For the moment this function simply consumes a single line triple slash doc comment
Token::Value Scanner::scanDocumentationComment()
{
	LiteralScope literal(this);
	advance(); //consume the last '/'
	while (!isSourcePastEndOfInput() && !isLineTerminator(m_char))
	{
		addCommentLiteralChar(m_char);
		advance();
	}
	literal.complete();
	return Token::COMMENT_LITERAL;
}

Token::Value Scanner::skipMultiLineComment()
{
	if (asserts(m_char == '*'))
		BOOST_THROW_EXCEPTION(InternalCompilerError());
	advance();
	while (!isSourcePastEndOfInput())
	{
		char ch = m_char;
		advance();

		// If we have reached the end of the multi-line comment, we
		// consume the '/' and insert a whitespace. This way all
		// multi-line comments are treated as whitespace.
		if (ch == '*' && m_char == '/')
		{
			m_char = ' ';
			return Token::WHITESPACE;
		}
	}
	// Unterminated multi-line comment.
	return Token::ILLEGAL;
}

bool Scanner::scanToken()
{
	bool foundDocComment = false;
	m_nextToken.literal.clear();
	Token::Value token;
	do
	{
		// Remember the position of the next token
		m_nextToken.location.start = getSourcePos();
		switch (m_char)
		{
		case '\n': // fall-through
		case ' ':
		case '\t':
			token = selectToken(Token::WHITESPACE);
			break;
		case '"':
		case '\'':
			token = scanString();
			break;
		case '<':
			// < <= << <<=
			advance();
			if (m_char == '=')
				token = selectToken(Token::LTE);
			else if (m_char == '<')
				token = selectToken('=', Token::ASSIGN_SHL, Token::SHL);
			else
				token = Token::LT;
			break;
		case '>':
			// > >= >> >>= >>> >>>=
			advance();
			if (m_char == '=')
				token = selectToken(Token::GTE);
			else if (m_char == '>')
			{
				// >> >>= >>> >>>=
				advance();
				if (m_char == '=')
					token = selectToken(Token::ASSIGN_SAR);
				else if (m_char == '>')
					token = selectToken('=', Token::ASSIGN_SHR, Token::SHR);
				else
					token = Token::SAR;
			}
			else
				token = Token::GT;
			break;
		case '=':
			// = == =>
			advance();
			if (m_char == '=')
				token = selectToken(Token::EQ);
			else if (m_char == '>')
				token = selectToken(Token::ARROW);
			else
				token = Token::ASSIGN;
			break;
		case '!':
			// ! !=
			advance();
			if (m_char == '=')
				token = selectToken(Token::NE);
			else
				token = Token::NOT;
			break;
		case '+':
			// + ++ +=
			advance();
			if (m_char == '+')
				token = selectToken(Token::INC);
			else if (m_char == '=')
				token = selectToken(Token::ASSIGN_ADD);
			else
				token = Token::ADD;
			break;
		case '-':
			// - -- -= Number
			advance();
			if (m_char == '-')
			{
				advance();
				token = Token::DEC;
			}
			else if (m_char == '=')
				token = selectToken(Token::ASSIGN_SUB);
			else if (m_char == '.' || isDecimalDigit(m_char))
				token = scanNumber('-');
			else
				token = Token::SUB;
			break;
		case '*':
			// * *=
			token = selectToken('=', Token::ASSIGN_MUL, Token::MUL);
			break;
		case '%':
			// % %=
			token = selectToken('=', Token::ASSIGN_MOD, Token::MOD);
			break;
		case '/':
			// /  // /* /=
			advance();
			if (m_char == '/')
			{
				if (!advance()) /* double slash comment directly before EOS */
					token = Token::WHITESPACE;
				else if (m_char == '/')
				{
					Token::Value comment;
					m_nextSkippedComment.location.start = getSourcePos();
					comment = scanDocumentationComment();
					m_nextSkippedComment.location.end = getSourcePos();
					m_nextSkippedComment.token = comment;
					token = Token::WHITESPACE;
					foundDocComment = true;
				}
				else
					token = skipSingleLineComment();
			}
			else if (m_char == '*')
				token = skipMultiLineComment();
			else if (m_char == '=')
				token = selectToken(Token::ASSIGN_DIV);
			else
				token = Token::DIV;
			break;
		case '&':
			// & && &=
			advance();
			if (m_char == '&')
				token = selectToken(Token::AND);
			else if (m_char == '=')
				token = selectToken(Token::ASSIGN_BIT_AND);
			else
				token = Token::BIT_AND;
			break;
		case '|':
			// | || |=
			advance();
			if (m_char == '|')
				token = selectToken(Token::OR);
			else if (m_char == '=')
				token = selectToken(Token::ASSIGN_BIT_OR);
			else
				token = Token::BIT_OR;
			break;
		case '^':
			// ^ ^=
			token = selectToken('=', Token::ASSIGN_BIT_XOR, Token::BIT_XOR);
			break;
		case '.':
			// . Number
			advance();
			if (isDecimalDigit(m_char))
				token = scanNumber('.');
			else
				token = Token::PERIOD;
			break;
		case ':':
			token = selectToken(Token::COLON);
			break;
		case ';':
			token = selectToken(Token::SEMICOLON);
			break;
		case ',':
			token = selectToken(Token::COMMA);
			break;
		case '(':
			token = selectToken(Token::LPAREN);
			break;
		case ')':
			token = selectToken(Token::RPAREN);
			break;
		case '[':
			token = selectToken(Token::LBRACK);
			break;
		case ']':
			token = selectToken(Token::RBRACK);
			break;
		case '{':
			token = selectToken(Token::LBRACE);
			break;
		case '}':
			token = selectToken(Token::RBRACE);
			break;
		case '?':
			token = selectToken(Token::CONDITIONAL);
			break;
		case '~':
			token = selectToken(Token::BIT_NOT);
			break;
		default:
			if (isIdentifierStart(m_char))
				token = scanIdentifierOrKeyword();
			else if (isDecimalDigit(m_char))
				token = scanNumber();
			else if (skipWhitespace())
				token = Token::WHITESPACE;
			else if (isSourcePastEndOfInput())
				token = Token::EOS;
			else
				token = selectToken(Token::ILLEGAL);
			break;
		}
		// Continue scanning for tokens as long as we're just skipping
		// whitespace.
	}
	while (token == Token::WHITESPACE);
	m_nextToken.location.end = getSourcePos();
	m_nextToken.token = token;

	return foundDocComment;
}

bool Scanner::scanEscape()
{
	char c = m_char;
	advance();
	// Skip escaped newlines.
	if (isLineTerminator(c))
		return true;
	switch (c)
	{
	case '\'':  // fall through
	case '"':  // fall through
	case '\\':
		break;
	case 'b':
		c = '\b';
		break;
	case 'f':
		c = '\f';
		break;
	case 'n':
		c = '\n';
		break;
	case 'r':
		c = '\r';
		break;
	case 't':
		c = '\t';
		break;
	case 'v':
		c = '\v';
		break;
	case 'x':
		if (!scanHexByte(c))
			return false;
		break;
	}

	addLiteralChar(c);
	return true;
}

Token::Value Scanner::scanString()
{
	char const quote = m_char;
	advance();  // consume quote
	LiteralScope literal(this);
	while (m_char != quote && !isSourcePastEndOfInput() && !isLineTerminator(m_char))
	{
		char c = m_char;
		advance();
		if (c == '\\')
		{
			if (isSourcePastEndOfInput() || !scanEscape())
				return Token::ILLEGAL;
		}
		else
			addLiteralChar(c);
	}
	if (m_char != quote)
		return Token::ILLEGAL;
	literal.complete();
	advance();  // consume quote
	return Token::STRING_LITERAL;
}


void Scanner::scanDecimalDigits()
{
	while (isDecimalDigit(m_char))
		addLiteralCharAndAdvance();
}


Token::Value Scanner::scanNumber(char _charSeen)
{
	enum { DECIMAL, HEX, BINARY } kind = DECIMAL;
	LiteralScope literal(this);
	if (_charSeen == '.')
	{
		// we have already seen a decimal point of the float
		addLiteralChar('.');
		scanDecimalDigits();  // we know we have at least one digit
	}
	else
	{
		if (_charSeen == '-')
			addLiteralChar('-');
		// if the first character is '0' we must check for octals and hex
		if (m_char == '0')
		{
			addLiteralCharAndAdvance();
			// either 0, 0exxx, 0Exxx, 0.xxx or a hex number
			if (m_char == 'x' || m_char == 'X')
			{
				// hex number
				kind = HEX;
				addLiteralCharAndAdvance();
				if (!isHexDigit(m_char))
					return Token::ILLEGAL; // we must have at least one hex digit after 'x'/'X'
				while (isHexDigit(m_char))
					addLiteralCharAndAdvance();
			}
		}
		// Parse decimal digits and allow trailing fractional part.
		if (kind == DECIMAL)
		{
			scanDecimalDigits();  // optional
			if (m_char == '.')
			{
				addLiteralCharAndAdvance();
				scanDecimalDigits();  // optional
			}
		}
	}
	// scan exponent, if any
	if (m_char == 'e' || m_char == 'E')
	{
		if (asserts(kind != HEX)) // 'e'/'E' must be scanned as part of the hex number
			BOOST_THROW_EXCEPTION(InternalCompilerError());
		if (kind != DECIMAL)
			return Token::ILLEGAL;
		// scan exponent
		addLiteralCharAndAdvance();
		if (m_char == '+' || m_char == '-')
			addLiteralCharAndAdvance();
		if (!isDecimalDigit(m_char))
			return Token::ILLEGAL; // we must have at least one decimal digit after 'e'/'E'
		scanDecimalDigits();
	}
	// The source character immediately following a numeric literal must
	// not be an identifier start or a decimal digit; see ECMA-262
	// section 7.8.3, page 17 (note that we read only one decimal digit
	// if the value is 0).
	if (isDecimalDigit(m_char) || isIdentifierStart(m_char))
		return Token::ILLEGAL;
	literal.complete();
	return Token::NUMBER;
}


// ----------------------------------------------------------------------------
// Keyword Matcher

#define KEYWORDS(KEYWORD_GROUP, KEYWORD)                                       \
	KEYWORD_GROUP('a')                                                         \
	KEYWORD("address", Token::ADDRESS)                                         \
	KEYWORD_GROUP('b')                                                         \
	KEYWORD("break", Token::BREAK)                                             \
	KEYWORD("bool", Token::BOOL)                                               \
	KEYWORD_GROUP('c')                                                         \
	KEYWORD("case", Token::CASE)                                               \
	KEYWORD("const", Token::CONST)                                             \
	KEYWORD("continue", Token::CONTINUE)                                       \
	KEYWORD("contract", Token::CONTRACT)                                       \
	KEYWORD_GROUP('d')                                                         \
	KEYWORD("default", Token::DEFAULT)                                         \
	KEYWORD("delete", Token::DELETE)                                           \
	KEYWORD("do", Token::DO)                                                   \
	KEYWORD_GROUP('e')                                                         \
	KEYWORD("else", Token::ELSE)                                               \
	KEYWORD("extends", Token::EXTENDS)                                         \
	KEYWORD_GROUP('f')                                                         \
	KEYWORD("false", Token::FALSE_LITERAL)                                     \
	KEYWORD("for", Token::FOR)                                                 \
	KEYWORD("function", Token::FUNCTION)                                       \
	KEYWORD_GROUP('h')                                                         \
	KEYWORD("hash", Token::HASH)                                               \
	KEYWORD("hash8", Token::HASH8)                                             \
	KEYWORD("hash16", Token::HASH16)                                           \
	KEYWORD("hash24", Token::HASH24)                                           \
	KEYWORD("hash32", Token::HASH32)                                           \
	KEYWORD("hash40", Token::HASH40)                                           \
	KEYWORD("hash48", Token::HASH48)                                           \
	KEYWORD("hash56", Token::HASH56)                                           \
	KEYWORD("hash64", Token::HASH64)                                           \
	KEYWORD("hash72", Token::HASH72)                                           \
	KEYWORD("hash80", Token::HASH80)                                           \
	KEYWORD("hash88", Token::HASH88)                                           \
	KEYWORD("hash96", Token::HASH96)                                           \
	KEYWORD("hash104", Token::HASH104)                                         \
	KEYWORD("hash112", Token::HASH112)                                         \
	KEYWORD("hash120", Token::HASH120)                                         \
	KEYWORD("hash128", Token::HASH128)                                         \
	KEYWORD("hash136", Token::HASH136)                                         \
	KEYWORD("hash144", Token::HASH144)                                         \
	KEYWORD("hash152", Token::HASH152)                                         \
	KEYWORD("hash160", Token::HASH160)                                         \
	KEYWORD("hash168", Token::HASH168)                                         \
	KEYWORD("hash178", Token::HASH176)                                         \
	KEYWORD("hash184", Token::HASH184)                                         \
	KEYWORD("hash192", Token::HASH192)                                         \
	KEYWORD("hash200", Token::HASH200)                                         \
	KEYWORD("hash208", Token::HASH208)                                         \
	KEYWORD("hash216", Token::HASH216)                                         \
	KEYWORD("hash224", Token::HASH224)                                         \
	KEYWORD("hash232", Token::HASH232)                                         \
	KEYWORD("hash240", Token::HASH240)                                         \
	KEYWORD("hash248", Token::HASH248)                                         \
	KEYWORD("hash256", Token::HASH256)                                         \
	KEYWORD_GROUP('i')                                                         \
	KEYWORD("if", Token::IF)                                                   \
	KEYWORD("in", Token::IN)                                                   \
	KEYWORD("int", Token::INT)                                                 \
	KEYWORD("int8", Token::INT8)                                               \
	KEYWORD("int16", Token::INT16)                                             \
	KEYWORD("int24", Token::INT24)                                             \
	KEYWORD("int32", Token::INT32)                                             \
	KEYWORD("int40", Token::INT40)                                             \
	KEYWORD("int48", Token::INT48)                                             \
	KEYWORD("int56", Token::INT56)                                             \
	KEYWORD("int64", Token::INT64)                                             \
	KEYWORD("int72", Token::INT72)                                             \
	KEYWORD("int80", Token::INT80)                                             \
	KEYWORD("int88", Token::INT88)                                             \
	KEYWORD("int96", Token::INT96)                                             \
	KEYWORD("int104", Token::INT104)                                           \
	KEYWORD("int112", Token::INT112)                                           \
	KEYWORD("int120", Token::INT120)                                           \
	KEYWORD("int128", Token::INT128)                                           \
	KEYWORD("int136", Token::INT136)                                           \
	KEYWORD("int144", Token::INT144)                                           \
	KEYWORD("int152", Token::INT152)                                           \
	KEYWORD("int160", Token::INT160)                                           \
	KEYWORD("int168", Token::INT168)                                           \
	KEYWORD("int178", Token::INT176)                                           \
	KEYWORD("int184", Token::INT184)                                           \
	KEYWORD("int192", Token::INT192)                                           \
	KEYWORD("int200", Token::INT200)                                           \
	KEYWORD("int208", Token::INT208)                                           \
	KEYWORD("int216", Token::INT216)                                           \
	KEYWORD("int224", Token::INT224)                                           \
	KEYWORD("int232", Token::INT232)                                           \
	KEYWORD("int240", Token::INT240)                                           \
	KEYWORD("int248", Token::INT248)                                           \
	KEYWORD("int256", Token::INT256)                                           \
	KEYWORD_GROUP('l')                                                         \
	KEYWORD_GROUP('m')                                                         \
	KEYWORD("mapping", Token::MAPPING)                                         \
	KEYWORD_GROUP('n')                                                         \
	KEYWORD("new", Token::NEW)                                                 \
	KEYWORD("null", Token::NULL_LITERAL)                                       \
	KEYWORD_GROUP('p')                                                         \
	KEYWORD("private", Token::PRIVATE)                                         \
	KEYWORD("public", Token::PUBLIC)                                           \
	KEYWORD_GROUP('r')                                                         \
	KEYWORD("real", Token::REAL)                                               \
	KEYWORD("return", Token::RETURN)                                           \
	KEYWORD("returns", Token::RETURNS)                                         \
	KEYWORD_GROUP('s')                                                         \
	KEYWORD("string", Token::STRING_TYPE)                                      \
	KEYWORD("struct", Token::STRUCT)                                           \
	KEYWORD("switch", Token::SWITCH)                                           \
	KEYWORD_GROUP('t')                                                         \
	KEYWORD("text", Token::TEXT)                                               \
	KEYWORD("true", Token::TRUE_LITERAL)                                       \
	KEYWORD_GROUP('u')                                                         \
	KEYWORD("uint", Token::UINT)                                               \
	KEYWORD("uint8", Token::UINT8)                                             \
	KEYWORD("uint16", Token::UINT16)                                           \
	KEYWORD("uint24", Token::UINT24)                                           \
	KEYWORD("uint32", Token::UINT32)                                           \
	KEYWORD("uint40", Token::UINT40)                                           \
	KEYWORD("uint48", Token::UINT48)                                           \
	KEYWORD("uint56", Token::UINT56)                                           \
	KEYWORD("uint64", Token::UINT64)                                           \
	KEYWORD("uint72", Token::UINT72)                                           \
	KEYWORD("uint80", Token::UINT80)                                           \
	KEYWORD("uint88", Token::UINT88)                                           \
	KEYWORD("uint96", Token::UINT96)                                           \
	KEYWORD("uint104", Token::UINT104)                                         \
	KEYWORD("uint112", Token::UINT112)                                         \
	KEYWORD("uint120", Token::UINT120)                                         \
	KEYWORD("uint128", Token::UINT128)                                         \
	KEYWORD("uint136", Token::UINT136)                                         \
	KEYWORD("uint144", Token::UINT144)                                         \
	KEYWORD("uint152", Token::UINT152)                                         \
	KEYWORD("uint160", Token::UINT160)                                         \
	KEYWORD("uint168", Token::UINT168)                                         \
	KEYWORD("uint178", Token::UINT176)                                         \
	KEYWORD("uint184", Token::UINT184)                                         \
	KEYWORD("uint192", Token::UINT192)                                         \
	KEYWORD("uint200", Token::UINT200)                                         \
	KEYWORD("uint208", Token::UINT208)                                         \
	KEYWORD("uint216", Token::UINT216)                                         \
	KEYWORD("uint224", Token::UINT224)                                         \
	KEYWORD("uint232", Token::UINT232)                                         \
	KEYWORD("uint240", Token::UINT240)                                         \
	KEYWORD("uint248", Token::UINT248)                                         \
	KEYWORD("uint256", Token::UINT256)                                         \
	KEYWORD("ureal", Token::UREAL)                                             \
	KEYWORD_GROUP('v')                                                         \
	KEYWORD("var", Token::VAR)                                                 \
	KEYWORD_GROUP('w')                                                         \
	KEYWORD("while", Token::WHILE)                                             \


static Token::Value KeywordOrIdentifierToken(string const& _input)
{
	if (asserts(!_input.empty()))
		BOOST_THROW_EXCEPTION(InternalCompilerError());
	int const kMinLength = 2;
	int const kMaxLength = 10;
	if (_input.size() < kMinLength || _input.size() > kMaxLength)
		return Token::IDENTIFIER;
	switch (_input[0])
	{
	default:
#define KEYWORD_GROUP_CASE(ch)					\
		break;									\
	case ch:
#define KEYWORD(keyword, token)                                         \
		{                                                               \
			/* 'keyword' is a char array, so sizeof(keyword) is */      \
			/* strlen(keyword) plus 1 for the NUL char. */              \
			int const keywordLength = sizeof(keyword) - 1;              \
			BOOST_STATIC_ASSERT(keywordLength >= kMinLength);           \
			BOOST_STATIC_ASSERT(keywordLength <= kMaxLength);           \
			if (_input == keyword)                                     \
				return token;                                           \
	}
		KEYWORDS(KEYWORD_GROUP_CASE, KEYWORD)
	}
	return Token::IDENTIFIER;
}

Token::Value Scanner::scanIdentifierOrKeyword()
{
	if (asserts(isIdentifierStart(m_char)))
		BOOST_THROW_EXCEPTION(InternalCompilerError());
	LiteralScope literal(this);
	addLiteralCharAndAdvance();
	// Scan the rest of the identifier characters.
	while (isIdentifierPart(m_char))
		addLiteralCharAndAdvance();
	literal.complete();
	return KeywordOrIdentifierToken(m_nextToken.literal);
}

char CharStream::advanceAndGet()
{
	if (isPastEndOfInput())
		return 0;
	++m_pos;
	if (isPastEndOfInput())
		return 0;
	return get();
}

char CharStream::rollback(size_t _amount)
{
	if (asserts(m_pos >= _amount))
		BOOST_THROW_EXCEPTION(InternalCompilerError());
	m_pos -= _amount;
	return get();
}

string CharStream::getLineAtPosition(int _position) const
{
	// if _position points to \n, it returns the line before the \n
	using size_type = string::size_type;
	size_type searchStart = min<size_type>(m_source.size(), _position);
	if (searchStart > 0)
		searchStart--;
	size_type lineStart = m_source.rfind('\n', searchStart);
	if (lineStart == string::npos)
		lineStart = 0;
	else
		lineStart++;
	return m_source.substr(lineStart, min(m_source.find('\n', lineStart),
										  m_source.size()) - lineStart);
}

tuple<int, int> CharStream::translatePositionToLineColumn(int _position) const
{
	using size_type = string::size_type;
	size_type searchPosition = min<size_type>(m_source.size(), _position);
	int lineNumber = count(m_source.begin(), m_source.begin() + searchPosition, '\n');
	size_type lineStart;
	if (searchPosition == 0)
		lineStart = 0;
	else
	{
		lineStart = m_source.rfind('\n', searchPosition - 1);
		lineStart = lineStart == string::npos ? 0 : lineStart + 1;
	}
	return tuple<int, int>(lineNumber, searchPosition - lineStart);
}


}
}
