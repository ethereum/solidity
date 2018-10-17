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

#include <libsolidity/formal/SymbolicTypes.h>

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
	solAssert(isNumber(_type.category()), "");
}

smt::Expression SymbolicIntVariable::valueAtIndex(int _index) const
{
	return m_interface.newInteger(uniqueSymbol(_index));
}

void SymbolicIntVariable::setZeroValue()
{
	m_interface.addAssertion(currentValue() == 0);
}

void SymbolicIntVariable::setUnknownValue()
{
	auto intType = dynamic_cast<IntegerType const*>(&m_type);
	solAssert(intType, "");
	m_interface.addAssertion(currentValue() >= minValue(*intType));
	m_interface.addAssertion(currentValue() <= maxValue(*intType));
}
