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

using namespace std;
using namespace dev;
using namespace dev::solidity;


void ConstantEvaluator::endVisit(UnaryOperation const& _operation)
{
	TypePointer const& subType = _operation.subExpression().annotation().type;
	if (!dynamic_cast<RationalNumberType const*>(subType.get()))
		BOOST_THROW_EXCEPTION(_operation.subExpression().createTypeError("Invalid constant expression."));
	TypePointer t = subType->unaryOperatorResult(_operation.getOperator());
	_operation.annotation().type = t;
}

void ConstantEvaluator::endVisit(BinaryOperation const& _operation)
{
	TypePointer const& leftType = _operation.leftExpression().annotation().type;
	TypePointer const& rightType = _operation.rightExpression().annotation().type;
	if (!dynamic_cast<RationalNumberType const*>(leftType.get()))
		BOOST_THROW_EXCEPTION(_operation.leftExpression().createTypeError("Invalid constant expression."));
	if (!dynamic_cast<RationalNumberType const*>(rightType.get()))
		BOOST_THROW_EXCEPTION(_operation.rightExpression().createTypeError("Invalid constant expression."));
	TypePointer commonType = leftType->binaryOperatorResult(_operation.getOperator(), rightType);
	if (Token::isCompareOp(_operation.getOperator()))
		commonType = make_shared<BoolType>();
	_operation.annotation().type = commonType;
}

void ConstantEvaluator::endVisit(Literal const& _literal)
{
	_literal.annotation().type = Type::forLiteral(_literal);
	if (!_literal.annotation().type)
		BOOST_THROW_EXCEPTION(_literal.createTypeError("Invalid literal value."));
}
