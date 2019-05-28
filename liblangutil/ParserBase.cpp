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

#include <liblangutil/ParserBase.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>

using namespace std;
using namespace langutil;

int ParserBase::position() const
{
	return m_scanner->currentLocation().start;
}

int ParserBase::endPosition() const
{
	return m_scanner->currentLocation().end;
}

Token ParserBase::currentToken() const
{
	return m_scanner->currentToken();
}

Token ParserBase::peekNextToken() const
{
	return m_scanner->peekNextToken();
}

string ParserBase::currentLiteral() const
{
	return m_scanner->currentLiteral();
}

Token ParserBase::advance()
{
	return m_scanner->next();
}

string ParserBase::tokenName(Token _token)
{
	if (_token == Token::Identifier)
		return "identifier";
	else if (_token == Token::EOS)
		return "end of source";
	else if (TokenTraits::isReservedKeyword(_token))
		return "reserved keyword '" + TokenTraits::friendlyName(_token) + "'";
	else if (TokenTraits::isElementaryTypeName(_token)) //for the sake of accuracy in reporting
	{
		ElementaryTypeNameToken elemTypeName = m_scanner->currentElementaryTypeNameToken();
		return "'" + elemTypeName.toString() + "'";
	}
	else
		return "'" + TokenTraits::friendlyName(_token) + "'";
}

void ParserBase::expectToken(Token _value, bool _advance)
{
	Token tok = m_scanner->currentToken();
	if (tok != _value)
	{
		string const expectedToken = ParserBase::tokenName(_value);
		if (m_parserErrorRecovery)
			parserError("Expected " + expectedToken + " but got " + tokenName(tok));
		else
			fatalParserError("Expected " + expectedToken + " but got " + tokenName(tok));
		// Do not advance so that recovery can sync or make use of the current token.
		// This is especially useful if the expected token
		// is the only one that is missing and is at the end of a construct.
		// "{ ... ; }" is such an example.
		//        ^
		_advance = false;
	}
	if (_advance)
		m_scanner->next();
}

void ParserBase::expectTokenOrConsumeUntil(Token _value, string const& _currentNodeName, bool _advance)
{
	Token tok = m_scanner->currentToken();
	if (tok != _value)
	{
		int startPosition = position();
		SourceLocation errorLoc = SourceLocation{startPosition, endPosition(), source()};
		while (m_scanner->currentToken() != _value && m_scanner->currentToken() != Token::EOS)
			m_scanner->next();

		string const expectedToken = ParserBase::tokenName(_value);
		string const msg = "In " + _currentNodeName + ", " + expectedToken + "is expected; got " +  ParserBase::tokenName(tok) +  "instead.";
		if (m_scanner->currentToken() == Token::EOS)
		{
			// rollback to where the token started, and raise exception to be caught at a higher level.
			m_scanner->setPosition(startPosition);
			m_inParserRecovery = true;
			fatalParserError(errorLoc, msg);
		}
		else
		{
			if (m_inParserRecovery)
				parserWarning("Recovered in " + _currentNodeName + " at " + expectedToken + ".");
			else
				parserError(errorLoc, msg + "Recovered at next " + expectedToken);
			m_inParserRecovery = false;
		}
	}
	else if (m_inParserRecovery)
	{
		string expectedToken = ParserBase::tokenName(_value);
		parserWarning("Recovered in " + _currentNodeName + " at " + expectedToken + ".");
		m_inParserRecovery = false;
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

void ParserBase::parserWarning(string const& _description)
{
	m_errorReporter.warning(SourceLocation{position(), endPosition(), source()}, _description);
}

void ParserBase::parserError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.parserError(_location, _description);
}

void ParserBase::parserError(string const& _description)
{
	parserError(SourceLocation{position(), endPosition(), source()}, _description);
}

void ParserBase::fatalParserError(string const& _description)
{
	fatalParserError(SourceLocation{position(), endPosition(), source()}, _description);
}

void ParserBase::fatalParserError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.fatalParserError(_location, _description);
}
