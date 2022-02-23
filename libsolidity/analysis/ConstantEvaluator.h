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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Evaluator for types of constant expressions.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <utility>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

class TypeChecker;

/**
 * Small drop-in replacement for TypeChecker to evaluate simple expressions of integer constants.
 *
 * Note: This always use "checked arithmetic" in the sense that any over- or underflow
 * results in "unknown" value.
 */
class ConstantEvaluator: private ASTConstVisitor
{
public:
	struct TypedRational
	{
		Type const* type;
		rational value;
	};

	static std::optional<TypedRational> evaluate(
		langutil::ErrorReporter& _errorReporter,
		Expression const& _expr
	);

	/// Performs arbitrary-precision evaluation of a binary operator. Returns nullopt on cases like
	/// division by zero or e.g. bit operators applied to fractional values.
	static std::optional<rational> evaluateBinaryOperator(Token _operator, rational const& _left, rational const&  _right);

	/// Performs arbitrary-precision evaluation of a unary operator. Returns nullopt on cases like
	/// bit operators applied to fractional values.
	static std::optional<rational> evaluateUnaryOperator(Token _operator, rational const& _input);

private:
	explicit ConstantEvaluator(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	std::optional<TypedRational> evaluate(ASTNode const& _node);

	void endVisit(BinaryOperation const& _operation) override;
	void endVisit(UnaryOperation const& _operation) override;
	void endVisit(Literal const& _literal) override;
	void endVisit(Identifier const& _identifier) override;
	void endVisit(TupleExpression const& _tuple) override;

	langutil::ErrorReporter& m_errorReporter;
	/// Current recursion depth.
	size_t m_depth = 0;
	/// Values of sub-expressions and variable declarations.
	std::map<ASTNode const*, std::optional<TypedRational>> m_values;
};

}
