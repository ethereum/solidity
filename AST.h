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

#include <libsolidity/BaseTypes.h>
#include <libsolidity/Token.h>

namespace dev {
namespace solidity {

template <class T>
using ptr = std::shared_ptr<T>;
template <class T>
using vecptr = std::vector<ptr<T>>;

class VariableDeclaration;
class StructDefinition;
class FunctionDefinition;
class TypeName;
class Block;
class Expression;

class ASTNode
{
public:
    explicit ASTNode(const Location& _location)
        : m_location(_location)
    {}
private:
    Location m_location;
};

class ContractDefinition : public ASTNode
{
public:
    ContractDefinition(const Location& _location,
                       const std::string& _name,
                       const vecptr<StructDefinition>& _definedStructs,
                       const vecptr<VariableDeclaration>& _stateVariables,
                       const vecptr<FunctionDefinition>& _definedFunctions)
        : ASTNode(_location),
          m_name(_name),
          m_definedStructs(_definedStructs),
          m_stateVariables(_stateVariables),
          m_definedFunctions(_definedFunctions)
    {}

private:
    std::string m_name;
    vecptr<StructDefinition> m_definedStructs;
    vecptr<VariableDeclaration> m_stateVariables;
    vecptr<FunctionDefinition> m_definedFunctions;
};

class StructDefinition : public ASTNode
{
private:
    std::string m_name;
    vecptr<VariableDeclaration> m_members;
};

class FunctionDefinition : public ASTNode
{
private:
    std::string m_name;
    vecptr<VariableDeclaration> m_arguments;
    bool m_isDeclaredConst;
    vecptr<VariableDeclaration> m_returns;
    ptr<Block> m_body;
};

class VariableDeclaration : public ASTNode
{
public:
    VariableDeclaration(const Location& _location,
                        const ptr<TypeName>& _type,
                        const std::string& _name)
        : ASTNode(_location),
          m_type(_type),
          m_name(_name)
    {}
private:
    ptr<TypeName> m_type; ///<s can be empty ("var")
    std::string m_name;
};

/// types
/// @{

class TypeName : public ASTNode
{
public:
    explicit TypeName(const Location& _location)
        : ASTNode(_location)
    {}
};

/// any pre-defined type that is not a mapping
class ElementaryTypeName : public TypeName
{
public:
    explicit ElementaryTypeName(const Location& _location, Token::Value _type)
        : TypeName(_location), m_type(_type)
    {}
private:
    Token::Value m_type;
};

class UserDefinedTypeName : public TypeName
{
public:
    UserDefinedTypeName(const Location& _location, const std::string& _name)
        : TypeName(_location), m_name(_name)
    {}
private:
    std::string m_name;
};

class MappingTypeName : public TypeName
{
public:
    explicit MappingTypeName(const Location& _location)
        : TypeName(_location)
    {}
private:
    ptr<ElementaryTypeName> m_keyType;
    ptr<TypeName> m_valueType;
};

/// @}

/// Statements
/// @{

class Statement : public ASTNode
{
};

class Block : public Statement
{
private:
    vecptr<Statement> m_statements;
};

class IfStatement : public Statement
{

private:
    ptr<Expression> m_condition;
    ptr<Statement> m_trueBody;
    ptr<Statement> m_falseBody;
};

class BreakableStatement : public Statement
{

};

class WhileStatement : public BreakableStatement
{
private:
    ptr<Expression> m_condition;
    ptr<Statement> m_body;
};

class Continue : public Statement
{

};

class Break : public Statement
{

};

class Return : public Statement
{
private:
    ptr<Expression> m_expression;
};

class VariableAssignment : public Statement
{
private:
    ptr<VariableDeclaration> m_variable;
    Token::Value m_assigmentOperator;
    ptr<Expression> m_rightHandSide; ///< can be missing
};

class Expression : public Statement
{
private:
};

/// @}

/// Expressions
/// @{

class Assignment : public Expression
{
private:
    ptr<Expression> m_leftHandSide;
    Token::Value m_assigmentOperator;
    ptr<Expression> m_rightHandSide;
};

class UnaryOperation : public Expression
{
private:
    Token::Value m_operator;
    ptr<Expression> m_subExpression;
    bool isPrefix;
};

class BinaryOperation : public Expression
{
private:
    ptr<Expression> m_left;
    ptr<Expression> m_right;
    Token::Value m_operator;
};

class FunctionCall : public Expression
{
private:
    std::string m_functionName; // TODO only calls to fixed, named functions for now
    vecptr<Expression> m_arguments;
};

class MemberAccess : public Expression
{
private:
    ptr<Expression> m_expression;
    std::string m_memberName;
};

class IndexAccess : public Expression
{
    ptr<Expression> m_base;
    ptr<Expression> m_index;
};

class PrimaryExpression : public Expression
{
};

class Identifier : public PrimaryExpression
{
private:
    std::string m_name;
};

class Literal : public PrimaryExpression
{
private:
    std::string m_value;
};

/// @}


} }
