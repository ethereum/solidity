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
#include <boost/noncopyable.hpp>
#include <libsolidity/ASTForward.h>
#include <libsolidity/BaseTypes.h>
#include <libsolidity/Token.h>
#include <libsolidity/Types.h>
#include <libsolidity/Exceptions.h>

namespace dev
{
namespace solidity
{

class ASTVisitor;

class ASTNode: private boost::noncopyable
{
public:
	explicit ASTNode(Location const& _location): m_location(_location) {}

	virtual ~ASTNode() {}

	virtual void accept(ASTVisitor& _visitor) = 0;
	template <class T>
	static void listAccept(std::vector<ASTPointer<T>>& _list, ASTVisitor& _visitor)
	{
		for (ASTPointer<T>& element: _list)
			element->accept(_visitor);
	}

	Location const& getLocation() const { return m_location; }

	/// Creates a @ref TypeError exception and decorates it with the location of the node and
	/// the given description
	TypeError createTypeError(std::string const& _description);

private:
	Location m_location;
};

class Declaration: public ASTNode
{
public:
	Declaration(Location const& _location, ASTPointer<ASTString> const& _name):
		ASTNode(_location), m_name(_name) {}

	const ASTString& getName() const { return *m_name; }

private:
	ASTPointer<ASTString> m_name;
};

class ContractDefinition: public Declaration
{
public:
	ContractDefinition(Location const& _location,
					   ASTPointer<ASTString> const& _name,
					   std::vector<ASTPointer<StructDefinition>> const& _definedStructs,
					   std::vector<ASTPointer<VariableDeclaration>> const& _stateVariables,
					   std::vector<ASTPointer<FunctionDefinition>> const& _definedFunctions):
		Declaration(_location, _name),
		m_definedStructs(_definedStructs),
		m_stateVariables(_stateVariables),
		m_definedFunctions(_definedFunctions)
	{}

	virtual void accept(ASTVisitor& _visitor) override;

	std::vector<ASTPointer<StructDefinition>> const& getDefinedStructs() { return m_definedStructs; }
	std::vector<ASTPointer<VariableDeclaration>> const& getStateVariables() { return m_stateVariables; }
	std::vector<ASTPointer<FunctionDefinition>> const& getDefinedFunctions() { return m_definedFunctions; }

private:
	std::vector<ASTPointer<StructDefinition>> m_definedStructs;
	std::vector<ASTPointer<VariableDeclaration>> m_stateVariables;
	std::vector<ASTPointer<FunctionDefinition>> m_definedFunctions;
};

class StructDefinition: public Declaration
{
public:
	StructDefinition(Location const& _location,
					 ASTPointer<ASTString> const& _name,
					 std::vector<ASTPointer<VariableDeclaration>> const& _members):
		Declaration(_location, _name), m_members(_members) {}
	virtual void accept(ASTVisitor& _visitor) override;

private:
	std::vector<ASTPointer<VariableDeclaration>> m_members;
};

/// Used as function parameter list and return list
/// None of the parameters is allowed to contain mappings (not even recursively
/// inside structs)
class ParameterList: public ASTNode
{
public:
	ParameterList(Location const& _location,
				  std::vector<ASTPointer<VariableDeclaration>> const& _parameters):
		ASTNode(_location), m_parameters(_parameters) {}
	virtual void accept(ASTVisitor& _visitor) override;

	std::vector<ASTPointer<VariableDeclaration>> const& getParameters() { return m_parameters; }

private:
	std::vector<ASTPointer<VariableDeclaration>> m_parameters;
};

class FunctionDefinition: public Declaration
{
public:
	FunctionDefinition(Location const& _location, ASTPointer<ASTString> const& _name, bool _isPublic,
					   ASTPointer<ParameterList> const& _parameters,
					   bool _isDeclaredConst,
					   ASTPointer<ParameterList> const& _returnParameters,
					   ASTPointer<Block> const& _body):
		Declaration(_location, _name), m_isPublic(_isPublic), m_parameters(_parameters),
		m_isDeclaredConst(_isDeclaredConst), m_returnParameters(_returnParameters),
		m_body(_body) {}
	virtual void accept(ASTVisitor& _visitor) override;

	bool isPublic() const { return m_isPublic; }
	bool isDeclaredConst() const { return m_isDeclaredConst; }
	std::vector<ASTPointer<VariableDeclaration>> const& getParameters() const { return m_parameters->getParameters(); }
	ParameterList& getParameterList() { return *m_parameters; }
	ASTPointer<ParameterList> const& getReturnParameterList() const { return m_returnParameters; }
	Block& getBody() { return *m_body; }

private:
	bool m_isPublic;
	ASTPointer<ParameterList> m_parameters;
	bool m_isDeclaredConst;
	ASTPointer<ParameterList> m_returnParameters;
	ASTPointer<Block> m_body;
};

class VariableDeclaration: public Declaration
{
public:
	VariableDeclaration(Location const& _location, ASTPointer<TypeName> const& _type,
						ASTPointer<ASTString> const& _name):
		Declaration(_location, _name), m_typeName(_type) {}
	virtual void accept(ASTVisitor& _visitor) override;

	bool isTypeGivenExplicitly() const { return bool(m_typeName); }
	TypeName* getTypeName() const { return m_typeName.get(); }

	//! Returns the declared or inferred type. Can be an empty pointer if no type was explicitly
	//! declared and there is no assignment to the variable that fixes the type.
	std::shared_ptr<Type const> const& getType() const { return m_type; }
	void setType(std::shared_ptr<Type const> const& _type) { m_type = _type; }

private:
	ASTPointer<TypeName> m_typeName; ///< can be empty ("var")

	std::shared_ptr<Type const> m_type;
};

/// types
/// @{

class TypeName: public ASTNode
{
public:
	explicit TypeName(Location const& _location): ASTNode(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;

	virtual std::shared_ptr<Type> toType() = 0;
};

/// any pre-defined type that is not a mapping
class ElementaryTypeName: public TypeName
{
public:
	explicit ElementaryTypeName(Location const& _location, Token::Value _type):
		TypeName(_location), m_type(_type) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual std::shared_ptr<Type> toType() override { return Type::fromElementaryTypeName(m_type); }

	Token::Value getType() const { return m_type; }

private:
	Token::Value m_type;
};

class UserDefinedTypeName: public TypeName
{
public:
	UserDefinedTypeName(Location const& _location, ASTPointer<ASTString> const& _name):
		TypeName(_location), m_name(_name) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual std::shared_ptr<Type> toType() override { return Type::fromUserDefinedTypeName(*this); }

	const ASTString& getName() const { return *m_name; }
	void setReferencedStruct(StructDefinition& _referencedStruct) { m_referencedStruct = &_referencedStruct; }
	StructDefinition const* getReferencedStruct() const { return m_referencedStruct; }

private:
	ASTPointer<ASTString> m_name;

	StructDefinition* m_referencedStruct;
};

class Mapping: public TypeName
{
public:
	Mapping(Location const& _location, ASTPointer<ElementaryTypeName> const& _keyType,
			ASTPointer<TypeName> const& _valueType):
		TypeName(_location), m_keyType(_keyType), m_valueType(_valueType) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual std::shared_ptr<Type> toType() override { return Type::fromMapping(*this); }

private:
	ASTPointer<ElementaryTypeName> m_keyType;
	ASTPointer<TypeName> m_valueType;
};

/// @}

/// Statements
/// @{

class Statement: public ASTNode
{
public:
	explicit Statement(Location const& _location): ASTNode(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;

	//! Check all type requirements, throws exception if some requirement is not met.
	//! For expressions, this also returns the inferred type of the expression. For other
	//! statements, returns the empty pointer.
	virtual void checkTypeRequirements() = 0;

protected:
	//! Check that the inferred type for _expression is _expectedType or at least implicitly
	//! convertible to _expectedType. If not, throw exception.
	void expectType(Expression& _expression, Type const& _expectedType);
};

class Block: public Statement
{
public:
	Block(Location const& _location, std::vector<ASTPointer<Statement>> const& _statements):
		Statement(_location), m_statements(_statements) {}
	virtual void accept(ASTVisitor& _visitor) override;

	virtual void checkTypeRequirements() override;

private:
	std::vector<ASTPointer<Statement>> m_statements;
};

class IfStatement: public Statement
{
public:
	IfStatement(Location const& _location, ASTPointer<Expression> const& _condition,
				ASTPointer<Statement> const& _trueBody, ASTPointer<Statement> const& _falseBody):
		Statement(_location),
		m_condition(_condition), m_trueBody(_trueBody), m_falseBody(_falseBody) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Statement> m_trueBody;
	ASTPointer<Statement> m_falseBody; //< "else" part, optional
};

class BreakableStatement: public Statement
{
public:
	BreakableStatement(Location const& _location): Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
};

class WhileStatement: public BreakableStatement
{
public:
	WhileStatement(Location const& _location, ASTPointer<Expression> const& _condition,
				   ASTPointer<Statement> const& _body):
		BreakableStatement(_location), m_condition(_condition), m_body(_body) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Statement> m_body;
};

class Continue: public Statement
{
public:
	Continue(Location const& _location): Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;
};

class Break: public Statement
{
public:
	Break(Location const& _location): Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;
};

class Return: public Statement
{
public:
	Return(Location const& _location, ASTPointer<Expression> _expression):
		Statement(_location), m_expression(_expression) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	void setFunctionReturnParameters(ParameterList& _parameters) { m_returnParameters = &_parameters; }

private:
	ASTPointer<Expression> m_expression; //< value to return, optional

	ParameterList* m_returnParameters; //< extracted from the function declaration
};

class VariableDefinition: public Statement
{
public:
	VariableDefinition(Location const& _location, ASTPointer<VariableDeclaration> _variable,
					   ASTPointer<Expression> _value):
		Statement(_location), m_variable(_variable), m_value(_value) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<VariableDeclaration> m_variable;
	ASTPointer<Expression> m_value; ///< can be missing
};

class Expression: public Statement
{
public:
	Expression(Location const& _location): Statement(_location) {}
	std::shared_ptr<Type const> const& getType() const { return m_type; }

protected:
	//! Inferred type of the expression, only filled after a call to checkTypeRequirements().
	std::shared_ptr<Type const> m_type;
};

/// @}

/// Expressions
/// @{

class Assignment: public Expression
{
public:
	Assignment(Location const& _location, ASTPointer<Expression> const& _leftHandSide,
			   Token::Value _assignmentOperator, ASTPointer<Expression> const& _rightHandSide):
		Expression(_location), m_leftHandSide(_leftHandSide),
		m_assigmentOperator(_assignmentOperator), m_rightHandSide(_rightHandSide) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	Token::Value getAssignmentOperator() const { return m_assigmentOperator; }

private:
	ASTPointer<Expression> m_leftHandSide;
	Token::Value m_assigmentOperator;
	ASTPointer<Expression> m_rightHandSide;
};

class UnaryOperation: public Expression
{
public:
	UnaryOperation(Location const& _location, Token::Value _operator,
				   ASTPointer<Expression> const& _subExpression, bool _isPrefix):
		Expression(_location), m_operator(_operator),
		m_subExpression(_subExpression), m_isPrefix(_isPrefix) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	Token::Value getOperator() const { return m_operator; }
	bool isPrefixOperation() const { return m_isPrefix; }

private:
	Token::Value m_operator;
	ASTPointer<Expression> m_subExpression;
	bool m_isPrefix;
};

class BinaryOperation: public Expression
{
public:
	BinaryOperation(Location const& _location, ASTPointer<Expression> const& _left,
					Token::Value _operator, ASTPointer<Expression> const& _right):
		Expression(_location), m_left(_left), m_operator(_operator), m_right(_right) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	Token::Value getOperator() const { return m_operator; }

private:
	ASTPointer<Expression> m_left;
	Token::Value m_operator;
	ASTPointer<Expression> m_right;

	std::shared_ptr<Type const> m_commonType;
};

/// Can be ordinary function call, type cast or struct construction.
class FunctionCall: public Expression
{
public:
	FunctionCall(Location const& _location, ASTPointer<Expression> const& _expression,
				 std::vector<ASTPointer<Expression>> const& _arguments):
		Expression(_location), m_expression(_expression), m_arguments(_arguments) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<Expression> m_expression;
	std::vector<ASTPointer<Expression>> m_arguments;
};

class MemberAccess: public Expression
{
public:
	MemberAccess(Location const& _location, ASTPointer<Expression> _expression,
				 ASTPointer<ASTString> const& _memberName):
		Expression(_location), m_expression(_expression), m_memberName(_memberName) {}
	virtual void accept(ASTVisitor& _visitor) override;
	const ASTString& getMemberName() const { return *m_memberName; }
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<Expression> m_expression;
	ASTPointer<ASTString> m_memberName;
};

class IndexAccess: public Expression
{
public:
	IndexAccess(Location const& _location, ASTPointer<Expression> const& _base,
				ASTPointer<Expression> const& _index):
		Expression(_location), m_base(_base), m_index(_index) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

private:
	ASTPointer<Expression> m_base;
	ASTPointer<Expression> m_index;
};

class PrimaryExpression: public Expression
{
public:
	PrimaryExpression(Location const& _location): Expression(_location) {}
};

class Identifier: public PrimaryExpression
{
public:
	Identifier(Location const& _location, ASTPointer<ASTString> const& _name):
		PrimaryExpression(_location), m_name(_name) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	ASTString const& getName() const { return *m_name; }
	void setReferencedDeclaration(Declaration& _referencedDeclaration) { m_referencedDeclaration = &_referencedDeclaration; }
	Declaration* getReferencedDeclaration() { return m_referencedDeclaration; }

private:
	ASTPointer<ASTString> m_name;

	//! Declaration the name refers to.
	Declaration* m_referencedDeclaration;
};

class ElementaryTypeNameExpression: public PrimaryExpression
{
public:
	ElementaryTypeNameExpression(Location const& _location, Token::Value _typeToken):
		PrimaryExpression(_location), m_typeToken(_typeToken) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	Token::Value getTypeToken() const { return m_typeToken; }

private:
	Token::Value m_typeToken;
};

class Literal: public PrimaryExpression
{
public:
	Literal(Location const& _location, Token::Value _token, ASTPointer<ASTString> const& _value):
		PrimaryExpression(_location), m_token(_token), m_value(_value) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void checkTypeRequirements() override;

	Token::Value getToken() const { return m_token; }
	ASTString const& getValue() const { return *m_value; }

private:
	Token::Value m_token;
	ASTPointer<ASTString> m_value;
};

/// @}

}
}
