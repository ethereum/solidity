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

#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::langutil;

using TypedRational = ConstantEvaluator::TypedRational;

namespace
{

/// Check whether (_base ** _exp) fits into 4096 bits.
bool fitsPrecisionExp(bigint const& _base, bigint const& _exp)
{
	if (_base == 0)
		return true;

	solAssert(_base > 0, "");

	size_t const bitsMax = 4096;

	size_t mostSignificantBaseBit = static_cast<size_t>(boost::multiprecision::msb(_base));
	if (mostSignificantBaseBit == 0) // _base == 1
		return true;
	if (mostSignificantBaseBit > bitsMax) // _base >= 2 ^ 4096
		return false;

	bigint bitsNeeded = _exp * (mostSignificantBaseBit + 1);

	return bitsNeeded <= bitsMax;
}

/// Checks whether _mantissa * (2 ** _expBase10) fits into 4096 bits.
bool fitsPrecisionBase2(bigint const& _mantissa, uint32_t _expBase2)
{
	return fitsPrecisionBaseX(_mantissa, 1.0, _expBase2);
}

}

optional<rational> ConstantEvaluator::evaluateBinaryOperator(Token _operator, rational const& _left, rational const& _right)
{
	bool fractional = _left.denominator() != 1 || _right.denominator() != 1;
	switch (_operator)
	{
	//bit operations will only be enabled for integers and fixed types that resemble integers
	case Token::BitOr:
		if (fractional)
			return nullopt;
		else
			return _left.numerator() | _right.numerator();
	case Token::BitXor:
		if (fractional)
			return nullopt;
		else
			return _left.numerator() ^ _right.numerator();
	case Token::BitAnd:
		if (fractional)
			return nullopt;
		else
			return _left.numerator() & _right.numerator();
	case Token::Add: return _left + _right;
	case Token::Sub: return _left - _right;
	case Token::Mul: return _left * _right;
	case Token::Div:
		if (_right == rational(0))
			return nullopt;
		else
			return _left / _right;
	case Token::Mod:
		if (_right == rational(0))
			return nullopt;
		else if (fractional)
		{
			rational tempValue = _left / _right;
			return _left - (tempValue.numerator() / tempValue.denominator()) * _right;
		}
		else
			return _left.numerator() % _right.numerator();
		break;
	case Token::Exp:
	{
		if (_right.denominator() != 1)
			return nullopt;
		bigint const& exp = _right.numerator();

		// x ** 0 = 1
		// for 0, 1 and -1 the size of the exponent doesn't have to be restricted
		if (exp == 0)
			return 1;
		else if (_left == 0 || _left == 1)
			return _left;
		else if (_left == -1)
		{
			bigint isOdd = abs(exp) & bigint(1);
			return 1 - 2 * isOdd.convert_to<int>();
		}
		else
		{
			if (abs(exp) > numeric_limits<uint32_t>::max())
				return nullopt; // This will need too much memory to represent.

			uint32_t absExp = bigint(abs(exp)).convert_to<uint32_t>();

			if (!fitsPrecisionExp(abs(_left.numerator()), absExp) || !fitsPrecisionExp(abs(_left.denominator()), absExp))
				return nullopt;

			static auto const optimizedPow = [](bigint const& _base, uint32_t _exponent) -> bigint {
				if (_base == 1)
					return 1;
				else if (_base == -1)
					return 1 - 2 * static_cast<int>(_exponent & 1);
				else
					return boost::multiprecision::pow(_base, _exponent);
			};

			bigint numerator = optimizedPow(_left.numerator(), absExp);
			bigint denominator = optimizedPow(_left.denominator(), absExp);

			if (exp >= 0)
				return makeRational(numerator, denominator);
			else
				// invert
				return makeRational(denominator, numerator);
		}
		break;
	}
	case Token::SHL:
	{
		if (fractional)
			return nullopt;
		else if (_right < 0)
			return nullopt;
		else if (_right > numeric_limits<uint32_t>::max())
			return nullopt;
		if (_left.numerator() == 0)
			return 0;
		else
		{
			uint32_t exponent = _right.numerator().convert_to<uint32_t>();
			if (!fitsPrecisionBase2(abs(_left.numerator()), exponent))
				return nullopt;
			return _left.numerator() * boost::multiprecision::pow(bigint(2), exponent);
		}
		break;
	}
	// NOTE: we're using >> (SAR) to denote right shifting. The type of the LValue
	//       determines the resulting type and the type of shift (SAR or SHR).
	case Token::SAR:
	{
		if (fractional)
			return nullopt;
		else if (_right < 0)
			return nullopt;
		else if (_right > numeric_limits<uint32_t>::max())
			return nullopt;
		if (_left.numerator() == 0)
			return 0;
		else
		{
			uint32_t exponent = _right.numerator().convert_to<uint32_t>();
			if (exponent > boost::multiprecision::msb(boost::multiprecision::abs(_left.numerator())))
				return _left.numerator() < 0 ? -1 : 0;
			else
			{
				if (_left.numerator() < 0)
					// Add 1 to the negative value before dividing to get a result that is strictly too large,
					// then subtract 1 afterwards to round towards negative infinity.
					// This is the same algorithm as used in ExpressionCompiler::appendShiftOperatorCode(...).
					// To see this note that for negative x, xor(x,all_ones) = (-x-1) and
					// therefore xor(div(xor(x,all_ones), exp(2, shift_amount)), all_ones) is
					// -(-x - 1) / 2^shift_amount - 1, which is the same as
					// (x + 1) / 2^shift_amount - 1.
					return rational((_left.numerator() + 1) / boost::multiprecision::pow(bigint(2), exponent) - bigint(1), 1);
				else
					return rational(_left.numerator() / boost::multiprecision::pow(bigint(2), exponent), 1);
			}
		}
		break;
	}
	default:
		return nullopt;
	}
}

optional<rational> ConstantEvaluator::evaluateUnaryOperator(Token _operator, rational const& _input)
{
	switch (_operator)
	{
	case Token::BitNot:
		if (_input.denominator() != 1)
			return nullopt;
		else
			return ~_input.numerator();
	case Token::Sub:
		return -_input;
	default:
		return nullopt;
	}
}

namespace
{

optional<TypedRational> convertType(rational const& _value, Type const& _type)
{
	if (_type.category() == Type::Category::RationalNumber)
		return TypedRational{TypeProvider::rationalNumber(_value), _value};
	else if (auto const* integerType = dynamic_cast<IntegerType const*>(&_type))
	{
		if (_value > integerType->maxValue() || _value < integerType->minValue())
			return nullopt;
		else
			return TypedRational{&_type, _value.numerator() / _value.denominator()};
	}
	else
		return nullopt;
}

optional<TypedRational> convertType(optional<TypedRational> const& _value, Type const& _type)
{
	return _value ? convertType(_value->value, _type) : nullopt;
}

optional<TypedRational> constantToTypedValue(Type const& _type)
{
	if (_type.category() == Type::Category::RationalNumber)
		return TypedRational{&_type, dynamic_cast<RationalNumberType const&>(_type).value()};
	else
		return nullopt;
}

}

optional<TypedRational> ConstantEvaluator::evaluate(
	langutil::ErrorReporter& _errorReporter,
	Expression const& _expr
)
{
	return ConstantEvaluator{_errorReporter}.evaluate(_expr);
}


optional<TypedRational> ConstantEvaluator::evaluate(ASTNode const& _node)
{
	if (!m_values.count(&_node))
	{
		if (auto const* varDecl = dynamic_cast<VariableDeclaration const*>(&_node))
		{
			solAssert(varDecl->isConstant(), "");
			// In some circumstances, we do not yet have a type for the variable.
			if (!varDecl->value() || !varDecl->type())
				m_values[&_node] = nullopt;
			else
			{
				m_depth++;
				if (m_depth > 32)
					m_errorReporter.fatalTypeError(
						5210_error,
						varDecl->location(),
						"Cyclic constant definition (or maximum recursion depth exhausted)."
					);
				m_values[&_node] = convertType(evaluate(*varDecl->value()), *varDecl->type());
				m_depth--;
			}
		}
		else if (auto const* expression = dynamic_cast<Expression const*>(&_node))
		{
			expression->accept(*this);
			if (!m_values.count(&_node))
				m_values[&_node] = nullopt;
		}
	}
	return m_values.at(&_node);
}

void ConstantEvaluator::endVisit(UnaryOperation const& _operation)
{
	optional<TypedRational> value = evaluate(_operation.subExpression());
	if (!value)
		return;

	Type const* resultType = value->type->unaryOperatorResult(_operation.getOperator());
	if (!resultType)
		return;
	value = convertType(value, *resultType);
	if (!value)
		return;

	if (optional<rational> result = evaluateUnaryOperator(_operation.getOperator(), value->value))
	{
		optional<TypedRational> convertedValue = convertType(*result, *resultType);
		if (!convertedValue)
			m_errorReporter.fatalTypeError(
				3667_error,
				_operation.location(),
				"Arithmetic error when computing constant value."
			);
		m_values[&_operation] = convertedValue;
	}
}

void ConstantEvaluator::endVisit(BinaryOperation const& _operation)
{
	optional<TypedRational> left = evaluate(_operation.leftExpression());
	optional<TypedRational> right = evaluate(_operation.rightExpression());
	if (!left || !right)
		return;

	// If this is implemented in the future: Comparison operators have a "binaryOperatorResult"
	// that is non-bool, but the result has to be bool.
	if (TokenTraits::isCompareOp(_operation.getOperator()))
		return;

	Type const* resultType = left->type->binaryOperatorResult(_operation.getOperator(), right->type);
	if (!resultType)
	{
		m_errorReporter.fatalTypeError(
			6020_error,
			_operation.location(),
			"Operator " +
			string(TokenTraits::toString(_operation.getOperator())) +
			" not compatible with types " +
			left->type->toString() +
			" and " +
			right->type->toString()
			);
		return;
	}

	left = convertType(left, *resultType);
	right = convertType(right, *resultType);
	if (!left || !right)
		return;

	if (optional<rational> value = evaluateBinaryOperator(_operation.getOperator(), left->value, right->value))
	{
		optional<TypedRational> convertedValue = convertType(*value, *resultType);
		if (!convertedValue)
			m_errorReporter.fatalTypeError(
				2643_error,
				_operation.location(),
				"Arithmetic error when computing constant value."
			);
		m_values[&_operation] = convertedValue;
	}
}

void ConstantEvaluator::endVisit(Literal const& _literal)
{
	// TODO handle user suffix
	if (Type const* literalType = TypeProvider::forLiteral(_literal))
		m_values[&_literal] = constantToTypedValue(*literalType);
}

void ConstantEvaluator::endVisit(Identifier const& _identifier)
{
	VariableDeclaration const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration);
	if (variableDeclaration && variableDeclaration->isConstant())
		m_values[&_identifier] = evaluate(*variableDeclaration);
}

void ConstantEvaluator::endVisit(TupleExpression const& _tuple)
{
	if (!_tuple.isInlineArray() && _tuple.components().size() == 1)
		m_values[&_tuple] = evaluate(*_tuple.components().front());
}
