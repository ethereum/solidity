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

#include <vector>
#include <libdevcore/Log.h>
#include <libsolidity/BaseTypes.h>
#include <libsolidity/Parser.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Exceptions.h>

using namespace std;

namespace dev
{
namespace solidity
{

/// AST node factory that also tracks the begin and end position of an AST node
/// while it is being parsed
class Parser::ASTNodeFactory
{
public:
	ASTNodeFactory(Parser const& _parser):
		m_parser(_parser), m_location(_parser.getPosition(), -1, _parser.getSourceName()) {}

	void markEndPosition() { m_location.end = m_parser.getEndPosition(); }
	void setLocationEmpty() { m_location.end = m_location.start; }
	/// Set the end position to the one of the given node.
	void setEndPositionFromNode(ASTPointer<ASTNode> const& _node) { m_location.end = _node->getLocation().end; }

	template <class NodeType, typename... Args>
	ASTPointer<NodeType> createNode(Args&& ... _args)
	{
		if (m_location.end < 0)
			markEndPosition();
		return make_shared<NodeType>(m_location, forward<Args>(_args)...);
	}

private:
	Parser const& m_parser;
	Location m_location;
};

ASTPointer<SourceUnit> Parser::parse(shared_ptr<Scanner> const& _scanner)
{
	m_scanner = _scanner;
	ASTNodeFactory nodeFactory(*this);
	vector<ASTPointer<ASTNode>> nodes;
	while (_scanner->getCurrentToken() != Token::EOS)
	{
		switch (m_scanner->getCurrentToken())
		{
		case Token::IMPORT:
			nodes.push_back(parseImportDirective());
			break;
		case Token::CONTRACT:
			nodes.push_back(parseContractDefinition());
			break;
		default:
			BOOST_THROW_EXCEPTION(createParserError(std::string("Expected import directive or contract definition.")));
		}
	}
	return nodeFactory.createNode<SourceUnit>(nodes);
}

std::shared_ptr<const string> const& Parser::getSourceName() const
{
	return m_scanner->getSourceName();
}

int Parser::getPosition() const
{
	return m_scanner->getCurrentLocation().start;
}

int Parser::getEndPosition() const
{
	return m_scanner->getCurrentLocation().end;
}

ASTPointer<ImportDirective> Parser::parseImportDirective()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::IMPORT);
	if (m_scanner->getCurrentToken() != Token::STRING_LITERAL)
		BOOST_THROW_EXCEPTION(createParserError("Expected string literal (URL)."));
	ASTPointer<ASTString> url = getLiteralAndAdvance();
	nodeFactory.markEndPosition();
	expectToken(Token::SEMICOLON);
	return nodeFactory.createNode<ImportDirective>(url);
}

ASTPointer<ContractDefinition> Parser::parseContractDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docstring;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docstring = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());
	expectToken(Token::CONTRACT);
	ASTPointer<ASTString> name = expectIdentifierToken();
	expectToken(Token::LBRACE);
	vector<ASTPointer<StructDefinition>> structs;
	vector<ASTPointer<VariableDeclaration>> stateVariables;
	vector<ASTPointer<FunctionDefinition>> functions;
	bool visibilityIsPublic = true;
	while (true)
	{
		Token::Value currentToken = m_scanner->getCurrentToken();
		if (currentToken == Token::RBRACE)
			break;
		else if (currentToken == Token::PUBLIC || currentToken == Token::PRIVATE)
		{
			visibilityIsPublic = (m_scanner->getCurrentToken() == Token::PUBLIC);
			m_scanner->next();
			expectToken(Token::COLON);
		}
		else if (currentToken == Token::FUNCTION)
			functions.push_back(parseFunctionDefinition(visibilityIsPublic));
		else if (currentToken == Token::STRUCT)
			structs.push_back(parseStructDefinition());
		else if (currentToken == Token::IDENTIFIER || currentToken == Token::MAPPING ||
				 Token::isElementaryTypeName(currentToken))
		{
			bool const allowVar = false;
			stateVariables.push_back(parseVariableDeclaration(allowVar));
			expectToken(Token::SEMICOLON);
		}
		else
			BOOST_THROW_EXCEPTION(createParserError("Function, variable or struct declaration expected."));
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);
	return nodeFactory.createNode<ContractDefinition>(name, docstring, structs, stateVariables, functions);
}

ASTPointer<FunctionDefinition> Parser::parseFunctionDefinition(bool _isPublic)
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docstring;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docstring = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());

	expectToken(Token::FUNCTION);
	ASTPointer<ASTString> name(expectIdentifierToken());
	ASTPointer<ParameterList> parameters(parseParameterList());
	bool isDeclaredConst = false;
	if (m_scanner->getCurrentToken() == Token::CONST)
	{
		isDeclaredConst = true;
		m_scanner->next();
	}
	ASTPointer<ParameterList> returnParameters;
	if (m_scanner->getCurrentToken() == Token::RETURNS)
	{
		bool const permitEmptyParameterList = false;
		m_scanner->next();
		returnParameters = parseParameterList(permitEmptyParameterList);
	}
	else
	{
		// create an empty parameter list at a zero-length location
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.setLocationEmpty();
		returnParameters = nodeFactory.createNode<ParameterList>(vector<ASTPointer<VariableDeclaration>>());
	}
	ASTPointer<Block> block = parseBlock();
	nodeFactory.setEndPositionFromNode(block);
	return nodeFactory.createNode<FunctionDefinition>(name, _isPublic, docstring,
													  parameters,
													  isDeclaredConst, returnParameters, block);
}

ASTPointer<StructDefinition> Parser::parseStructDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::STRUCT);
	ASTPointer<ASTString> name = expectIdentifierToken();
	vector<ASTPointer<VariableDeclaration>> members;
	expectToken(Token::LBRACE);
	while (m_scanner->getCurrentToken() != Token::RBRACE)
	{
		bool const allowVar = false;
		members.push_back(parseVariableDeclaration(allowVar));
		expectToken(Token::SEMICOLON);
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);
	return nodeFactory.createNode<StructDefinition>(name, members);
}

ASTPointer<VariableDeclaration> Parser::parseVariableDeclaration(bool _allowVar)
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<TypeName> type = parseTypeName(_allowVar);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<VariableDeclaration>(type, expectIdentifierToken());
}

ASTPointer<TypeName> Parser::parseTypeName(bool _allowVar)
{
	ASTPointer<TypeName> type;
	Token::Value token = m_scanner->getCurrentToken();
	if (Token::isElementaryTypeName(token))
	{
		type = ASTNodeFactory(*this).createNode<ElementaryTypeName>(token);
		m_scanner->next();
	}
	else if (token == Token::VAR)
	{
		if (!_allowVar)
			BOOST_THROW_EXCEPTION(createParserError("Expected explicit type name."));
		m_scanner->next();
	}
	else if (token == Token::MAPPING)
	{
		type = parseMapping();
	}
	else if (token == Token::IDENTIFIER)
	{
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.markEndPosition();
		type = nodeFactory.createNode<UserDefinedTypeName>(expectIdentifierToken());
	}
	else
		BOOST_THROW_EXCEPTION(createParserError("Expected type name"));
	return type;
}

ASTPointer<Mapping> Parser::parseMapping()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::MAPPING);
	expectToken(Token::LPAREN);
	if (!Token::isElementaryTypeName(m_scanner->getCurrentToken()))
		BOOST_THROW_EXCEPTION(createParserError("Expected elementary type name for mapping key type"));
	ASTPointer<ElementaryTypeName> keyType;
	keyType = ASTNodeFactory(*this).createNode<ElementaryTypeName>(m_scanner->getCurrentToken());
	m_scanner->next();
	expectToken(Token::ARROW);
	bool const allowVar = false;
	ASTPointer<TypeName> valueType = parseTypeName(allowVar);
	nodeFactory.markEndPosition();
	expectToken(Token::RPAREN);
	return nodeFactory.createNode<Mapping>(keyType, valueType);
}

ASTPointer<ParameterList> Parser::parseParameterList(bool _allowEmpty)
{
	ASTNodeFactory nodeFactory(*this);
	vector<ASTPointer<VariableDeclaration>> parameters;
	expectToken(Token::LPAREN);
	if (!_allowEmpty || m_scanner->getCurrentToken() != Token::RPAREN)
	{
		bool const allowVar = false;
		parameters.push_back(parseVariableDeclaration(allowVar));
		while (m_scanner->getCurrentToken() != Token::RPAREN)
		{
			expectToken(Token::COMMA);
			parameters.push_back(parseVariableDeclaration(allowVar));
		}
	}
	nodeFactory.markEndPosition();
	m_scanner->next();
	return nodeFactory.createNode<ParameterList>(parameters);
}

ASTPointer<Block> Parser::parseBlock()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::LBRACE);
	vector<ASTPointer<Statement>> statements;
	while (m_scanner->getCurrentToken() != Token::RBRACE)
		statements.push_back(parseStatement());
	nodeFactory.markEndPosition();
	expectToken(Token::RBRACE);
	return nodeFactory.createNode<Block>(statements);
}

ASTPointer<Statement> Parser::parseStatement()
{
	ASTPointer<Statement> statement;
	switch (m_scanner->getCurrentToken())
	{
	case Token::IF:
		return parseIfStatement();
	case Token::WHILE:
		return parseWhileStatement();
	case Token::FOR:
		return parseForStatement();
	case Token::LBRACE:
		return parseBlock();
		// starting from here, all statements must be terminated by a semicolon
	case Token::CONTINUE:
		statement = ASTNodeFactory(*this).createNode<Continue>();
		m_scanner->next();
		break;
	case Token::BREAK:
		statement = ASTNodeFactory(*this).createNode<Break>();
		m_scanner->next();
		break;
	case Token::RETURN:
	{
		ASTNodeFactory nodeFactory(*this);
		ASTPointer<Expression> expression;
		if (m_scanner->next() != Token::SEMICOLON)
		{
			expression = parseExpression();
			nodeFactory.setEndPositionFromNode(expression);
		}
		statement = nodeFactory.createNode<Return>(expression);
	}
	break;
	default:
		if (peekVariableDefinition())
			statement = parseVariableDefinition();
		else // "ordinary" expression statement
			statement = parseExpressionStatement();
	}
	expectToken(Token::SEMICOLON);
	return statement;
}

ASTPointer<IfStatement> Parser::parseIfStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::IF);
	expectToken(Token::LPAREN);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RPAREN);
	ASTPointer<Statement> trueBody = parseStatement();
	ASTPointer<Statement> falseBody;
	if (m_scanner->getCurrentToken() == Token::ELSE)
	{
		m_scanner->next();
		falseBody = parseStatement();
		nodeFactory.setEndPositionFromNode(falseBody);
	}
	else
		nodeFactory.setEndPositionFromNode(trueBody);
	return nodeFactory.createNode<IfStatement>(condition, trueBody, falseBody);
}

ASTPointer<WhileStatement> Parser::parseWhileStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::WHILE);
	expectToken(Token::LPAREN);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RPAREN);
	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<WhileStatement>(condition, body);
}

ASTPointer<ForStatement> Parser::parseForStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::FOR);
	expectToken(Token::LPAREN);
	ASTPointer<ASTNode> initExpression = parseVardefOrExprstatement();
	expectToken(Token::SEMICOLON);
	ASTPointer<Expression> conditionExpression = parseExpression();
	expectToken(Token::SEMICOLON);
	ASTPointer<ExpressionStatement> loopExpression = parseExpressionStatement();
	expectToken(Token::RPAREN);
	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<ForStatement>(initExpression,
												conditionExpression,
												loopExpression,
												body);
}

ASTPointer<ASTNode> Parser::parseVardefOrExprstatement()
{
	if (peekVariableDefinition())
		return parseVariableDefinition();
	else
		return parseExpressionStatement();
}

ASTPointer<VariableDefinition> Parser::parseVariableDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	bool const allowVar = true;
	ASTPointer<VariableDeclaration> variable = parseVariableDeclaration(allowVar);
	ASTPointer<Expression> value;
	if (m_scanner->getCurrentToken() == Token::ASSIGN)
	{
		m_scanner->next();
		value = parseExpression();
		nodeFactory.setEndPositionFromNode(value);
	}
	else
		nodeFactory.setEndPositionFromNode(variable);
	return nodeFactory.createNode<VariableDefinition>(variable, value);
}

ASTPointer<ExpressionStatement> Parser::parseExpressionStatement()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Expression> expression = parseExpression();
	nodeFactory.setEndPositionFromNode(expression);
	return nodeFactory.createNode<ExpressionStatement>(expression);
}

ASTPointer<Expression> Parser::parseExpression()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Expression> expression = parseBinaryExpression();
	if (!Token::isAssignmentOp(m_scanner->getCurrentToken()))
		return expression;
	Token::Value assignmentOperator = expectAssignmentOperator();
	ASTPointer<Expression> rightHandSide = parseExpression();
	nodeFactory.setEndPositionFromNode(rightHandSide);
	return nodeFactory.createNode<Assignment>(expression, assignmentOperator, rightHandSide);
}

ASTPointer<Expression> Parser::parseBinaryExpression(int _minPrecedence)
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Expression> expression = parseUnaryExpression();
	int precedence = Token::precedence(m_scanner->getCurrentToken());
	for (; precedence >= _minPrecedence; --precedence)
	{
		while (Token::precedence(m_scanner->getCurrentToken()) == precedence)
		{
			Token::Value op = m_scanner->getCurrentToken();
			m_scanner->next();
			ASTPointer<Expression> right = parseBinaryExpression(precedence + 1);
			nodeFactory.setEndPositionFromNode(right);
			expression = nodeFactory.createNode<BinaryOperation>(expression, op, right);
		}
	}
	return expression;
}

ASTPointer<Expression> Parser::parseUnaryExpression()
{
	ASTNodeFactory nodeFactory(*this);
	Token::Value token = m_scanner->getCurrentToken();
	if (token == Token::NEW)
	{
		expectToken(Token::NEW);
		ASTPointer<Identifier> contractName = ASTNodeFactory(*this).createNode<Identifier>(expectIdentifierToken());
		expectToken(Token::LPAREN);
		vector<ASTPointer<Expression>> arguments(parseFunctionCallArguments());
		expectToken(Token::RPAREN);
		nodeFactory.markEndPosition();
		return nodeFactory.createNode<NewExpression>(contractName, arguments);
	}
	else if (Token::isUnaryOp(token) || Token::isCountOp(token))
	{
		// prefix expression
		m_scanner->next();
		ASTPointer<Expression> subExpression = parseUnaryExpression();
		nodeFactory.setEndPositionFromNode(subExpression);
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, true);
	}
	else
	{
		// potential postfix expression
		ASTPointer<Expression> subExpression = parseLeftHandSideExpression();
		token = m_scanner->getCurrentToken();
		if (!Token::isCountOp(token))
			return subExpression;
		nodeFactory.markEndPosition();
		m_scanner->next();
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, false);
	}
}

ASTPointer<Expression> Parser::parseLeftHandSideExpression()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Expression> expression = parsePrimaryExpression();
	while (true)
	{
		switch (m_scanner->getCurrentToken())
		{
		case Token::LBRACK:
		{
			m_scanner->next();
			ASTPointer<Expression> index = parseExpression();
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
			vector<ASTPointer<Expression>> arguments = parseFunctionCallArguments();
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

ASTPointer<Expression> Parser::parsePrimaryExpression()
{
	ASTNodeFactory nodeFactory(*this);
	Token::Value token = m_scanner->getCurrentToken();
	ASTPointer<Expression> expression;
	switch (token)
	{
	case Token::TRUE_LITERAL:
	case Token::FALSE_LITERAL:
		expression = nodeFactory.createNode<Literal>(token, getLiteralAndAdvance());
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
		ASTPointer<Expression> expression = parseExpression();
		expectToken(Token::RPAREN);
		return expression;
	}
	default:
		if (Token::isElementaryTypeName(token))
		{
			// used for casts
			expression = nodeFactory.createNode<ElementaryTypeNameExpression>(token);
			m_scanner->next();
		}
		else
		{
			BOOST_THROW_EXCEPTION(createParserError("Expected primary expression."));
			return ASTPointer<Expression>(); // this is not reached
		}
		break;
	}
	return expression;
}

vector<ASTPointer<Expression>> Parser::parseFunctionCallArguments()
{
	vector<ASTPointer<Expression>> arguments;
	if (m_scanner->getCurrentToken() != Token::RPAREN)
	{
		arguments.push_back(parseExpression());
		while (m_scanner->getCurrentToken() != Token::RPAREN)
		{
			expectToken(Token::COMMA);
			arguments.push_back(parseExpression());
		}
	}
	return arguments;
}


// distinguish between variable definition (and potentially assignment) and expression statement
// (which include assignments to other expressions and pre-declared variables)
// We have a variable definition if we get a keyword that specifies a type name, or
// in the case of a user-defined type, we have two identifiers following each other.
bool Parser::peekVariableDefinition()
{
	return (m_scanner->getCurrentToken() == Token::MAPPING ||
			m_scanner->getCurrentToken() == Token::VAR ||
			((Token::isElementaryTypeName(m_scanner->getCurrentToken()) ||
			  m_scanner->getCurrentToken() == Token::IDENTIFIER) &&
			 m_scanner->peekNextToken() == Token::IDENTIFIER));
}

void Parser::expectToken(Token::Value _value)
{
	if (m_scanner->getCurrentToken() != _value)
		BOOST_THROW_EXCEPTION(createParserError(string("Expected token ") + string(Token::getName(_value))));
	m_scanner->next();
}

Token::Value Parser::expectAssignmentOperator()
{
	Token::Value op = m_scanner->getCurrentToken();
	if (!Token::isAssignmentOp(op))
		BOOST_THROW_EXCEPTION(createParserError("Expected assignment operator"));
	m_scanner->next();
	return op;
}

ASTPointer<ASTString> Parser::expectIdentifierToken()
{
	if (m_scanner->getCurrentToken() != Token::IDENTIFIER)
		BOOST_THROW_EXCEPTION(createParserError("Expected identifier"));
	return getLiteralAndAdvance();
}

ASTPointer<ASTString> Parser::getLiteralAndAdvance()
{
	ASTPointer<ASTString> identifier = make_shared<ASTString>(m_scanner->getCurrentLiteral());
	m_scanner->next();
	return identifier;
}

ParserError Parser::createParserError(string const& _description) const
{
	return ParserError() << errinfo_sourceLocation(Location(getPosition(), getPosition(), getSourceName()))
						 << errinfo_comment(_description);
}


}
}
