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

#include <libsolidity/formal/SymbolicIntVariable.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicIntVariable::SymbolicIntVariable(
	Type const& _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(_type, _uniqueName, _interface)
{
	solAssert(
		_type.category() == Type::Category::Integer ||
		_type.category() == Type::Category::Address,
		""
	);
}

smt::Expression SymbolicIntVariable::valueAtSequence(int _seq) const
{
	return m_interface.newInteger(uniqueSymbol(_seq));
}

void SymbolicIntVariable::setZeroValue(int _seq)
{
	m_interface.addAssertion(valueAtSequence(_seq) == 0);
}

void SymbolicIntVariable::setUnknownValue(int _seq)
{
	if (m_type.category() == Type::Category::Integer)
	{
		auto intType = dynamic_cast<IntegerType const*>(&m_type);
		solAssert(intType, "");
		m_interface.addAssertion(valueAtSequence(_seq) >= minValue(*intType));
		m_interface.addAssertion(valueAtSequence(_seq) <= maxValue(*intType));
	}
	else
	{
		solAssert(m_type.category() == Type::Category::Address, "");
		IntegerType addrType{160};
		m_interface.addAssertion(valueAtSequence(_seq) >= minValue(addrType));
		m_interface.addAssertion(valueAtSequence(_seq) <= maxValue(addrType));
	}
}

smt::Expression SymbolicIntVariable::minValue(IntegerType const& _t)
{
	return smt::Expression(_t.minValue());
}

smt::Expression SymbolicIntVariable::maxValue(IntegerType const& _t)
{
	return smt::Expression(_t.maxValue());
}
