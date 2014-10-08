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

// Used as pointers to AST nodes, to be replaced by more clever pointers, e.g. pointers which do
// not do reference counting but point to a special memory area that is completely released
// explicitly.
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
    explicit ASTNode(Location const& _location)
        : m_location(_location)
    {}

    Location getLocation() const { return m_location; }
private:
    Location m_location;
};

class ContractDefinition : public ASTNode
{
public:
    ContractDefinition(Location const& _location,
                       std::string const& _name,
                       vecptr<StructDefinition> const& _definedStructs,
                       vecptr<VariableDeclaration> const& _stateVariables,
                       vecptr<FunctionDefinition> const& _definedFunctions)
        : ASTNode(_location), m_name(_name),
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
public:
    StructDefinition(Location const& _location,
                     std::string const& _name,
                     vecptr<VariableDeclaration> const& _members)
        : ASTNode(_location), m_name(_name), m_members(_members)
    {}
private:
    std::string m_name;
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
private:
    vecptr<VariableDeclaration> m_parameters;
};

class FunctionDefinition : public ASTNode
{
public:
    FunctionDefinition(Location const& _location, std::string const& _name, bool _isPublic,
                       ptr<ParameterList> const& _parameters,
                       bool _isDeclaredConst,
                       ptr<ParameterList> const& _returnParameters,
                       ptr<Block> const& _body)
        : ASTNode(_location), m_name(_name), m_isPublic(_isPublic), m_parameters(_parameters),
          m_isDeclaredConst(_isDeclaredConst), m_returnParameters(_returnParameters),
          m_body(_body)
    {}
private:
    std::string m_name;
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
                        std::string const& _name)
        : ASTNode(_location), m_type(_type), m_name(_name)
    {}
private:
    ptr<TypeName> m_type; ///< can be empty ("var")
    std::string m_name;
};

/// types
/// @{

class TypeName : public ASTNode
{
public:
    explicit TypeName(Location const& _location)
        : ASTNode(_location)
    {}
};

/// any pre-defined type that is not a mapping
class ElementaryTypeName : public TypeName
{
public:
    explicit ElementaryTypeName(Location const& _location, Token::Value _type)
        : TypeName(_location), m_type(_type)
    {}
private:
    Token::Value m_type;
};

class UserDefinedTypeName : public TypeName
{
public:
    UserDefinedTypeName(Location const& _location, std::string const& _name)
        : TypeName(_location), m_name(_name)
    {}
private:
    std::string m_name;
};

class Mapping : public TypeName
{
public:
    Mapping(Location const& _location, ptr<ElementaryTypeName> const& _keyType,
            ptr<TypeName> const& _valueType)
        : TypeName(_location), m_keyType(_keyType), m_valueType(_valueType)
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
public:
    explicit Statement(Location const& _location)
        : ASTNode(_location)
    {}
};

class Block : public Statement
{
public:
    explicit Block(Location const& _location)
        : Statement(_location)
    {}
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

/// Can be ordinary function call, type cast or struct construction.
class FunctionCall : public Expression
{
private:
    // if m_functionName is the name of a type, store the token directly
    std::string m_functionName; // "in place" calls of return values are not possible for now
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
