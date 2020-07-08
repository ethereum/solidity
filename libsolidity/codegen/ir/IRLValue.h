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
