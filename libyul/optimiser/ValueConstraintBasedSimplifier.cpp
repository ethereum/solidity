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
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#include <libyul/optimiser/ValueConstraintBasedSimplifier.h>

#include <libyul/optimiser/Semantics.h>

#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;

namespace
{
class ConstraintDeduction: public boost::static_visitor<ValueConstraint>
{
public:
	explicit ConstraintDeduction(map<YulString, ValueConstraint> const& _variableConstraints):
		m_variableConstraints{_variableConstraints}
	{}

	ValueConstraint visit(Expression const& _expr)
	{
		return boost::apply_visitor(*this, _expr);
	}

	ValueConstraint operator()(Literal const& _literal)
	{
		return ValueConstraint::constant(valueOfLiteral(_literal));
	}

	ValueConstraint operator()(Identifier const& _identifier)
	{
		auto it = m_variableConstraints.find(_identifier.name);
		if (it != m_variableConstraints.end())
			return it->second;
		else
			return ValueConstraint{};
	}

	ValueConstraint operator()(FunctionCall const&)
	{
		return {};
	}

	ValueConstraint operator()(FunctionalInstruction const& _instr)
	{
		vector<Expression> const& args = _instr.arguments;
		switch (_instr.instruction)
		{
		case dev::solidity::Instruction::ADD:
			return visit(args.at(0)) + visit(args.at(1));
		// TODO MUL
		case dev::solidity::Instruction::SUB:
			return visit(args.at(0)) - visit(args.at(1));
		// TODO DIV, SDIV, MOD, SMOD, ADDMOD, MULMOD, EXP, SIGNEXTEND
		case dev::solidity::Instruction::LT:
			return visit(args.at(0)) < visit(args.at(1));
		case dev::solidity::Instruction::GT:
			return visit(args.at(1)) < visit(args.at(0));
		// TODO SLT, SGT
		case dev::solidity::Instruction::EQ:
			return visit(args.at(1)) == visit(args.at(0));
		case dev::solidity::Instruction::ISZERO:
			return visit(args.at(0)) == ValueConstraint::constant(0);
		case dev::solidity::Instruction::AND:
			return visit(args.at(0)) & visit(args.at(1));
		case dev::solidity::Instruction::OR:
			return visit(args.at(0)) | visit(args.at(1));
		// TODO XOR
		case dev::solidity::Instruction::NOT:
			return ~visit(args.at(0));
		case dev::solidity::Instruction::BYTE:
			// TODO Could be more specific.
			return ValueConstraint::bitRange(7, 0);
		case dev::solidity::Instruction::SHL:
			return visit(args.at(1)) << visit(args.at(0));
		case dev::solidity::Instruction::SHR:
			return visit(args.at(1)) >> visit(args.at(0));
		// TODO SAR
		case dev::solidity::Instruction::ADDRESS:
		case dev::solidity::Instruction::ORIGIN:
		case dev::solidity::Instruction::CALLER:
		case dev::solidity::Instruction::COINBASE:
		case dev::solidity::Instruction::CREATE:
		case dev::solidity::Instruction::CREATE2:
			return ValueConstraint::bitRange(159, 0);

		case dev::solidity::Instruction::CALL:
		case dev::solidity::Instruction::CALLCODE:
		case dev::solidity::Instruction::DELEGATECALL:
		case dev::solidity::Instruction::STATICCALL:
			return ValueConstraint::boolean();

		case dev::solidity::Instruction::KECCAK256:
		case dev::solidity::Instruction::EXTCODEHASH:
		case dev::solidity::Instruction::BLOCKHASH:
		case dev::solidity::Instruction::MLOAD:
		case dev::solidity::Instruction::SLOAD:

		// These could be restricted in a real-world setting:
		case dev::solidity::Instruction::BALANCE:
		case dev::solidity::Instruction::CALLVALUE:
		case dev::solidity::Instruction::CALLDATASIZE:
		case dev::solidity::Instruction::GASPRICE:
		case dev::solidity::Instruction::EXTCODESIZE:
		case dev::solidity::Instruction::RETURNDATASIZE:
		case dev::solidity::Instruction::TIMESTAMP:
		case dev::solidity::Instruction::NUMBER:
		case dev::solidity::Instruction::DIFFICULTY:
		case dev::solidity::Instruction::GASLIMIT:
		case dev::solidity::Instruction::PC:
		case dev::solidity::Instruction::MSIZE:
		case dev::solidity::Instruction::GAS:
		default:
			return {};
		}
	}

private:

	map<YulString, ValueConstraint> const& m_variableConstraints;
};
}

ValueConstraint ValueConstraint::constant(u256 const& _value)
{
	return ValueConstraint{_value, _value, _value, _value};
}

ValueConstraint ValueConstraint::boolean()
{
	return ValueConstraint{0, 1, 0, 1};
}

ValueConstraint ValueConstraint::bitRange(size_t _highest, size_t _lowest)
{
	yulAssert(_highest >= _lowest, "");
	return valueFromBits(
		0,
		(u256(-1) >> (255 - _highest)) & ~(u256(-1) >> (256 - _lowest))
	);
}

ValueConstraint ValueConstraint::valueFromBits(u256 _minBits, u256 _maxBits)
{
	// If x fulfills the bit restriction, i.e. minb_i <= x_i <= maxb_i,
	// then by monotonicity sum_i 2^i minb_i <= sum_i 2^i x_i <= sum_i 2^i maxb_i
	// and thus _minBits and _maxBits are also valid value range constraints.
	return ValueConstraint{
		_minBits,
		_maxBits,
		_minBits,
		_maxBits
	};
}

ValueConstraint ValueConstraint::bitsFromValue(u256 _minValue, u256 _maxValue)
{
	// Summary of algorithm: Retain the comon higher order bit prefix between
	// _minValue and _maxValue and starting from the point of divergence,
	// set all following _maxBits bits to 1, and all following _minBits bits to 0.
	// Reasoning: If we count up from _minValue to _maxValue, then the common upper
	// bits will not change.
	int divergence = highestBitSet(_minValue ^ _maxValue);
	u256 lowerBits = u256(-1) >> (255 - divergence);
	u256 minBits = _minValue & (~lowerBits);
	u256 maxBits = _maxValue | lowerBits;
	return ValueConstraint{
		move(_minValue),
		move(_maxValue),
		move(minBits),
		move(maxBits)
	};
}

ValueConstraint ValueConstraint::operator&(ValueConstraint const& _other)
{
	return valueFromBits(
		minBits & _other.minBits,
		maxBits & _other.maxBits
	);
}

ValueConstraint ValueConstraint::operator|(ValueConstraint const& _other)
{
	return valueFromBits(
		minBits | _other.minBits,
		maxBits | _other.maxBits
	);
}

ValueConstraint ValueConstraint::operator~()
{
	return valueFromBits(~maxBits, ~minBits);
}

ValueConstraint ValueConstraint::operator+(ValueConstraint const& _other)
{
	if (bigint(maxValue) + bigint(_other.maxValue) > u256(-1))
		return ValueConstraint{}; // overflow
	else
		return bitsFromValue(
			minValue + _other.minValue,
			maxValue + _other.maxValue
		);
}

ValueConstraint ValueConstraint::operator-(ValueConstraint const& _other)
{
	if (minValue < _other.maxValue)
		return ValueConstraint{}; // underflow
	else
		return bitsFromValue(
			minValue - _other.maxValue,
			maxValue - _other.minValue
		);
}

ValueConstraint ValueConstraint::operator<(ValueConstraint const& _other)
{
	if (maxValue < _other.minValue)
		return constant(1);
	else if (minValue >= _other.maxValue)
		return constant(0);
	else
		return boolean();
}

ValueConstraint ValueConstraint::operator==(ValueConstraint const& _other)
{
	if (minValue == maxValue && minValue == _other.minValue && _other.minValue == _other.maxValue)
		return constant(1);
	else if (minBits == maxBits && minBits == _other.maxBits && _other.minBits == _other.maxBits)
		return constant(1);
	else if (maxValue < _other.minValue || minValue > _other.maxValue)
		return constant(0);
	else if ((maxBits == 0 && _other.minBits == 1) || (minBits == 1 && _other.maxBits == 0))
		return constant(0);
	else
		return boolean();
}

ValueConstraint ValueConstraint::operator<<(ValueConstraint const& _other)
{
	if (boost::optional<u256> c = _other.isConstant())
	{
		if (*c >= 256)
			return constant(0);
		unsigned amount = unsigned(*c);
		return ValueConstraint{
			dev::bigintShiftLeftWorkaround(minValue, amount),
			dev::bigintShiftLeftWorkaround(maxValue, amount),
			dev::bigintShiftLeftWorkaround(minBits, amount),
			dev::bigintShiftLeftWorkaround(maxBits, amount)
		};
	}
	else
	{
		size_t lowestBit = lowestBitSet(maxBits);
		if (lowestBit + bigint(_other.minValue) > 255)
			return constant(0);
		else
			/// TODO could be more accurate, also taking "lastBit set" and _other.maxValue into account.
			return bitRange(255, lowestBit + size_t(_other.minValue));
	}
}

ValueConstraint ValueConstraint::operator>>(ValueConstraint const& _other)
{
	if (boost::optional<u256> c = _other.isConstant())
	{
		if (*c >= 256)
			return constant(0);
		unsigned amount = unsigned(*c);
		return ValueConstraint{
			minValue >> amount,
			maxValue >> amount,
			minBits >> amount,
			maxBits >> amount
		};
	}
	else
	{
		int highestBit = highestBitSet(maxBits);
		if (highestBit - bigint(_other.minValue) < 0)
			return constant(0);
		else
			/// TODO could be more accurate, also taking "lastBit set" and _other.maxValue into account.
			return bitRange(size_t(highestBit - _other.minValue), 0);
	}
}

boost::optional<u256> ValueConstraint::isConstant() const
{
	if (minValue == maxValue)
		return minValue;
	else
		return {};
}

void ValueConstraintBasedSimplifier::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);

	// TODO this runs constraint deduction multiple times on the same sub-expression.

	// Replace movable expressions that are equal to constants by constants.
	if (_expression.type() != typeid(Literal) && MovableChecker(m_dialect, _expression).movable())
	{
		ValueConstraint c = ConstraintDeduction{m_variableConstraints}.visit(_expression);
		if (boost::optional<u256> value = c.isConstant())
			_expression = Literal{
				locationOf(_expression),
				LiteralKind::Number,
				YulString{formatNumber(*value)},
				{}
			};
	}


	// TODO We need more complicated rules that also do things like
	// and(x, 0xff) -> x if x is known to be less than 256
	// These rules might be just rules from the ruleList,
	// where the `feasible` function gets access to the value constraints.
	// It might also be a new RuleList that re-uses the existing classes.
	// We could add another template to SimplificationRule to modify the
	// parameter of the feasibility function, or we capture somehow.

	// Another next step is to add new information in control-flow branches.
	// If such a branch is terminating, also the 'else' branch can
	// get new information.

	// Finally, the ValueConstrainst should be extended such that the upper
	// and lower bounds can be other variables.
}

void ValueConstraintBasedSimplifier::run(Dialect const& _dialect, Block& _ast)
{
	ValueConstraintBasedSimplifier{_dialect}(_ast);
}

void ValueConstraintBasedSimplifier::handleAssignment(set<YulString> const& _names, Expression* _value)
{
	// Determine the constraint before the assignment is performed, because
	// that will change the values of variables.

	ValueConstraint constraint = ValueConstraint::constant(0);
	if (_value)
		constraint = ConstraintDeduction{m_variableConstraints}.visit(*_value);

	// This calls valuesCleared internally, which might clear more values
	// than just _names.
	DataFlowAnalyzer::handleAssignment(_names, _value);

	if (_value)
	{
		if (_names.size() == 1)
			m_variableConstraints[*_names.begin()] = constraint;
	}
	else
		for (auto const& name: _names)
			m_variableConstraints[name] = constraint;
}

void ValueConstraintBasedSimplifier::valuesCleared(set<YulString> const& _names)
{
	for (auto const& name: _names)
		m_variableConstraints.erase(name);
}

