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

#pragma once

#include <boost/assert.hpp>

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
	CharStream() : m_pos(0) {}
	explicit CharStream(const std::string& _source): m_source(_source), m_pos(0) {}
	int getPos() const { return m_pos; }
	bool isPastEndOfInput() const { return m_pos >= m_source.size(); }
	char get() const { return m_source[m_pos]; }
	char advanceAndGet();
	char rollback(size_t _amount);

	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	/// @{
	std::string getLineAtPosition(int _position) const;
	std::tuple<int, int> translatePositionToLineColumn(int _position) const;
	/// @}
private:
	std::string m_source;
	size_t m_pos;
};


class Scanner
{
public:
	// Scoped helper for literal recording. Automatically drops the literal
	// if aborting the scanning before it's complete.
	class LiteralScope
	{
	public:
		explicit LiteralScope(Scanner* self): scanner_(self), complete_(false) { scanner_->startNewLiteral(); }
		~LiteralScope() { if (!complete_) scanner_->dropLiteral(); }
		void Complete() { complete_ = true; }

	private:
		Scanner* scanner_;
		bool complete_;
	};

	explicit Scanner(const CharStream& _source);

	/// Resets the scanner as if newly constructed with _input as input.
	void reset(const CharStream& _source);

	/// Returns the next token and advances input.
	Token::Value next();

	/// Information about the current token
	/// @{

	/// Returns the current token
	Token::Value getCurrentToken() { return m_current_token.token; }
	Location getCurrentLocation() const { return m_current_token.location; }
	const std::string& getCurrentLiteral() const { return m_current_token.literal; }
	/// @}

	/// Information about the next token
	/// @{
	/// Returns the next token without advancing input.
	Token::Value peekNextToken() const { return m_next_token.token; }
	Location peekLocation() const { return m_next_token.location; }
	const std::string& peekLiteral() const { return m_next_token.literal; }
	/// @}

	/// Functions that help pretty-printing parse errors.
	/// Do only use in error cases, they are quite expensive.
	/// @{
	std::string getLineAtPosition(int _position) const { return m_source.getLineAtPosition(_position); }
	std::tuple<int, int> translatePositionToLineColumn(int _position) const { return m_source.translatePositionToLineColumn(_position); }
	/// @}

private:
	// Used for the current and look-ahead token.
	struct TokenDesc
	{
		Token::Value token;
		Location location;
		std::string literal;
	};

	/// Literal buffer support
	/// @{
	inline void startNewLiteral() { m_next_token.literal.clear(); }
	inline void addLiteralChar(char c) { m_next_token.literal.push_back(c); }
	inline void dropLiteral() { m_next_token.literal.clear(); }
	inline void addLiteralCharAndAdvance() { addLiteralChar(m_char); advance(); }
	/// @}

	bool advance() { m_char = m_source.advanceAndGet(); return !m_source.isPastEndOfInput(); }
	void rollback(int _amount) { m_char = m_source.rollback(_amount); }

	inline Token::Value selectToken(Token::Value _tok) { advance(); return _tok; }
	/// If the next character is _next, advance and return _then, otherwise return _else.
	inline Token::Value selectToken(char _next, Token::Value _then, Token::Value _else);

	bool scanHexNumber(char& o_scannedNumber, int _expectedLength);

	// Scans a single JavaScript token.
	void scanToken();

	bool skipWhitespace();
	Token::Value skipSingleLineComment();
	Token::Value skipMultiLineComment();

	void scanDecimalDigits();
	Token::Value scanNumber(bool _periodSeen);
	Token::Value scanIdentifierOrKeyword();

	Token::Value scanString();

	/// Scans an escape-sequence which is part of a string and adds the
	/// decoded character to the current literal. Returns true if a pattern
	/// is scanned.
	bool scanEscape();

	/// Return the current source position.
	int getSourcePos() { return m_source.getPos(); }
	bool isSourcePastEndOfInput() { return m_source.isPastEndOfInput(); }

	TokenDesc m_current_token;  // desc for current token (as returned by Next())
	TokenDesc m_next_token;     // desc for next token (one token look-ahead)

	CharStream m_source;

	/// one character look-ahead, equals 0 at end of input
	char m_char;
};

}
}
