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
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/formal/SymbolicVariables.h>

#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolutil/Algorithms.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

SymbolicVariable::SymbolicVariable(
	frontend::Type const* _type,
	frontend::Type const* _originalType,
	std::string _uniqueName,
	EncodingContext& _context
):
	m_type(_type),
	m_originalType(_originalType),
	m_uniqueName(std::move(_uniqueName)),
	m_context(_context),
	m_ssa(std::make_unique<SSAVariable>())
{
	solAssert(m_type, "");
	m_sort = smtSort(*m_type);
	solAssert(m_sort, "");
}

SymbolicVariable::SymbolicVariable(
	SortPointer _sort,
	std::string _uniqueName,
	EncodingContext& _context
):
	m_sort(std::move(_sort)),
	m_uniqueName(std::move(_uniqueName)),
	m_context(_context),
	m_ssa(std::make_unique<SSAVariable>())
{
	solAssert(m_sort, "");
}

smtutil::Expression SymbolicVariable::currentValue(frontend::Type const*) const
{
	return valueAtIndex(m_ssa->index());
}

std::string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

smtutil::Expression SymbolicVariable::valueAtIndex(unsigned _index) const
{
	return m_context.newVariable(uniqueSymbol(_index), m_sort);
}

std::string SymbolicVariable::nameAtIndex(unsigned _index) const
{
	return uniqueSymbol(_index);
}

std::string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + std::to_string(_index);
}

smtutil::Expression SymbolicVariable::resetIndex()
{
	m_ssa->resetIndex();
	return currentValue();
}

smtutil::Expression SymbolicVariable::setIndex(unsigned _index)
{
	m_ssa->setIndex(_index);
	return currentValue();
}

smtutil::Expression SymbolicVariable::increaseIndex()
{
	++(*m_ssa);
	return currentValue();
}

SymbolicBoolVariable::SymbolicBoolVariable(
	frontend::Type const* _type,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, std::move(_uniqueName), _context)
{
	solAssert(m_type->category() == frontend::Type::Category::Bool, "");
}

SymbolicIntVariable::SymbolicIntVariable(
	frontend::Type const* _type,
	frontend::Type const* _originalType,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _originalType, std::move(_uniqueName), _context)
{
	solAssert(isNumber(*m_type), "");
}

SymbolicAddressVariable::SymbolicAddressVariable(
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicIntVariable(TypeProvider::uint(160), TypeProvider::uint(160), std::move(_uniqueName), _context)
{
}

SymbolicFixedBytesVariable::SymbolicFixedBytesVariable(
	frontend::Type const* _originalType,
	unsigned _numBytes,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicIntVariable(TypeProvider::uint(_numBytes * 8), _originalType, std::move(_uniqueName), _context)
{
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	frontend::Type const* _type,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, std::move(_uniqueName), _context),
	m_declaration(m_context.newVariable(currentName(), m_sort))
{
	solAssert(m_type->category() == frontend::Type::Category::Function, "");
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	SortPointer _sort,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(std::move(_sort), std::move(_uniqueName), _context),
	m_declaration(m_context.newVariable(currentName(), m_sort))
{
	solAssert(m_sort->kind == Kind::Function, "");
}

smtutil::Expression SymbolicFunctionVariable::currentValue(frontend::Type const* _targetType) const
{
	return m_abstract.currentValue(_targetType);
}

smtutil::Expression SymbolicFunctionVariable::currentFunctionValue() const
{
	return m_declaration;
}

smtutil::Expression SymbolicFunctionVariable::valueAtIndex(unsigned _index) const
{
	return m_abstract.valueAtIndex(_index);
}

smtutil::Expression SymbolicFunctionVariable::functionValueAtIndex(unsigned _index) const
{
	return SymbolicVariable::valueAtIndex(_index);
}

smtutil::Expression SymbolicFunctionVariable::resetIndex()
{
	SymbolicVariable::resetIndex();
	return m_abstract.resetIndex();
}

smtutil::Expression SymbolicFunctionVariable::setIndex(unsigned _index)
{
	SymbolicVariable::setIndex(_index);
	return m_abstract.setIndex(_index);
}

smtutil::Expression SymbolicFunctionVariable::increaseIndex()
{
	++(*m_ssa);
	resetDeclaration();
	m_abstract.increaseIndex();
	return m_abstract.currentValue();
}

smtutil::Expression SymbolicFunctionVariable::operator()(std::vector<smtutil::Expression> _arguments) const
{
	return m_declaration(_arguments);
}

void SymbolicFunctionVariable::resetDeclaration()
{
	m_declaration = m_context.newVariable(currentName(), m_sort);
}

SymbolicEnumVariable::SymbolicEnumVariable(
	frontend::Type const* _type,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, std::move(_uniqueName), _context)
{
	solAssert(isEnum(*m_type), "");
}

SymbolicTupleVariable::SymbolicTupleVariable(
	frontend::Type const* _type,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, std::move(_uniqueName), _context)
{
	solAssert(isTuple(*m_type), "");
}

SymbolicTupleVariable::SymbolicTupleVariable(
	SortPointer _sort,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(std::move(_sort), std::move(_uniqueName), _context)
{
	solAssert(m_sort->kind == Kind::Tuple, "");
}

smtutil::Expression SymbolicTupleVariable::currentValue(frontend::Type const* _targetType) const
{
	if (!_targetType || sort() == smtSort(*_targetType))
		return SymbolicVariable::currentValue();

	auto thisTuple = std::dynamic_pointer_cast<TupleSort>(sort());
	auto otherTuple = std::dynamic_pointer_cast<TupleSort>(smtSort(*_targetType));
	solAssert(thisTuple && otherTuple, "");
	solAssert(thisTuple->components.size() == otherTuple->components.size(), "");
	std::vector<smtutil::Expression> args;
	for (size_t i = 0; i < thisTuple->components.size(); ++i)
		args.emplace_back(component(i, type(), _targetType));
	return smtutil::Expression::tuple_constructor(
		smtutil::Expression(std::make_shared<smtutil::SortSort>(smtSort(*_targetType)), ""),
		args
	);
}

std::vector<SortPointer> const& SymbolicTupleVariable::components() const
{
	auto tupleSort = std::dynamic_pointer_cast<TupleSort>(m_sort);
	solAssert(tupleSort, "");
	return tupleSort->components;
}

smtutil::Expression SymbolicTupleVariable::component(
	size_t _index,
	frontend::Type const* _fromType,
	frontend::Type const* _toType
) const
{
	std::optional<smtutil::Expression> conversion = symbolicTypeConversion(_fromType, _toType);
	if (conversion)
		return *conversion;

	return smtutil::Expression::tuple_get(currentValue(), _index);
}

SymbolicArrayVariable::SymbolicArrayVariable(
	frontend::Type const* _type,
	frontend::Type const* _originalType,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _originalType, std::move(_uniqueName), _context),
	m_pair(
		smtSort(*_type),
		m_uniqueName + "_length_pair",
		m_context
	)
{
	solAssert(isArray(*m_type) || isMapping(*m_type), "");
}

SymbolicArrayVariable::SymbolicArrayVariable(
	SortPointer _sort,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(std::move(_sort), std::move(_uniqueName), _context),
	m_pair(
		std::make_shared<TupleSort>(
			"array_length_pair",
			std::vector<std::string>{"array", "length"},
			std::vector<SortPointer>{m_sort, SortProvider::uintSort}
		),
		m_uniqueName + "_array_length_pair",
		m_context
	)
{
	solAssert(m_sort->kind == Kind::Array, "");
}

smtutil::Expression SymbolicArrayVariable::currentValue(frontend::Type const* _targetType) const
{
	std::optional<smtutil::Expression> conversion = symbolicTypeConversion(m_originalType, _targetType);
	if (conversion)
		return *conversion;

	return m_pair.currentValue();
}

smtutil::Expression SymbolicArrayVariable::valueAtIndex(unsigned _index) const
{
	return m_pair.valueAtIndex(_index);
}

smtutil::Expression SymbolicArrayVariable::elements() const
{
	return m_pair.component(0);
}

smtutil::Expression SymbolicArrayVariable::length() const
{
	return m_pair.component(1);
}

SymbolicStructVariable::SymbolicStructVariable(
	frontend::Type const* _type,
	std::string _uniqueName,
	EncodingContext& _context
):
	SymbolicVariable(_type, _type, std::move(_uniqueName), _context)
{
	solAssert(isNonRecursiveStruct(*m_type), "");
	auto const* structType = dynamic_cast<StructType const*>(_type);
	solAssert(structType, "");
	auto const& members = structType->structDefinition().members();
	for (unsigned i = 0; i < members.size(); ++i)
	{
		solAssert(members.at(i), "");
		m_memberIndices.emplace(members.at(i)->name(), i);
	}
}

smtutil::Expression SymbolicStructVariable::member(std::string const& _member) const
{
	return smtutil::Expression::tuple_get(currentValue(), m_memberIndices.at(_member));
}

smtutil::Expression SymbolicStructVariable::assignMember(std::string const& _member, smtutil::Expression const& _memberValue)
{
	auto const* structType = dynamic_cast<StructType const*>(m_type);
	solAssert(structType, "");
	auto const& structDef = structType->structDefinition();
	auto const& structMembers = structDef.members();
	auto oldMembers = applyMap(
		structMembers,
		[&](auto _member) { return member(_member->name()); }
	);
	increaseIndex();
	for (unsigned i = 0; i < structMembers.size(); ++i)
	{
		auto const& memberName = structMembers.at(i)->name();
		auto newMember = memberName == _member ? _memberValue : oldMembers.at(i);
		m_context.addAssertion(member(memberName) == newMember);
	}

	return currentValue();
}

smtutil::Expression SymbolicStructVariable::assignAllMembers(std::vector<smtutil::Expression> const& _memberValues)
{
	auto structType = dynamic_cast<StructType const*>(m_type);
	solAssert(structType, "");

	auto const& structDef = structType->structDefinition();
	auto const& structMembers = structDef.members();
	solAssert(_memberValues.size() == structMembers.size(), "");
	increaseIndex();
	for (unsigned i = 0; i < _memberValues.size(); ++i)
		m_context.addAssertion(_memberValues[i] == member(structMembers[i]->name()));

	return currentValue();
}
