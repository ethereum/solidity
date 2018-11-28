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
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity parser shared functionality.
 */

#pragma once

#include <liblangutil/Token.h>
#include <liblangutil/Scanner.h>
#include <memory>
#include <string>

namespace langutil
{

class ErrorReporter;
class Scanner;

class ParserBase
{
public:
	explicit ParserBase(ErrorReporter& errorReporter): m_errorReporter(errorReporter) {}

	std::shared_ptr<std::string const> const& sourceName() const;
	std::shared_ptr<CharStream> source() const { return m_scanner->charStream(); }

protected:
	/// Utility class that creates an error and throws an exception if the
	/// recursion depth is too deep.
	class RecursionGuard
	{
	public:
		explicit RecursionGuard(ParserBase& _parser): m_parser(_parser)
		{
			m_parser.increaseRecursionDepth();
		}
		~RecursionGuard() { m_parser.decreaseRecursionDepth(); }
	private:
		ParserBase& m_parser;
	};

	/// Start position of the current token
	int position() const;
	/// End position of the current token
	int endPosition() const;

	///@{
	///@name Helper functions
	/// If current token value is not _value, throw exception otherwise advance token.
	void expectToken(Token _value, bool _advance = true);
	Token currentToken() const;
	Token peekNextToken() const;
	std::string currentLiteral() const;
	Token advance();
	///@}

	/// Increases the recursion depth and throws an exception if it is too deep.
	void increaseRecursionDepth();
	void decreaseRecursionDepth();

	/// Creates a @ref ParserError and annotates it with the current position and the
	/// given @a _description.
	void parserError(std::string const& _description);

	/// Creates a @ref ParserError and annotates it with the current position and the
	/// given @a _description. Throws the FatalError.
	void fatalParserError(std::string const& _description);

	std::shared_ptr<Scanner> m_scanner;
	/// The reference to the list of errors and warning to add errors/warnings during parsing
	ErrorReporter& m_errorReporter;
	/// Current recursion depth during parsing.
	size_t m_recursionDepth = 0;
};

}
