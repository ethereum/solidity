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
 * This file is derived from the file "scanner.h", which was part of the
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

#pragma once

#include <liblangutil/Token.h>
#include <liblangutil/CharStream.h>
#include <liblangutil/SourceLocation.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <iosfwd>

namespace langutil
{

class AstRawString;
class AstValueFactory;
class ParserRecorder;

enum class ScannerError
{
	NoError,

	IllegalToken,
	IllegalHexString,
	IllegalHexDigit,
	IllegalCommentTerminator,
	IllegalEscapeSequence,
	IllegalStringEndQuote,
	IllegalNumberSeparator,
	IllegalExponent,
	IllegalNumberEnd,

	OctalNotAllowed,
};

std::string to_string(ScannerError _errorCode);
std::ostream& operator<<(std::ostream& os, ScannerError _errorCode);

class Scanner
{
	friend class LiteralScope;
public:
	explicit Scanner(std::shared_ptr<CharStream> _source) { reset(std::move(_source)); }
	explicit Scanner(CharStream _source = CharStream()) { reset(std::move(_source)); }

	std::string const& source() const noexcept { return m_source->source(); }

	std::shared_ptr<CharStream> charStream() noexcept { return m_source; }

	/// Resets the scanner as if newly constructed with _source as input.
	void reset(CharStream _source);
	void reset(std::shared_ptr<CharStream> _source);
	/// Resets scanner to the start of input.
	void reset();

	/// @returns the next token and advances input
	Token next();

	///@{
	///@name Information about the current token

	/// @returns the current token
	Token currentToken() const
	{
		return m_currentToken.token;
	}
	ElementaryTypeNameToken currentElementaryTypeNameToken() const
	{
		unsigned firstSize;
		unsigned secondSize;
		std::tie(firstSize, secondSize) = m_currentToken.extendedTokenInfo;
		return ElementaryTypeNameToken(m_currentToken.token, firstSize, secondSize);
	}

	SourceLocation currentLocation() const { return m_currentToken.location; }
	std::string const& currentLiteral() const { return m_currentToken.literal; }
	std::tuple<unsigned, unsigned> const& currentTokenInfo() const { return m_currentToken.extendedTokenInfo; }

	/// Retrieves the last error that occurred during lexical analysis.
	/// @note If no error occurred, the value is undefined.
	ScannerError currentError() const noexcept { return m_currentToken.error; }
	///@}

	///@{
	///@name Information about the current comment token

	SourceLocation currentCommentLocation() const { return m_skippedComment.location; }
	std::string const& currentCommentLiteral() const { return m_skippedComment.literal; }
	/// Called by the parser during FunctionDefinition parsing to clear the current comment
	void clearCurrentCommentLiteral() { m_skippedComment.literal.clear(); }

	///@}

	///@{
	///@name Information about the next token

	/// @returns the next token without advancing input.
	Token peekNextToken() const { return m_nextToken.token; }
	SourceLocation peekLocation() const { return m_nextToken.location; }
	std::string const& peekLiteral() const { return m_nextToken.literal; }
	///@}

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string lineAtPosition(int _position) const { return m_source->lineAtPosition(_position); }
	std::tuple<int, int> translatePositionToLineColumn(int _position) const { return m_source->translatePositionToLineColumn(_position); }
	std::string sourceAt(SourceLocation const& _location) const
	{
		solAssert(!_location.isEmpty(), "");
		solAssert(m_source.get() == _location.source.get(), "CharStream memory locations must match.");
		return m_source->source().substr(_location.start, _location.end - _location.start);
	}
	///@}

private:
	inline Token setError(ScannerError _error) noexcept
	{
		m_nextToken.error = _error;
		return Token::Illegal;
	}

	/// Used for the current and look-ahead token and comments
	struct TokenDesc
	{
		Token token;
		SourceLocation location;
		std::string literal;
		ScannerError error = ScannerError::NoError;
		std::tuple<unsigned, unsigned> extendedTokenInfo;
	};

	///@{
	///@name Literal buffer support
	inline void addLiteralChar(char c) { m_nextToken.literal.push_back(c); }
	inline void addCommentLiteralChar(char c) { m_nextSkippedComment.literal.push_back(c); }
	inline void addLiteralCharAndAdvance() { addLiteralChar(m_char); advance(); }
	void addUnicodeAsUTF8(unsigned codepoint);
	///@}

	bool advance() { m_char = m_source->advanceAndGet(); return !m_source->isPastEndOfInput(); }
	void rollback(int _amount) { m_char = m_source->rollback(_amount); }

	inline Token selectErrorToken(ScannerError _err) { advance(); return setError(_err); }
	inline Token selectToken(Token _tok) { advance(); return _tok; }
	/// If the next character is _next, advance and return _then, otherwise return _else.
	inline Token selectToken(char _next, Token _then, Token _else);

	bool scanHexByte(char& o_scannedByte);
	boost::optional<unsigned> scanUnicode();

	/// Scans a single Solidity token.
	void scanToken();

	/// Skips all whitespace and @returns true if something was skipped.
	bool skipWhitespace();
	/// Skips all whitespace that are neither '\r' nor '\n'.
	void skipWhitespaceExceptUnicodeLinebreak();
	Token skipSingleLineComment();
	Token skipMultiLineComment();

	void scanDecimalDigits();
	Token scanNumber(char _charSeen = 0);
	std::tuple<Token, unsigned, unsigned> scanIdentifierOrKeyword();

	Token scanString();
	Token scanHexString();
	Token scanSingleLineDocComment();
	Token scanMultiLineDocComment();
	/// Scans a slash '/' and depending on the characters returns the appropriate token
	Token scanSlash();

	/// Scans an escape-sequence which is part of a string and adds the
	/// decoded character to the current literal. Returns true if a pattern
	/// is scanned.
	bool scanEscape();

	/// @returns true iff we are currently positioned at a unicode line break.
	bool isUnicodeLinebreak();

	/// Return the current source position.
	int sourcePos() const { return m_source->position(); }
	bool isSourcePastEndOfInput() const { return m_source->isPastEndOfInput(); }

	TokenDesc m_skippedComment;  // desc for current skipped comment
	TokenDesc m_nextSkippedComment; // desc for next skipped comment

	TokenDesc m_currentToken;  // desc for current token (as returned by Next())
	TokenDesc m_nextToken;     // desc for next token (one token look-ahead)

	std::shared_ptr<CharStream> m_source;

	/// one character look-ahead, equals 0 at end of input
	char m_char;
};

}
