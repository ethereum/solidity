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

#include <libsolidity/experimental/ast/TypeSystem.h>

#include <optional>
#include <string>
#include <vector>

namespace solidity::frontend
{
class VariableDeclaration;
class Expression;
}

namespace solidity::frontend::experimental
{

class IRGenerationContext;

class IRVariable
{
public:
	/// IR variable with explicit base name @a _baseName and type @a _type.
	IRVariable(IRGenerationContext const& _analysis, std::string _baseName, Type _type);
	/// IR variable referring to the declaration @a _decl.
	IRVariable(IRGenerationContext const& _analysis, VariableDeclaration const& _decl);
	/// IR variable referring to the expression @a _expr.
	IRVariable(IRGenerationContext const& _analysis, Expression const& _expression);

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
	Type const& type() const { return m_type; }

	/// @returns an IRVariable referring to the stack component @a _slot of the variable.
	/// @a _slot must be among the stack slots in ``m_type.stackItems()``.
	/// The returned IRVariable is itself typed with the type of the stack slot as defined
	/// in ``m_type.stackItems()`` and may again occupy multiple stack slots.
	IRVariable part(std::string const& _slot) const;

	/// @returns true if variable contains @a _name component
	/// @a _name name of the component that is being checked
	bool hasPart(std::string const& _name) const;

	/// @returns a vector containing the names of the stack slots of the variable.
	std::vector<std::string> stackSlots() const;

private:
	/// @returns a name consisting of the base name appended with an underscore and @æ _suffix,
	/// unless @a _suffix is empty, in which case the base name itself is returned.
	std::string suffixedName(std::string const& _suffix) const;
	std::string m_baseName;
	Type m_type;
};


}
