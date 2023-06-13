/*
 * This file is part of solidity.
 *
 * solidity is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * solidity is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with solidity.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is derived from the file "scanner.cc", which was part of the
 * V8 project. The original copyright header follows:
 *
 * Copyright 2006-2012, the V8 project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * * Neither the name of Google Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity scanner.
 */

#include <liblangutil/Common.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>

#include <boost/algorithm/string/classification.hpp>

#include <optional>
#include <string_view>
#include <tuple>
#include <array>


namespace solidity::langutil
{

std::string to_string(ScannerError _errorCode)
{
	switch (_errorCode)
	{
		case ScannerError::NoError: return "No error.";
		case ScannerError::IllegalToken: return "Invalid token.";
		case ScannerError::IllegalHexString: return "Expected even number of hex-nibbles.";
		case ScannerError::IllegalHexDigit: return "Hexadecimal digit missing or invalid.";
		case ScannerError::IllegalCommentTerminator: return "Expected multi-line comment-terminator.";
		case ScannerError::IllegalEscapeSequence: return "Invalid escape sequence.";
		case ScannerError::UnicodeCharacterInNonUnicodeString: return "Invalid character in string. If you are trying to use Unicode characters, use a unicode\"...\" string literal.";
		case ScannerError::IllegalCharacterInString: return "Invalid character in string.";
		case ScannerError::IllegalStringEndQuote: return "Expected string end-quote.";
		case ScannerError::IllegalNumberSeparator: return "Invalid use of number separator '_'.";
		case ScannerError::IllegalExponent: return "Invalid exponent.";
		case ScannerError::IllegalNumberEnd: return "Identifier-start is not allowed at end of a number.";
		case ScannerError::OctalNotAllowed: return "Octal numbers not allowed.";
		case ScannerError::DirectionalOverrideUnderflow: return "Unicode direction override underflow in comment or string literal.";
		case ScannerError::DirectionalOverrideMismatch: return "Mismatching directional override markers in comment or string literal.";
		default:
			solAssert(false, "Unhandled case in to_string(ScannerError)");
			return "";
	}
}


std::ostream& operator<<(std::ostream& os, ScannerError _errorCode)
{
	return os << to_string(_errorCode);
}

/// Scoped helper for literal recording. Automatically drops the literal
/// if aborting the scanning before it's complete.
enum LiteralType
{
	LITERAL_TYPE_STRING,
	LITERAL_TYPE_NUMBER, // not really different from string type in behaviour
	LITERAL_TYPE_COMMENT
};

class LiteralScope
{
public:
	explicit LiteralScope(Scanner* _self, enum LiteralType _type):
		m_type(_type),
		m_scanner(_self),
		m_complete(false)
	{
		if (_type == LITERAL_TYPE_COMMENT)
			m_scanner->m_skippedComments[Scanner::NextNext].literal.clear();
		else
			m_scanner->m_tokens[Scanner::NextNext].literal.clear();
	}
	~LiteralScope()
	{
		if (!m_complete)
		{
			if (m_type == LITERAL_TYPE_COMMENT)
				m_scanner->m_skippedComments[Scanner::NextNext].literal.clear();
			else
				m_scanner->m_tokens[Scanner::NextNext].literal.clear();
		}
	}
	void complete() { m_complete = true; }

private:
	enum LiteralType m_type;
	Scanner* m_scanner;
	bool m_complete;
};

void Scanner::reset()
{
	m_source.reset();
	m_kind = ScannerKind::Solidity;
	m_char = m_source.get();
	skipWhitespace();
	next();
	next();
	next();
}

void Scanner::setPosition(size_t _offset)
{
	m_char = m_source.setPosition(_offset);
	scanToken();
	next();
	next();
}

bool Scanner::scanHexByte(char& o_scannedByte)
{
	char x = 0;
	for (size_t i = 0; i < 2; i++)
	{
		int d = hexValue(m_char);
		if (d < 0)
		{
			rollback(i);
			return false;
		}
		x = static_cast<char>(x * 16 + d);
		advance();
	}
	o_scannedByte = x;
	return true;
}

std::optional<unsigned> Scanner::scanUnicode()
{
	unsigned x = 0;
	for (size_t i = 0; i < 4; i++)
	{
		int d = hexValue(m_char);
		if (d < 0)
		{
			rollback(i);
			return {};
		}
		x = x * 16 + static_cast<unsigned>(d);
		advance();
	}
	return x;
}

// This supports codepoints between 0000 and FFFF.
void Scanner::addUnicodeAsUTF8(unsigned codepoint)
{
	if (codepoint <= 0x7f)
		addLiteralChar(char(codepoint));
	else if (codepoint <= 0x7ff)
	{
		addLiteralChar(char(0xc0u | (codepoint >> 6u)));
		addLiteralChar(char(0x80u | (codepoint & 0x3fu)));
	}
	else
	{
		addLiteralChar(char(0xe0u | (codepoint >> 12u)));
		addLiteralChar(char(0x80u | ((codepoint >> 6u) & 0x3fu)));
		addLiteralChar(char(0x80u | (codepoint & 0x3fu)));
	}
}

void Scanner::rescan()
{
	size_t rollbackTo = 0;
	if (m_skippedComments[Current].literal.empty())
		rollbackTo = static_cast<size_t>(m_tokens[Current].location.start);
	else
		rollbackTo = static_cast<size_t>(m_skippedComments[Current].location.start);
	m_char = m_source.rollback(m_source.position() - rollbackTo);
	next();
	next();
	next();
}

// Ensure that tokens can be stored in a byte.
BOOST_STATIC_ASSERT(TokenTraits::count() <= 0x100);

Token Scanner::next()
{
	m_tokens[Current] = std::move(m_tokens[Next]);
	m_tokens[Next] = std::move(m_tokens[NextNext]);
	m_skippedComments[Current] = std::move(m_skippedComments[Next]);
	m_skippedComments[Next] = std::move(m_skippedComments[NextNext]);

	scanToken();

	return m_tokens[Current].token;
}

Token Scanner::selectToken(char _next, Token _then, Token _else)
{
	advance();
	if (m_char == _next)
		return selectToken(_then);
	else
		return _else;
}

bool Scanner::skipWhitespace()
{
	size_t const startPosition = sourcePos();
	while (isWhiteSpace(m_char))
		advance();
	// Return whether or not we skipped any characters.
	return sourcePos() != startPosition;
}

bool Scanner::skipWhitespaceExceptUnicodeLinebreak()
{
	size_t const startPosition = sourcePos();
	while (isWhiteSpace(m_char) && !isUnicodeLinebreak())
		advance();
	// Return whether or not we skipped any characters.
	return sourcePos() != startPosition;
}


namespace
{

/// Tries to scan for an RLO/LRO/RLE/LRE/PDF and keeps track of script writing direction override depth.
///
/// @returns ScannerError::NoError in case of successful parsing and directional encodings are paired
///          and error code in case the input's lexical parser state is invalid and this error should be reported
///          to the user.
static ScannerError validateBiDiMarkup(CharStream& _stream, size_t _startPosition)
{
	static std::array<std::pair<std::string_view, int>, 5> constexpr directionalSequences{
		std::pair<std::string_view, int>{"\xE2\x80\xAD", 1}, // U+202D (LRO - Left-to-Right Override)
		std::pair<std::string_view, int>{"\xE2\x80\xAE", 1}, // U+202E (RLO - Right-to-Left Override)
		std::pair<std::string_view, int>{"\xE2\x80\xAA", 1}, // U+202A (LRE - Left-to-Right Embedding)
		std::pair<std::string_view, int>{"\xE2\x80\xAB", 1}, // U+202B (RLE - Right-to-Left Embedding)
		std::pair<std::string_view, int>{"\xE2\x80\xAC", -1} // U+202C (PDF - Pop Directional Formatting
	};

	size_t endPosition = _stream.position();
	_stream.setPosition(_startPosition);

	int directionOverrideDepth = 0;

	for (size_t currentPos = _startPosition; currentPos < endPosition; ++currentPos)
	{
		_stream.setPosition(currentPos);

		for (auto const& [sequence, depthChange]: directionalSequences)
			if (_stream.prefixMatch(sequence))
				directionOverrideDepth += depthChange;

		if (directionOverrideDepth < 0)
			return ScannerError::DirectionalOverrideUnderflow;
	}

	_stream.setPosition(endPosition);

	return directionOverrideDepth > 0 ? ScannerError::DirectionalOverrideMismatch : ScannerError::NoError;
}

}

Token Scanner::skipSingleLineComment()
{
	// Line terminator is not part of the comment. If it is a
	// non-ascii line terminator, it will result in a parser error.
	size_t startPosition = m_source.position();
	while (!isUnicodeLinebreak())
		if (!advance())
			break;

	ScannerError unicodeDirectionError = validateBiDiMarkup(m_source, startPosition);
	if (unicodeDirectionError != ScannerError::NoError)
		return setError(unicodeDirectionError);

	return Token::Whitespace;
}

bool Scanner::atEndOfLine() const
{
	return m_char == '\n' || m_char == '\r';
}

bool Scanner::tryScanEndOfLine()
{
	if (m_char == '\n')
	{
		advance();
		return true;
	}

	if (m_char == '\r')
	{
		if (advance() && m_char == '\n')
			advance();
		return true;
	}

	return false;
}

size_t Scanner::scanSingleLineDocComment()
{
	LiteralScope literal(this, LITERAL_TYPE_COMMENT);
	size_t endPosition = m_source.position();

	skipWhitespaceExceptUnicodeLinebreak();

	while (!isSourcePastEndOfInput())
	{
		endPosition = m_source.position();
		if (tryScanEndOfLine())
		{
			// Check if next line is also a single-line comment.
			// If any whitespaces were skipped, use source position before.
			if (!skipWhitespaceExceptUnicodeLinebreak())
				endPosition = m_source.position();

			if (!m_source.isPastEndOfInput(3) &&
				m_source.get(0) == '/' &&
				m_source.get(1) == '/' &&
				m_source.get(2) == '/')
			{
				if (!m_source.isPastEndOfInput(4) && m_source.get(3) == '/')
					break; // "////" is not a documentation comment
				m_char = m_source.advanceAndGet(3);
				if (atEndOfLine())
					continue;
				addCommentLiteralChar('\n');
			}
			else
				break; // next line is not a documentation comment, we are done
		}
		else if (isUnicodeLinebreak())
			// Any line terminator that is not '\n' is considered to end the
			// comment.
			break;
		addCommentLiteralChar(m_char);
		advance();
	}
	literal.complete();
	return endPosition;
}

Token Scanner::skipMultiLineComment()
{
	size_t startPosition = m_source.position();
	while (!isSourcePastEndOfInput())
	{
		char prevChar = m_char;
		advance();

		// If we have reached the end of the multi-line comment, we
		// consume the '/' and insert a whitespace. This way all
		// multi-line comments are treated as whitespace.
		if (prevChar == '*' && m_char == '/')
		{
			ScannerError unicodeDirectionError = validateBiDiMarkup(m_source, startPosition);
			if (unicodeDirectionError != ScannerError::NoError)
				return setError(unicodeDirectionError);

			m_char = ' ';
			return Token::Whitespace;
		}
	}
	// Unterminated multi-line comment.
	return setError(ScannerError::IllegalCommentTerminator);
}

Token Scanner::scanMultiLineDocComment()
{
	LiteralScope literal(this, LITERAL_TYPE_COMMENT);
	bool endFound = false;
	bool charsAdded = false;

	while (isWhiteSpace(m_char) && !atEndOfLine())
		advance();

	while (!isSourcePastEndOfInput())
	{
		// handle newlines in multiline comments
		if (atEndOfLine())
		{
			skipWhitespace();
			if (!m_source.isPastEndOfInput(1) && m_source.get(0) == '*' && m_source.get(1) == '*')
			{ // it is unknown if this leads to the end of the comment
				addCommentLiteralChar('*');
				advance();
			}
			else if (!m_source.isPastEndOfInput(1) && m_source.get(0) == '*' && m_source.get(1) != '/')
			{ // skip first '*' in subsequent lines
				m_char = m_source.advanceAndGet(1);
				if (atEndOfLine()) // ignores empty lines
					continue;
				if (charsAdded)
					addCommentLiteralChar('\n'); // corresponds to the end of previous line
			}
			else if (!m_source.isPastEndOfInput(1) && m_source.get(0) == '*' && m_source.get(1) == '/')
			{ // if after newline the comment ends, don't insert the newline
				m_char = m_source.advanceAndGet(2);
				endFound = true;
				break;
			}
			else if (charsAdded)
				addCommentLiteralChar('\n');
		}

		if (!m_source.isPastEndOfInput(1) && m_source.get(0) == '*' && m_source.get(1) == '/')
		{
			m_char = m_source.advanceAndGet(2);
			endFound = true;
			break;
		}
		addCommentLiteralChar(m_char);
		charsAdded = true;
		advance();
	}
	literal.complete();
	if (!endFound)
		return setError(ScannerError::IllegalCommentTerminator);
	else
		return Token::CommentLiteral;
}

Token Scanner::scanSlash()
{
	int firstSlashPosition = static_cast<int>(sourcePos());
	advance();
	if (m_char == '/')
	{
		if (!advance()) /* double slash comment directly before EOS */
			return Token::Whitespace;
		else if (m_char == '/')
		{
			advance(); //consume the last '/' at ///

			// "////"
			if (m_char == '/')
				return skipSingleLineComment();
			// doxygen style /// comment
			m_skippedComments[NextNext].location.start = firstSlashPosition;
			m_skippedComments[NextNext].location.sourceName = m_sourceName;
			m_skippedComments[NextNext].token = Token::CommentLiteral;
			m_skippedComments[NextNext].location.end = static_cast<int>(scanSingleLineDocComment());
			return Token::Whitespace;
		}
		else
			return skipSingleLineComment();
	}
	else if (m_char == '*')
	{
		// doxygen style /** natspec comment
		if (!advance()) /* slash star comment before EOS */
			return setError(ScannerError::IllegalCommentTerminator);
		else if (m_char == '*')
		{
			advance(); //consume the last '*' at /**

			// "/**/"
			if (m_char == '/')
			{
				advance(); //skip the closing slash
				return Token::Whitespace;
			}
			// "/***"
			if (m_char == '*')
				// "/***/" may be interpreted as empty natspec or skipped; skipping is simpler
				return skipMultiLineComment();
			// we actually have a multiline documentation comment
			m_skippedComments[NextNext].location.start = firstSlashPosition;
			m_skippedComments[NextNext].location.sourceName = m_sourceName;
			Token comment = scanMultiLineDocComment();
			m_skippedComments[NextNext].location.end = static_cast<int>(sourcePos());
			m_skippedComments[NextNext].token = comment;
			if (comment == Token::Illegal)
				return Token::Illegal; // error already set
			else
				return Token::Whitespace;
		}
		else
			return skipMultiLineComment();
	}
	else if (m_char == '=')
		return selectToken(Token::AssignDiv);
	else
		return Token::Div;
}

void Scanner::scanToken()
{
	m_tokens[NextNext] = {};
	m_skippedComments[NextNext] = {};

	Token token;
	// M and N are for the purposes of grabbing different type sizes
	unsigned m = 0;
	unsigned n = 0;
	do
	{
		// Remember the position of the next token
		m_tokens[NextNext].location.start = static_cast<int>(sourcePos());
		switch (m_char)
		{
		case '"':
		case '\'':
			token = scanString(false);
			break;
		case '<':
			// < <= << <<=
			advance();
			if (m_char == '=')
				token = selectToken(Token::LessThanOrEqual);
			else if (m_char == '<')
				token = selectToken('=', Token::AssignShl, Token::SHL);
			else
				token = Token::LessThan;
			break;
		case '>':
			// > >= >> >>= >>> >>>=
			advance();
			if (m_char == '=')
				token = selectToken(Token::GreaterThanOrEqual);
			else if (m_char == '>')
			{
				// >> >>= >>> >>>=
				advance();
				if (m_char == '=')
					token = selectToken(Token::AssignSar);
				else if (m_char == '>')
					token = selectToken('=', Token::AssignShr, Token::SHR);
				else
					token = Token::SAR;
			}
			else
				token = Token::GreaterThan;
			break;
		case '=':
			// = == =>
			advance();
			if (m_char == '=')
				token = selectToken(Token::Equal);
			else if (m_char == '>')
				token = selectToken(Token::DoubleArrow);
			else
				token = Token::Assign;
			break;
		case '!':
			// ! !=
			advance();
			if (m_char == '=')
				token = selectToken(Token::NotEqual);
			else
				token = Token::Not;
			break;
		case '+':
			// + ++ +=
			advance();
			if (m_char == '+')
				token = selectToken(Token::Inc);
			else if (m_char == '=')
				token = selectToken(Token::AssignAdd);
			else
				token = Token::Add;
			break;
		case '-':
			// - -- -= ->
			advance();
			if (m_char == '-')
				token = selectToken(Token::Dec);
			else if (m_char == '=')
				token = selectToken(Token::AssignSub);
			else if (m_char == '>')
				token = selectToken(Token::RightArrow);
			else
				token = Token::Sub;
			break;
		case '*':
			// * ** *=
			advance();
			if (m_char == '*')
				token = selectToken(Token::Exp);
			else if (m_char == '=')
				token = selectToken(Token::AssignMul);
			else
				token = Token::Mul;
			break;
		case '%':
			// % %=
			token = selectToken('=', Token::AssignMod, Token::Mod);
			break;
		case '/':
			// /  // /* /=
			token = scanSlash();
			break;
		case '&':
			// & && &=
			advance();
			if (m_char == '&')
				token = selectToken(Token::And);
			else if (m_char == '=')
				token = selectToken(Token::AssignBitAnd);
			else
				token = Token::BitAnd;
			break;
		case '|':
			// | || |=
			advance();
			if (m_char == '|')
				token = selectToken(Token::Or);
			else if (m_char == '=')
				token = selectToken(Token::AssignBitOr);
			else
				token = Token::BitOr;
			break;
		case '^':
			// ^ ^=
			token = selectToken('=', Token::AssignBitXor, Token::BitXor);
			break;
		case '.':
			// . Number
			advance();
			if (isDecimalDigit(m_char))
				token = scanNumber('.');
			else
				token = Token::Period;
			break;
		case ':':
			// : :=
			advance();
			if (m_char == '=')
				token = selectToken(Token::AssemblyAssign);
			else
				token = Token::Colon;
			break;
		case ';':
			token = selectToken(Token::Semicolon);
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
		case '{':
			token = selectToken(Token::LBrace);
			break;
		case '}':
			token = selectToken(Token::RBrace);
			break;
		case '?':
			token = selectToken(Token::Conditional);
			break;
		case '~':
			token = selectToken(Token::BitNot);
			break;
		default:
			if (isIdentifierStart(m_char))
			{
				std::tie(token, m, n) = scanIdentifierOrKeyword();

				// Special case for hexadecimal literals
				if (token == Token::Hex)
				{
					// reset
					m = 0;
					n = 0;

					// Special quoted hex string must follow
					if (m_char == '"' || m_char == '\'')
						token = scanHexString();
					else
						token = setError(ScannerError::IllegalToken);
				}
				else if (token == Token::Unicode && m_kind != ScannerKind::Yul)
				{
					// reset
					m = 0;
					n = 0;

					// Special quoted hex string must follow
					if (m_char == '"' || m_char == '\'')
						token = scanString(true);
					else
						token = setError(ScannerError::IllegalToken);
				}
			}
			else if (isDecimalDigit(m_char))
				token = scanNumber();
			else if (skipWhitespace())
				token = Token::Whitespace;
			else if (isSourcePastEndOfInput())
				token = Token::EOS;
			else
				token = selectErrorToken(ScannerError::IllegalToken);
			break;
		}
		// Continue scanning for tokens as long as we're just skipping
		// whitespace.
	}
	while (token == Token::Whitespace);
	m_tokens[NextNext].location.end = static_cast<int>(sourcePos());
	m_tokens[NextNext].location.sourceName = m_sourceName;
	m_tokens[NextNext].token = token;
	m_tokens[NextNext].extendedTokenInfo = std::make_tuple(m, n);
}

bool Scanner::scanEscape()
{
	char c = m_char;

	// Skip escaped newlines.
	if (tryScanEndOfLine())
		return true;
	advance();

	switch (c)
	{
	case '\'':  // fall through
	case '"':  // fall through
	case '\\':
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
	case 'u':
	{
		if (auto const codepoint = scanUnicode(); codepoint.has_value())
			addUnicodeAsUTF8(*codepoint);
		else
			return false;
		return true;
	}
	case 'x':
		if (!scanHexByte(c))
			return false;
		break;
	default:
		return false;
	}

	addLiteralChar(c);
	return true;
}

bool Scanner::isUnicodeLinebreak()
{
	if (0x0a <= m_char && m_char <= 0x0d)
		// line feed, vertical tab, form feed, carriage return
		return true;
	if (!m_source.isPastEndOfInput(1) && uint8_t(m_source.get(0)) == 0xc2 && uint8_t(m_source.get(1)) == 0x85)
		// NEL - U+0085, C2 85 in utf8
		return true;
	if (!m_source.isPastEndOfInput(2) && uint8_t(m_source.get(0)) == 0xe2 && uint8_t(m_source.get(1)) == 0x80 && (
		uint8_t(m_source.get(2)) == 0xa8 || uint8_t(m_source.get(2)) == 0xa9
	))
		// LS - U+2028, E2 80 A8  in utf8
		// PS - U+2029, E2 80 A9  in utf8
		return true;
	return false;
}

Token Scanner::scanString(bool const _isUnicode)
{
	size_t startPosition = m_source.position();
	char const quote = m_char;
	advance();  // consume quote
	LiteralScope literal(this, LITERAL_TYPE_STRING);
	while (m_char != quote && !isSourcePastEndOfInput() && !isUnicodeLinebreak())
	{
		char c = m_char;
		advance();
		if (c == '\\')
		{
			if (isSourcePastEndOfInput() || !scanEscape())
				return setError(ScannerError::IllegalEscapeSequence);
		}
		else
		{
			// Report error on non-printable characters in string literals, however
			// allow anything for unicode string literals, because their validity will
			// be verified later (in the syntax checker).
			//
			// We are using a manual range and not isprint() to avoid
			// any potential complications with locale.
			if (!_isUnicode && (static_cast<unsigned>(c) <= 0x1f || static_cast<unsigned>(c) >= 0x7f))
			{
				if (m_kind == ScannerKind::Yul)
					return setError(ScannerError::IllegalCharacterInString);
				return setError(ScannerError::UnicodeCharacterInNonUnicodeString);
			}
			addLiteralChar(c);
		}
	}
	if (m_char != quote)
		return setError(ScannerError::IllegalStringEndQuote);

	if (_isUnicode)
	{
		ScannerError unicodeDirectionError = validateBiDiMarkup(m_source, startPosition);
		if (unicodeDirectionError != ScannerError::NoError)
			return setError(unicodeDirectionError);
	}

	literal.complete();
	advance();  // consume quote
	return _isUnicode ? Token::UnicodeStringLiteral : Token::StringLiteral;
}

Token Scanner::scanHexString()
{
	char const quote = m_char;
	advance();  // consume quote
	LiteralScope literal(this, LITERAL_TYPE_STRING);
	bool allowUnderscore = false;
	while (m_char != quote && !isSourcePastEndOfInput())
	{
		char c = m_char;

		if (scanHexByte(c))
		{
			addLiteralChar(c);
			allowUnderscore = true;
		}
		else if (c == '_')
		{
			advance();
			if (!allowUnderscore || m_char == quote)
				return setError(ScannerError::IllegalNumberSeparator);
			allowUnderscore = false;
		}
		else
			return setError(ScannerError::IllegalHexString);
	}

	if (m_char != quote)
		return setError(ScannerError::IllegalStringEndQuote);

	literal.complete();
	advance();  // consume quote
	return Token::HexStringLiteral;
}

// Parse for regex [:digit:]+(_[:digit:]+)*
void Scanner::scanDecimalDigits()
{
	// MUST begin with a decimal digit.
	if (!isDecimalDigit(m_char))
		return;

	// May continue with decimal digit or underscore for grouping.
	do
		addLiteralCharAndAdvance();
	while (!m_source.isPastEndOfInput() && (isDecimalDigit(m_char) || m_char == '_'));

	// Defer further validation of underscore to SyntaxChecker.
}

Token Scanner::scanNumber(char _charSeen)
{
	enum { DECIMAL, HEX, BINARY } kind = DECIMAL;
	LiteralScope literal(this, LITERAL_TYPE_NUMBER);
	if (_charSeen == '.')
	{
		// we have already seen a decimal point of the float
		addLiteralChar('.');
		if (m_char == '_')
			return setError(ScannerError::IllegalToken);
		scanDecimalDigits();  // we know we have at least one digit
	}
	else
	{
		solAssert(_charSeen == 0, "");
		// if the first character is '0' we must check for octals and hex
		if (m_char == '0')
		{
			addLiteralCharAndAdvance();
			// either 0, 0exxx, 0Exxx, 0.xxx or a hex number
			if (m_char == 'x')
			{
				// hex number
				kind = HEX;
				addLiteralCharAndAdvance();
				if (!isHexDigit(m_char))
					return setError(ScannerError::IllegalHexDigit); // we must have at least one hex digit after 'x'

				while (isHexDigit(m_char) || m_char == '_') // We keep the underscores for later validation
					addLiteralCharAndAdvance();
			}
			else if (isDecimalDigit(m_char))
				// We do not allow octal numbers
				return setError(ScannerError::OctalNotAllowed);
		}
		// Parse decimal digits and allow trailing fractional part.
		if (kind == DECIMAL)
		{
			scanDecimalDigits();  // optional
			if (m_char == '.')
			{
				if (!m_source.isPastEndOfInput(1) && m_source.get(1) == '_')
				{
					// Assume the input may be a floating point number with leading '_' in fraction part.
					// Recover by consuming it all but returning `Illegal` right away.
					addLiteralCharAndAdvance(); // '.'
					addLiteralCharAndAdvance(); // '_'
					scanDecimalDigits();
				}
				if (m_source.isPastEndOfInput() || !isDecimalDigit(m_source.get(1)))
				{
					// A '.' has to be followed by a number.
					literal.complete();
					return Token::Number;
				}
				addLiteralCharAndAdvance();
				scanDecimalDigits();
			}
		}
	}
	// scan exponent, if any
	if (m_char == 'e' || m_char == 'E')
	{
		solAssert(kind != HEX, "'e'/'E' must be scanned as part of the hex number");
		if (kind != DECIMAL)
			return setError(ScannerError::IllegalExponent);
		else if (!m_source.isPastEndOfInput(1) && m_source.get(1) == '_')
		{
			// Recover from wrongly placed underscore as delimiter in literal with scientific
			// notation by consuming until the end.
			addLiteralCharAndAdvance(); // 'e'
			addLiteralCharAndAdvance(); // '_'
			scanDecimalDigits();
			literal.complete();
			return Token::Number;
		}
		// scan exponent
		addLiteralCharAndAdvance(); // 'e' | 'E'
		if (m_char == '+' || m_char == '-')
			addLiteralCharAndAdvance();
		if (!isDecimalDigit(m_char)) // we must have at least one decimal digit after 'e'/'E'
			return setError(ScannerError::IllegalExponent);
		scanDecimalDigits();
	}
	// The source character immediately following a numeric literal must
	// not be an identifier start or a decimal digit; see ECMA-262
	// section 7.8.3, page 17 (note that we read only one decimal digit
	// if the value is 0).
	if (isDecimalDigit(m_char) || isIdentifierStart(m_char))
		return setError(ScannerError::IllegalNumberEnd);
	literal.complete();
	return Token::Number;
}

std::tuple<Token, unsigned, unsigned> Scanner::scanIdentifierOrKeyword()
{
	solAssert(isIdentifierStart(m_char), "");
	LiteralScope literal(this, LITERAL_TYPE_STRING);
	addLiteralCharAndAdvance();
	// Scan the rest of the identifier characters.
	while (isIdentifierPart(m_char) || (m_char == '.' && m_kind == ScannerKind::Yul))
		addLiteralCharAndAdvance();
	literal.complete();

	auto const token = TokenTraits::fromIdentifierOrKeyword(m_tokens[NextNext].literal);
	switch (m_kind)
	{
	case ScannerKind::Solidity:
		// Turn experimental Solidity keywords that are not keywords in legacy Solidity into identifiers.
		if (TokenTraits::isExperimentalSolidityOnlyKeyword(std::get<0>(token)))
			return std::make_tuple(Token::Identifier, 0, 0);
		break;
	case ScannerKind::Yul:
		// Turn Solidity identifier into a Yul keyword
		if (m_tokens[NextNext].literal == "leave")
			return std::make_tuple(Token::Leave, 0, 0);
		// Turn non-Yul keywords into identifiers.
		if (!TokenTraits::isYulKeyword(std::get<0>(token)))
			return std::make_tuple(Token::Identifier, 0, 0);
		break;
	case ScannerKind::ExperimentalSolidity:
		// Turn Solidity keywords that are not keywords in experimental solidity into identifiers.
		if (!TokenTraits::isExperimentalSolidityKeyword(std::get<0>(token)))
			return std::make_tuple(Token::Identifier, 0, 0);
		break;
	}
	return token;
}

} // namespace solidity::langutil
