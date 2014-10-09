// Copyright 2006-2012, the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//    * Neither the name of Google Inc. nor the names of its
//      contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Modifications as part of cpp-ethereum under the following license:
//
// cpp-ethereum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cpp-ethereum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.

#include <algorithm>
#include <tuple>

#include <libsolidity/Scanner.h>

namespace dev {
namespace solidity {

namespace {
	bool IsDecimalDigit(char c) {
		return '0' <= c && c <= '9';
	}
	bool IsHexDigit(char c) {
		return IsDecimalDigit(c)
				|| ('a' <= c && c <= 'f')
				|| ('A' <= c && c <= 'F');
	}
	bool IsLineTerminator(char c) { return c == '\n'; }
	bool IsWhiteSpace(char c) {
		return c == ' ' || c == '\n' || c == '\t';
	}
	bool IsIdentifierStart(char c) {
		return c == '_' || c == '$' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
	}
	bool IsIdentifierPart(char c) {
		return IsIdentifierStart(c) || IsDecimalDigit(c);
	}

	int HexValue(char c) {
		if (c >= '0' && c <= '9') return c - '0';
		else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		else return -1;
	}
}

Scanner::Scanner(const CharStream& _source)
{
	reset(_source);
}

void Scanner::reset(const CharStream& _source)
{
	m_source = _source;

	m_char = m_source.get();
	skipWhitespace();
	scanToken();
	next();
}


bool Scanner::scanHexNumber(char& scanned_number, int expected_length)
{
	BOOST_ASSERT(expected_length <= 4);  // prevent overflow

	char x = 0;
	for (int i = 0; i < expected_length; i++) {
		int d = HexValue(m_char);
		if (d < 0) {
			rollback(i);
			return false;
		}
		x = x * 16 + d;
		advance();
	}

	scanned_number = x;
	return true;
}


// Ensure that tokens can be stored in a byte.
BOOST_STATIC_ASSERT(Token::NUM_TOKENS <= 0x100);

Token::Value Scanner::next()
{
	m_current_token = m_next_token;
	m_hasLineTerminatorBeforeNext = false;
	m_hasMultilineCommentBeforeNext = false;
	scanToken();
	return m_current_token.token;
}


bool Scanner::skipWhitespace()
{
	const int start_position = getSourcePos();

	while (true) {
	  if (IsLineTerminator(m_char)) {
		m_hasLineTerminatorBeforeNext = true;
	  } else if (!IsWhiteSpace(m_char)) {
		break;
	  }
	  advance();
	}

	// Return whether or not we skipped any characters.
	return getSourcePos() != start_position;
}


Token::Value Scanner::skipSingleLineComment()
{
  // The line terminator at the end of the line is not considered
  // to be part of the single-line comment; it is recognized
  // separately by the lexical grammar and becomes part of the
  // stream of input elements for the syntactic grammar
  while (advance() && !IsLineTerminator(m_char)) { };

  return Token::WHITESPACE;
}

Token::Value Scanner::skipMultiLineComment()
{
	BOOST_ASSERT(m_char == '*');
	advance();

	while (!isSourcePastEndOfInput()) {
		char ch = m_char;
		advance();
		if (IsLineTerminator(ch)) {
			// Following ECMA-262, section 7.4, a comment containing
			// a newline will make the comment count as a line-terminator.
			m_hasMultilineCommentBeforeNext = true;
		}
		// If we have reached the end of the multi-line comment, we
		// consume the '/' and insert a whitespace. This way all
		// multi-line comments are treated as whitespace.
		if (ch == '*' && m_char == '/') {
			m_char = ' ';
			return Token::WHITESPACE;
		}
	}

	// Unterminated multi-line comment.
	return Token::ILLEGAL;
}

void Scanner::scanToken()
{
  m_next_token.literal.clear();
  Token::Value token;
  do {
	// Remember the position of the next token
	m_next_token.location.start = getSourcePos();

	switch (m_char) {
	  case '\n':
		m_hasLineTerminatorBeforeNext = true; // fall-through
	  case ' ':
	  case '\t':
		token = selectToken(Token::WHITESPACE);
		break;

	  case '"': case '\'':
		token = scanString();
		break;

	  case '<':
		// < <= << <<=
		advance();
		if (m_char == '=') {
		  token = selectToken(Token::LTE);
		} else if (m_char == '<') {
		  token = selectToken('=', Token::ASSIGN_SHL, Token::SHL);
		} else {
		  token = Token::LT;
		}
		break;

	  case '>':
		// > >= >> >>= >>> >>>=
		advance();
		if (m_char == '=') {
		  token = selectToken(Token::GTE);
		} else if (m_char == '>') {
		  // >> >>= >>> >>>=
		  advance();
		  if (m_char == '=') {
			token = selectToken(Token::ASSIGN_SAR);
		  } else if (m_char == '>') {
			token = selectToken('=', Token::ASSIGN_SHR, Token::SHR);
		  } else {
			token = Token::SAR;
		  }
		} else {
		  token = Token::GT;
		}
		break;

	  case '=':
		// = == =>
		advance();
		if (m_char == '=') {
		  token = selectToken(Token::EQ);
		} else if (m_char == '>') {
		  token = selectToken(Token::ARROW);
		} else {
		  token = Token::ASSIGN;
		}
		break;

	  case '!':
		// ! != !==
		advance();
		if (m_char == '=') {
		  token = selectToken(Token::NE);
		} else {
		  token = Token::NOT;
		}
		break;

	  case '+':
		// + ++ +=
		advance();
		if (m_char == '+') {
		  token = selectToken(Token::INC);
		} else if (m_char == '=') {
		  token = selectToken(Token::ASSIGN_ADD);
		} else {
		  token = Token::ADD;
		}
		break;

	  case '-':
		// - -- -=
		advance();
		if (m_char == '-') {
		  advance();
		  token = Token::DEC;
		} else if (m_char == '=') {
		  token = selectToken(Token::ASSIGN_SUB);
		} else {
		  token = Token::SUB;
		}
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
		if (m_char == '/') {
		  token = skipSingleLineComment();
		} else if (m_char == '*') {
		  token = skipMultiLineComment();
		} else if (m_char == '=') {
		  token = selectToken(Token::ASSIGN_DIV);
		} else {
		  token = Token::DIV;
		}
		break;

	  case '&':
		// & && &=
		advance();
		if (m_char == '&') {
		  token = selectToken(Token::AND);
		} else if (m_char == '=') {
		  token = selectToken(Token::ASSIGN_BIT_AND);
		} else {
		  token = Token::BIT_AND;
		}
		break;

	  case '|':
		// | || |=
		advance();
		if (m_char == '|') {
		  token = selectToken(Token::OR);
		} else if (m_char == '=') {
		  token = selectToken(Token::ASSIGN_BIT_OR);
		} else {
		  token = Token::BIT_OR;
		}
		break;

	  case '^':
		// ^ ^=
		token = selectToken('=', Token::ASSIGN_BIT_XOR, Token::BIT_XOR);
		break;

	  case '.':
		// . Number
		advance();
		if (IsDecimalDigit(m_char)) {
		  token = scanNumber(true);
		} else {
		  token = Token::PERIOD;
		}
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
		if (IsIdentifierStart(m_char)) {
		  token = scanIdentifierOrKeyword();
		} else if (IsDecimalDigit(m_char)) {
		  token = scanNumber(false);
		} else if (skipWhitespace()) {
		  token = Token::WHITESPACE;
		} else if (isSourcePastEndOfInput()) {
		  token = Token::EOS;
		} else {
		  token = selectToken(Token::ILLEGAL);
		}
		break;
	}

	// Continue scanning for tokens as long as we're just skipping
	// whitespace.
  } while (token == Token::WHITESPACE);

  m_next_token.location.end = getSourcePos();
  m_next_token.token = token;
}

bool Scanner::scanEscape()
{
	char c = m_char;
	advance();

	// Skip escaped newlines.
	if (IsLineTerminator(c))
		return true;

	switch (c) {
	case '\'':  // fall through
	case '"' :  // fall through
	case '\\': break;
	case 'b' : c = '\b'; break;
	case 'f' : c = '\f'; break;
	case 'n' : c = '\n'; break;
	case 'r' : c = '\r'; break;
	case 't' : c = '\t'; break;
	case 'u' : {
		if (!scanHexNumber(c, 4)) return false;
		break;
	}
	case 'v' : c = '\v'; break;
	case 'x' : {
		if (!scanHexNumber(c, 2)) return false;
		break;
	}
	}

	// According to ECMA-262, section 7.8.4, characters not covered by the
	// above cases should be illegal, but they are commonly handled as
	// non-escaped characters by JS VMs.
	addLiteralChar(c);
	return true;
}

Token::Value Scanner::scanString()
{
	const char quote = m_char;
	advance();  // consume quote

	LiteralScope literal(this);
	while (m_char != quote && !isSourcePastEndOfInput() && !IsLineTerminator(m_char)) {
		char c = m_char;
		advance();
		if (c == '\\') {
			if (isSourcePastEndOfInput() || !scanEscape()) return Token::ILLEGAL;
		} else {
			addLiteralChar(c);
		}
	}
	if (m_char != quote) return Token::ILLEGAL;
	literal.Complete();

	advance();  // consume quote
	return Token::STRING_LITERAL;
}


void Scanner::scanDecimalDigits()
{
	while (IsDecimalDigit(m_char))
		addLiteralCharAndAdvance();
}


Token::Value Scanner::scanNumber(bool _periodSeen)
{
  BOOST_ASSERT(IsDecimalDigit(m_char));  // the first digit of the number or the fraction

  enum { DECIMAL, HEX, OCTAL, IMPLICIT_OCTAL, BINARY } kind = DECIMAL;

  LiteralScope literal(this);
  if (_periodSeen) {
	// we have already seen a decimal point of the float
	addLiteralChar('.');
	scanDecimalDigits();  // we know we have at least one digit
  } else {
	// if the first character is '0' we must check for octals and hex
	if (m_char == '0') {
	  addLiteralCharAndAdvance();

	  // either 0, 0exxx, 0Exxx, 0.xxx, a hex number, a binary number or
	  // an octal number.
	  if (m_char == 'x' || m_char == 'X') {
		// hex number
		kind = HEX;
		addLiteralCharAndAdvance();
		if (!IsHexDigit(m_char)) {
		  // we must have at least one hex digit after 'x'/'X'
		  return Token::ILLEGAL;
		}
		while (IsHexDigit(m_char)) {
		  addLiteralCharAndAdvance();
		}
	  }
	}

	// Parse decimal digits and allow trailing fractional part.
	if (kind == DECIMAL) {
	  scanDecimalDigits();  // optional
	  if (m_char == '.') {
		addLiteralCharAndAdvance();
		scanDecimalDigits();  // optional
	  }
	}
  }

  // scan exponent, if any
  if (m_char == 'e' || m_char == 'E') {
	BOOST_ASSERT(kind != HEX);  // 'e'/'E' must be scanned as part of the hex number
	if (kind != DECIMAL) return Token::ILLEGAL;
	// scan exponent
	addLiteralCharAndAdvance();
	if (m_char == '+' || m_char == '-')
	  addLiteralCharAndAdvance();
	if (!IsDecimalDigit(m_char)) {
	  // we must have at least one decimal digit after 'e'/'E'
	  return Token::ILLEGAL;
	}
	scanDecimalDigits();
  }

  // The source character immediately following a numeric literal must
  // not be an identifier start or a decimal digit; see ECMA-262
  // section 7.8.3, page 17 (note that we read only one decimal digit
  // if the value is 0).
  if (IsDecimalDigit(m_char) || IsIdentifierStart(m_char))
	return Token::ILLEGAL;

  literal.Complete();

  return Token::NUMBER;
}


// ----------------------------------------------------------------------------
// Keyword Matcher

#define KEYWORDS(KEYWORD_GROUP, KEYWORD)                                     \
  KEYWORD_GROUP('a')                                                         \
  KEYWORD("address", Token::ADDRESS)                                           \
  KEYWORD_GROUP('b')                                                         \
  KEYWORD("break", Token::BREAK)                                             \
  KEYWORD("bool", Token::BOOL)                                               \
  KEYWORD_GROUP('c')                                                         \
  KEYWORD("case", Token::CASE)                                               \
  KEYWORD("catch", Token::CATCH)                                             \
  KEYWORD("const", Token::CONST)                                             \
  KEYWORD("continue", Token::CONTINUE)                                       \
  KEYWORD("contract", Token::CONTRACT)                                       \
  KEYWORD_GROUP('d')                                                         \
  KEYWORD("debugger", Token::DEBUGGER)                                       \
  KEYWORD("default", Token::DEFAULT)                                         \
  KEYWORD("delete", Token::DELETE)                                           \
  KEYWORD("do", Token::DO)                                                   \
  KEYWORD_GROUP('e')                                                         \
  KEYWORD("else", Token::ELSE)                                               \
  KEYWORD("enum", Token::FUTURE_RESERVED_WORD)                               \
  KEYWORD_GROUP('f')                                                         \
  KEYWORD("false", Token::FALSE_LITERAL)                                     \
  KEYWORD("finally", Token::FINALLY)                                         \
  KEYWORD("for", Token::FOR)                                                 \
  KEYWORD("function", Token::FUNCTION)                                       \
  KEYWORD_GROUP('h')                                                         \
  KEYWORD("hash", Token::HASH)                                               \
  KEYWORD("hash32", Token::HASH32)                                           \
  KEYWORD("hash64", Token::HASH64)                                           \
  KEYWORD("hash128", Token::HASH128)                                         \
  KEYWORD("hash256", Token::HASH256)                                         \
  KEYWORD_GROUP('i')                                                         \
  KEYWORD("if", Token::IF)                                                   \
  KEYWORD("implements", Token::FUTURE_STRICT_RESERVED_WORD)                  \
  KEYWORD("in", Token::IN)                                                   \
  KEYWORD("instanceof", Token::INSTANCEOF)                                   \
  KEYWORD("int", Token::INT)                                                 \
  KEYWORD("int32", Token::INT32)                                             \
  KEYWORD("int64", Token::INT64)                                             \
  KEYWORD("int128", Token::INT128)                                           \
  KEYWORD("int256", Token::INT256)                                           \
  KEYWORD("interface", Token::FUTURE_STRICT_RESERVED_WORD)                   \
  KEYWORD_GROUP('l')                                                         \
  KEYWORD_GROUP('m')                                                         \
  KEYWORD("mapping", Token::MAPPING)                                         \
  KEYWORD_GROUP('n')                                                         \
  KEYWORD("new", Token::NEW)                                                 \
  KEYWORD("null", Token::NULL_LITERAL)                                       \
  KEYWORD_GROUP('p')                                                         \
  KEYWORD("package", Token::FUTURE_STRICT_RESERVED_WORD)                     \
  KEYWORD("private", Token::PRIVATE)                                         \
  KEYWORD("protected", Token::FUTURE_STRICT_RESERVED_WORD)                   \
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
  KEYWORD("this", Token::THIS)                                               \
  KEYWORD("throw", Token::THROW)                                             \
  KEYWORD("true", Token::TRUE_LITERAL)                                       \
  KEYWORD("try", Token::TRY)                                                 \
  KEYWORD("typeof", Token::TYPEOF)                                           \
  KEYWORD_GROUP('u')                                                         \
  KEYWORD("uint", Token::UINT)                                               \
  KEYWORD("uint32", Token::UINT32)                                           \
  KEYWORD("uint64", Token::UINT64)                                           \
  KEYWORD("uint128", Token::UINT128)                                         \
  KEYWORD("uint256", Token::UINT256)                                         \
  KEYWORD("ureal", Token::UREAL)                                             \
  KEYWORD_GROUP('v')                                                         \
  KEYWORD("var", Token::VAR)                                                 \
  KEYWORD("void", Token::VOID)                                               \
  KEYWORD_GROUP('w')                                                         \
  KEYWORD("while", Token::WHILE)                                             \
  KEYWORD("with", Token::WITH)


static Token::Value KeywordOrIdentifierToken(const std::string& input)
{
  BOOST_ASSERT(!input.empty());
  const int kMinLength = 2;
  const int kMaxLength = 10;
  if (input.size() < kMinLength || input.size() > kMaxLength) {
	return Token::IDENTIFIER;
  }
  switch (input[0]) {
	default:
#define KEYWORD_GROUP_CASE(ch)                                \
	  break;                                                  \
	case ch:
#define KEYWORD(keyword, token)                               \
	{                                                         \
	  /* 'keyword' is a char array, so sizeof(keyword) is */  \
	  /* strlen(keyword) plus 1 for the NUL char. */          \
	  const int keyword_length = sizeof(keyword) - 1;         \
	  BOOST_STATIC_ASSERT(keyword_length >= kMinLength);      \
	  BOOST_STATIC_ASSERT(keyword_length <= kMaxLength);      \
	  if (input == keyword) {                                 \
		return token;                                         \
	  }                                                       \
	}
	KEYWORDS(KEYWORD_GROUP_CASE, KEYWORD)
  }
  return Token::IDENTIFIER;
}

Token::Value Scanner::scanIdentifierOrKeyword()
{
	BOOST_ASSERT(IsIdentifierStart(m_char));
	LiteralScope literal(this);

	addLiteralCharAndAdvance();

	// Scan the rest of the identifier characters.
	while (IsIdentifierPart(m_char))
		addLiteralCharAndAdvance();

	literal.Complete();

	return KeywordOrIdentifierToken(m_next_token.literal);
}

std::string CharStream::getLineAtPosition(int _position) const
{
	// if _position points to \n, it returns the line before the \n
	using size_type = std::string::size_type;
	size_type searchStart = std::min<size_type>(m_source.size(), _position);
	if (searchStart > 0) searchStart--;
	size_type lineStart = m_source.rfind('\n', searchStart);
	if (lineStart == std::string::npos)
		lineStart = 0;
	else
		lineStart++;
	return m_source.substr(lineStart,
						   std::min(m_source.find('\n', lineStart),
									m_source.size()) - lineStart);
}

std::tuple<int, int> CharStream::translatePositionToLineColumn(int _position) const
{
	using size_type = std::string::size_type;
	size_type searchPosition = std::min<size_type>(m_source.size(), _position);
	int lineNumber = std::count(m_source.begin(), m_source.begin() + searchPosition, '\n');

	size_type lineStart;
	if (searchPosition == 0) {
		lineStart = 0;
	} else {
		lineStart = m_source.rfind('\n', searchPosition - 1);
		lineStart = lineStart == std::string::npos ? 0 : lineStart + 1;
	}

	return std::tuple<int, int>(lineNumber, searchPosition - lineStart);
}


} }
