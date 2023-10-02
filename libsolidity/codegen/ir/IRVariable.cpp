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
#include <libsolidity/codegen/ir/Common.h>
#include <libsolidity/codegen/ir/IRVariable.h>
#include <libsolidity/ast/AST.h>
#include <libsolutil/StringUtils.h>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::util;

IRVariable::IRVariable(std::string _baseName, Type const& _type):
	m_baseName(std::move(_baseName)), m_type(_type)
{
}

IRVariable::IRVariable(VariableDeclaration const& _declaration):
	IRVariable(IRNames::localVariable(_declaration), *_declaration.annotation().type)
{
	solAssert(!_declaration.isStateVariable(), "");
}

IRVariable::IRVariable(Expression const& _expression):
	IRVariable(IRNames::localVariable(_expression), *_expression.annotation().type)
{
}

IRVariable IRVariable::part(std::string const& _name) const
{
	for (auto const& [itemName, itemType]: m_type.stackItems())
		if (itemName == _name)
		{
			solAssert(itemName.empty() || itemType, "");
			return IRVariable{suffixedName(itemName), itemType ? *itemType : m_type};
		}
	solAssert(false, "Invalid stack item name: " + _name);
}

bool IRVariable::hasPart(std::string const& _name) const
{
	for (auto const& [itemName, itemType]: m_type.stackItems())
		if (itemName == _name)
		{
			solAssert(itemName.empty() || itemType, "");
			return true;
		}
	return false;
}

std::vector<std::string> IRVariable::stackSlots() const
{
	std::vector<std::string> result;
	for (auto const& [itemName, itemType]: m_type.stackItems())
		if (itemType)
		{
			solAssert(!itemName.empty(), "");
			solAssert(m_type != *itemType, "");
			result += IRVariable{suffixedName(itemName), *itemType}.stackSlots();
		}
		else
		{
			solAssert(itemName.empty(), "");
			result.emplace_back(m_baseName);
		}
	return result;
}

std::string IRVariable::commaSeparatedList() const
{
	return joinHumanReadable(stackSlots());
}

std::string IRVariable::commaSeparatedListPrefixed() const
{
	return joinHumanReadablePrefixed(stackSlots());
}

std::string IRVariable::name() const
{
	solAssert(m_type.sizeOnStack() == 1, "");
	auto const& [itemName, type] = m_type.stackItems().front();
	solAssert(!type, "Expected null type for name " + itemName);
	return suffixedName(itemName);
}

IRVariable IRVariable::tupleComponent(size_t _i) const
{
	solAssert(
		m_type.category() == Type::Category::Tuple,
		"Requested tuple component of non-tuple IR variable."
	);
	return part(IRNames::tupleComponent(_i));
}

std::string IRVariable::suffixedName(std::string const& _suffix) const
{
	if (_suffix.empty())
		return m_baseName;
	else
		return m_baseName + '_' + _suffix;
}
