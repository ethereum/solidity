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

#include <libsolidity/experimental/analysis/TypeInference.h>
#include <libsolidity/experimental/codegen/Common.h>
#include <libsolidity/experimental/codegen/IRVariable.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <libsolutil/StringUtils.h>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend::experimental;

IRVariable::IRVariable(std::string _baseName, Type _type, size_t _stackSize):
	m_baseName(std::move(_baseName)), m_type(_type), m_stackSize(_stackSize)
{
}

IRVariable::IRVariable(VariableDeclaration const& _declaration, Type _type, size_t _stackSize):
	IRVariable(IRNames::localVariable(_declaration), _type, _stackSize)
{
	solAssert(!_declaration.isStateVariable(), "");
}

IRVariable::IRVariable(Expression const& _expression, Type _type, size_t _stackSize):
	IRVariable(IRNames::localVariable(_expression), _type, _stackSize)
{
}

std::vector<std::string> IRVariable::stackSlots() const
{
	std::vector<std::string> result;
	result.reserve(m_stackSize);
	if (m_stackSize == 1)
		result.emplace_back(m_baseName);
	else
		for (size_t i = 0; i < m_stackSize; ++i)
			result.emplace_back(suffixedName(std::to_string(i)));
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
	solAssert(m_stackSize == 1, "");
	return m_baseName;
}

IRVariable IRVariable::tupleComponent(size_t) const
{
	// TODO
	solAssert(false, "IRVariable::tupleComponent Not implemented");
	return IRVariable{"", Type{}, 1};
}

std::string IRVariable::suffixedName(std::string const& _suffix) const
{
	if (_suffix.empty())
		return m_baseName;
	else
		return m_baseName + '_' + _suffix;
}
