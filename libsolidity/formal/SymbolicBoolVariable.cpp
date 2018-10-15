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

#include <libsolidity/formal/SymbolicBoolVariable.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicBoolVariable::SymbolicBoolVariable(
	Type const& _type,
	string const& _uniqueName,
	smt::SolverInterface&_interface
):
	SymbolicVariable(_type, _uniqueName, _interface)
{
	solAssert(_type.category() == Type::Category::Bool, "");
}

smt::Expression SymbolicBoolVariable::valueAtSequence(int _seq) const
{
	return m_interface.newBool(uniqueSymbol(_seq));
}

void SymbolicBoolVariable::setZeroValue()
{
	m_interface.addAssertion(current() == smt::Expression(false));
}

void SymbolicBoolVariable::setUnknownValue()
{
}
