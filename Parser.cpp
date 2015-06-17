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
#include <libevmasm/SourceLocation.h>
#include <libsolidity/Parser.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/InterfaceHandler.h>

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
	ASTNodeFactory(Parser const& _parser, ASTPointer<ASTNode> const& _childNode):
		m_parser(_parser), m_location(_childNode->getLocation()) {}

	void markEndPosition() { m_location.end = m_parser.getEndPosition(); }
	void setLocation(SourceLocation const& _location) { m_location = _location; }
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
	SourceLocation m_location;
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
		case Token::Import:
			nodes.push_back(parseImportDirective());
			break;
		case Token::Contract:
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
	expectToken(Token::Import);
	if (m_scanner->getCurrentToken() != Token::StringLiteral)
		BOOST_THROW_EXCEPTION(createParserError("Expected string literal (URL)."));
	ASTPointer<ASTString> url = getLiteralAndAdvance();
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<ImportDirective>(url);
}

ASTPointer<ContractDefinition> Parser::parseContractDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docString;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docString = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());
	expectToken(Token::Contract);
	ASTPointer<ASTString> name = expectIdentifierToken();
	vector<ASTPointer<InheritanceSpecifier>> baseContracts;
	vector<ASTPointer<StructDefinition>> structs;
	vector<ASTPointer<EnumDefinition>> enums;
	vector<ASTPointer<VariableDeclaration>> stateVariables;
	vector<ASTPointer<FunctionDefinition>> functions;
	vector<ASTPointer<ModifierDefinition>> modifiers;
	vector<ASTPointer<EventDefinition>> events;
	if (m_scanner->getCurrentToken() == Token::Is)
		do
		{
			m_scanner->next();
			baseContracts.push_back(parseInheritanceSpecifier());
		}
		while (m_scanner->getCurrentToken() == Token::Comma);
	expectToken(Token::LBrace);
	while (true)
	{
		Token::Value currentToken = m_scanner->getCurrentToken();
		if (currentToken == Token::RBrace)
			break;
		else if (currentToken == Token::Function)
			functions.push_back(parseFunctionDefinition(name.get()));
		else if (currentToken == Token::Struct)
			structs.push_back(parseStructDefinition());
		else if (currentToken == Token::Enum)
			enums.push_back(parseEnumDefinition());
		else if (currentToken == Token::Identifier || currentToken == Token::Mapping ||
				 Token::isElementaryTypeName(currentToken))
		{
			VarDeclParserOptions options;
			options.isStateVariable = true;
			options.allowInitialValue = true;
			stateVariables.push_back(parseVariableDeclaration(options));
			expectToken(Token::Semicolon);
		}
		else if (currentToken == Token::Modifier)
			modifiers.push_back(parseModifierDefinition());
		else if (currentToken == Token::Event)
			events.push_back(parseEventDefinition());
		else
			BOOST_THROW_EXCEPTION(createParserError("Function, variable, struct or modifier declaration expected."));
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<ContractDefinition>(
		name,
		docString,
		baseContracts,
		structs,
		enums,
		stateVariables,
		functions,
		modifiers,
		events
	);
}

ASTPointer<InheritanceSpecifier> Parser::parseInheritanceSpecifier()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Identifier> name(parseIdentifier());
	vector<ASTPointer<Expression>> arguments;
	if (m_scanner->getCurrentToken() == Token::LParen)
	{
		m_scanner->next();
		arguments = parseFunctionCallListArguments();
		nodeFactory.markEndPosition();
		expectToken(Token::RParen);
	}
	else
		nodeFactory.setEndPositionFromNode(name);
	return nodeFactory.createNode<InheritanceSpecifier>(name, arguments);
}

Declaration::Visibility Parser::parseVisibilitySpecifier(Token::Value _token)
{
	Declaration::Visibility visibility(Declaration::Visibility::Default);
	if (_token == Token::Public)
		visibility = Declaration::Visibility::Public;
	else if (_token == Token::Internal)
		visibility = Declaration::Visibility::Internal;
	else if (_token == Token::Private)
		visibility = Declaration::Visibility::Private;
	else if (_token == Token::External)
		visibility = Declaration::Visibility::External;
	else
		solAssert(false, "Invalid visibility specifier.");
	m_scanner->next();
	return visibility;
}

ASTPointer<FunctionDefinition> Parser::parseFunctionDefinition(ASTString const* _contractName)
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docstring;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docstring = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());

	expectToken(Token::Function);
	ASTPointer<ASTString> name;
	if (m_scanner->getCurrentToken() == Token::LParen)
		name = make_shared<ASTString>(); // anonymous function
	else
		name = expectIdentifierToken();
	VarDeclParserOptions options;
	options.allowLocationSpecifier = true;
	ASTPointer<ParameterList> parameters(parseParameterList(options));
	bool isDeclaredConst = false;
	Declaration::Visibility visibility(Declaration::Visibility::Default);
	vector<ASTPointer<ModifierInvocation>> modifiers;
	while (true)
	{
		Token::Value token = m_scanner->getCurrentToken();
		if (token == Token::Const)
		{
			isDeclaredConst = true;
			m_scanner->next();
		}
		else if (token == Token::Identifier)
			modifiers.push_back(parseModifierInvocation());
		else if (Token::isVisibilitySpecifier(token))
		{
			if (visibility != Declaration::Visibility::Default)
				BOOST_THROW_EXCEPTION(createParserError("Multiple visibility specifiers."));
			visibility = parseVisibilitySpecifier(token);
		}
		else
			break;
	}
	ASTPointer<ParameterList> returnParameters;
	if (m_scanner->getCurrentToken() == Token::Returns)
	{
		bool const permitEmptyParameterList = false;
		m_scanner->next();
		returnParameters = parseParameterList(options, permitEmptyParameterList);
	}
	else
		returnParameters = createEmptyParameterList();
	ASTPointer<Block> block = ASTPointer<Block>();
	nodeFactory.markEndPosition();
	if (m_scanner->getCurrentToken() != Token::Semicolon)
	{
		block = parseBlock();
		nodeFactory.setEndPositionFromNode(block);
	}
	else
		m_scanner->next(); // just consume the ';'
	bool const c_isConstructor = (_contractName && *name == *_contractName);
	return nodeFactory.createNode<FunctionDefinition>(name, visibility, c_isConstructor, docstring,
													  parameters, isDeclaredConst, modifiers,
													  returnParameters, block);
}

ASTPointer<StructDefinition> Parser::parseStructDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Struct);
	ASTPointer<ASTString> name = expectIdentifierToken();
	vector<ASTPointer<VariableDeclaration>> members;
	expectToken(Token::LBrace);
	while (m_scanner->getCurrentToken() != Token::RBrace)
	{
		members.push_back(parseVariableDeclaration());
		expectToken(Token::Semicolon);
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<StructDefinition>(name, members);
}

ASTPointer<EnumValue> Parser::parseEnumValue()
{
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<EnumValue>(expectIdentifierToken());
}

ASTPointer<EnumDefinition> Parser::parseEnumDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Enum);
	ASTPointer<ASTString> name = expectIdentifierToken();
	vector<ASTPointer<EnumValue>> members;
	expectToken(Token::LBrace);

	while (m_scanner->getCurrentToken() != Token::RBrace)
	{
		members.push_back(parseEnumValue());
		if (m_scanner->getCurrentToken() == Token::RBrace)
			break;
		expectToken(Token::Comma);
		if (m_scanner->getCurrentToken() != Token::Identifier)
			BOOST_THROW_EXCEPTION(createParserError("Expected Identifier after ','"));
	}

	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<EnumDefinition>(name, members);
}

ASTPointer<VariableDeclaration> Parser::parseVariableDeclaration(
	VarDeclParserOptions const& _options,
	ASTPointer<TypeName> const& _lookAheadArrayType
)
{
	ASTNodeFactory nodeFactory = _lookAheadArrayType ?
		ASTNodeFactory(*this, _lookAheadArrayType) : ASTNodeFactory(*this);
	ASTPointer<TypeName> type;
	if (_lookAheadArrayType)
		type = _lookAheadArrayType;
	else
	{
		type = parseTypeName(_options.allowVar);
		if (type != nullptr)
			nodeFactory.setEndPositionFromNode(type);
	}
	bool isIndexed = false;
	bool isDeclaredConst = false;
	Declaration::Visibility visibility(Declaration::Visibility::Default);
	VariableDeclaration::Location location = VariableDeclaration::Location::Default;
	ASTPointer<ASTString> identifier;

	while (true)
	{
		Token::Value token = m_scanner->getCurrentToken();
		if (_options.isStateVariable && Token::isVariableVisibilitySpecifier(token))
		{
			if (visibility != Declaration::Visibility::Default)
				BOOST_THROW_EXCEPTION(createParserError("Visibility already specified."));
			visibility = parseVisibilitySpecifier(token);
		}
		else
		{
			if (_options.allowIndexed && token == Token::Indexed)
				isIndexed = true;
			else if (token == Token::Const)
				isDeclaredConst = true;
			else if (_options.allowLocationSpecifier && Token::isLocationSpecifier(token))
			{
				if (location != VariableDeclaration::Location::Default)
					BOOST_THROW_EXCEPTION(createParserError("Location already specified."));
				if (!type)
					BOOST_THROW_EXCEPTION(createParserError("Location specifier needs explicit type name."));
				location = (
					token == Token::Memory ?
					VariableDeclaration::Location::Memory :
					VariableDeclaration::Location::Storage
				);
			}
			else
				break;
			m_scanner->next();
		}
	}
	nodeFactory.markEndPosition();

	if (_options.allowEmptyName && m_scanner->getCurrentToken() != Token::Identifier)
	{
		identifier = make_shared<ASTString>("");
		solAssert(type != nullptr, "");
		nodeFactory.setEndPositionFromNode(type);
	}
	else
		identifier = expectIdentifierToken();
	ASTPointer<Expression> value;
	if (_options.allowInitialValue)
	{
		if (m_scanner->getCurrentToken() == Token::Assign)
		{
			m_scanner->next();
			value = parseExpression();
			nodeFactory.setEndPositionFromNode(value);
		}
	}
	return nodeFactory.createNode<VariableDeclaration>(
		type,
		identifier,
		value,
		visibility,
		_options.isStateVariable,
		isIndexed,
		isDeclaredConst,
		location
	);
}

ASTPointer<ModifierDefinition> Parser::parseModifierDefinition()
{
	ScopeGuard resetModifierFlag([this]() { m_insideModifier = false; });
	m_insideModifier = true;

	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docstring;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docstring = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());

	expectToken(Token::Modifier);
	ASTPointer<ASTString> name(expectIdentifierToken());
	ASTPointer<ParameterList> parameters;
	if (m_scanner->getCurrentToken() == Token::LParen)
	{
		VarDeclParserOptions options;
		options.allowIndexed = true;
		options.allowLocationSpecifier = true;
		parameters = parseParameterList(options);
	}
	else
		parameters = createEmptyParameterList();
	ASTPointer<Block> block = parseBlock();
	nodeFactory.setEndPositionFromNode(block);
	return nodeFactory.createNode<ModifierDefinition>(name, docstring, parameters, block);
}

ASTPointer<EventDefinition> Parser::parseEventDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> docstring;
	if (m_scanner->getCurrentCommentLiteral() != "")
		docstring = make_shared<ASTString>(m_scanner->getCurrentCommentLiteral());

	expectToken(Token::Event);
	ASTPointer<ASTString> name(expectIdentifierToken());
	ASTPointer<ParameterList> parameters;
	if (m_scanner->getCurrentToken() == Token::LParen)
	{
		VarDeclParserOptions options;
		options.allowIndexed = true;
		parameters = parseParameterList(options);
	}
	else
		parameters = createEmptyParameterList();
	bool anonymous = false;
	if (m_scanner->getCurrentToken() == Token::Anonymous)
	{
		anonymous = true;
		m_scanner->next();
	}
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<EventDefinition>(name, docstring, parameters, anonymous);
}

ASTPointer<ModifierInvocation> Parser::parseModifierInvocation()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Identifier> name(parseIdentifier());
	vector<ASTPointer<Expression>> arguments;
	if (m_scanner->getCurrentToken() == Token::LParen)
	{
		m_scanner->next();
		arguments = parseFunctionCallListArguments();
		nodeFactory.markEndPosition();
		expectToken(Token::RParen);
	}
	else
		nodeFactory.setEndPositionFromNode(name);
	return nodeFactory.createNode<ModifierInvocation>(name, arguments);
}

ASTPointer<Identifier> Parser::parseIdentifier()
{
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<Identifier>(expectIdentifierToken());
}

ASTPointer<TypeName> Parser::parseTypeName(bool _allowVar)
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<TypeName> type;
	Token::Value token = m_scanner->getCurrentToken();
	if (Token::isElementaryTypeName(token))
	{
		type = ASTNodeFactory(*this).createNode<ElementaryTypeName>(token);
		m_scanner->next();
	}
	else if (token == Token::Var)
	{
		if (!_allowVar)
			BOOST_THROW_EXCEPTION(createParserError("Expected explicit type name."));
		m_scanner->next();
	}
	else if (token == Token::Mapping)
		type = parseMapping();
	else if (token == Token::Identifier)
	{
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.markEndPosition();
		type = nodeFactory.createNode<UserDefinedTypeName>(expectIdentifierToken());
	}
	else
		BOOST_THROW_EXCEPTION(createParserError("Expected type name"));

	if (type)
		// Parse "[...]" postfixes for arrays.
		while (m_scanner->getCurrentToken() == Token::LBrack)
		{
			m_scanner->next();
			ASTPointer<Expression> length;
			if (m_scanner->getCurrentToken() != Token::RBrack)
				length = parseExpression();
			nodeFactory.markEndPosition();
			expectToken(Token::RBrack);
			type = nodeFactory.createNode<ArrayTypeName>(type, length);
		}
	return type;
}

ASTPointer<Mapping> Parser::parseMapping()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Mapping);
	expectToken(Token::LParen);
	if (!Token::isElementaryTypeName(m_scanner->getCurrentToken()))
		BOOST_THROW_EXCEPTION(createParserError("Expected elementary type name for mapping key type"));
	ASTPointer<ElementaryTypeName> keyType;
	keyType = ASTNodeFactory(*this).createNode<ElementaryTypeName>(m_scanner->getCurrentToken());
	m_scanner->next();
	expectToken(Token::Arrow);
	bool const allowVar = false;
	ASTPointer<TypeName> valueType = parseTypeName(allowVar);
	nodeFactory.markEndPosition();
	expectToken(Token::RParen);
	return nodeFactory.createNode<Mapping>(keyType, valueType);
}

ASTPointer<ParameterList> Parser::parseParameterList(
	VarDeclParserOptions const& _options,
	bool _allowEmpty
)
{
	ASTNodeFactory nodeFactory(*this);
	vector<ASTPointer<VariableDeclaration>> parameters;
	VarDeclParserOptions options(_options);
	options.allowEmptyName = true;
	expectToken(Token::LParen);
	if (!_allowEmpty || m_scanner->getCurrentToken() != Token::RParen)
	{
		parameters.push_back(parseVariableDeclaration(options));
		while (m_scanner->getCurrentToken() != Token::RParen)
		{
			expectToken(Token::Comma);
			parameters.push_back(parseVariableDeclaration(options));
		}
	}
	nodeFactory.markEndPosition();
	m_scanner->next();
	return nodeFactory.createNode<ParameterList>(parameters);
}

ASTPointer<Block> Parser::parseBlock()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::LBrace);
	vector<ASTPointer<Statement>> statements;
	while (m_scanner->getCurrentToken() != Token::RBrace)
		statements.push_back(parseStatement());
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<Block>(statements);
}

ASTPointer<Statement> Parser::parseStatement()
{
	ASTPointer<Statement> statement;
	switch (m_scanner->getCurrentToken())
	{
	case Token::If:
		return parseIfStatement();
	case Token::While:
		return parseWhileStatement();
	case Token::For:
		return parseForStatement();
	case Token::LBrace:
		return parseBlock();
		// starting from here, all statements must be terminated by a semicolon
	case Token::Continue:
		statement = ASTNodeFactory(*this).createNode<Continue>();
		m_scanner->next();
		break;
	case Token::Break:
		statement = ASTNodeFactory(*this).createNode<Break>();
		m_scanner->next();
		break;
	case Token::Return:
	{
		ASTNodeFactory nodeFactory(*this);
		ASTPointer<Expression> expression;
		if (m_scanner->next() != Token::Semicolon)
		{
			expression = parseExpression();
			nodeFactory.setEndPositionFromNode(expression);
		}
		statement = nodeFactory.createNode<Return>(expression);
		break;
	}
	case Token::Identifier:
		if (m_insideModifier && m_scanner->getCurrentLiteral() == "_")
		{
			statement = ASTNodeFactory(*this).createNode<PlaceholderStatement>();
			m_scanner->next();
			return statement;
		}
	// fall-through
	default:
		statement = parseSimpleStatement();
	}
	expectToken(Token::Semicolon);
	return statement;
}

ASTPointer<IfStatement> Parser::parseIfStatement()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::If);
	expectToken(Token::LParen);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RParen);
	ASTPointer<Statement> trueBody = parseStatement();
	ASTPointer<Statement> falseBody;
	if (m_scanner->getCurrentToken() == Token::Else)
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
	expectToken(Token::While);
	expectToken(Token::LParen);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RParen);
	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<WhileStatement>(condition, body);
}

ASTPointer<ForStatement> Parser::parseForStatement()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Statement> initExpression;
	ASTPointer<Expression> conditionExpression;
	ASTPointer<ExpressionStatement> loopExpression;
	expectToken(Token::For);
	expectToken(Token::LParen);

	// LTODO: Maybe here have some predicate like peekExpression() instead of checking for semicolon and RParen?
	if (m_scanner->getCurrentToken() != Token::Semicolon)
		initExpression = parseSimpleStatement();
	expectToken(Token::Semicolon);

	if (m_scanner->getCurrentToken() != Token::Semicolon)
		conditionExpression = parseExpression();
	expectToken(Token::Semicolon);

	if (m_scanner->getCurrentToken() != Token::RParen)
		loopExpression = parseExpressionStatement();
	expectToken(Token::RParen);

	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<ForStatement>(initExpression,
												conditionExpression,
												loopExpression,
												body);
}

ASTPointer<Statement> Parser::parseSimpleStatement()
{
	// These two cases are very hard to distinguish:
	// x[7 * 20 + 3] a;  -  x[7 * 20 + 3] = 9;
	// In the first case, x is a type name, in the second it is the name of a variable.
	switch (peekStatementType())
	{
	case LookAheadInfo::VariableDeclarationStatement:
		return parseVariableDeclarationStatement();
	case LookAheadInfo::ExpressionStatement:
		return parseExpressionStatement();
	default:
		break;
	}

	// At this point, we have '(Identifier|ElementaryTypeName) "["'.
	// We parse '(Identifier|ElementaryTypeName) ( "[" Expression "]" )+' and then decide whether to hand this over
	// to ExpressionStatement or create a VariableDeclarationStatement out of it.
	ASTPointer<PrimaryExpression> primary;
	if (m_scanner->getCurrentToken() == Token::Identifier)
		primary = parseIdentifier();
	else
	{
		primary = ASTNodeFactory(*this).createNode<ElementaryTypeNameExpression>(m_scanner->getCurrentToken());
		m_scanner->next();
	}
	vector<pair<ASTPointer<Expression>, SourceLocation>> indices;
	solAssert(m_scanner->getCurrentToken() == Token::LBrack, "");
	SourceLocation indexLocation = primary->getLocation();
	do
	{
		expectToken(Token::LBrack);
		ASTPointer<Expression> index;
		if (m_scanner->getCurrentToken() != Token::RBrack)
			index = parseExpression();
		indexLocation.end = getEndPosition();
		indices.push_back(make_pair(index, indexLocation));
		expectToken(Token::RBrack);
	}
	while (m_scanner->getCurrentToken() == Token::LBrack);

	if (m_scanner->getCurrentToken() == Token::Identifier || Token::isLocationSpecifier(m_scanner->getCurrentToken()))
		return parseVariableDeclarationStatement(typeNameIndexAccessStructure(primary, indices));
	else
		return parseExpressionStatement(expressionFromIndexAccessStructure(primary, indices));
}

ASTPointer<VariableDeclarationStatement> Parser::parseVariableDeclarationStatement(
	ASTPointer<TypeName> const& _lookAheadArrayType)
{
	VarDeclParserOptions options;
	options.allowVar = true;
	options.allowInitialValue = true;
	options.allowLocationSpecifier = true;
	ASTPointer<VariableDeclaration> variable = parseVariableDeclaration(options, _lookAheadArrayType);
	ASTNodeFactory nodeFactory(*this, variable);
	return nodeFactory.createNode<VariableDeclarationStatement>(variable);
}

ASTPointer<ExpressionStatement> Parser::parseExpressionStatement(
	ASTPointer<Expression> const& _lookAheadIndexAccessStructure)
{
	ASTPointer<Expression> expression = parseExpression(_lookAheadIndexAccessStructure);
	return ASTNodeFactory(*this, expression).createNode<ExpressionStatement>(expression);
}

ASTPointer<Expression> Parser::parseExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure)
{
	ASTPointer<Expression> expression = parseBinaryExpression(4, _lookAheadIndexAccessStructure);
	if (!Token::isAssignmentOp(m_scanner->getCurrentToken()))
		return expression;
	Token::Value assignmentOperator = expectAssignmentOperator();
	ASTPointer<Expression> rightHandSide = parseExpression();
	ASTNodeFactory nodeFactory(*this, expression);
	nodeFactory.setEndPositionFromNode(rightHandSide);
	return nodeFactory.createNode<Assignment>(expression, assignmentOperator, rightHandSide);
}

ASTPointer<Expression> Parser::parseBinaryExpression(int _minPrecedence,
	ASTPointer<Expression> const& _lookAheadIndexAccessStructure)
{
	ASTPointer<Expression> expression = parseUnaryExpression(_lookAheadIndexAccessStructure);
	ASTNodeFactory nodeFactory(*this, expression);
	int precedence = Token::precedence(m_scanner->getCurrentToken());
	for (; precedence >= _minPrecedence; --precedence)
		while (Token::precedence(m_scanner->getCurrentToken()) == precedence)
		{
			Token::Value op = m_scanner->getCurrentToken();
			m_scanner->next();
			ASTPointer<Expression> right = parseBinaryExpression(precedence + 1);
			nodeFactory.setEndPositionFromNode(right);
			expression = nodeFactory.createNode<BinaryOperation>(expression, op, right);
		}
	return expression;
}

ASTPointer<Expression> Parser::parseUnaryExpression(
	ASTPointer<Expression> const& _lookAheadIndexAccessStructure)
{
	ASTNodeFactory nodeFactory = _lookAheadIndexAccessStructure ?
		ASTNodeFactory(*this, _lookAheadIndexAccessStructure) : ASTNodeFactory(*this);
	Token::Value token = m_scanner->getCurrentToken();
	if (!_lookAheadIndexAccessStructure && (Token::isUnaryOp(token) || Token::isCountOp(token)))
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
		ASTPointer<Expression> subExpression = parseLeftHandSideExpression(_lookAheadIndexAccessStructure);
		token = m_scanner->getCurrentToken();
		if (!Token::isCountOp(token))
			return subExpression;
		nodeFactory.markEndPosition();
		m_scanner->next();
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, false);
	}
}

ASTPointer<Expression> Parser::parseLeftHandSideExpression(
	ASTPointer<Expression> const& _lookAheadIndexAccessStructure)
{
	ASTNodeFactory nodeFactory = _lookAheadIndexAccessStructure ?
		ASTNodeFactory(*this, _lookAheadIndexAccessStructure) : ASTNodeFactory(*this);

	ASTPointer<Expression> expression;
	if (_lookAheadIndexAccessStructure)
		expression = _lookAheadIndexAccessStructure;
	else if (m_scanner->getCurrentToken() == Token::New)
	{
		expectToken(Token::New);
		ASTPointer<Identifier> contractName(parseIdentifier());
		nodeFactory.setEndPositionFromNode(contractName);
		expression = nodeFactory.createNode<NewExpression>(contractName);
	}
	else
		expression = parsePrimaryExpression();

	while (true)
	{
		switch (m_scanner->getCurrentToken())
		{
		case Token::LBrack:
		{
			m_scanner->next();
			ASTPointer<Expression> index;
			if (m_scanner->getCurrentToken() != Token::RBrack)
				index = parseExpression();
			nodeFactory.markEndPosition();
			expectToken(Token::RBrack);
			expression = nodeFactory.createNode<IndexAccess>(expression, index);
		}
		break;
		case Token::Period:
		{
			m_scanner->next();
			nodeFactory.markEndPosition();
			expression = nodeFactory.createNode<MemberAccess>(expression, expectIdentifierToken());
		}
		break;
		case Token::LParen:
		{
			m_scanner->next();
			vector<ASTPointer<Expression>> arguments;
			vector<ASTPointer<ASTString>> names;
			std::tie(arguments, names) = parseFunctionCallArguments();
			nodeFactory.markEndPosition();
			expectToken(Token::RParen);
			expression = nodeFactory.createNode<FunctionCall>(expression, arguments, names);
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
	case Token::TrueLiteral:
	case Token::FalseLiteral:
		expression = nodeFactory.createNode<Literal>(token, getLiteralAndAdvance());
		break;
	case Token::Number:
		if (Token::isEtherSubdenomination(m_scanner->peekNextToken()))
		{
			ASTPointer<ASTString> literal = getLiteralAndAdvance();
			nodeFactory.markEndPosition();
			Literal::SubDenomination subdenomination = static_cast<Literal::SubDenomination>(m_scanner->getCurrentToken());
			m_scanner->next();
			expression = nodeFactory.createNode<Literal>(token, literal, subdenomination);
			break;
		}
		if (Token::isTimeSubdenomination(m_scanner->peekNextToken()))
		{
			ASTPointer<ASTString> literal = getLiteralAndAdvance();
			nodeFactory.markEndPosition();
			Literal::SubDenomination subdenomination = static_cast<Literal::SubDenomination>(m_scanner->getCurrentToken());
			m_scanner->next();
			expression = nodeFactory.createNode<Literal>(token, literal, subdenomination);
			break;
		}
		// fall-through
	case Token::StringLiteral:
		nodeFactory.markEndPosition();
		expression = nodeFactory.createNode<Literal>(token, getLiteralAndAdvance());
		break;
	case Token::Identifier:
		nodeFactory.markEndPosition();
		expression = nodeFactory.createNode<Identifier>(getLiteralAndAdvance());
		break;
	case Token::LParen:
	{
		m_scanner->next();
		ASTPointer<Expression> expression = parseExpression();
		expectToken(Token::RParen);
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
			BOOST_THROW_EXCEPTION(createParserError("Expected primary expression."));
		break;
	}
	return expression;
}

vector<ASTPointer<Expression>> Parser::parseFunctionCallListArguments()
{
	vector<ASTPointer<Expression>> arguments;
	if (m_scanner->getCurrentToken() != Token::RParen)
	{
		arguments.push_back(parseExpression());
		while (m_scanner->getCurrentToken() != Token::RParen)
		{
			expectToken(Token::Comma);
			arguments.push_back(parseExpression());
		}
	}
	return arguments;
}

pair<vector<ASTPointer<Expression>>, vector<ASTPointer<ASTString>>> Parser::parseFunctionCallArguments()
{
	pair<vector<ASTPointer<Expression>>, vector<ASTPointer<ASTString>>> ret;
	Token::Value token = m_scanner->getCurrentToken();
	if (token == Token::LBrace)
	{
		// call({arg1 : 1, arg2 : 2 })
		expectToken(Token::LBrace);
		while (m_scanner->getCurrentToken() != Token::RBrace)
		{
			ret.second.push_back(expectIdentifierToken());
			expectToken(Token::Colon);
			ret.first.push_back(parseExpression());

			if (m_scanner->getCurrentToken() == Token::Comma)
				expectToken(Token::Comma);
			else
				break;
		}
		expectToken(Token::RBrace);
	}
	else
		ret.first = parseFunctionCallListArguments();
	return ret;
}

Parser::LookAheadInfo Parser::peekStatementType() const
{
	// Distinguish between variable declaration (and potentially assignment) and expression statement
	// (which include assignments to other expressions and pre-declared variables).
	// We have a variable declaration if we get a keyword that specifies a type name.
	// If it is an identifier or an elementary type name followed by an identifier, we also have
	// a variable declaration.
	// If we get an identifier followed by a "[", it can be both ("type[9] a;" or "arr[9] = 7;").
	// In all other cases, we have an expression statement.
	Token::Value token(m_scanner->getCurrentToken());
	bool mightBeTypeName = (Token::isElementaryTypeName(token) || token == Token::Identifier);

	if (token == Token::Mapping || token == Token::Var)
		return LookAheadInfo::VariableDeclarationStatement;
	if (mightBeTypeName)
	{
		Token::Value next = m_scanner->peekNextToken();
		if (next == Token::Identifier || Token::isLocationSpecifier(next))
			return LookAheadInfo::VariableDeclarationStatement;
		if (m_scanner->peekNextToken() == Token::LBrack)
			return LookAheadInfo::IndexAccessStructure;
	}
	return LookAheadInfo::ExpressionStatement;
}

ASTPointer<TypeName> Parser::typeNameIndexAccessStructure(
	ASTPointer<PrimaryExpression> const& _primary, vector<pair<ASTPointer<Expression>, SourceLocation>> const& _indices)
{
	ASTNodeFactory nodeFactory(*this, _primary);
	ASTPointer<TypeName> type;
	if (auto identifier = dynamic_cast<Identifier const*>(_primary.get()))
		type = nodeFactory.createNode<UserDefinedTypeName>(make_shared<ASTString>(identifier->getName()));
	else if (auto typeName = dynamic_cast<ElementaryTypeNameExpression const*>(_primary.get()))
		type = nodeFactory.createNode<ElementaryTypeName>(typeName->getTypeToken());
	else
		solAssert(false, "Invalid type name for array look-ahead.");
	for (auto const& lengthExpression: _indices)
	{
		nodeFactory.setLocation(lengthExpression.second);
		type = nodeFactory.createNode<ArrayTypeName>(type, lengthExpression.first);
	}
	return type;
}

ASTPointer<Expression> Parser::expressionFromIndexAccessStructure(
	ASTPointer<PrimaryExpression> const& _primary, vector<pair<ASTPointer<Expression>, SourceLocation>> const& _indices)
{
	ASTNodeFactory nodeFactory(*this, _primary);
	ASTPointer<Expression> expression(_primary);
	for (auto const& index: _indices)
	{
		nodeFactory.setLocation(index.second);
		expression = nodeFactory.createNode<IndexAccess>(expression, index.first);
	}
	return expression;
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
	if (m_scanner->getCurrentToken() != Token::Identifier)
		BOOST_THROW_EXCEPTION(createParserError("Expected identifier"));
	return getLiteralAndAdvance();
}

ASTPointer<ASTString> Parser::getLiteralAndAdvance()
{
	ASTPointer<ASTString> identifier = make_shared<ASTString>(m_scanner->getCurrentLiteral());
	m_scanner->next();
	return identifier;
}

ASTPointer<ParameterList> Parser::createEmptyParameterList()
{
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.setLocationEmpty();
	return nodeFactory.createNode<ParameterList>(vector<ASTPointer<VariableDeclaration>>());
}

ParserError Parser::createParserError(string const& _description) const
{
	return ParserError() << errinfo_sourceLocation(SourceLocation(getPosition(), getPosition(), getSourceName()))
						 << errinfo_comment(_description);
}


}
}
