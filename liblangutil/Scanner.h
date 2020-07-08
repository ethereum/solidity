// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity scanner.
 */

#pragma once

#include <liblangutil/Token.h>
#include <liblangutil/CharStream.h>
#include <liblangutil/SourceLocation.h>
#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>

#include <optional>
#include <iosfwd>

namespace solidity::langutil
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
	std::shared_ptr<CharStream const> charStream() const noexcept { return m_source; }

	/// Resets the scanner as if newly constructed with _source as input.
	void reset(CharStream _source);
	void reset(std::shared_ptr<CharStream> _source);
	/// Resets scanner to the start of input.
	void reset();

	/// Enables or disables support for period in identifier.
	/// This re-scans the current token and comment literal and thus invalidates it.
	void supportPeriodInIdentifier(bool _value);

	/// @returns the next token and advances input
	Token next();

	/// Set scanner to a specific offset. This is used in error recovery.
	void setPosition(size_t _offset);

	///@{
	///@name Information about the current token

	/// @returns the current token
	Token currentToken() const
	{
		return m_tokens[Current].token;
	}
	ElementaryTypeNameToken currentElementaryTypeNameToken() const
	{
		unsigned firstSize;
		unsigned secondSize;
		std::tie(firstSize, secondSize) = m_tokens[Current].extendedTokenInfo;
		return ElementaryTypeNameToken(m_tokens[Current].token, firstSize, secondSize);
	}

	SourceLocation currentLocation() const { return m_tokens[Current].location; }
	std::string const& currentLiteral() const { return m_tokens[Current].literal; }
	std::tuple<unsigned, unsigned> const& currentTokenInfo() const { return m_tokens[Current].extendedTokenInfo; }

	/// Retrieves the last error that occurred during lexical analysis.
	/// @note If no error occurred, the value is undefined.
	ScannerError currentError() const noexcept { return m_tokens[Current].error; }
	///@}

	///@{
	///@name Information about the current comment token

	SourceLocation currentCommentLocation() const { return m_skippedComments[Current].location; }
	std::string const& currentCommentLiteral() const { return m_skippedComments[Current].literal; }
	/// Called by the parser during FunctionDefinition parsing to clear the current comment
	void clearCurrentCommentLiteral() { m_skippedComments[Current].literal.clear(); }

	///@}

	///@{
	///@name Information about the next token

	/// @returns the next token without advancing input.
	Token peekNextToken() const { return m_tokens[Next].token; }
	SourceLocation peekLocation() const { return m_tokens[Next].location; }
	std::string const& peekLiteral() const { return m_tokens[Next].literal; }

	Token peekNextNextToken() const { return m_tokens[NextNext].token; }
	///@}

	///@{
	///@name Error printing helper functions
	/// Functions that help pretty-printing parse errors
	/// Do only use in error cases, they are quite expensive.
	std::string lineAtPosition(int _position) const { return m_source->lineAtPosition(_position); }
	std::tuple<int, int> translatePositionToLineColumn(int _position) const { return m_source->translatePositionToLineColumn(_position); }
	///@}

private:
	inline Token setError(ScannerError _error) noexcept
	{
		m_tokens[NextNext].error = _error;
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
	inline void addLiteralChar(char c) { m_tokens[NextNext].literal.push_back(c); }
	inline void addCommentLiteralChar(char c) { m_skippedComments[NextNext].literal.push_back(c); }
	inline void addLiteralCharAndAdvance() { addLiteralChar(m_char); advance(); }
	void addUnicodeAsUTF8(unsigned codepoint);
	///@}

	bool advance() { m_char = m_source->advanceAndGet(); return !m_source->isPastEndOfInput(); }
	void rollback(size_t _amount) { m_char = m_source->rollback(_amount); }
	/// Rolls back to the start of the current token and re-runs the scanner.
	void rescan();

	inline Token selectErrorToken(ScannerError _err) { advance(); return setError(_err); }
	inline Token selectToken(Token _tok) { advance(); return _tok; }
	/// If the next character is _next, advance and return _then, otherwise return _else.
	inline Token selectToken(char _next, Token _then, Token _else);

	bool scanHexByte(char& o_scannedByte);
	std::optional<unsigned> scanUnicode();

	/// Scans a single Solidity token.
	void scanToken();

	/// Skips all whitespace and @returns true if something was skipped.
	bool skipWhitespace();
	/// Skips all whitespace that are neither '\r' nor '\n'.
	bool skipWhitespaceExceptUnicodeLinebreak();
	Token skipSingleLineComment();
	Token skipMultiLineComment();

	/// Tests if current source position is CR, LF or CRLF.
	bool atEndOfLine() const;

	/// Tries to consume CR, LF or CRLF line terminators and returns success or failure.
	bool tryScanEndOfLine();

	void scanDecimalDigits();
	Token scanNumber(char _charSeen = 0);
	std::tuple<Token, unsigned, unsigned> scanIdentifierOrKeyword();

	Token scanString();
	Token scanHexString();
	/// Scans a single line comment and returns its corrected end position.
	size_t scanSingleLineDocComment();
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
	size_t sourcePos() const { return m_source->position(); }
	bool isSourcePastEndOfInput() const { return m_source->isPastEndOfInput(); }

	bool m_supportPeriodInIdentifier = false;

	enum TokenIndex { Current, Next, NextNext };

	TokenDesc m_skippedComments[3] = {}; // desc for the current, next and nextnext skipped comment
	TokenDesc m_tokens[3] = {}; // desc for the current, next and nextnext token

	std::shared_ptr<CharStream> m_source;

	/// one character look-ahead, equals 0 at end of input
	char m_char;
};

}
