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
 * Solidity abstract syntax tree.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include <libsolidity/ASTForward.h>
#include <libsolidity/BaseTypes.h>
#include <libsolidity/Token.h>

namespace dev {
namespace solidity {

class ASTVisitor;

class ASTNode
{
public:
	explicit ASTNode(Location const& _location)
		: m_location(_location)
	{}

	virtual ~ASTNode() {}

	virtual void accept(ASTVisitor& _visitor) = 0;
	template <class T>
	static void listAccept(vecptr<T>& _list, ASTVisitor& _visitor) {
		for (ptr<T>& element : _list) element->accept(_visitor);
	}

	Location const& getLocation() const { return m_location; }
private:
	Location m_location;
};

class ContractDefinition : public ASTNode
{
public:
	ContractDefinition(Location const& _location,
					   ptr<ASTString> const& _name,
					   vecptr<StructDefinition> const& _definedStructs,
					   vecptr<VariableDeclaration> const& _stateVariables,
					   vecptr<FunctionDefinition> const& _definedFunctions)
		: ASTNode(_location), m_name(_name),
		  m_definedStructs(_definedStructs),
		  m_stateVariables(_stateVariables),
		  m_definedFunctions(_definedFunctions)
	{}

	virtual void accept(ASTVisitor& _visitor) override;

	const ASTString& getName() const { return *m_name; }
private:
	ptr<ASTString> m_name;
	vecptr<StructDefinition> m_definedStructs;
	vecptr<VariableDeclaration> m_stateVariables;
	vecptr<FunctionDefinition> m_definedFunctions;
};

class StructDefinition : public ASTNode
{
public:
	StructDefinition(Location const& _location,
					 ptr<ASTString> const& _name,
					 vecptr<VariableDeclaration> const& _members)
		: ASTNode(_location), m_name(_name), m_members(_members)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	const ASTString& getName() const { return *m_name; }
private:
	ptr<ASTString> m_name;
	vecptr<VariableDeclaration> m_members;
};

/// Used as function parameter list and return list
/// None of the parameters is allowed to contain mappings (not even recursively
/// inside structs)
class ParameterList : public ASTNode
{
public:
	ParameterList(Location const& _location, vecptr<VariableDeclaration> const& _parameters)
		: ASTNode(_location), m_parameters(_parameters)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	vecptr<VariableDeclaration> m_parameters;
};

class FunctionDefinition : public ASTNode
{
public:
	FunctionDefinition(Location const& _location, ptr<ASTString> const& _name, bool _isPublic,
					   ptr<ParameterList> const& _parameters,
					   bool _isDeclaredConst,
					   ptr<ParameterList> const& _returnParameters,
					   ptr<Block> const& _body)
		: ASTNode(_location), m_name(_name), m_isPublic(_isPublic), m_parameters(_parameters),
		  m_isDeclaredConst(_isDeclaredConst), m_returnParameters(_returnParameters),
		  m_body(_body)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	const ASTString& getName() const { return *m_name; }
	bool isPublic() const { return m_isPublic; }
	bool isDeclaredConst() const { return m_isDeclaredConst; }
private:
	ptr<ASTString> m_name;
	bool m_isPublic;
	ptr<ParameterList> m_parameters;
	bool m_isDeclaredConst;
	ptr<ParameterList> m_returnParameters;
	ptr<Block> m_body;
};

class VariableDeclaration : public ASTNode
{
public:
	VariableDeclaration(Location const& _location,
						ptr<TypeName> const& _type,
						ptr<ASTString> const& _name)
		: ASTNode(_location), m_type(_type), m_name(_name)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	const ASTString& getName() const { return *m_name; }
private:
	ptr<TypeName> m_type; ///< can be empty ("var")
	ptr<ASTString> m_name;
};

/// types
/// @{

class TypeName : public ASTNode
{
public:
	explicit TypeName(Location const& _location) : ASTNode(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

/// any pre-defined type that is not a mapping
class ElementaryTypeName : public TypeName
{
public:
	explicit ElementaryTypeName(Location const& _location, Token::Value _type)
		: TypeName(_location), m_type(_type)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getType() const { return m_type; }
private:
	Token::Value m_type;
};

class UserDefinedTypeName : public TypeName
{
public:
	UserDefinedTypeName(Location const& _location, ptr<ASTString> const& _name)
		: TypeName(_location), m_name(_name)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	const ASTString& getName() const { return *m_name; }
private:
	ptr<ASTString> m_name;
};

class Mapping : public TypeName
{
public:
	Mapping(Location const& _location, ptr<ElementaryTypeName> const& _keyType,
			ptr<TypeName> const& _valueType)
		: TypeName(_location), m_keyType(_keyType), m_valueType(_valueType)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<ElementaryTypeName> m_keyType;
	ptr<TypeName> m_valueType;
};

/// @}

/// Statements
/// @{

class Statement : public ASTNode
{
public:
	explicit Statement(Location const& _location) : ASTNode(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class Block : public Statement
{
public:
	Block(Location const& _location, vecptr<Statement> const& _statements)
		: Statement(_location), m_statements(_statements)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	vecptr<Statement> m_statements;
};

class IfStatement : public Statement
{
public:
	IfStatement(Location const& _location, ptr<Expression> const& _condition,
				ptr<Statement> const& _trueBody, ptr<Statement> const& _falseBody)
		: Statement(_location), m_condition(_condition),
		  m_trueBody(_trueBody), m_falseBody(_falseBody)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<Expression> m_condition;
	ptr<Statement> m_trueBody;
	ptr<Statement> m_falseBody; //< "else" part, optional
};

class BreakableStatement : public Statement
{
public:
	BreakableStatement(Location const& _location) : Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class WhileStatement : public BreakableStatement
{
public:
	WhileStatement(Location const& _location, ptr<Expression> const& _condition,
				   ptr<Statement> const& _body)
		: BreakableStatement(_location), m_condition(_condition), m_body(_body)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<Expression> m_condition;
	ptr<Statement> m_body;
};

class Continue : public Statement
{
public:
	Continue(Location const& _location) : Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class Break : public Statement
{
public:
	Break(Location const& _location) : Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class Return : public Statement
{
public:
	Return(Location const& _location, ptr<Expression> _expression)
		: Statement(_location), m_expression(_expression)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<Expression> m_expression; //< value to return, optional
};

class VariableDefinition : public Statement
{
public:
	VariableDefinition(Location const& _location, ptr<VariableDeclaration> _variable,
					   ptr<Expression> _value)
		: Statement(_location), m_variable(_variable), m_value(_value)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<VariableDeclaration> m_variable;
	ptr<Expression> m_value; ///< can be missing
};

class Expression : public Statement
{
public:
	Expression(Location const& _location) : Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

/// @}

/// Expressions
/// @{

class Assignment : public Expression
{
public:
	Assignment(Location const& _location, ptr<Expression> const& _leftHandSide,
			   Token::Value _assignmentOperator, ptr<Expression> const& _rightHandSide)
		: Expression(_location), m_leftHandSide(_leftHandSide),
		  m_assigmentOperator(_assignmentOperator), m_rightHandSide(_rightHandSide)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getAssignmentOperator() const { return m_assigmentOperator; }
private:
	ptr<Expression> m_leftHandSide;
	Token::Value m_assigmentOperator;
	ptr<Expression> m_rightHandSide;
};

class UnaryOperation : public Expression
{
public:
	UnaryOperation(Location const& _location, Token::Value _operator,
				   ptr<Expression> const& _subExpression, bool _isPrefix)
		: Expression(_location), m_operator(_operator),
		  m_subExpression(_subExpression), m_isPrefix(_isPrefix)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getOperator() const { return m_operator; }
	bool isPrefixOperation() const { return m_isPrefix; }
private:
	Token::Value m_operator;
	ptr<Expression> m_subExpression;
	bool m_isPrefix;
};

class BinaryOperation : public Expression
{
public:
	BinaryOperation(Location const& _location, ptr<Expression> const& _left,
					Token::Value _operator, ptr<Expression> const& _right)
		: Expression(_location), m_left(_left), m_operator(_operator), m_right(_right)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getOperator() const { return m_operator; }
private:
	ptr<Expression> m_left;
	Token::Value m_operator;
	ptr<Expression> m_right;
};

/// Can be ordinary function call, type cast or struct construction.
class FunctionCall : public Expression
{
public:
	FunctionCall(Location const& _location, ptr<Expression> const& _expression,
				 vecptr<Expression> const& _arguments)
		: Expression(_location), m_expression(_expression), m_arguments(_arguments)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<Expression> m_expression;
	vecptr<Expression> m_arguments;
};

class MemberAccess : public Expression
{
public:
	MemberAccess(Location const& _location, ptr<Expression> _expression,
				 ptr<ASTString> const& _memberName)
		: Expression(_location), m_expression(_expression), m_memberName(_memberName)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	const ASTString& getMemberName() const { return *m_memberName; }
private:
	ptr<Expression> m_expression;
	ptr<ASTString> m_memberName;
};

class IndexAccess : public Expression
{
public:
	IndexAccess(Location const& _location, ptr<Expression> const& _base,
				ptr<Expression> const& _index)
		: Expression(_location), m_base(_base), m_index(_index)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
private:
	ptr<Expression> m_base;
	ptr<Expression> m_index;
};

class PrimaryExpression : public Expression
{
public:
	PrimaryExpression(Location const& _location) : Expression(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class Identifier : public PrimaryExpression
{
public:
	Identifier(Location const& _location, ptr<ASTString> const& _name)
		: PrimaryExpression(_location), m_name(_name) {}
	virtual void accept(ASTVisitor& _visitor) override;

	ASTString const& getName() const { return *m_name; }
private:
	ptr<ASTString> m_name;
};

class ElementaryTypeNameExpression : public PrimaryExpression
{
public:
	ElementaryTypeNameExpression(Location const& _location, Token::Value _type)
		: PrimaryExpression(_location), m_type(_type) {}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getType() const { return m_type; }
private:
	Token::Value m_type;
};

class Literal : public PrimaryExpression
{
public:
	Literal(Location const& _location, Token::Value _token, ptr<ASTString> const& _value)
		: PrimaryExpression(_location), m_token(_token), m_value(_value)
	{}
	virtual void accept(ASTVisitor& _visitor) override;

	Token::Value getToken() const { return m_token; }
	ASTString const& getValue() const { return *m_value; }
private:
	Token::Value m_token;
	ptr<ASTString> m_value;
};

/// @}

} }
