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

#include <libsolidity/experimental/codegen/IRGenerationContext.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <functional>
#include <sstream>

namespace solidity::frontend::experimental
{
class Analysis;

class IRGeneratorForStatements: public ASTConstVisitor
{
public:
	IRGeneratorForStatements(IRGenerationContext& _context): m_context(_context) {}

	std::string generate(ASTNode const& _node);
private:
	bool visit(ExpressionStatement const& _expressionStatement) override;
	bool visit(Block const& _block) override;
	bool visit(IfStatement const& _ifStatement) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(FunctionCall const& _functionCall) override;
	void endVisit(FunctionCall const& _functionCall) override;
	bool visit(ElementaryTypeNameExpression const& _elementaryTypeNameExpression) override;
	bool visit(MemberAccess const&) override { return true; }
	bool visit(TupleExpression const&) override;
	void endVisit(MemberAccess const& _memberAccess) override;
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(BinaryOperation const&) override { return true; }
	void endVisit(BinaryOperation const& _binaryOperation) override;
	bool visit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	bool visit(Return const&) override { return true; }
	void endVisit(Return const& _return) override;
	/// Default visit will reject all AST nodes that are not explicitly supported.
	bool visitNode(ASTNode const& _node) override;
	IRGenerationContext& m_context;
	std::stringstream m_code;
	enum class Builtins
	{
		Identity,
		FromBool,
		ToBool
	};
	std::map<Expression const*, std::variant<Declaration const*, Builtins>> m_expressionDeclaration;
	Type type(ASTNode const& _node) const;

	FunctionDefinition const& resolveTypeClassFunction(TypeClass _class, std::string _name, Type _type);
	FunctionDefinition const& resolveFunctionExpression(Expression const& _expression, Type _type);
};

}
