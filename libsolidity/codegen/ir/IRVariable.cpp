// SPDX-License-Identifier: GPL-3.0
#include <libsolidity/codegen/ir/Common.h>
#include <libsolidity/codegen/ir/IRVariable.h>
#include <libsolidity/ast/AST.h>
#include <boost/range/adaptor/transformed.hpp>
#include <libsolutil/StringUtils.h>

using namespace std;
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

IRVariable IRVariable::part(string const& _name) const
{
	for (auto const& [itemName, itemType]: m_type.stackItems())
		if (itemName == _name)
		{
			solAssert(itemName.empty() || itemType, "");
			return IRVariable{suffixedName(itemName), itemType ? *itemType : m_type};
		}
	solAssert(false, "Invalid stack item name: " + _name);
}

vector<string> IRVariable::stackSlots() const
{
	vector<string> result;
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

string IRVariable::commaSeparatedList() const
{
	return joinHumanReadable(stackSlots());
}

string IRVariable::commaSeparatedListPrefixed() const
{
	return joinHumanReadablePrefixed(stackSlots());
}

string IRVariable::name() const
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

string IRVariable::suffixedName(string const& _suffix) const
{
	if (_suffix.empty())
		return m_baseName;
	else
		return m_baseName + '_' + _suffix;
}
