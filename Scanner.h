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

	This file is derived from the file "scanner.h", which was part of the
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

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/CommonData.h>
#include <libsolidity/BaseTypes.h>
#include <libsolidity/Token.h>

namespace dev
{
namespace solidity
{


class AstRawString;
class AstValueFactory;
class ParserRecorder;

class CharStream
{
public:
	CharStream(): m_pos(0) {}
	explicit CharStream(std::string const& _source): m_source(_source), m_pos(0) {}
	int getPos() const { return m_pos; }
	bool isPastEndOfInput(size_t _charsForward = 0) const { return (m_pos + _charsForward) >= m_source.size(); }
	char get(size_t _charsForward = 0) const { return m_source[m_pos + _charsForward]; }
	char advanceAndGet();
	void advanceBy(size_t _chars);
	char rollback(size_t _amount);

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string getLineAtPosition(int _position) const;
	std::tuple<int, int> translatePositionToLineColumn(int _position) const;
	///@}

private:
	std::string m_source;
	size_t m_pos;
};



class Scanner
{
public:

	enum LiteralType {
		LITERAL_TYPE_STRING,
		LITERAL_TYPE_NUMBER, // not really different from string type in behaviour
		LITERAL_TYPE_COMMENT
	};
	/// Scoped helper for literal recording. Automatically drops the literal
	/// if aborting the scanning before it's complete.
	class LiteralScope
	{
	public:
		explicit LiteralScope(Scanner* _self, enum LiteralType _type)
		: m_type(_type)
		, m_scanner(_self)
		, m_complete(false)
		{
			if (_type == LITERAL_TYPE_COMMENT)
				m_scanner->startNewCommentLiteral();
			else
				m_scanner->startNewLiteral();
		}
		~LiteralScope()
		{
			if (!m_complete)
			{
				if (m_type == LITERAL_TYPE_COMMENT)
					m_scanner->dropCommentLiteral();
				else
					m_scanner->dropLiteral();
			}
		}
		void complete() { m_complete = true; }

	private:
		enum LiteralType m_type;
		Scanner* m_scanner;
		bool m_complete;
	};

	Scanner() { reset(CharStream()); }
	explicit Scanner(CharStream const& _source) { reset(_source); }

	/// Resets the scanner as if newly constructed with _input as input.
	void reset(CharStream const& _source);

	/// Returns the next token and advances input
	Token::Value next();

	///@{
	///@name Information about the current token

	/// Returns the current token
	Token::Value getCurrentToken()
	{
		return m_currentToken.token;
	}
	Location getCurrentLocation() const { return m_currentToken.location; }
	std::string const& getCurrentLiteral() const { return m_currentToken.literal; }
	///@}

	///@{
	///@name Information about the current comment token

	Location getCurrentCommentLocation() const { return m_skippedComment.location; }
	std::string const& getCurrentCommentLiteral() const { return m_skippedComment.literal; }
	/// Called by the parser during FunctionDefinition parsing to clear the current comment
	void clearCurrentCommentLiteral() { m_skippedComment.literal.clear(); }

	///@}

	///@{
	///@name Information about the next token

	/// Returns the next token without advancing input.
	Token::Value peekNextToken() const { return m_nextToken.token; }
	Location peekLocation() const { return m_nextToken.location; }
	std::string const& peekLiteral() const { return m_nextToken.literal; }
	///@}

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string getLineAtPosition(int _position) const { return m_source.getLineAtPosition(_position); }
	std::tuple<int, int> translatePositionToLineColumn(int _position) const { return m_source.translatePositionToLineColumn(_position); }
	///@}

private:
	/// Used for the current and look-ahead token and comments
	struct TokenDesc
	{
		Token::Value token;
		Location location;
		std::string literal;
	};

	///@{
	///@name Literal buffer support
	inline void startNewLiteral() { m_nextToken.literal.clear(); }
	inline void startNewCommentLiteral() { m_nextSkippedComment.literal.clear(); }
	inline void addLiteralChar(char c) { m_nextToken.literal.push_back(c); }
	inline void addCommentLiteralChar(char c) { m_nextSkippedComment.literal.push_back(c); }
	inline void dropLiteral() { m_nextToken.literal.clear(); }
	inline void dropCommentLiteral() { m_nextSkippedComment.literal.clear(); }
	inline void addLiteralCharAndAdvance() { addLiteralChar(m_char); advance(); }
	///@}

	bool advance() { m_char = m_source.advanceAndGet(); return !m_source.isPastEndOfInput(); }
	void rollback(int _amount) { m_char = m_source.rollback(_amount); }

	inline Token::Value selectToken(Token::Value _tok) { advance(); return _tok; }
	/// If the next character is _next, advance and return _then, otherwise return _else.
	inline Token::Value selectToken(char _next, Token::Value _then, Token::Value _else);

	bool scanHexByte(char& o_scannedByte);

	/// Scans a single Solidity token. Returns true if the scanned token was
	/// a skipped documentation comment. False in all other cases.
	bool scanToken();

	/// Skips all whitespace and @returns true if something was skipped.
	bool skipWhitespace();
	Token::Value skipSingleLineComment();
	Token::Value skipMultiLineComment();

	void scanDecimalDigits();
	Token::Value scanNumber(char _charSeen = 0);
	Token::Value scanIdentifierOrKeyword();

	Token::Value scanString();
	Token::Value scanDocumentationComment();

	/// Scans an escape-sequence which is part of a string and adds the
	/// decoded character to the current literal. Returns true if a pattern
	/// is scanned.
	bool scanEscape();

	/// Return the current source position.
	int getSourcePos() { return m_source.getPos(); }
	bool isSourcePastEndOfInput() { return m_source.isPastEndOfInput(); }

	TokenDesc m_skippedComment;  // desc for current skipped comment
	TokenDesc m_nextSkippedComment; // desc for next skiped comment

	TokenDesc m_currentToken;  // desc for current token (as returned by Next())
	TokenDesc m_nextToken;     // desc for next token (one token look-ahead)

	CharStream m_source;

	/// one character look-ahead, equals 0 at end of input
	char m_char;
};

}
}
