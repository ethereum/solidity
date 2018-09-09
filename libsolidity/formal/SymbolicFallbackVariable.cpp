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

#include <libsolidity/formal/SymbolicFallbackVariable.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicFallbackVariable::SymbolicFallbackVariable(
	Declaration const& _decl,
	smt::SolverInterface& _interface
):
	SymbolicVariable(_decl, _interface)
{ }

smt::Expression SymbolicFallbackVariable::valueAtSequence(int _seq) const
{
	return m_interface.newInteger(uniqueSymbol(_seq));
}

void SymbolicFallbackVariable::setZeroValue(int _seq)
{
	m_interface.addAssertion(valueAtSequence(_seq) == 0);
}

void SymbolicFallbackVariable::setUnknownValue(int _seq)
{
	auto const& intType = IntegerType(256);
	m_interface.addAssertion(valueAtSequence(_seq) >= minValue(intType));
	m_interface.addAssertion(valueAtSequence(_seq) <= maxValue(intType));
}

smt::Expression SymbolicFallbackVariable::minValue(IntegerType const& _t)
{
	return smt::Expression(_t.minValue());
}

smt::Expression SymbolicFallbackVariable::maxValue(IntegerType const& _t)
{
	return smt::Expression(_t.maxValue());
}
