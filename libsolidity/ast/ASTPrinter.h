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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable), used for debugging.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/GasEstimator.h>
#include <ostream>

namespace dev
{
namespace solidity
{

/**
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable) for debugging purposes.
 */
class ASTPrinter: public ASTConstVisitor
{
public:
	/// Create a printer for the given abstract syntax tree. If the source is specified,
	/// the corresponding parts of the source are printed with each node.
	ASTPrinter(
		ASTNode const& _ast,
		std::string const& _source = std::string(),
		GasEstimator::ASTGasConsumption const& _gasCosts = GasEstimator::ASTGasConsumption()
	);
	/// Output the string representation of the AST to _stream.
	void print(std::ostream& _stream);

	bool visit(PragmaDirective const& _node) override;
	bool visit(ImportDirective const& _node) override;
	bool visit(ContractDefinition const& _node) override;
	bool visit(InheritanceSpecifier const& _node) override;
	bool visit(UsingForDirective const& _node) override;
	bool visit(StructDefinition const& _node) override;
	bool visit(EnumDefinition const& _node) override;
	bool visit(EnumValue const& _node) override;
	bool visit(ParameterList const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	bool visit(VariableDeclaration const& _node) override;
	bool visit(ModifierDefinition const& _node) override;
	bool visit(ModifierInvocation const& _node) override;
	bool visit(EventDefinition const& _node) override;
	bool visit(ElementaryTypeName const& _node) override;
	bool visit(UserDefinedTypeName const& _node) override;
	bool visit(FunctionTypeName const& _node) override;
	bool visit(Mapping const& _node) override;
	bool visit(ArrayTypeName const& _node) override;
	bool visit(InlineAssembly const& _node) override;
	bool visit(Block const& _node) override;
	bool visit(PlaceholderStatement const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	bool visit(Continue const& _node) override;
	bool visit(Break const& _node) override;
	bool visit(Return const& _node) override;
	bool visit(Throw const& _node) override;
	bool visit(EmitStatement const& _node) override;
	bool visit(VariableDeclarationStatement const& _node) override;
	bool visit(ExpressionStatement const& _node) override;
	bool visit(Conditional const& _node) override;
	bool visit(Assignment const& _node) override;
	bool visit(TupleExpression const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	bool visit(FunctionCall const& _node) override;
	bool visit(NewExpression const& _node) override;
	bool visit(MemberAccess const& _node) override;
	bool visit(IndexAccess const& _node) override;
	bool visit(Identifier const& _node) override;
	bool visit(ElementaryTypeNameExpression const& _node) override;
	bool visit(Literal const& _node) override;

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
	void printSourcePart(ASTNode const& _node);
	void printType(Expression const& _expression);
	std::string indentation() const;
	void writeLine(std::string const& _line);
	bool goDeeper() { m_indentation++; return true; }

	int m_indentation;
	std::string m_source;
	ASTNode const* m_ast;
	GasEstimator::ASTGasConsumption m_gasCosts;
	std::ostream* m_ostream = nullptr;
};

}
}
