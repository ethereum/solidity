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

#pragma once

#include <libdevcore/CommonData.h>
#include <libsolidity/ast/Types.h>
#include <liblangutil/Exceptions.h>

#include <iosfwd>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace dev
{
namespace solidity
{
namespace test
{

/**
 * All soltest tokens.
 */
#define SOLT_TOKEN_LIST(T, K)      \
	T(Unknown, "unknown", 0)       \
	T(Invalid, "invalid", 0)       \
	T(EOS, "EOS", 0)               \
	T(Whitespace, "_", 0)          \
	/* punctuations */             \
	T(LParen, "(", 0)              \
	T(RParen, ")", 0)              \
	T(LBrack, "[", 0)              \
	T(RBrack, "]", 0)              \
	T(LBrace, "{", 0)              \
	T(RBrace, "}", 0)              \
	T(Sub,    "-", 0)              \
	T(Colon,  ":", 0)              \
	T(Comma,  ",", 0)              \
	T(Period, ".", 0)              \
	T(Arrow, "->", 0)              \
	T(Newline, "//", 0)            \
	/* Literals & identifier */    \
	T(Comment, "#", 0)             \
	T(Number, "number", 0)         \
	T(Identifier, "identifier", 0) \
	/* type keywords */            \
	K(Ether, "ether", 0)           \
	/* special keywords */         \
	K(Failure, "FAILURE", 0)       \

namespace soltest
{
	enum class Token : unsigned int {
	#define T(name, string, precedence) name,
		SOLT_TOKEN_LIST(T, T)
		NUM_TOKENS
	#undef T
	};

	/// Prints a friendly string representation of \param _token.
	inline std::string formatToken(Token _token)
	{
		switch (_token)
		{
	#define T(name, string, precedence) case Token::name: return string;
			SOLT_TOKEN_LIST(T, T)
	#undef T
			default: // Token::NUM_TOKENS:
				return "";
		}
	}
}



/**
 * The purpose of the ABI type is the storage of type information
 * retrieved while parsing a test. This information is used
 * for the conversion of human-readable function arguments and
 * return values to `bytes` and vice-versa.
 * Defaults to None, a 0-byte representation. 0-bytes
 * can also be interpreted as Failure, which means
 * either a REVERT or another EVM failure.
 */
struct ABIType
{
	enum Type {
		UnsignedDec,
		SignedDec,
		Failure,
		None
	};
	Type type = ABIType::None;
	size_t size = 0;
};

/**
 * Helper that can hold format information retrieved
 * while scanning through a parameter list in soltest.
 */
struct FormatInfo
{
	bool newline;
};

/**
 * Parameter abstraction used for the encoding and decoding of
 * function parameter and expectation / return value lists.
 * A parameter list is usually a comma-separated list of literals.
 * It should not be possible to call create a parameter holding
 * an identifier, but if so, the ABI type would be invalid.
 */
struct Parameter
{
	/// ABI encoded / decoded `bytes` of values.
	/// These `bytes` are used to pass values to function calls
	/// and also to store expected return vales. These are
	/// compared to the actual result of a function call
	/// and used for validating it.
	bytes rawBytes;
	/// Types that were used to encode `rawBytes`. Expectations
	/// are usually comma separated literals. Their type is auto-
	/// detected and retained in order to format them later on.
	ABIType abiType;
	/// Format info attached to the parameter. It handles newlines given
	/// in the declaration of it.
	FormatInfo format;
};
using ParameterList = std::vector<Parameter>;

/**
 * Represents the expected result of a function call after it has been executed. This may be a single
 * return value or a comma-separated list of return values. It also contains the detected input
 * formats used to convert the values to `bytes` needed for the comparison with the actual result
 * of a call. In addition to that, it also stores the expected transaction status.
 * An optional comment can be assigned.
 */
struct FunctionCallExpectations
{
	/// Representation of the comma-separated (or empty) list of expected result values
	/// attached to the function call object. It is checked against the actual result of
	/// a function call when used in test framework.
	ParameterList result;
	/// Expected status of the transaction. It can be either
	/// a REVERT or a different EVM failure (e.g. out-of-gas).
	bool failure = true;
	/// A Comment that can be attached to the expectations,
	/// that is retained and can be displayed.
	std::string comment;
	/// ABI encoded `bytes` of parsed expected return values. It is checked
	/// against the actual result of a function call when used in test framework.
	bytes rawBytes() const
	{
		bytes raw;
		for (auto const& param: result)
			raw += param.rawBytes;
		return raw;
	}
};

/**
 * Represents the arguments passed to a function call. This can be a single
 * argument or a comma-separated list of arguments. It also contains the detected input
 * formats used to convert the arguments to `bytes` needed for the call.
 * An optional comment can be assigned.
 */
struct FunctionCallArgs
{
	/// Types that were used to encode `rawBytes`. Parameters
	/// are usually comma separated literals. Their type is auto-
	/// detected and retained in order to format them later on.
	ParameterList parameters;
	/// A Comment that can be attached to the expectations,
	/// that is retained and can be displayed.
	std::string comment;
	/// ABI encoded `bytes` of parsed parameters. These `bytes`
	/// passed to the function call.
	bytes rawBytes() const
	{
		bytes raw;
		for (auto const& param: parameters)
			raw += param.rawBytes;
		return raw;
	}
};

/**
 * Represents a function call read from an input stream. It contains the signature, the
 * arguments, an optional ether value and an expected execution result.
 */
struct FunctionCall
{
	/// Signature of the function call, e.g. `f(uint256, uint256)`.
	std::string signature;
	/// Optional `ether` value that can be send with the call.
	u256 value;
	/// Object that holds all function parameters in their `bytes`
	/// representations given by the contract ABI.
	FunctionCallArgs arguments;
	/// Object that holds all function call expectation in
	/// their `bytes` representations given by the contract ABI.
	/// They are checked against the actual results and their
	/// `bytes` representation, as well as the transaction status.
	FunctionCallExpectations expectations;
	/// single / multi-line mode will be detected as follows:
	/// every newline (//) in source results in a function call
	/// that has its display mode set to multi-mode. Function and
	/// result parameter lists are an exception: a single parameter
	/// stores a format information that contains a newline definition.
	enum DisplayMode {
		SingleLine,
		MultiLine
	};
	DisplayMode displayMode = DisplayMode::SingleLine;
};

/**
 * Class that is able to parse an additional and well-formed comment section in a Solidity
 * source file used by the file-based unit test environment. For now, it parses function
 * calls and their expected result after the call was made.
 *
 * - Function calls defined in blocks:
 * // f(uint256, uint256): 1, 1 # Signature and comma-separated list of arguments #
 * // -> 1, 1                   # Expected result value #
 * // g(), 2 ether              # (Optional) Ether to be send with the call #
 * // -> 2, 3
 * // h(uint256), 1 ether: 42
 * // -> FAILURE                # If REVERT or other EVM failure was detected #
 * ...
 */
class TestFileParser
{
public:
	/// Constructor that takes an input stream \param _stream to operate on
	/// and creates the internal scanner.
	TestFileParser(std::istream& _stream): m_scanner(_stream) {}

	/// Parses function calls blockwise and returns a list of function calls found.
	/// Throws an exception if a function call cannot be parsed because of its
	/// incorrect structure, an invalid or unsupported encoding
	/// of its arguments or expected results.
	std::vector<FunctionCall> parseFunctionCalls();

private:
	using Token = soltest::Token;
	/**
	 * Token scanner that is used internally to abstract away character traversal.
	 */
	class Scanner
	{
	public:
		/// Constructor that takes an input stream \param _stream to operate on.
		/// It reads all lines into one single line, keeping the newlines.
		Scanner(std::istream& _stream) { readStream(_stream); }

		/// Reads input stream into a single line and resets the current iterator.
		void readStream(std::istream& _stream);

		/// Reads character stream and creates token.
		void scanNextToken();

		soltest::Token currentToken() { return m_currentToken.first; }
		std::string currentLiteral() { return m_currentToken.second; }

		std::string scanComment();
		std::string scanIdentifierOrKeyword();
		std::string scanNumber();

	private:
		using TokenDesc = std::pair<Token, std::string>;

		/// Advances current position in the input stream.
		void advance() { ++m_char; }
		/// Returns the current character.
		char current() const { return *m_char; }
		/// Peeks the next character.
		char peek() const { auto it = m_char; return *(it + 1); }
		/// Returns true if the end of a line is reached, false otherwise.
		bool isEndOfLine() const { return m_char == m_line.end(); }

		std::string m_line;
		std::string::iterator m_char;

		std::string m_currentLiteral;

		TokenDesc m_currentToken;
	};

	bool accept(soltest::Token _token, bool const _expect = false);
	bool expect(soltest::Token _token, bool const _advance = true);

	/// Parses a function call signature in the form of f(uint256, ...).
	std::string parseFunctionSignature();

	/// Parses the optional ether value that can be passed alongside the
	/// function call arguments. Throws an InvalidEtherValueEncoding exception
	/// if given value cannot be converted to `u256`.
	u256 parseFunctionCallValue();

	/// Parses a comma-separated list of arguments passed with a function call.
	/// Does not check for a potential mismatch between the signature and the number
	/// or types of arguments.
	FunctionCallArgs parseFunctionCallArguments();

	/// Parses the expected result of a function call execution.
	FunctionCallExpectations parseFunctionCallExpectations();

	/// Parses the next parameter in a comma separated list.
	/// Takes a newly parsed, and type-annotated `bytes` argument,
	/// appends it to the internal `bytes` buffer of the parameter. It can also
	/// store newlines found in the source, that are needed to
	/// format input and output of the interactive update.
	Parameter parseParameter();

	/// Parses and converts the current literal to its byte representation and
	/// preserves the chosen ABI type. Based on that type information, the driver of
	/// this parser can format arguments, expectations and results. Supported types:
	/// - unsigned and signed decimal number literals.
	/// Returns invalid ABI type for empty literal. This is needed in order
	/// to detect empty expectations. Throws a ParserError if data is encoded incorrectly or
	/// if data type is not supported.
	std::pair<bytes, ABIType> parseABITypeLiteral();

	/// Recursively parses an identifier or a tuple definition that contains identifiers
	/// and / or parentheses like `((uint, uint), (uint, (uint, uint)), uint)`.
	std::string parseIdentifierOrTuple();

	/// Parses a comment that is defined like this:
	/// # A nice comment. #
	std::string parseComment();

	/// Parses the current number literal.
	std::string parseNumber();

	/// Tries to convert \param _literal to `uint256` and throws if
	/// conversion fails.
	u256 convertNumber(std::string const& _literal);

	/// A scanner instance
	Scanner m_scanner;
};

}
}
}
