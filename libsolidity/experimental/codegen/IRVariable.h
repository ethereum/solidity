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

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/experimental/codegen/IRGenerationContext.h>
#include <libsolidity/experimental/ast/Type.h>

#include <optional>
#include <string>
#include <vector>

namespace solidity::frontend::experimental
{
/**
 * An IRVariable refers to a set of yul variables that correspond to the stack layout of a Solidity variable or expression
 * of a specific Solidity type.
 */
class IRVariable
{

public:
	/// IR variable with explicit base name @a _baseName, type @a _type and stack size @_stackSize.
	IRVariable(std::string _baseName, Type _type, size_t _stackSize);
	/// IR variable referring to the declaration @a _declaration.
	explicit IRVariable(VariableDeclaration const& _declaration, Type _type, size_t _stackSize);
	/// IR variable referring to the expression @a _expression.
	IRVariable(Expression const& _expression, Type _type, size_t _stackSize);

	/// @returns the name of the variable, if it occupies a single stack slot (otherwise throws).
	std::string name() const;

	/// @returns a comma-separated list of the stack slots of the variable.
	std::string commaSeparatedList() const;

	/// @returns a comma-separated list of the stack slots of the variable that is
	/// prefixed with a comma, unless it is empty.
	std::string commaSeparatedListPrefixed() const;

	/// @returns an IRVariable referring to the tuple component @a _i of a tuple variable.
	IRVariable tupleComponent(std::size_t _i) const;

	/// @returns the type of the variable.
	Type type() const { return m_type; }

	/// @returns the stack size of the variable.
	size_t stackSize() const { return m_stackSize; }

	/// @returns a vector containing the names of the stack slots of the variable.
	std::vector<std::string> stackSlots() const;

private:
	/// @returns a name consisting of the base name appended with an underscore and @a _suffix,
	/// unless @a _suffix is empty, in which case the base name itself is returned.
	std::string suffixedName(std::string const& _suffix) const;
	std::string m_baseName;
	Type m_type;
	size_t m_stackSize;
};

}
