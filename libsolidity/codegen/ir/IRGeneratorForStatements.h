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
 * Component that translates Solidity code into Yul at statement level and below.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

namespace dev
{
namespace solidity
{

class IRGenerationContext;
class YulUtilFunctions;

/**
 * Component that translates Solidity's AST into Yul at statement level and below.
 * It is an AST visitor that appends to an internal string buffer.
 */
class IRGeneratorForStatements: public ASTConstVisitor
{
public:
	IRGeneratorForStatements(IRGenerationContext& _context, YulUtilFunctions& _utils):
		m_context(_context),
		m_utils(_utils)
	{}

	std::string code() const { return m_code.str(); }

	bool visit(VariableDeclarationStatement const& _variableDeclaration) override;
	bool visit(Assignment const& _assignment) override;
	bool visit(ForStatement const& _forStatement) override;
	bool visit(Continue const& _continueStatement) override;
	bool visit(Break const& _breakStatement) override;
	bool visit(Return const& _return) override;
	void endVisit(BinaryOperation const& _binOp) override;
	bool visit(FunctionCall const& _funCall) override;
	bool visit(InlineAssembly const& _inlineAsm) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(Literal const& _literal) override;

private:
	/// @returns a Yul expression representing the current value of @a _expression,
	/// converted to type @a _to if it does not yet have that type.
	std::string expressionAsType(Expression const& _expression, Type const& _to);
	std::ostream& defineExpression(Expression const& _expression);

	std::ostringstream m_code;
	IRGenerationContext& m_context;
	YulUtilFunctions& m_utils;
};

}
}
