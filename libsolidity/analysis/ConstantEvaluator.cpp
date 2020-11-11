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

#include <libsolidity/analysis/ConstantEvaluator.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/Common.h>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::langutil;

using std::optional;
using std::nullopt;
using std::string;

void ConstantEvaluator::endVisit(UnaryOperation const& _operation)
{
	if (auto const sub = result(_operation.subExpression()); sub.has_value())
	{
		auto const res = sub.value().type->unaryOperatorResult(_operation.getOperator());
		if (auto const rationalType = dynamic_cast<RationalNumberType const*>(res.get()))
		{
			auto const subType = sub.value().type;
			if (subType && subType->category() == Type::Category::Integer)
			{
				rational const frac = rationalType->value();
				bigint const num = frac.numerator() / frac.denominator();
				setValue(_operation, rational(num, 1));
			}
			else
				setValue(_operation, rationalType->value());
		}
	}
}

void ConstantEvaluator::endVisit(BinaryOperation const& _operation)
{
	auto left = value(_operation.leftExpression());
	auto right = value(_operation.rightExpression());
	if (left && right)
	{
		TypePointer const commonType = TypeProvider::rationalNumber(*left)->binaryOperatorResult(
			_operation.getOperator(),
			TypeProvider::rationalNumber(*right)
		);

		auto const leftType = result(_operation.leftExpression()).value().type;
		auto const rightType = result(_operation.rightExpression()).value().type;

		if (!commonType)
			m_errorReporter.fatalTypeError(
				6020_error,
				_operation.location(),
				"Operator " +
				string(TokenTraits::toString(_operation.getOperator())) +
				" not compatible with types " +
				leftType->toString() +
				" and " +
				rightType->toString()
			);

		if (auto const rationalCommonType = dynamic_cast<RationalNumberType const*>(commonType))
		{
			if (leftType && leftType->category() == Type::Category::Integer &&
				rightType && rightType->category() == Type::Category::Integer)
			{
				rational const frac = rationalCommonType->value();
				bigint const num = frac.numerator() / frac.denominator();
				setValue(_operation, rational(num, 1));
			}
			else
				setValue(_operation, rationalCommonType->value());
		}

		// other types, such as BoolType are currently impossible to get, and in the old
		// code, have been ignored, too.
		// When we want to widen the constexpr support in Solidity, then we
		// need to touch here, too.
	}
}

void ConstantEvaluator::endVisit(Literal const& _literal)
{
	auto const literalType = TypeProvider::forLiteral(_literal);

	if (auto const p = dynamic_cast<RationalNumberType const*>(literalType))
		setResult(_literal, TypedValue{literalType, p->value()});
}

bool ConstantEvaluator::evaluated(ASTNode const& _node) const noexcept
{
	return m_evaluations.count(&_node) != 0;
}

void ConstantEvaluator::endVisit(Identifier const& _identifier)
{
	VariableDeclaration const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration);
	if (!variableDeclaration)
		return;
	if (!variableDeclaration->isConstant())
		return;

	ASTPointer<Expression> const& value = variableDeclaration->value();
	if (!value)
		return;
	else if (!evaluated(*value))
	{
		if (m_depth > 32)
			m_errorReporter.fatalTypeError(5210_error, _identifier.location(), "Cyclic constant definition (or maximum recursion depth exhausted).");

		evaluate(*value);
	}

	// Link LHS's identifier to the evaluation result of the RHS expression.
	if (auto const resultOpt = result(*value); resultOpt.has_value())
		setResult(_identifier, TypedValue{variableDeclaration->annotation().type, resultOpt.value().value});
}

void ConstantEvaluator::endVisit(TupleExpression const& _tuple) // TODO: do we actually ever need this code path here?
{
	if (!_tuple.isInlineArray() && _tuple.components().size() == 1)
		if (auto v = value(*_tuple.components().front()); v.has_value())
			setValue(_tuple, v.value());
}

void ConstantEvaluator::setResult(ASTNode const& _node, optional<ConstantEvaluator::TypedValue> _result)
{
	if (_result.has_value())
	{
		auto const type = _result.value().type;
		auto const value = _result.value().value;
		m_evaluations[&_node] = {type, value};
	}
}

optional<ConstantEvaluator::TypedValue> ConstantEvaluator::result(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return {p->second};

	return nullopt;
}

TypePointer ConstantEvaluator::type(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return p->second.type;

	return nullptr;
}

optional<rational> ConstantEvaluator::value(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return p->second.value;

	return nullopt;
}

std::optional<rational> ConstantEvaluator::evaluate(langutil::ErrorReporter& _errorReporter, Expression const& _expr)
{
	EvaluationMap evaluations;
	ConstantEvaluator evaluator(_errorReporter, evaluations);
	return evaluator.evaluate(_expr);
}

std::optional<rational> ConstantEvaluator::evaluate(Expression const& _expr)
{
	m_depth++;
	ScopeGuard _([&]() { m_depth--; });

	_expr.accept(*this);

	return value(_expr);
}
