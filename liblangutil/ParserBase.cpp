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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity parser shared functionality.
 */

#include <liblangutil/ParserBase.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ErrorReporter.h>

using namespace solidity;
using namespace solidity::langutil;

SourceLocation ParserBase::currentLocation() const
{
	return m_scanner->currentLocation();
}

Token ParserBase::currentToken() const
{
	return m_scanner->currentToken();
}

Token ParserBase::peekNextToken() const
{
	return m_scanner->peekNextToken();
}

std::string ParserBase::currentLiteral() const
{
	return m_scanner->currentLiteral();
}

Token ParserBase::advance()
{
	return m_scanner->next();
}

std::string ParserBase::tokenName(Token _token)
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
		std::string const expectedToken = ParserBase::tokenName(_value);
		if (m_parserErrorRecovery)
			parserError(6635_error, "Expected " + expectedToken + " but got " + tokenName(tok));
		else
			fatalParserError(2314_error, "Expected " + expectedToken + " but got " + tokenName(tok));
		// Do not advance so that recovery can sync or make use of the current token.
		// This is especially useful if the expected token
		// is the only one that is missing and is at the end of a construct.
		// "{ ... ; }" is such an example.
		//        ^
		_advance = false;
	}
	if (_advance)
		advance();
}

void ParserBase::expectTokenOrConsumeUntil(Token _value, std::string const& _currentNodeName, bool _advance)
{
	solAssert(m_inParserRecovery, "The function is supposed to be called during parser recovery only.");

	Token tok = m_scanner->currentToken();
	if (tok != _value)
	{
		SourceLocation errorLoc = currentLocation();
		int startPosition = errorLoc.start;
		while (m_scanner->currentToken() != _value && m_scanner->currentToken() != Token::EOS)
			advance();

		std::string const expectedToken = ParserBase::tokenName(_value);
		if (m_scanner->currentToken() == Token::EOS)
		{
			// rollback to where the token started, and raise exception to be caught at a higher level.
			m_scanner->setPosition(static_cast<size_t>(startPosition));
			std::string const msg = "In " + _currentNodeName + ", " + expectedToken + "is expected; got " + ParserBase::tokenName(tok) + " instead.";
			fatalParserError(1957_error, errorLoc, msg);
		}
		else
		{
			parserWarning(3796_error, "Recovered in " + _currentNodeName + " at " + expectedToken + ".");
			m_inParserRecovery = false;
		}
	}
	else
	{
		std::string expectedToken = ParserBase::tokenName(_value);
		parserWarning(3347_error, "Recovered in " + _currentNodeName + " at " + expectedToken + ".");
		m_inParserRecovery = false;
	}

	if (_advance)
		advance();
}

void ParserBase::increaseRecursionDepth()
{
	m_recursionDepth++;
	if (m_recursionDepth >= 1200)
		fatalParserError(7319_error, "Maximum recursion depth reached during parsing.");
}

void ParserBase::decreaseRecursionDepth()
{
	solAssert(m_recursionDepth > 0, "");
	m_recursionDepth--;
}

void ParserBase::parserWarning(ErrorId _error, std::string const& _description)
{
	m_errorReporter.warning(_error, currentLocation(), _description);
}

void ParserBase::parserWarning(ErrorId _error, SourceLocation const& _location, std::string const& _description)
{
	m_errorReporter.warning(_error, _location, _description);
}

void ParserBase::parserError(ErrorId _error, SourceLocation const& _location, std::string const& _description)
{
	m_errorReporter.parserError(_error, _location, _description);
}

void ParserBase::parserError(ErrorId _error, std::string const& _description)
{
	parserError(_error, currentLocation(), _description);
}

void ParserBase::fatalParserError(ErrorId _error, std::string const& _description)
{
	fatalParserError(_error, currentLocation(), _description);
}

void ParserBase::fatalParserError(ErrorId _error, SourceLocation const& _location, std::string const& _description)
{
	m_errorReporter.fatalParserError(_error, _location, _description);
}
