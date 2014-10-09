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

#include "libdevcore/Log.h"
#include "libsolidity/BaseTypes.h"
#include "libsolidity/Parser.h"
#include "libsolidity/Scanner.h"

namespace dev {
namespace solidity {

ptr<ASTNode> Parser::parse(std::shared_ptr<Scanner> const& _scanner)
{
	m_scanner = _scanner;

	return parseContractDefinition();
}


/// AST node factory that also tracks the begin and end position of an AST node
/// while it is being parsed
class Parser::ASTNodeFactory
{
public:
	ASTNodeFactory(const Parser& _parser)
		: m_parser(_parser), m_location(_parser.getPosition(), -1)
	{}

	void markEndPosition() { m_location.end = m_parser.getEndPosition(); }

	/// Set the end position to the one of the given node.
	void setEndPositionFromNode(const ptr<ASTNode>& _node)
	{
		m_location.end = _node->getLocation().end;
	}

	/// @todo: check that this actually uses perfect forwarding
	template <class NodeType, typename... Args>
	ptr<NodeType> createNode(Args&&... _args)
	{
		if (m_location.end < 0) markEndPosition();
		return std::make_shared<NodeType>(m_location, std::forward<Args>(_args)...);
	}

private:
	const Parser& m_parser;
	Location m_location;
};

int Parser::getPosition() const
{
	return m_scanner->getCurrentLocation().start;
}

int Parser::getEndPosition() const
{
	return m_scanner->getCurrentLocation().end;
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
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::FUNCTION);
	std::string name(expectIdentifier());
	ptr<ParameterList> parameters(parseParameterList());
	bool isDeclaredConst = false;
	if (m_scanner->getCurrentToken() == Token::CONST) {
		isDeclaredConst = true;
		m_scanner->next();
	}
	ptr<ParameterList> returnParameters;
	if (m_scanner->getCurrentToken() == Token::RETURNS) {
		m_scanner->next();
		returnParameters = parseParameterList();
	}
	ptr<Block> block = parseBlock();
	nodeFactory.setEndPositionFromNode(block);
	return nodeFactory.createNode<FunctionDefinition>(name, _isPublic, parameters,
													  isDeclaredConst, returnParameters, block);
}

ptr<StructDefinition> Parser::parseStructDefinition()
{
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::STRUCT);
	std::string name = expectIdentifier();
	vecptr<VariableDeclaration> members;
	expectToken(Token::LBRACE);
	while (m_scanner->getCurrentToken() != Token::RBRACE) {
		members.push_back(parseVariableDeclaration());
		expectToken(Token::SEMICOLON);
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);

	return nodeFactory.createNode<StructDefinition>(name, members);
}

ptr<VariableDeclaration> Parser::parseVariableDeclaration()
{
	ASTNodeFactory nodeFactory(*this);

	ptr<TypeName> type = parseTypeName();
	nodeFactory.markEndPosition();
	std::string name = expectIdentifier();
	return nodeFactory.createNode<VariableDeclaration>(type, name);
}

ptr<TypeName> Parser::parseTypeName()
{
	ptr<TypeName> type;
	Token::Value token = m_scanner->getCurrentToken();
	if (Token::IsElementaryTypeName(token)) {
		type = ASTNodeFactory(*this).createNode<ElementaryTypeName>(token);
		m_scanner->next();
	} else if (token == Token::VAR) {
		type = ASTNodeFactory(*this).createNode<TypeName>();
		m_scanner->next();
	} else if (token == Token::MAPPING) {
		type = parseMapping();
	} else if (token == Token::IDENTIFIER) {
		type = ASTNodeFactory(*this).createNode<UserDefinedTypeName>(m_scanner->getCurrentLiteral());
		m_scanner->next();
	} else {
		throwExpectationError("Expected type name");
	}

	return type;
}

ptr<Mapping> Parser::parseMapping()
{
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::MAPPING);
	expectToken(Token::LPAREN);

	if (!Token::IsElementaryTypeName(m_scanner->getCurrentToken()))
		throwExpectationError("Expected elementary type name for mapping key type");
	ptr<ElementaryTypeName> keyType;
	keyType = ASTNodeFactory(*this).createNode<ElementaryTypeName>(m_scanner->getCurrentToken());
	m_scanner->next();

	expectToken(Token::ARROW);
	ptr<TypeName> valueType = parseTypeName();
	nodeFactory.markEndPosition();
	expectToken(Token::RPAREN);

	return nodeFactory.createNode<Mapping>(keyType, valueType);
}

ptr<ParameterList> Parser::parseParameterList()
{
	ASTNodeFactory nodeFactory(*this);

	vecptr<VariableDeclaration> parameters;
	expectToken(Token::LPAREN);
	if (m_scanner->getCurrentToken() != Token::RPAREN) {
		parameters.push_back(parseVariableDeclaration());
		while (m_scanner->getCurrentToken() != Token::RPAREN) {
			expectToken(Token::COMMA);
			parameters.push_back(parseVariableDeclaration());
		}
	}
	nodeFactory.markEndPosition();
	m_scanner->next();
	return nodeFactory.createNode<ParameterList>(parameters);
}

ptr<Block> Parser::parseBlock()
{

	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::LBRACE);
	vecptr<Statement> statements;
	while (m_scanner->getCurrentToken() != Token::RBRACE) {
		m_scanner->next();
		statements.push_back(parseStatement());
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);
	return nodeFactory.createNode<Block>(statements);
}

ptr<Statement> Parser::parseStatement()
{

	switch (m_scanner->getCurrentToken()) {
	case Token::IF:
		return parseIfStatement();
	case Token::WHILE:
		return parseWhileStatement();
	case Token::LBRACE:
		return parseBlock();
	// starting from here, all statements must be terminated by a semicolon
	case Token::CONTINUE: // all following
		return
	}
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
	int line, column;
	std::tie(line, column) = m_scanner->translatePositionToLineColumn(getPosition());
	cwarn << "Solidity parser error: " << _description
		  << "at line " << (line + 1)
		  << ", column " << (column + 1);
	cwarn << m_scanner->getLineAtPosition(getPosition());
	cwarn << std::string(column, ' ') << "^";

	/// @todo make a proper exception hierarchy
	throw std::exception();
}


} }
