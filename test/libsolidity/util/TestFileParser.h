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
#include <test/libsolidity/util/SoltestTypes.h>

#include <iosfwd>
#include <iterator>
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
		std::string scanDecimalNumber();
		std::string scanHexNumber();
		std::string scanString();

	private:
		using TokenDesc = std::pair<Token, std::string>;

		/// Advances current position in the input stream.
		void advance()
		{
			solAssert(m_char != m_line.end(), "Cannot advance beyond end.");
			++m_char;
		}

		/// Returns the current character or '\0' if at end of input.
		char current() const noexcept
		{
			if (m_char == m_line.end())
				return '\0';

			return *m_char;
		}

		/// Retrieves the next character ('\0' if that would be at (or beyond) the end of input)
		/// without advancing the input stream iterator.
		char peek() const noexcept;

		/// Returns true if the end of a line is reached, false otherwise.
		bool isEndOfLine() const { return m_char == m_line.end(); }

		std::string m_line;
		std::string::const_iterator m_char;

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
	///  Parses and converts the current literal to its byte representation and
	/// preserves the chosen ABI type, as well as a raw, unformatted string representation
	/// of this literal.
	/// Based on the type information retrieved, the driver of this parser may format arguments,
	/// expectations and results. Supported types:
	/// - unsigned and signed decimal number literals.
	/// Returns invalid ABI type for empty literal. This is needed in order
	/// to detect empty expectations. Throws a ParserError if data is encoded incorrectly or
	/// if data type is not supported.
	Parameter parseParameter();

	/// Recursively parses an identifier or a tuple definition that contains identifiers
	/// and / or parentheses like `((uint, uint), (uint, (uint, uint)), uint)`.
	std::string parseIdentifierOrTuple();

	/// Parses a boolean literal.
	std::string parseBoolean();

	/// Parses a comment that is defined like this:
	/// # A nice comment. #
	std::string parseComment();

	/// Parses the current decimal number literal.
	std::string parseDecimalNumber();

	/// Parses the current hex number literal.
	std::string parseHexNumber();

	/// Parses the current string literal.
	std::string parseString();

	/// A scanner instance
	Scanner m_scanner;
};

}
}
}
