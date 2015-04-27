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

#include <string>
#include <libsolidity/AST.h>

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
	virtual bool visit(SourceUnit& _node) { return visitNode(_node); }
	virtual bool visit(ImportDirective& _node) { return visitNode(_node); }
	virtual bool visit(ContractDefinition& _node) { return visitNode(_node); }
	virtual bool visit(InheritanceSpecifier& _node) { return visitNode(_node); }
	virtual bool visit(StructDefinition& _node) { return visitNode(_node); }
	virtual bool visit(EnumDefinition& _node) { return visitNode(_node); }
	virtual bool visit(EnumValue& _node) { return visitNode(_node); }
	virtual bool visit(ParameterList& _node) { return visitNode(_node); }
	virtual bool visit(FunctionDefinition& _node) { return visitNode(_node); }
	virtual bool visit(VariableDeclaration& _node) { return visitNode(_node); }
	virtual bool visit(ModifierDefinition& _node) { return visitNode(_node); }
	virtual bool visit(ModifierInvocation& _node) { return visitNode(_node); }
	virtual bool visit(EventDefinition& _node) { return visitNode(_node); }
	virtual bool visit(TypeName& _node) { return visitNode(_node); }
	virtual bool visit(ElementaryTypeName& _node) { return visitNode(_node); }
	virtual bool visit(UserDefinedTypeName& _node) { return visitNode(_node); }
	virtual bool visit(Mapping& _node) { return visitNode(_node); }
	virtual bool visit(ArrayTypeName& _node) { return visitNode(_node); }
	virtual bool visit(Block& _node) { return visitNode(_node); }
	virtual bool visit(PlaceholderStatement& _node) { return visitNode(_node); }
	virtual bool visit(IfStatement& _node) { return visitNode(_node); }
	virtual bool visit(WhileStatement& _node) { return visitNode(_node); }
	virtual bool visit(ForStatement& _node) { return visitNode(_node); }
	virtual bool visit(Continue& _node) { return visitNode(_node); }
	virtual bool visit(Break& _node) { return visitNode(_node); }
	virtual bool visit(Return& _node) { return visitNode(_node); }
	virtual bool visit(VariableDeclarationStatement& _node) { return visitNode(_node); }
	virtual bool visit(ExpressionStatement& _node) { return visitNode(_node); }
	virtual bool visit(Assignment& _node) { return visitNode(_node); }
	virtual bool visit(UnaryOperation& _node) { return visitNode(_node); }
	virtual bool visit(BinaryOperation& _node) { return visitNode(_node); }
	virtual bool visit(FunctionCall& _node) { return visitNode(_node); }
	virtual bool visit(NewExpression& _node) { return visitNode(_node); }
	virtual bool visit(MemberAccess& _node) { return visitNode(_node); }
	virtual bool visit(IndexAccess& _node) { return visitNode(_node); }
	virtual bool visit(Identifier& _node) { return visitNode(_node); }
	virtual bool visit(ElementaryTypeNameExpression& _node) { return visitNode(_node); }
	virtual bool visit(Literal& _node) { return visitNode(_node); }

	virtual void endVisit(SourceUnit& _node) { endVisitNode(_node); }
	virtual void endVisit(ImportDirective& _node) { endVisitNode(_node); }
	virtual void endVisit(ContractDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(InheritanceSpecifier& _node) { endVisitNode(_node); }
	virtual void endVisit(StructDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(EnumDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(EnumValue& _node) { endVisitNode(_node); }
	virtual void endVisit(ParameterList& _node) { endVisitNode(_node); }
	virtual void endVisit(FunctionDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(VariableDeclaration& _node) { endVisitNode(_node); }
	virtual void endVisit(ModifierDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(ModifierInvocation& _node) { endVisitNode(_node); }
	virtual void endVisit(EventDefinition& _node) { endVisitNode(_node); }
	virtual void endVisit(TypeName& _node) { endVisitNode(_node); }
	virtual void endVisit(ElementaryTypeName& _node) { endVisitNode(_node); }
	virtual void endVisit(UserDefinedTypeName& _node) { endVisitNode(_node); }
	virtual void endVisit(Mapping& _node) { endVisitNode(_node); }
	virtual void endVisit(ArrayTypeName& _node) { endVisitNode(_node); }
	virtual void endVisit(Block& _node) { endVisitNode(_node); }
	virtual void endVisit(PlaceholderStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(IfStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(WhileStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(ForStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(Continue& _node) { endVisitNode(_node); }
	virtual void endVisit(Break& _node) { endVisitNode(_node); }
	virtual void endVisit(Return& _node) { endVisitNode(_node); }
	virtual void endVisit(VariableDeclarationStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(ExpressionStatement& _node) { endVisitNode(_node); }
	virtual void endVisit(Assignment& _node) { endVisitNode(_node); }
	virtual void endVisit(UnaryOperation& _node) { endVisitNode(_node); }
	virtual void endVisit(BinaryOperation& _node) { endVisitNode(_node); }
	virtual void endVisit(FunctionCall& _node) { endVisitNode(_node); }
	virtual void endVisit(NewExpression& _node) { endVisitNode(_node); }
	virtual void endVisit(MemberAccess& _node) { endVisitNode(_node); }
	virtual void endVisit(IndexAccess& _node) { endVisitNode(_node); }
	virtual void endVisit(Identifier& _node) { endVisitNode(_node); }
	virtual void endVisit(ElementaryTypeNameExpression& _node) { endVisitNode(_node); }
	virtual void endVisit(Literal& _node) { endVisitNode(_node); }

protected:
	/// Generic function called by default for each node, to be overridden by derived classes
	/// if behaviour unspecific to a node type is desired.
	virtual bool visitNode(ASTNode&) { return true; }
	/// Generic function called by default for each node, to be overridden by derived classes
	/// if behaviour unspecific to a node type is desired.
	virtual void endVisitNode(ASTNode&) { }
};

class ASTConstVisitor
{
public:
	virtual bool visit(SourceUnit const& _node) { return visitNode(_node); }
	virtual bool visit(ImportDirective const& _node) { return visitNode(_node); }
	virtual bool visit(ContractDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(InheritanceSpecifier const& _node) { return visitNode(_node); }
	virtual bool visit(StructDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(EnumDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(EnumValue const& _node) { return visitNode(_node); }
	virtual bool visit(ParameterList const& _node) { return visitNode(_node); }
	virtual bool visit(FunctionDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(VariableDeclaration const& _node) { return visitNode(_node); }
	virtual bool visit(ModifierDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(ModifierInvocation const& _node) { return visitNode(_node); }
	virtual bool visit(EventDefinition const& _node) { return visitNode(_node); }
	virtual bool visit(TypeName const& _node) { return visitNode(_node); }
	virtual bool visit(ElementaryTypeName const& _node) { return visitNode(_node); }
	virtual bool visit(UserDefinedTypeName const& _node) { return visitNode(_node); }
	virtual bool visit(Mapping const& _node) { return visitNode(_node); }
	virtual bool visit(ArrayTypeName const& _node) { return visitNode(_node); }
	virtual bool visit(Block const& _node) { return visitNode(_node); }
	virtual bool visit(PlaceholderStatement const& _node) { return visitNode(_node); }
	virtual bool visit(IfStatement const& _node) { return visitNode(_node); }
	virtual bool visit(WhileStatement const& _node) { return visitNode(_node); }
	virtual bool visit(ForStatement const& _node) { return visitNode(_node); }
	virtual bool visit(Continue const& _node) { return visitNode(_node); }
	virtual bool visit(Break const& _node) { return visitNode(_node); }
	virtual bool visit(Return const& _node) { return visitNode(_node); }
	virtual bool visit(VariableDeclarationStatement const& _node) { return visitNode(_node); }
	virtual bool visit(ExpressionStatement const& _node) { return visitNode(_node); }
	virtual bool visit(Assignment const& _node) { return visitNode(_node); }
	virtual bool visit(UnaryOperation const& _node) { return visitNode(_node); }
	virtual bool visit(BinaryOperation const& _node) { return visitNode(_node); }
	virtual bool visit(FunctionCall const& _node) { return visitNode(_node); }
	virtual bool visit(NewExpression const& _node) { return visitNode(_node); }
	virtual bool visit(MemberAccess const& _node) { return visitNode(_node); }
	virtual bool visit(IndexAccess const& _node) { return visitNode(_node); }
	virtual bool visit(Identifier const& _node) { return visitNode(_node); }
	virtual bool visit(ElementaryTypeNameExpression const& _node) { return visitNode(_node); }
	virtual bool visit(Literal const& _node) { return visitNode(_node); }

	virtual void endVisit(SourceUnit const& _node) { endVisitNode(_node); }
	virtual void endVisit(ImportDirective const& _node) { endVisitNode(_node); }
	virtual void endVisit(ContractDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(InheritanceSpecifier const& _node) { endVisitNode(_node); }
	virtual void endVisit(StructDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(EnumDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(EnumValue const& _node) { endVisitNode(_node); }
	virtual void endVisit(ParameterList const& _node) { endVisitNode(_node); }
	virtual void endVisit(FunctionDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(VariableDeclaration const& _node) { endVisitNode(_node); }
	virtual void endVisit(ModifierDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(ModifierInvocation const& _node) { endVisitNode(_node); }
	virtual void endVisit(EventDefinition const& _node) { endVisitNode(_node); }
	virtual void endVisit(TypeName const& _node) { endVisitNode(_node); }
	virtual void endVisit(ElementaryTypeName const& _node) { endVisitNode(_node); }
	virtual void endVisit(UserDefinedTypeName const& _node) { endVisitNode(_node); }
	virtual void endVisit(Mapping const& _node) { endVisitNode(_node); }
	virtual void endVisit(ArrayTypeName const& _node) { endVisitNode(_node); }
	virtual void endVisit(Block const& _node) { endVisitNode(_node); }
	virtual void endVisit(PlaceholderStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(IfStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(WhileStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(ForStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(Continue const& _node) { endVisitNode(_node); }
	virtual void endVisit(Break const& _node) { endVisitNode(_node); }
	virtual void endVisit(Return const& _node) { endVisitNode(_node); }
	virtual void endVisit(VariableDeclarationStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(ExpressionStatement const& _node) { endVisitNode(_node); }
	virtual void endVisit(Assignment const& _node) { endVisitNode(_node); }
	virtual void endVisit(UnaryOperation const& _node) { endVisitNode(_node); }
	virtual void endVisit(BinaryOperation const& _node) { endVisitNode(_node); }
	virtual void endVisit(FunctionCall const& _node) { endVisitNode(_node); }
	virtual void endVisit(NewExpression const& _node) { endVisitNode(_node); }
	virtual void endVisit(MemberAccess const& _node) { endVisitNode(_node); }
	virtual void endVisit(IndexAccess const& _node) { endVisitNode(_node); }
	virtual void endVisit(Identifier const& _node) { endVisitNode(_node); }
	virtual void endVisit(ElementaryTypeNameExpression const& _node) { endVisitNode(_node); }
	virtual void endVisit(Literal const& _node) { endVisitNode(_node); }

protected:
	/// Generic function called by default for each node, to be overridden by derived classes
	/// if behaviour unspecific to a node type is desired.
	virtual bool visitNode(ASTNode const&) { return true; }
	/// Generic function called by default for each node, to be overridden by derived classes
	/// if behaviour unspecific to a node type is desired.
	virtual void endVisitNode(ASTNode const&) { }
};

}
}
