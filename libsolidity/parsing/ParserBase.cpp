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

void ParserBase::expectToken(Token::Value _value)
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
	m_scanner->next();
}

Token::Value ParserBase::expectAssignmentOperator()
{
	Token::Value op = m_scanner->currentToken();
	if (!Token::isAssignmentOp(op))
	{
		if (Token::isElementaryTypeName(op)) //for the sake of accuracy in reporting
		{
			ElementaryTypeNameToken elemTypeName = m_scanner->currentElementaryTypeNameToken();
			fatalParserError(
				string("Expected assignment operator,  got '") +
				elemTypeName.toString() +
				string("'")
			);
		}
		else
			fatalParserError(
				string("Expected assignment operator,  got '") +
				string(Token::name(m_scanner->currentToken())) +
				string("'")
			);
	}
	m_scanner->next();
	return op;
}

ASTPointer<ASTString> ParserBase::expectIdentifierToken()
{
	Token::Value id = m_scanner->currentToken();
	if (id != Token::Identifier)
	{
		if (Token::isElementaryTypeName(id)) //for the sake of accuracy in reporting
		{
			ElementaryTypeNameToken elemTypeName = m_scanner->currentElementaryTypeNameToken();
			fatalParserError(
				string("Expected identifier, got '") +
				elemTypeName.toString() +
				string("'")
			);
		}
		else
			fatalParserError(
				string("Expected identifier, got '") +
				string(Token::name(id)) +
				string("'")
			);
	}
	return getLiteralAndAdvance();
}

ASTPointer<ASTString> ParserBase::getLiteralAndAdvance()
{
	ASTPointer<ASTString> identifier = make_shared<ASTString>(m_scanner->currentLiteral());
	m_scanner->next();
	return identifier;
}

void ParserBase::parserError(string const& _description)
{
	auto err = make_shared<Error>(Error::Type::ParserError);
	*err <<
		errinfo_sourceLocation(SourceLocation(position(), position(), sourceName())) <<
		errinfo_comment(_description);

	m_errors.push_back(err);
}

void ParserBase::fatalParserError(string const& _description)
{
	parserError(_description);
	BOOST_THROW_EXCEPTION(FatalError());
}
