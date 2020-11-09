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

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::langutil;

void ConstantEvaluator::endVisit(UnaryOperation const& _operation)
{
	auto sub = evaluatedValue(_operation.subExpression());
	if (sub)
		setValue(_operation, sub->unaryOperatorResult(_operation.getOperator()));
}

void ConstantEvaluator::endVisit(BinaryOperation const& _operation)
{
	auto left = evaluatedValue(_operation.leftExpression());
	auto right = evaluatedValue(_operation.rightExpression());
	if (left && right)
	{
		TypePointer commonType = left->binaryOperatorResult(_operation.getOperator(), right);
		if (!commonType)
			m_errorReporter.fatalTypeError(
				6020_error,
				_operation.location(),
				"Operator " +
				string(TokenTraits::toString(_operation.getOperator())) +
				" not compatible with types " +
				left->toString() +
				" and " +
				right->toString()
			);
		setValue(
			_operation,
			TokenTraits::isCompareOp(_operation.getOperator()) ?
			TypeProvider::boolean() :
			commonType
		);
	}
}

void ConstantEvaluator::endVisit(Literal const& _literal)
{
	setValue(_literal, TypeProvider::forLiteral(_literal));
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
	setResult(_identifier, result(*value));
}

void ConstantEvaluator::endVisit(TupleExpression const& _tuple) // TODO: do we actually ever need this code path here?
{
	if (!_tuple.isInlineArray() && _tuple.components().size() == 1)
		setValue(_tuple, evaluatedValue(*_tuple.components().front()));
}

void ConstantEvaluator::setValue(ASTNode const& _node, TypePointer const& _value)
{
	setResult(_node, TypedValue{_value, _value});
}

void ConstantEvaluator::setResult(ASTNode const& _node, optional<ConstantEvaluator::TypedValue> _result)
{
	if (_result.has_value())
	{
		auto const sourceType = _result.value().sourceType;
		auto const value = _result.value().evaluatedValue;
		if (value && value->category() == Type::Category::RationalNumber)
			m_evaluations[&_node] = {sourceType, value};
	}
}

optional<ConstantEvaluator::TypedValue> ConstantEvaluator::result(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return {p->second};

	return nullopt;
}

TypePointer ConstantEvaluator::sourceType(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return p->second.sourceType;

	return nullptr;
}

TypePointer ConstantEvaluator::evaluatedValue(ASTNode const& _node)
{
	if (auto p = m_evaluations.find(&_node); p != m_evaluations.end())
		return p->second.evaluatedValue;

	return nullptr;
}

TypePointer ConstantEvaluator::evaluate(langutil::ErrorReporter& _errorReporter, Expression const& _expr)
{
	EvaluationMap evaluations;
	ConstantEvaluator evaluator(_errorReporter, evaluations);
	return evaluator.evaluate(_expr);
}

TypePointer ConstantEvaluator::evaluate(Expression const& _expr)
{
	m_depth++;
	ScopeGuard _([&]() { m_depth--; });

	_expr.accept(*this);

	return evaluatedValue(_expr);
}
