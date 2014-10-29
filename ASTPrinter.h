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
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable), used for debugging.
 */

#pragma once

#include <ostream>
#include <libsolidity/ASTVisitor.h>

namespace dev
{
namespace solidity
{

/**
 * Pretty-printer for the abstract syntax tree (the "pretty" is arguable) for debugging purposes.
 */
class ASTPrinter: public ASTVisitor
{
public:
	/// Create a printer for the given abstract syntax tree. If the source is specified,
	/// the corresponding parts of the source are printed with each node.
	ASTPrinter(ASTPointer<ASTNode> const& _ast, std::string const& _source = std::string());
	/// Output the string representation of the AST to _stream.
	void print(std::ostream& _stream);

	bool visit(ContractDefinition& _node) override;
	bool visit(StructDefinition& _node) override;
	bool visit(ParameterList& _node) override;
	bool visit(FunctionDefinition& _node) override;
	bool visit(VariableDeclaration& _node) override;
	bool visit(TypeName& _node) override;
	bool visit(ElementaryTypeName& _node) override;
	bool visit(UserDefinedTypeName& _node) override;
	bool visit(Mapping& _node) override;
	bool visit(Statement& _node) override;
	bool visit(Block& _node) override;
	bool visit(IfStatement& _node) override;
	bool visit(BreakableStatement& _node) override;
	bool visit(WhileStatement& _node) override;
	bool visit(Continue& _node) override;
	bool visit(Break& _node) override;
	bool visit(Return& _node) override;
	bool visit(VariableDefinition& _node) override;
	bool visit(Expression& _node) override;
	bool visit(Assignment& _node) override;
	bool visit(UnaryOperation& _node) override;
	bool visit(BinaryOperation& _node) override;
	bool visit(FunctionCall& _node) override;
	bool visit(MemberAccess& _node) override;
	bool visit(IndexAccess& _node) override;
	bool visit(PrimaryExpression& _node) override;
	bool visit(Identifier& _node) override;
	bool visit(ElementaryTypeNameExpression& _node) override;
	bool visit(Literal& _node) override;

	void endVisit(ASTNode& _node) override;
	void endVisit(ContractDefinition&) override;
	void endVisit(StructDefinition&) override;
	void endVisit(ParameterList&) override;
	void endVisit(FunctionDefinition&) override;
	void endVisit(VariableDeclaration&) override;
	void endVisit(TypeName&) override;
	void endVisit(ElementaryTypeName&) override;
	void endVisit(UserDefinedTypeName&) override;
	void endVisit(Mapping&) override;
	void endVisit(Statement&) override;
	void endVisit(Block&) override;
	void endVisit(IfStatement&) override;
	void endVisit(BreakableStatement&) override;
	void endVisit(WhileStatement&) override;
	void endVisit(Continue&) override;
	void endVisit(Break&) override;
	void endVisit(Return&) override;
	void endVisit(VariableDefinition&) override;
	void endVisit(Expression&) override;
	void endVisit(Assignment&) override;
	void endVisit(UnaryOperation&) override;
	void endVisit(BinaryOperation&) override;
	void endVisit(FunctionCall&) override;
	void endVisit(MemberAccess&) override;
	void endVisit(IndexAccess&) override;
	void endVisit(PrimaryExpression&) override;
	void endVisit(Identifier&) override;
	void endVisit(ElementaryTypeNameExpression&) override;
	void endVisit(Literal&) override;

private:
	void printSourcePart(ASTNode const& _node);
	void printType(Expression const& _expression);
	std::string getIndentation() const;
	void writeLine(std::string const& _line);
	bool goDeeper() { m_indentation++; return true; }

	int m_indentation;
	std::string m_source;
	ASTPointer<ASTNode> m_ast;
	std::ostream* m_ostream;
};

}
}
