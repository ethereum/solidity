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
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Overflow analyzer and checker.
 */

#pragma once

#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/analysis/IntegerOverflowBounds.h>

namespace dev
{
namespace solidity
{

class ErrorReporter;

/**
 * The module that performs control flow analysis and tests expressions
 * and variables for potential overflow conditions.
 */

class OverflowChecker: private ASTConstVisitor
{
public:
	/// @param _errors the reference to the list of errors and warnings to add them found during type checking.
	OverflowChecker(ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	/// Performs overflow checking on the given contract and all of its sub-nodes.
	void checkOverflow(ASTNode const& _contract);

private:

	virtual bool visit(FunctionDefinition const&) override;
	virtual bool visit(Literal const& _literal) override;
	virtual bool visit(BinaryOperation const& _operation) override;
	virtual bool visit(UnaryOperation const& _operation) override;
	virtual bool visit(VariableDeclaration const& _variable) override;

	virtual bool visit(Assignment const& _statement) override;
	virtual bool visit(Identifier const& _statement) override;
	virtual bool visit(VariableDeclarationStatement const& _statement);
	virtual bool visit(FunctionCall const& _functionCall);
	virtual bool visit(IfStatement const& _ifStatement);

	bool lockConstantExpression(Expression const& _expression);

	ContractDefinition const* m_scope = nullptr;

	ErrorReporter& m_errorReporter;

	std::map<const Expression *, IntegerOverflowBounds> m_overflowInfo;
	std::map<VariableDeclaration const*, IntegerOverflowBounds> m_varOverflowInfo;
	std::map<VariableDeclaration const*, IntegerOverflowBounds> m_constraints;
};

}
}
