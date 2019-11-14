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
using namespace dev::solidity::smt;

SymbolicVariable::SymbolicVariable(
	solidity::TypePointer _type,
	solidity::TypePointer _originalType,
	string _uniqueName,
	EncodingContext& _context
):
	m_type(_type),
	m_originalType(_originalType),
	m_uniqueName(move(_uniqueName)),
	m_context(_context),
	m_ssa(make_unique<SSAVariable>())
{
	solAssert(m_type, "");
	m_sort = smtSort(*m_type);
	solAssert(m_sort, "");
}

SymbolicVariable::SymbolicVariable(
	SortPointer _sort,
	string _uniqueName,
	EncodingContext& _context
):
	m_sort(move(_sort)),
	m_uniqueName(move(_uniqueName)),
	m_context(_context),
	m_ssa(make_unique<SSAVariable>())
{
	solAssert(m_sort, "");
}

Expression SymbolicVariable::currentValue(solidity::TypePointer const&) const
{
	return valueAtIndex(m_ssa->index());
}

string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

Expression SymbolicVariable::valueAtIndex(int _index) const
{
	return m_context.newVariable(uniqueSymbol(_index), m_sort);
}

string SymbolicVariable::nameAtIndex(int _index) const
{
	return uniqueSymbol(_index);
}

string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + to_string(_index);
}

Expression SymbolicVariable::resetIndex()
{
	m_ssa->resetIndex();
	return currentValue();
}

Expression SymbolicVariable::increaseIndex()
{
	++(*m_ssa);
	return currentValue();
}

SymbolicBoolVariable::SymbolicBoolVariable(
	solidity::TypePointer _type,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, move(_uniqueName), _context)
{
	solAssert(m_type->category() == solidity::Type::Category::Bool, "");
}

SymbolicIntVariable::SymbolicIntVariable(
	solidity::TypePointer _type,
	solidity::TypePointer _originalType,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _originalType, move(_uniqueName), _context)
{
	solAssert(isNumber(m_type->category()), "");
}

SymbolicAddressVariable::SymbolicAddressVariable(
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicIntVariable(TypeProvider::uint(160), TypeProvider::uint(160), move(_uniqueName), _context)
{
}

SymbolicFixedBytesVariable::SymbolicFixedBytesVariable(
	solidity::TypePointer _originalType,
	unsigned _numBytes,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicIntVariable(TypeProvider::uint(_numBytes * 8), _originalType, move(_uniqueName), _context)
{
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	solidity::TypePointer _type,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, move(_uniqueName), _context),
	m_declaration(m_context.newVariable(currentName(), m_sort))
{
	solAssert(m_type->category() == solidity::Type::Category::Function, "");
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	SortPointer _sort,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(move(_sort), move(_uniqueName), _context),
	m_declaration(m_context.newVariable(currentName(), m_sort))
{
	solAssert(m_sort->kind == Kind::Function, "");
}

Expression SymbolicFunctionVariable::currentValue(solidity::TypePointer const& _targetType) const
{
	return m_abstract.currentValue(_targetType);
}

Expression SymbolicFunctionVariable::currentFunctionValue() const
{
	return m_declaration;
}

Expression SymbolicFunctionVariable::valueAtIndex(int _index) const
{
	return m_abstract.valueAtIndex(_index);
}

Expression SymbolicFunctionVariable::functionValueAtIndex(int _index) const
{
	return SymbolicVariable::valueAtIndex(_index);
}

Expression SymbolicFunctionVariable::resetIndex()
{
	SymbolicVariable::resetIndex();
	return m_abstract.resetIndex();
}

Expression SymbolicFunctionVariable::increaseIndex()
{
	++(*m_ssa);
	resetDeclaration();
	m_abstract.increaseIndex();
	return m_abstract.currentValue();
}

Expression SymbolicFunctionVariable::operator()(vector<Expression> _arguments) const
{
	return m_declaration(_arguments);
}

void SymbolicFunctionVariable::resetDeclaration()
{
	m_declaration = m_context.newVariable(currentName(), m_sort);
}

SymbolicMappingVariable::SymbolicMappingVariable(
	solidity::TypePointer _type,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, move(_uniqueName), _context)
{
	solAssert(isMapping(m_type->category()), "");
}

SymbolicArrayVariable::SymbolicArrayVariable(
	solidity::TypePointer _type,
	solidity::TypePointer _originalType,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _originalType, move(_uniqueName), _context)
{
	solAssert(isArray(m_type->category()), "");
}

Expression SymbolicArrayVariable::currentValue(solidity::TypePointer const& _targetType) const
{
	if (_targetType)
		// StringLiterals are encoded as SMT arrays in the generic case,
		// but they can also be compared/assigned to fixed bytes, in which
		// case they'd need to be encoded as numbers.
		if (auto strType = dynamic_cast<StringLiteralType const*>(m_originalType))
			if (_targetType->category() == solidity::Type::Category::FixedBytes)
				return smt::Expression(u256(toHex(asBytes(strType->value()), HexPrefix::Add)));

	return SymbolicVariable::currentValue(_targetType);
}

SymbolicEnumVariable::SymbolicEnumVariable(
	solidity::TypePointer _type,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, move(_uniqueName), _context)
{
	solAssert(isEnum(m_type->category()), "");
}

SymbolicTupleVariable::SymbolicTupleVariable(
	solidity::TypePointer _type,
	string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, move(_uniqueName), _context)
{
	solAssert(isTuple(m_type->category()), "");
}

void SymbolicTupleVariable::setComponents(vector<shared_ptr<SymbolicVariable>> _components)
{
	solAssert(m_components.empty(), "");
	auto const& tupleType = dynamic_cast<solidity::TupleType const*>(m_type);
	solAssert(_components.size() == tupleType->components().size(), "");
	m_components = move(_components);
}
