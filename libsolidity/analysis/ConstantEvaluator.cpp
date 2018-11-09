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

#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/ErrorReporter.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

void ConstantEvaluator::endVisit(UnaryOperation const& _operation)
{
	auto sub = type(_operation.subExpression());
	if (sub)
		setType(_operation, sub->unaryOperatorResult(_operation.getOperator()));
}

void ConstantEvaluator::endVisit(BinaryOperation const& _operation)
{
	auto left = type(_operation.leftExpression());
	auto right = type(_operation.rightExpression());
	if (left && right)
	{
		auto commonType = left->binaryOperatorResult(_operation.getOperator(), right);
		if (!commonType)
			m_errorReporter.fatalTypeError(
				_operation.location(),
				"Operator " +
				string(TokenTraits::toString(_operation.getOperator())) +
				" not compatible with types " +
				left->toString() +
				" and " +
				right->toString()
			);
		setType(
			_operation,
			TokenTraits::isCompareOp(_operation.getOperator()) ?
			make_shared<BoolType>() :
			commonType
		);
	}
}

void ConstantEvaluator::endVisit(Literal const& _literal)
{
	setType(_literal, Type::forLiteral(_literal));
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
	else if (!m_types->count(value.get()))
	{
		if (m_depth > 32)
			m_errorReporter.fatalTypeError(_identifier.location(), "Cyclic constant definition (or maximum recursion depth exhausted).");
		ConstantEvaluator(m_errorReporter, m_depth + 1, m_types).evaluate(*value);
	}

	setType(_identifier, type(*value));
}

void ConstantEvaluator::endVisit(TupleExpression const& _tuple)
{
	if (!_tuple.isInlineArray() && _tuple.components().size() == 1)
		setType(_tuple, type(*_tuple.components().front()));
}

void ConstantEvaluator::setType(ASTNode const& _node, TypePointer const& _type)
{
	if (_type && _type->category() == Type::Category::RationalNumber)
		(*m_types)[&_node] = _type;
}

TypePointer ConstantEvaluator::type(ASTNode const& _node)
{
	return (*m_types)[&_node];
}

TypePointer ConstantEvaluator::evaluate(Expression const& _expr)
{
	_expr.accept(*this);
	return type(_expr);
}
