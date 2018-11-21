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

#include <libsolidity/formal/SymbolicVariables.h>

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicVariable::SymbolicVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	m_type(move(_type)),
	m_uniqueName(_uniqueName),
	m_interface(_interface),
	m_ssa(make_shared<SSAVariable>())
{
}

string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + to_string(_index);
}

SymbolicBoolVariable::SymbolicBoolVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface&_interface
):
	SymbolicVariable(move(_type), _uniqueName, _interface)
{
	solAssert(m_type->category() == Type::Category::Bool, "");
}

smt::Expression SymbolicBoolVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), make_shared<smt::Sort>(smt::Kind::Bool));
}

void SymbolicBoolVariable::setZeroValue()
{
	m_interface.addAssertion(currentValue() == smt::Expression(false));
}

void SymbolicBoolVariable::setUnknownValue()
{
}

SymbolicIntVariable::SymbolicIntVariable(
	TypePointer _type,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), _uniqueName, _interface)
{
	solAssert(isNumber(m_type->category()), "");
}

smt::Expression SymbolicIntVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), make_shared<smt::Sort>(smt::Kind::Int));
}

void SymbolicIntVariable::setZeroValue()
{
	m_interface.addAssertion(currentValue() == 0);
}

void SymbolicIntVariable::setUnknownValue()
{
	auto intType = dynamic_cast<IntegerType const*>(m_type.get());
	solAssert(intType, "");
	m_interface.addAssertion(currentValue() >= minValue(*intType));
	m_interface.addAssertion(currentValue() <= maxValue(*intType));
}

SymbolicAddressVariable::SymbolicAddressVariable(
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicIntVariable(make_shared<IntegerType>(160), _uniqueName, _interface)
{
}

SymbolicFixedBytesVariable::SymbolicFixedBytesVariable(
	unsigned _numBytes,
	string const& _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicIntVariable(make_shared<IntegerType>(_numBytes * 8), _uniqueName, _interface)
{
}
