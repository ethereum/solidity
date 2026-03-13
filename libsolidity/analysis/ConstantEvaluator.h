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
 * Small drop-in replacement for TypeChecker to evaluate simple expressions of integer and string constants.
 *
 * Note: This always use "checked arithmetic" in the sense that any over- or underflow
 * results in "unknown" value.
 */
class ConstantEvaluator: private ASTConstVisitor
{
public:
	class TypedValue
	{
	public:
		using Value = std::variant<std::monostate, rational, std::string>;

		TypedValue(): m_type(nullptr), m_value(std::monostate()) {}
		TypedValue(Type const* _type, Value _value);
		bool empty() const { return std::holds_alternative<std::monostate>(m_value); }
		bool isString() const { return std::holds_alternative<std::string>(m_value); }
		bool isRational() const { return std::holds_alternative<rational>(m_value); }
		Type const* type() const { return m_type; }
		Value const& value() const { return m_value; }
		std::string const& asString() const;
		rational const& asRational() const;
	private:
		// Type may be RationalType or IntegerType for value rational
		Type const* m_type;
		Value m_value;
	};

	static TypedValue evaluate(
		langutil::ErrorReporter& _errorReporter,
		Expression const& _expr
	);

	/// Works the same as `evaluate` but swallows any errors that might occur in the evaluation and simply returns
	/// `TypedValue` containing `std::monostate` instead.
	static TypedValue tryEvaluate(Expression const& _expr);

	/// Performs arbitrary-precision evaluation of a binary operator. Returns nullopt on cases like
	/// division by zero or e.g. bit operators applied to fractional values.
	static std::optional<rational> evaluateBinaryOperator(Token _operator, rational const& _left, rational const&  _right);

	/// Performs arbitrary-precision evaluation of a unary operator. Returns nullopt on cases like
	/// bit operators applied to fractional values.
	static std::optional<rational> evaluateUnaryOperator(Token _operator, rational const& _input);

private:
	explicit ConstantEvaluator(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	TypedValue evaluate(ASTNode const& _node);

	void endVisit(BinaryOperation const& _operation) override;
	void endVisit(UnaryOperation const& _operation) override;
	void endVisit(Literal const& _literal) override;
	void endVisit(Identifier const& _identifier) override;
	void endVisit(TupleExpression const& _tuple) override;
	void endVisit(FunctionCall const& _functionCall) override;

	langutil::ErrorReporter& m_errorReporter;
	/// Current recursion depth.
	size_t m_depth = 0;
	/// Values of sub-expressions and variable declarations.
	std::map<ASTNode const*, TypedValue> m_values;
};

}
