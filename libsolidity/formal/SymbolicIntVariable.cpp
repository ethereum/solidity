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

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicIntVariable::SymbolicIntVariable(
	Declaration const& _decl,
	smt::SolverInterface& _interface
):
	SymbolicVariable(_decl, _interface)
{
	solAssert(
		m_declaration.type()->category() == Type::Category::Integer ||
		m_declaration.type()->category() == Type::Category::Address,
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
	auto intType = dynamic_pointer_cast<IntegerType const>(m_declaration.type());
	if (!intType)
		intType = make_shared<IntegerType>(160);
	m_interface.addAssertion(valueAtSequence(_seq) >= minValue(*intType));
	m_interface.addAssertion(valueAtSequence(_seq) <= maxValue(*intType));
}

smt::Expression SymbolicIntVariable::minValue(IntegerType const& _t)
{
	return smt::Expression(_t.minValue());
}

smt::Expression SymbolicIntVariable::maxValue(IntegerType const& _t)
{
	return smt::Expression(_t.maxValue());
}
