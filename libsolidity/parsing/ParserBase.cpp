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

#include <libsolidity/parsing/ParserBase.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/ErrorReporter.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

std::shared_ptr<string const> const& ParserBase::sourceName() const
{
	return m_scanner->sourceName();
}

int ParserBase::position() const
{
	return m_scanner->currentLocation().start;
}

int ParserBase::endPosition() const
{
	return m_scanner->currentLocation().end;
}

Token::Value ParserBase::currentToken() const
{
	return m_scanner->currentToken();
}

Token::Value ParserBase::peekNextToken() const
{
	return m_scanner->peekNextToken();
}

std::string ParserBase::currentLiteral() const
{
	return m_scanner->currentLiteral();
}

Token::Value ParserBase::advance()
{
	return m_scanner->next();
}

void ParserBase::expectToken(Token::Value _value, bool _advance)
{
	Token::Value tok = m_scanner->currentToken();
	if (tok != _value)
	{
		if (Token::isReservedKeyword(tok))
		{
			fatalParserError(
				string("Expected token ") +
				string(Token::name(_value)) +
				string(" got reserved keyword '") +
				string(Token::name(tok)) +
				string("'")
			);
		}
		else if (Token::isElementaryTypeName(tok)) //for the sake of accuracy in reporting
		{
			ElementaryTypeNameToken elemTypeName = m_scanner->currentElementaryTypeNameToken();
			fatalParserError(
				string("Expected token ") +
				string(Token::name(_value)) +
				string(" got '") +
				elemTypeName.toString() +
				string("'")
			);
		}
		else
			fatalParserError(
				string("Expected token ") +
				string(Token::name(_value)) +
				string(" got '") +
				string(Token::name(m_scanner->currentToken())) +
				string("'")
			);
	}
	if (_advance)
		m_scanner->next();
}

void ParserBase::increaseRecursionDepth()
{
	m_recursionDepth++;
	if (m_recursionDepth >= 2560)
		fatalParserError("Maximum recursion depth reached during parsing.");
}

void ParserBase::decreaseRecursionDepth()
{
	solAssert(m_recursionDepth > 0, "");
	m_recursionDepth--;
}

void ParserBase::parserError(string const& _description)
{
	m_errorReporter.parserError(SourceLocation(position(), position(), sourceName()), _description);
}

void ParserBase::fatalParserError(string const& _description)
{
	m_errorReporter.fatalParserError(SourceLocation(position(), position(), sourceName()), _description);
}
