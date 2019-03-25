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

#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/optimiser/DataFlowAnalyzer.h>

#include <libdevcore/Common.h>

namespace yul
{
struct Dialect;

struct ValueConstraint
{
	static ValueConstraint constant(dev::u256 const& _value);
	static ValueConstraint boolean();
	/// @returns a constraint that does not restrict the bits between
	/// (inclusive) _highest and _lowest bit but forces all other
	/// bits to zero.
	static ValueConstraint bitRange(size_t _highest, size_t _lowest);

	/// Fill the minValue and maxValue fields from the minBits and maxBits fields.
	static ValueConstraint valueFromBits(dev::u256 _minBits, dev::u256 _maxBits);
	/// Fill the minBits and maxBits fields from the minValue and maxValue fields.
	static ValueConstraint bitsFromValue(dev::u256 _minValue, dev::u256 _maxValue);

	ValueConstraint operator&(ValueConstraint const& _other);
	ValueConstraint operator|(ValueConstraint const& _other);
	ValueConstraint operator~();
	ValueConstraint operator+(ValueConstraint const& _other);
	ValueConstraint operator-(ValueConstraint const& _other);
	ValueConstraint operator<(ValueConstraint const& _other);
	ValueConstraint operator==(ValueConstraint const& _other);
	ValueConstraint operator<<(ValueConstraint const& _other);
	ValueConstraint operator>>(ValueConstraint const& _other);

	boost::optional<dev::u256> isConstant() const;

	dev::u256 minValue = 0;
	dev::u256 maxValue = dev::u256(-1);
	dev::u256 minBits = 0; ///< For each 1-bit here, the value's bit is also 1
	dev::u256 maxBits = dev::u256(-1); ///< For each 0-bit here, the value's bit is also 0
};

/**
 * Performs simplifications that take value and bit constraints
 * of variables and expressions into account.
 *
 * Example:
 *
 * let x := and(callvalue(), 0xff)
 * if lt(x, 0x100) { ... }
 *
 * is reduced to
 *
 * let x := and(callvalue(), 0xff)
 * if 1 { ... }
 *
 * because ``x`` is known to be at most 0xff.
 *
 * Most effective if run on code in SSA form.
 *
 * Prerequisite: Disambiguator.
 */
class ValueConstraintBasedSimplifier: public DataFlowAnalyzer
{
public:
	using ASTModifier::operator();

	void visit(Expression& _expression) override;

	static void run(Dialect const& _dialect, Block& _ast);

protected:
	void handleAssignment(std::set<YulString> const& _names, Expression* _value) override;

	void valuesCleared(std::set<YulString> const& _names) override;

	explicit ValueConstraintBasedSimplifier(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

	std::map<YulString, ValueConstraint> m_variableConstraints;
	ValueConstraint m_currentConstraint;
};

}
