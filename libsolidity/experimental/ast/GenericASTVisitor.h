/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <functional>
#include <string>
#include <vector>
#include <utility>

namespace solidity::frontend
{
struct Visit {};
struct EndVisit {};
namespace detail {

template<typename Visitor>
struct Helper: public ASTConstVisitor
{
	Helper(Visitor _visitor): visitor(_visitor) {}
	Visitor visitor;
private:
	template<typename NodeType>
	bool genericVisit(NodeType const& _node)
	{
		return visitor(_node, Visit{});
	}
	template<typename NodeType>
	void genericEndVisit(NodeType const& _node)
	{
		visitor(_node, EndVisit{});
	}
	bool visit(SourceUnit const& _node) override { return genericVisit(_node); }
	bool visit(PragmaDirective const& _node) override { return genericVisit(_node); }
	bool visit(ImportDirective const& _node) override { return genericVisit(_node); }
	bool visit(ContractDefinition const& _node) override { return genericVisit(_node); }
	bool visit(IdentifierPath const& _node) override { return genericVisit(_node); }
	bool visit(InheritanceSpecifier const& _node) override { return genericVisit(_node); }
	bool visit(UsingForDirective const& _node) override { return genericVisit(_node); }
	bool visit(UserDefinedValueTypeDefinition const& _node) override { return genericVisit(_node); }
	bool visit(StructDefinition const& _node) override { return genericVisit(_node); }
	bool visit(EnumDefinition const& _node) override { return genericVisit(_node); }
	bool visit(EnumValue const& _node) override { return genericVisit(_node); }
	bool visit(ParameterList const& _node) override { return genericVisit(_node); }
	bool visit(OverrideSpecifier const& _node) override { return genericVisit(_node); }
	bool visit(FunctionDefinition const& _node) override { return genericVisit(_node); }
	bool visit(VariableDeclaration const& _node) override { return genericVisit(_node); }
	bool visit(ModifierDefinition const& _node) override { return genericVisit(_node); }
	bool visit(ModifierInvocation const& _node) override { return genericVisit(_node); }
	bool visit(EventDefinition const& _node) override { return genericVisit(_node); }
	bool visit(ErrorDefinition const& _node) override { return genericVisit(_node); }
	bool visit(ElementaryTypeName const& _node) override { return genericVisit(_node); }
	bool visit(UserDefinedTypeName const& _node) override { return genericVisit(_node); }
	bool visit(FunctionTypeName const& _node) override { return genericVisit(_node); }
	bool visit(Mapping const& _node) override { return genericVisit(_node); }
	bool visit(ArrayTypeName const& _node) override { return genericVisit(_node); }
	bool visit(InlineAssembly const& _node) override { return genericVisit(_node); }
	bool visit(Block const& _node) override { return genericVisit(_node); }
	bool visit(PlaceholderStatement const& _node) override { return genericVisit(_node); }
	bool visit(IfStatement const& _node) override { return genericVisit(_node); }
	bool visit(TryCatchClause const& _node) override { return genericVisit(_node); }
	bool visit(TryStatement const& _node) override { return genericVisit(_node); }
	bool visit(WhileStatement const& _node) override { return genericVisit(_node); }
	bool visit(ForStatement const& _node) override { return genericVisit(_node); }
	bool visit(Continue const& _node) override { return genericVisit(_node); }
	bool visit(Break const& _node) override { return genericVisit(_node); }
	bool visit(Return const& _node) override { return genericVisit(_node); }
	bool visit(Throw const& _node) override { return genericVisit(_node); }
	bool visit(EmitStatement const& _node) override { return genericVisit(_node); }
	bool visit(RevertStatement const& _node) override { return genericVisit(_node); }
	bool visit(VariableDeclarationStatement const& _node) override { return genericVisit(_node); }
	bool visit(ExpressionStatement const& _node) override { return genericVisit(_node); }
	bool visit(Conditional const& _node) override { return genericVisit(_node); }
	bool visit(Assignment const& _node) override { return genericVisit(_node); }
	bool visit(TupleExpression const& _node) override { return genericVisit(_node); }
	bool visit(UnaryOperation const& _node) override { return genericVisit(_node); }
	bool visit(BinaryOperation const& _node) override { return genericVisit(_node); }
	bool visit(FunctionCall const& _node) override { return genericVisit(_node); }
	bool visit(FunctionCallOptions const& _node) override { return genericVisit(_node); }
	bool visit(NewExpression const& _node) override { return genericVisit(_node); }
	bool visit(MemberAccess const& _node) override { return genericVisit(_node); }
	bool visit(IndexAccess const& _node) override { return genericVisit(_node); }
	bool visit(IndexRangeAccess const& _node) override { return genericVisit(_node); }
	bool visit(Identifier const& _node) override { return genericVisit(_node); }
	bool visit(ElementaryTypeNameExpression const& _node) override { return genericVisit(_node); }
	bool visit(Literal const& _node) override { return genericVisit(_node); }
	bool visit(StructuredDocumentation const& _node) override { return genericVisit(_node); }
	/// Experimental Solidity nodes
	/// @{
	bool visit(TypeClassDefinition const& _node) override { return genericVisit(_node); }
	bool visit(TypeClassInstantiation const& _node) override { return genericVisit(_node); }
	///  @}

	void endVisit(SourceUnit const& _node) override { genericEndVisit(_node); }
	void endVisit(PragmaDirective const& _node) override { genericEndVisit(_node); }
	void endVisit(ImportDirective const& _node) override { genericEndVisit(_node); }
	void endVisit(ContractDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(IdentifierPath const& _node) override { genericEndVisit(_node); }
	void endVisit(InheritanceSpecifier const& _node) override { genericEndVisit(_node); }
	void endVisit(UsingForDirective const& _node) override { genericEndVisit(_node); }
	void endVisit(UserDefinedValueTypeDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(StructDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(EnumDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(EnumValue const& _node) override { genericEndVisit(_node); }
	void endVisit(ParameterList const& _node) override { genericEndVisit(_node); }
	void endVisit(OverrideSpecifier const& _node) override { genericEndVisit(_node); }
	void endVisit(FunctionDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(VariableDeclaration const& _node) override { genericEndVisit(_node); }
	void endVisit(ModifierDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(ModifierInvocation const& _node) override { genericEndVisit(_node); }
	void endVisit(EventDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(ErrorDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit(ElementaryTypeName const& _node) override { genericEndVisit(_node); }
	void endVisit(UserDefinedTypeName const& _node) override { genericEndVisit(_node); }
	void endVisit(FunctionTypeName const& _node) override { genericEndVisit(_node); }
	void endVisit(Mapping const& _node) override { genericEndVisit(_node); }
	void endVisit(ArrayTypeName const& _node) override { genericEndVisit(_node); }
	void endVisit(InlineAssembly const& _node) override { genericEndVisit(_node); }
	void endVisit(Block const& _node) override { genericEndVisit(_node); }
	void endVisit(PlaceholderStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(IfStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(TryCatchClause const& _node) override { genericEndVisit(_node); }
	void endVisit(TryStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(WhileStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(ForStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(Continue const& _node) override { genericEndVisit(_node); }
	void endVisit(Break const& _node) override { genericEndVisit(_node); }
	void endVisit(Return const& _node) override { genericEndVisit(_node); }
	void endVisit(Throw const& _node) override { genericEndVisit(_node); }
	void endVisit(EmitStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(RevertStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(VariableDeclarationStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(ExpressionStatement const& _node) override { genericEndVisit(_node); }
	void endVisit(Conditional const& _node) override { genericEndVisit(_node); }
	void endVisit(Assignment const& _node) override { genericEndVisit(_node); }
	void endVisit(TupleExpression const& _node) override { genericEndVisit(_node); }
	void endVisit(UnaryOperation const& _node) override { genericEndVisit(_node); }
	void endVisit(BinaryOperation const& _node) override { genericEndVisit(_node); }
	void endVisit(FunctionCall const& _node) override { genericEndVisit(_node); }
	void endVisit(FunctionCallOptions const& _node) override { genericEndVisit(_node); }
	void endVisit(NewExpression const& _node) override { genericEndVisit(_node); }
	void endVisit(MemberAccess const& _node) override { genericEndVisit(_node); }
	void endVisit(IndexAccess const& _node) override { genericEndVisit(_node); }
	void endVisit(IndexRangeAccess const& _node) override { genericEndVisit(_node); }
	void endVisit(Identifier const& _node) override { genericEndVisit(_node); }
	void endVisit(ElementaryTypeNameExpression const& _node) override { genericEndVisit(_node); }
	void endVisit(Literal const& _node) override { genericEndVisit(_node); }
	void endVisit(StructuredDocumentation const& _node) override { genericEndVisit(_node); }
	/// Experimental Solidity nodes
	/// @{
	void endVisit( TypeClassDefinition const& _node) override { genericEndVisit(_node); }
	void endVisit( TypeClassInstantiation const& _node) override { genericEndVisit(_node); }
	///  @}
	bool visitNode(ASTNode const&) override { solAssert(false); }
	void endVisitNode(ASTNode const&) override { solAssert(false); }

};

}

template<typename Visitor>
void visitAST(ASTNode const& _astRoot, Visitor&& _visitor)
{
	detail::Helper<Visitor> helper{std::forward<Visitor>(_visitor)};
	_astRoot.accept(helper);
}

}
