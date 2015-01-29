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
 * AST visitor base class.
 */

#pragma once

#include <libsolidity/ASTForward.h>
#include <string>

namespace dev
{
namespace solidity
{

/**
 * Visitor interface for the abstract syntax tree. This class is tightly bound to the
 * implementation of @ref ASTNode::accept and its overrides. After a call to
 * @ref ASTNode::accept, the function visit for the appropriate parameter is called and then
 * (if it returns true) this continues recursively for all child nodes in document order
 * (there is an exception for contracts). After all child nodes have been visited, endVisit is
 * called for the node.
 */
class ASTVisitor
{
public:
	virtual bool visit(ASTNode&) { return true; }
	virtual bool visit(SourceUnit&) { return true; }
	virtual bool visit(ImportDirective&) { return true; }
	virtual bool visit(ContractDefinition&) { return true; }
	virtual bool visit(InheritanceSpecifier&) { return true; }
	virtual bool visit(StructDefinition&) { return true; }
	virtual bool visit(ParameterList&) { return true; }
	virtual bool visit(FunctionDefinition&) { return true; }
	virtual bool visit(VariableDeclaration&) { return true; }
	virtual bool visit(ModifierDefinition&) { return true; }
	virtual bool visit(ModifierInvocation&) { return true; }
	virtual bool visit(EventDefinition&) { return true; }
	virtual bool visit(TypeName&) { return true; }
	virtual bool visit(ElementaryTypeName&) { return true; }
	virtual bool visit(UserDefinedTypeName&) { return true; }
	virtual bool visit(Mapping&) { return true; }
	virtual bool visit(Statement&) { return true; }
	virtual bool visit(Block&) { return true; }
	virtual bool visit(PlaceholderStatement&) { return true; }
	virtual bool visit(IfStatement&) { return true; }
	virtual bool visit(BreakableStatement&) { return true; }
	virtual bool visit(WhileStatement&) { return true; }
	virtual bool visit(ForStatement&) { return true; }
	virtual bool visit(Continue&) { return true; }
	virtual bool visit(Break&) { return true; }
	virtual bool visit(Return&) { return true; }
	virtual bool visit(VariableDefinition&) { return true; }
	virtual bool visit(ExpressionStatement&) { return true; }
	virtual bool visit(Expression&) { return true; }
	virtual bool visit(Assignment&) { return true; }
	virtual bool visit(UnaryOperation&) { return true; }
	virtual bool visit(BinaryOperation&) { return true; }
	virtual bool visit(FunctionCall&) { return true; }
	virtual bool visit(NewExpression&) { return true; }
	virtual bool visit(MemberAccess&) { return true; }
	virtual bool visit(IndexAccess&) { return true; }
	virtual bool visit(PrimaryExpression&) { return true; }
	virtual bool visit(Identifier&) { return true; }
	virtual bool visit(ElementaryTypeNameExpression&) { return true; }
	virtual bool visit(Literal&) { return true; }

	virtual void endVisit(ASTNode&) { }
	virtual void endVisit(SourceUnit&) { }
	virtual void endVisit(ImportDirective&) { }
	virtual void endVisit(ContractDefinition&) { }
	virtual void endVisit(InheritanceSpecifier&) { }
	virtual void endVisit(StructDefinition&) { }
	virtual void endVisit(ParameterList&) { }
	virtual void endVisit(FunctionDefinition&) { }
	virtual void endVisit(VariableDeclaration&) { }
	virtual void endVisit(ModifierDefinition&) { }
	virtual void endVisit(ModifierInvocation&) { }
	virtual void endVisit(EventDefinition&) { }
	virtual void endVisit(TypeName&) { }
	virtual void endVisit(ElementaryTypeName&) { }
	virtual void endVisit(UserDefinedTypeName&) { }
	virtual void endVisit(Mapping&) { }
	virtual void endVisit(Statement&) { }
	virtual void endVisit(Block&) { }
	virtual void endVisit(PlaceholderStatement&) { }
	virtual void endVisit(IfStatement&) { }
	virtual void endVisit(BreakableStatement&) { }
	virtual void endVisit(WhileStatement&) { }
	virtual void endVisit(ForStatement&) { }
	virtual void endVisit(Continue&) { }
	virtual void endVisit(Break&) { }
	virtual void endVisit(Return&) { }
	virtual void endVisit(VariableDefinition&) { }
	virtual void endVisit(ExpressionStatement&) { }
	virtual void endVisit(Expression&) { }
	virtual void endVisit(Assignment&) { }
	virtual void endVisit(UnaryOperation&) { }
	virtual void endVisit(BinaryOperation&) { }
	virtual void endVisit(FunctionCall&) { }
	virtual void endVisit(NewExpression&) { }
	virtual void endVisit(MemberAccess&) { }
	virtual void endVisit(IndexAccess&) { }
	virtual void endVisit(PrimaryExpression&) { }
	virtual void endVisit(Identifier&) { }
	virtual void endVisit(ElementaryTypeNameExpression&) { }
	virtual void endVisit(Literal&) { }
};

class ASTConstVisitor
{
public:
	virtual bool visit(ASTNode const&) { return true; }
	virtual bool visit(SourceUnit const&) { return true; }
	virtual bool visit(ImportDirective const&) { return true; }
	virtual bool visit(ContractDefinition const&) { return true; }
	virtual bool visit(InheritanceSpecifier const&) { return true; }
	virtual bool visit(StructDefinition const&) { return true; }
	virtual bool visit(ParameterList const&) { return true; }
	virtual bool visit(FunctionDefinition const&) { return true; }
	virtual bool visit(VariableDeclaration const&) { return true; }
	virtual bool visit(ModifierDefinition const&) { return true; }
	virtual bool visit(ModifierInvocation const&) { return true; }
	virtual bool visit(EventDefinition const&) { return true; }
	virtual bool visit(TypeName const&) { return true; }
	virtual bool visit(ElementaryTypeName const&) { return true; }
	virtual bool visit(UserDefinedTypeName const&) { return true; }
	virtual bool visit(Mapping const&) { return true; }
	virtual bool visit(Statement const&) { return true; }
	virtual bool visit(Block const&) { return true; }
	virtual bool visit(PlaceholderStatement const&) { return true; }
	virtual bool visit(IfStatement const&) { return true; }
	virtual bool visit(BreakableStatement const&) { return true; }
	virtual bool visit(WhileStatement const&) { return true; }
	virtual bool visit(ForStatement const&) { return true; }
	virtual bool visit(Continue const&) { return true; }
	virtual bool visit(Break const&) { return true; }
	virtual bool visit(Return const&) { return true; }
	virtual bool visit(VariableDefinition const&) { return true; }
	virtual bool visit(ExpressionStatement const&) { return true; }
	virtual bool visit(Expression const&) { return true; }
	virtual bool visit(Assignment const&) { return true; }
	virtual bool visit(UnaryOperation const&) { return true; }
	virtual bool visit(BinaryOperation const&) { return true; }
	virtual bool visit(FunctionCall const&) { return true; }
	virtual bool visit(NewExpression const&) { return true; }
	virtual bool visit(MemberAccess const&) { return true; }
	virtual bool visit(IndexAccess const&) { return true; }
	virtual bool visit(PrimaryExpression const&) { return true; }
	virtual bool visit(Identifier const&) { return true; }
	virtual bool visit(ElementaryTypeNameExpression const&) { return true; }
	virtual bool visit(Literal const&) { return true; }

	virtual void endVisit(ASTNode const&) { }
	virtual void endVisit(SourceUnit const&) { }
	virtual void endVisit(ImportDirective const&) { }
	virtual void endVisit(ContractDefinition const&) { }
	virtual void endVisit(InheritanceSpecifier const&) { }
	virtual void endVisit(StructDefinition const&) { }
	virtual void endVisit(ParameterList const&) { }
	virtual void endVisit(FunctionDefinition const&) { }
	virtual void endVisit(VariableDeclaration const&) { }
	virtual void endVisit(ModifierDefinition const&) { }
	virtual void endVisit(ModifierInvocation const&) { }
	virtual void endVisit(EventDefinition const&) { }
	virtual void endVisit(TypeName const&) { }
	virtual void endVisit(ElementaryTypeName const&) { }
	virtual void endVisit(UserDefinedTypeName const&) { }
	virtual void endVisit(Mapping const&) { }
	virtual void endVisit(Statement const&) { }
	virtual void endVisit(Block const&) { }
	virtual void endVisit(PlaceholderStatement const&) { }
	virtual void endVisit(IfStatement const&) { }
	virtual void endVisit(BreakableStatement const&) { }
	virtual void endVisit(WhileStatement const&) { }
	virtual void endVisit(ForStatement const&) { }
	virtual void endVisit(Continue const&) { }
	virtual void endVisit(Break const&) { }
	virtual void endVisit(Return const&) { }
	virtual void endVisit(VariableDefinition const&) { }
	virtual void endVisit(ExpressionStatement const&) { }
	virtual void endVisit(Expression const&) { }
	virtual void endVisit(Assignment const&) { }
	virtual void endVisit(UnaryOperation const&) { }
	virtual void endVisit(BinaryOperation const&) { }
	virtual void endVisit(FunctionCall const&) { }
	virtual void endVisit(NewExpression const&) { }
	virtual void endVisit(MemberAccess const&) { }
	virtual void endVisit(IndexAccess const&) { }
	virtual void endVisit(PrimaryExpression const&) { }
	virtual void endVisit(Identifier const&) { }
	virtual void endVisit(ElementaryTypeNameExpression const&) { }
	virtual void endVisit(Literal const&) { }
};

}
}
