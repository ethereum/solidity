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
#include <libsolidity/ast/TypeProvider.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SymbolicVariable::SymbolicVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	m_type(move(_type)),
	m_uniqueName(move(_uniqueName)),
	m_interface(_interface),
	m_ssa(make_shared<SSAVariable>())
{
	solAssert(m_type, "");
	m_sort = smtSort(*m_type);
	solAssert(m_sort, "");
}

SymbolicVariable::SymbolicVariable(
	smt::SortPointer _sort,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	m_sort(move(_sort)),
	m_uniqueName(move(_uniqueName)),
	m_interface(_interface),
	m_ssa(make_shared<SSAVariable>())
{
	solAssert(m_sort, "");
}

smt::Expression SymbolicVariable::currentValue() const
{
	return valueAtIndex(m_ssa->index());
}

string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

smt::Expression SymbolicVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), m_sort);
}

string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + to_string(_index);
}

smt::Expression SymbolicVariable::increaseIndex()
{
	++(*m_ssa);
	return currentValue();
}

SymbolicBoolVariable::SymbolicBoolVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	solAssert(m_type->category() == Type::Category::Bool, "");
}

SymbolicIntVariable::SymbolicIntVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	solAssert(isNumber(m_type->category()), "");
}

SymbolicAddressVariable::SymbolicAddressVariable(
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicIntVariable(TypeProvider::integerType(160), move(_uniqueName), _interface)
{
}

SymbolicFixedBytesVariable::SymbolicFixedBytesVariable(
	unsigned _numBytes,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicIntVariable(TypeProvider::integerType(_numBytes * 8), move(_uniqueName), _interface)
{
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface),
	m_declaration(m_interface.newVariable(currentName(), m_sort))
{
	solAssert(m_type->category() == Type::Category::Function, "");
}

void SymbolicFunctionVariable::resetDeclaration()
{
	m_declaration = m_interface.newVariable(currentName(), m_sort);
}

smt::Expression SymbolicFunctionVariable::increaseIndex()
{
	++(*m_ssa);
	resetDeclaration();
	return currentValue();
}

smt::Expression SymbolicFunctionVariable::operator()(vector<smt::Expression> _arguments) const
{
	return m_declaration(_arguments);
}

SymbolicMappingVariable::SymbolicMappingVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	solAssert(isMapping(m_type->category()), "");
}

SymbolicArrayVariable::SymbolicArrayVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	solAssert(isArray(m_type->category()), "");
}

SymbolicEnumVariable::SymbolicEnumVariable(
	TypePointer _type,
	string _uniqueName,
	smt::SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	solAssert(isEnum(m_type->category()), "");
}
