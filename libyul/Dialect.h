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

namespace solidity::yul
{

class YulString;
using Type = YulString;
enum class LiteralKind;

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
	/// Default type, can be omitted.
	YulString defaultType;
	/// Type used for the literals "true" and "false".
	YulString boolType;
	std::set<YulString> types = {{}};

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* builtin(YulString /*_name*/) const { return nullptr; }

	virtual BuiltinFunction const* discardFunction() const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction() const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	/// Check whether the given type is legal for the given literal value.
	/// Should only be called if the type exists in the dialect at all.
	virtual bool validTypeForLiteral(LiteralKind _kind, YulString _value, YulString _type) const;

	virtual std::set<YulString> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;

	/// Old "yul" dialect. This is only used for testing.
	static Dialect const& yulDeprecated();
};

}
