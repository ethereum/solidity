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

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <map>

namespace dev
{
namespace solidity
{

class ASTCopier: public ASTConstVisitor
{
public:
	virtual ~ASTCopier() = default;
	ASTPointer<SourceUnit> copy(ASTNode const& _ast);

	void endVisit(SourceUnit const&) override;
	void endVisit(PragmaDirective const&) override;
	void endVisit(ImportDirective const&) override;
	void endVisit(ContractDefinition const&) override;
	void endVisit(InheritanceSpecifier const&) override;
	void endVisit(UsingForDirective const&) override;
	void endVisit(StructDefinition const&) override;
	void endVisit(EnumDefinition const&) override;
	void endVisit(EnumValue const&) override;
	void endVisit(ParameterList const&) override;
	void endVisit(FunctionDefinition const&) override;
	void endVisit(VariableDeclaration const&) override;
	void endVisit(ModifierDefinition const&) override;
	void endVisit(ModifierInvocation const&) override;
	void endVisit(EventDefinition const&) override;
	void endVisit(ElementaryTypeName const&) override;
	void endVisit(UserDefinedTypeName const&) override;
	void endVisit(FunctionTypeName const&) override;
	void endVisit(Mapping const&) override;
	void endVisit(ArrayTypeName const&) override;
	void endVisit(InlineAssembly const&) override;
	void endVisit(Block const&) override;
	void endVisit(PlaceholderStatement const&) override;
	void endVisit(IfStatement const&) override;
	void endVisit(WhileStatement const&) override;
	void endVisit(ForStatement const&) override;
	void endVisit(Continue const&) override;
	void endVisit(Break const&) override;
	void endVisit(Return const&) override;
	void endVisit(Throw const&) override;
	void endVisit(EmitStatement const&) override;
	void endVisit(VariableDeclarationStatement const&) override;
	void endVisit(ExpressionStatement const&) override;
	void endVisit(Conditional const&) override;
	void endVisit(Assignment const&) override;
	void endVisit(TupleExpression const&) override;
	void endVisit(UnaryOperation const&) override;
	void endVisit(BinaryOperation const&) override;
	void endVisit(FunctionCall const&) override;
	void endVisit(NewExpression const&) override;
	void endVisit(MemberAccess const&) override;
	void endVisit(IndexAccess const&) override;
	void endVisit(Identifier const&) override;
	void endVisit(ElementaryTypeNameExpression const&) override;
	void endVisit(Literal const&) override;

private:
	void applySubstitution(ASTNode const& _node, ASTPointer<ASTNode> _new);
	ASTPointer<ASTNode> substitute(ASTNode const* _node);
	ASTPointer<ASTString> documentation(Documented const* _node);
	std::map<ASTNode const*, ASTPointer<ASTNode>> m_substitutions;
};

}
}
