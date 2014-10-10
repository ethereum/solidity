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
		if (m_location.end < 0)
			markEndPosition();
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
	ptr<ASTString> name = expectIdentifierToken();
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

	expectToken(Token::RBRACE);
	expectToken(Token::EOS);

	return nodeFactory.createNode<ContractDefinition>(name, structs, stateVariables, functions);
}

ptr<FunctionDefinition> Parser::parseFunctionDefinition(bool _isPublic)
{
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::FUNCTION);
	ptr<ASTString> name(expectIdentifierToken());
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
	ptr<ASTString> name = expectIdentifierToken();
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
	return nodeFactory.createNode<VariableDeclaration>(type, expectIdentifierToken());
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
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.markEndPosition();
		type = nodeFactory.createNode<UserDefinedTypeName>(expectIdentifierToken());
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
		statements.push_back(parseStatement());
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);
	return nodeFactory.createNode<Block>(statements);
}

ptr<Statement> Parser::parseStatement()
{
	ptr<Statement> statement;

	switch (m_scanner->getCurrentToken()) {
	case Token::IF:
		return parseIfStatement();
	case Token::WHILE:
		return parseWhileStatement();
	case Token::LBRACE:
		return parseBlock();

	// starting from here, all statements must be terminated by a semicolon
	case Token::CONTINUE:
		statement = ASTNodeFactory(*this).createNode<Continue>();
		break;
	case Token::BREAK:
		statement = ASTNodeFactory(*this).createNode<Break>();
		break;
	case Token::RETURN:
		{
			ASTNodeFactory nodeFactory(*this);
			ptr<Expression> expression;
			if (m_scanner->next() != Token::SEMICOLON) {
				expression = parseExpression();
				nodeFactory.setEndPositionFromNode(expression);
			}
			statement = nodeFactory.createNode<Return>(expression);
		}
		break;
	default:
		// distinguish between variable definition (and potentially assignment) and expressions
		// (which include assignments to other expressions and pre-declared variables)
		// We have a variable definition if we ge a keyword that specifies a type name, or
		// in the case of a user-defined type, we have two identifiers following each other.
		if (m_scanner->getCurrentToken() == Token::MAPPING ||
				m_scanner->getCurrentToken() == Token::VAR ||
				Token::IsElementaryTypeName(m_scanner->getCurrentToken()) ||
				(m_scanner->getCurrentToken() == Token::IDENTIFIER &&
				 m_scanner->peek() == Token::IDENTIFIER)) {
			statement = parseVariableDefinition();
		} else {
			// "ordinary" expression
			statement = parseExpression();
		}
	}
	expectToken(Token::SEMICOLON);
	return statement;
}

ptr<IfStatement> Parser::parseIfStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::IF);
	expectToken(Token::LPAREN);
	ptr<Expression> condition = parseExpression();
	expectToken(Token::RPAREN);
	ptr<Statement> trueBody = parseStatement();
	ptr<Statement> falseBody;
	if (m_scanner->getCurrentToken() == Token::ELSE) {
		m_scanner->next();
		falseBody = parseStatement();
		nodeFactory.setEndPositionFromNode(falseBody);
	} else {
		nodeFactory.setEndPositionFromNode(trueBody);
	}
	return nodeFactory.createNode<IfStatement>(condition, trueBody, falseBody);
}

ptr<WhileStatement> Parser::parseWhileStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::WHILE);
	expectToken(Token::LPAREN);
	ptr<Expression> condition = parseExpression();
	expectToken(Token::RPAREN);
	ptr<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<WhileStatement>(condition, body);
}

ptr<VariableDefinition> Parser::parseVariableDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	ptr<VariableDeclaration> variable = parseVariableDeclaration();
	ptr<Expression> value;
	if (m_scanner->getCurrentToken() == Token::ASSIGN) {
		m_scanner->next();
		value = parseExpression();
		nodeFactory.setEndPositionFromNode(value);
	} else {
		nodeFactory.setEndPositionFromNode(variable);
	}
	return nodeFactory.createNode<VariableDefinition>(variable, value);
}

ptr<Expression> Parser::parseExpression()
{
	ASTNodeFactory nodeFactory(*this);
	ptr<Expression> expression = parseBinaryExpression();
	if (!Token::IsAssignmentOp(m_scanner->getCurrentToken()))
		return expression;

	Token::Value assignmentOperator = expectAssignmentOperator();
	ptr<Expression> rightHandSide = parseExpression();
	nodeFactory.setEndPositionFromNode(rightHandSide);
	return nodeFactory.createNode<Assignment>(expression, assignmentOperator, rightHandSide);
}

ptr<Expression> Parser::parseBinaryExpression(int _minPrecedence)
{
	ASTNodeFactory nodeFactory(*this);
	ptr<Expression> expression = parseUnaryExpression();
	int precedence = Token::Precedence(m_scanner->getCurrentToken());
	for (; precedence >= _minPrecedence; --precedence) {
		while (Token::Precedence(m_scanner->getCurrentToken()) == precedence) {
			Token::Value op = m_scanner->getCurrentToken();
			m_scanner->next();
			ptr<Expression> right = parseBinaryExpression(precedence + 1);
			nodeFactory.setEndPositionFromNode(right);
			expression = nodeFactory.createNode<BinaryOperation>(expression, op, right);
		}
	}
	return expression;
}

ptr<Expression> Parser::parseUnaryExpression()
{
	ASTNodeFactory nodeFactory(*this);
	Token::Value token = m_scanner->getCurrentToken();
	if (Token::IsUnaryOp(token) || Token::IsCountOp(token)) {
		// prefix expression
		m_scanner->next();
		ptr<Expression> subExpression = parseUnaryExpression();
		nodeFactory.setEndPositionFromNode(subExpression);
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, true);
	} else {
		// potential postfix expression
		ptr<Expression> subExpression = parseLeftHandSideExpression();
		token = m_scanner->getCurrentToken();
		if (!Token::IsCountOp(token))
			return subExpression;
		nodeFactory.markEndPosition();
		m_scanner->next();
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, false);
	}
}

ptr<Expression> Parser::parseLeftHandSideExpression()
{
	ASTNodeFactory nodeFactory(*this);
	ptr<Expression> expression = parsePrimaryExpression();

	while (true) {
		switch (m_scanner->getCurrentToken()) {
		case Token::LBRACK:
			{
				m_scanner->next();
				ptr<Expression> index = parseExpression();
				nodeFactory.markEndPosition();
				expectToken(Token::RBRACK);
				expression = nodeFactory.createNode<IndexAccess>(expression, index);
			}
			break;
		case Token::PERIOD:
			{
				m_scanner->next();
				nodeFactory.markEndPosition();
				expression = nodeFactory.createNode<MemberAccess>(expression, expectIdentifierToken());
			}
			break;
		case Token::LPAREN:
			{
				m_scanner->next();
				vecptr<Expression> arguments = parseFunctionCallArguments();
				nodeFactory.markEndPosition();
				expectToken(Token::RPAREN);
				expression = nodeFactory.createNode<FunctionCall>(expression, arguments);
			}
			break;
		default:
			return expression;
		}
	}
}

ptr<Expression> Parser::parsePrimaryExpression()
{
	ASTNodeFactory nodeFactory(*this);
	Token::Value token = m_scanner->getCurrentToken();
	ptr<Expression> expression;

	switch (token) {
	case Token::TRUE_LITERAL:
	case Token::FALSE_LITERAL:
		expression = nodeFactory.createNode<Literal>(token, ptr<ASTString>());
		m_scanner->next();
		break;
	case Token::NUMBER:
	case Token::STRING_LITERAL:
		nodeFactory.markEndPosition();
		expression = nodeFactory.createNode<Literal>(token, getLiteralAndAdvance());
		break;
	case Token::IDENTIFIER:
		nodeFactory.markEndPosition();
		expression = nodeFactory.createNode<Identifier>(getLiteralAndAdvance());
		break;
	case Token::LPAREN:
		{
			m_scanner->next();
			ptr<Expression> expression = parseExpression();
			expectToken(Token::RPAREN);
			return expression;
		}
	default:
		if (Token::IsElementaryTypeName(token)) {
			// used for casts
			expression = nodeFactory.createNode<ElementaryTypeNameExpression>(token);
			m_scanner->next();
		} else {
			throwExpectationError("Expected primary expression.");
			return ptr<Expression>(); // this is not reached
		}
	}
	return expression;
}

vecptr<Expression> Parser::parseFunctionCallArguments()
{
	vecptr<Expression> arguments;
	if (m_scanner->getCurrentToken() != Token::RPAREN) {
		arguments.push_back(parseExpression());
		while (m_scanner->getCurrentToken() != Token::RPAREN) {
			expectToken(Token::COMMA);
			arguments.push_back(parseExpression());
		}
	}
	return arguments;
}

void Parser::expectToken(Token::Value _value)
{
	if (m_scanner->getCurrentToken() != _value)
		throwExpectationError(std::string("Expected token ") + std::string(Token::Name(_value)));
	m_scanner->next();
}

Token::Value Parser::expectAssignmentOperator()
{
	Token::Value op = m_scanner->getCurrentToken();
	if (!Token::IsAssignmentOp(op))
		throwExpectationError(std::string("Expected assignment operator"));
	m_scanner->next();
	return op;
}

ptr<ASTString> Parser::expectIdentifierToken()
{
	if (m_scanner->getCurrentToken() != Token::IDENTIFIER)
		throwExpectationError("Expected identifier");

	return getLiteralAndAdvance();
}

ptr<ASTString> Parser::getLiteralAndAdvance()
{
	ptr<ASTString> identifier = std::make_shared<ASTString>(m_scanner->getCurrentLiteral());
	m_scanner->next();
	return identifier;
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
