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
/**
 * Classes that store locations of lvalues.
 */

#pragma once

#include <libsolidity/codegen/ir/IRVariable.h>
#include <variant>

namespace solidity::frontend
{

class Type;

struct IRLValue
{
	Type const& type;
	struct Stack
	{
		IRVariable variable;
	};
	struct Immutable
	{
		VariableDeclaration const* variable = nullptr;
	};
	struct Storage
	{
		std::string const slot;
		/// unsigned: Used when the offset is known at compile time, uses optimized
		///           functions
		/// string: Used when the offset is determined at run time
		std::variant<std::string, unsigned> const offset;
		std::string offsetString() const
		{
			if (std::holds_alternative<unsigned>(offset))
				return std::to_string(std::get<unsigned>(offset));
			else
				return std::get<std::string>(offset);
		}
	};
	struct Memory
	{
		std::string const address;
		bool byteArrayElement = false;
	};
	struct Tuple
	{
		std::vector<std::optional<IRLValue>> components;
	};
	std::variant<Stack, Immutable, Storage, Memory, Tuple> kind;
};

}
