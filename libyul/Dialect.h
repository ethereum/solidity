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
#include <libyul/SideEffects.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <set>

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
	SideEffects sideEffects;
	/// If true, this is the msize instruction.
	bool isMSize = false;
	/// If true, can only accept literals as arguments and they cannot be moved to variables.
	bool literalArguments = false;
};

struct Dialect: boost::noncopyable
{
	AsmFlavour const flavour = AsmFlavour::Loose;
	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* builtin(YulString /*_name*/) const { return nullptr; }

	virtual BuiltinFunction const* discardFunction() const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction() const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	virtual std::set<YulString> fixedFunctionNames() const { return {}; }

	Dialect(AsmFlavour _flavour): flavour(_flavour) {}
	virtual ~Dialect() = default;

	static Dialect const& yul()
	{
		static Dialect yulDialect(AsmFlavour::Yul);
		return yulDialect;
	}
};

}
