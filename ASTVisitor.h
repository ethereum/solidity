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

namespace dev {
namespace solidity {

class ASTVisitor {
public:
	/// These functions are called after a call to ASTNode::accept,
	/// first visit, then (if visit returns true) recursively for all
	/// child nodes in document order (exception for contracts) and then
	/// endVisit.
	virtual bool visit(ASTNode&) { return true; }
	virtual bool visit(ContractDefinition&) { return true; }
	virtual bool visit(StructDefinition&) { return true; }
	virtual bool visit(ParameterList&) { return true; }
	virtual bool visit(FunctionDefinition&) { return true; }
	virtual bool visit(VariableDeclaration&) { return true; }
	virtual bool visit(TypeName&) { return true; }
	virtual bool visit(ElementaryTypeName&) { return true; }
	virtual bool visit(UserDefinedTypeName&) { return true; }
	virtual bool visit(Mapping&) { return true; }
	virtual bool visit(Statement&) { return true; }
	virtual bool visit(Block&) { return true; }
	virtual bool visit(IfStatement&) { return true; }
	virtual bool visit(BreakableStatement&) { return true; }
	virtual bool visit(WhileStatement&) { return true; }
	virtual bool visit(Continue&) { return true; }
	virtual bool visit(Break&) { return true; }
	virtual bool visit(Return&) { return true; }
	virtual bool visit(VariableDefinition&) { return true; }
	virtual bool visit(Expression&) { return true; }
	virtual bool visit(Assignment&) { return true; }
	virtual bool visit(UnaryOperation&) { return true; }
	virtual bool visit(BinaryOperation&) { return true; }
	virtual bool visit(FunctionCall&) { return true; }
	virtual bool visit(MemberAccess&) { return true; }
	virtual bool visit(IndexAccess&) { return true; }
	virtual bool visit(PrimaryExpression&) { return true; }
	virtual bool visit(Identifier&) { return true; }
	virtual bool visit(ElementaryTypeNameExpression&) { return true; }
	virtual bool visit(Literal&) { return true; }

	virtual void endVisit(ASTNode&) { }
	virtual void endVisit(ContractDefinition&) { }
	virtual void endVisit(StructDefinition&) { }
	virtual void endVisit(ParameterList&) { }
	virtual void endVisit(FunctionDefinition&) { }
	virtual void endVisit(VariableDeclaration&) { }
	virtual void endVisit(TypeName&) { }
	virtual void endVisit(ElementaryTypeName&) { }
	virtual void endVisit(UserDefinedTypeName&) { }
	virtual void endVisit(Mapping&) { }
	virtual void endVisit(Statement&) { }
	virtual void endVisit(Block&) { }
	virtual void endVisit(IfStatement&) { }
	virtual void endVisit(BreakableStatement&) { }
	virtual void endVisit(WhileStatement&) { }
	virtual void endVisit(Continue&) { }
	virtual void endVisit(Break&) { }
	virtual void endVisit(Return&) { }
	virtual void endVisit(VariableDefinition&) { }
	virtual void endVisit(Expression&) { }
	virtual void endVisit(Assignment&) { }
	virtual void endVisit(UnaryOperation&) { }
	virtual void endVisit(BinaryOperation&) { }
	virtual void endVisit(FunctionCall&) { }
	virtual void endVisit(MemberAccess&) { }
	virtual void endVisit(IndexAccess&) { }
	virtual void endVisit(PrimaryExpression&) { }
	virtual void endVisit(Identifier&) { }
	virtual void endVisit(ElementaryTypeNameExpression&) { }
	virtual void endVisit(Literal&) { }
};

} }
