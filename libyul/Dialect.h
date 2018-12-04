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
/**
 * Yul dialect.
 */

#pragma once

#include <libyul/YulString.h>

#include <memory>

namespace yul
{

class YulString;
using Type = YulString;

enum class AsmFlavour
{
	Loose,  // no types, EVM instructions as function, jumps and direct stack manipulations
	Strict, // no types, EVM instructions as functions, but no jumps and no direct stack manipulations
	Yul     // same as Strict mode with types
};

struct BuiltinFunction
{
	YulString name;
	std::vector<Type> parameters;
	std::vector<Type> returns;
	bool movable;
};

/**
 * Class to query for builtin functions and their semantics.
 */
struct Builtins
{
	virtual ~Builtins() = default;
	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* query(YulString /*_name*/) const { return nullptr; }
};

struct Dialect
{
	AsmFlavour flavour = AsmFlavour::Loose;
	std::shared_ptr<Builtins> builtins;

	Dialect(AsmFlavour _flavour, std::shared_ptr<Builtins> _builtins):
		flavour(_flavour), builtins(std::move(_builtins))
	{}
	static Dialect looseAssemblyForEVM()
	{
		return Dialect{AsmFlavour::Loose, std::make_shared<Builtins>()};
	}
	static Dialect strictAssemblyForEVM()
	{
		// The EVM instructions will be moved to builtins at some point.
		return Dialect{AsmFlavour::Strict, std::make_shared<Builtins>()};
	}
	static Dialect yul()
	{
		// Will have to add builtins later.
		return Dialect{AsmFlavour::Yul, std::make_shared<Builtins>()};
	}
};

}
