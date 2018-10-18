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

#include <libsolidity/formal/SymbolicAddressVariable.h>

#include <libsolidity/formal/SymbolicTypes.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicAddressVariable::SymbolicAddressVariable(
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicIntVariable(make_shared<IntegerType>(160), _uniqueName, _interface)
{
}

void SymbolicAddressVariable::setUnknownValue()
{
	auto intType = dynamic_cast<IntegerType const*>(m_type.get());
	solAssert(intType, "");
	m_interface.addAssertion(currentValue() >= minValue(*intType));
	m_interface.addAssertion(currentValue() <= maxValue(*intType));
}
