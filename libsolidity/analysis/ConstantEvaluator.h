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
 * @date 2015
 * Evaluator for types of constant expressions.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

namespace langutil
{
class ErrorReporter;
}

namespace dev
{
namespace solidity
{

class TypeChecker;

/**
 * Small drop-in replacement for TypeChecker to evaluate simple expressions of integer constants.
 */
class ConstantEvaluator: private ASTConstVisitor
{
public:
	ConstantEvaluator(
		langutil::ErrorReporter& _errorReporter,
		size_t _newDepth = 0,
		std::shared_ptr<std::map<ASTNode const*, TypePointer>> _types = std::make_shared<std::map<ASTNode const*, TypePointer>>()
	):
		m_errorReporter(_errorReporter),
		m_depth(_newDepth),
		m_types(_types)
	{
	}

	TypePointer evaluate(Expression const& _expr);

private:
	virtual void endVisit(BinaryOperation const& _operation);
	virtual void endVisit(UnaryOperation const& _operation);
	virtual void endVisit(Literal const& _literal);
	virtual void endVisit(Identifier const& _identifier);
	virtual void endVisit(TupleExpression const& _tuple);

	void setType(ASTNode const& _node, TypePointer const& _type);
	TypePointer type(ASTNode const& _node);

	langutil::ErrorReporter& m_errorReporter;
	/// Current recursion depth.
	size_t m_depth = 0;
	std::shared_ptr<std::map<ASTNode const*, TypePointer>> m_types;
};

}
}
