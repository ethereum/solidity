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
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity parser.
 */

#include "libsolidity/BaseTypes.h"
#include "libsolidity/Parser.h"
#include "libsolidity/Scanner.h"

namespace dev {
namespace solidity {

ptr<ASTNode> Parser::parse(Scanner& _scanner)
{
    m_scanner = &_scanner;

    return parseContractDefinition();
}


/// AST node factory that also tracks the begin and end position of an AST node
/// while it is being parsed
class Parser::ASTNodeFactory
{
public:
    ASTNodeFactory(const Parser& _parser)
        : m_parser(_parser),
          m_location(_parser.getPosition(), -1)
    {}

    void markEndPosition()
    {
        m_location.end_pos = m_parser.getEndPosition();
    }

    /// @todo: check that this actually uses perfect forwarding
    template <class NodeType, typename... Args>
    ptr<NodeType> createNode(Args&&... _args)
    {
        if (m_location.end_pos < 0) markEndPosition();
        return std::make_shared<NodeType>(m_location, std::forward<Args>(_args)...);
    }

private:
    const Parser& m_parser;
    Location m_location;
};

int Parser::getPosition() const
{
    return m_scanner->getCurrentLocation().beg_pos;
}

int Parser::getEndPosition() const
{
    return m_scanner->getCurrentLocation().end_pos;
}


ptr<ContractDefinition> Parser::parseContractDefinition()
{
    ASTNodeFactory nodeFactory(*this);

    expectToken(Token::CONTRACT);
    std::string name = expectIdentifier();
    expectToken(Token::LBRACE);

    vecptr<StructDefinition> structs;
    vecptr<VariableDeclaration> stateVariables;
    vecptr<FunctionDefinition> functions;
    bool visibilityIsPublic = true;
    while (true) {
        Token::Value currentToken = m_scanner->getCurrentToken();
        if (currentToken == Token::RBRACE) {
            break;
        } else if (currentToken == Token::PUBLIC || currentToken == Token::PRIVATE) {
            visibilityIsPublic = (m_scanner->getCurrentToken() == Token::PUBLIC);
            m_scanner->next();
            expectToken(Token::COLON);
        } else if (currentToken == Token::FUNCTION) {
            functions.push_back(parseFunctionDefinition(visibilityIsPublic));
        } else if (currentToken == Token::STRUCT) {
            structs.push_back(parseStructDefinition());
            expectToken(Token::SEMICOLON);
        } else if (currentToken == Token::IDENTIFIER || currentToken == Token::MAPPING ||
                   Token::IsElementaryTypeName(currentToken)) {
            stateVariables.push_back(parseVariableDeclaration());
            expectToken(Token::SEMICOLON);
        } else {
            throwExpectationError("Function, variable or struct declaration expected.");
        }
    }
    nodeFactory.markEndPosition();

    m_scanner->next();
    expectToken(Token::EOS);

    return nodeFactory.createNode<ContractDefinition>(name, structs, stateVariables, functions);
}

ptr<FunctionDefinition> Parser::parseFunctionDefinition(bool _isPublic)
{
    (void) _isPublic;
    throwExpectationError("Function parsing is not yet implemented.");
}

ptr<StructDefinition> Parser::parseStructDefinition()
{
    throwExpectationError("Struct definition parsing is not yet implemented.");
}

ptr<VariableDeclaration> Parser::parseVariableDeclaration()
{
    ASTNodeFactory nodeFactory(*this);

    ptr<TypeName> type;
    Token::Value token = m_scanner->getCurrentToken();
    if (Token::IsElementaryTypeName(token)) {
        type = ASTNodeFactory(*this).createNode<ElementaryTypeName>(token);
        m_scanner->next();
    } else if (token == Token::VAR) {
        type = ASTNodeFactory(*this).createNode<TypeName>();
        m_scanner->next();
    } else if (token == Token::MAPPING) {
        // TODO
        throwExpectationError("mappings are not yet implemented");
    } else if (token == Token::IDENTIFIER) {
        type = ASTNodeFactory(*this).createNode<UserDefinedTypeName>(m_scanner->getCurrentLiteral());
        m_scanner->next();
    } else {
        throwExpectationError("Expected variable declaration");
    }
    nodeFactory.markEndPosition();
    std::string name = expectIdentifier();
    return nodeFactory.createNode<VariableDeclaration>(type, name);
}

void Parser::expectToken(Token::Value _value)
{
    if (m_scanner->getCurrentToken() != _value)
        throwExpectationError(std::string("Expected token ") + std::string(Token::Name(_value)));
    m_scanner->next();
}

std::string Parser::expectIdentifier()
{
    if (m_scanner->getCurrentToken() != Token::IDENTIFIER)
        throwExpectationError("Expected identifier");

    std::string literal = m_scanner->getCurrentLiteral();
    m_scanner->next();
    return literal;
}

void Parser::throwExpectationError(const std::string& _description)
{
    (void) _description;
    /// @todo make a proper exception hierarchy
    throw std::exception();//_description);
}


} }
