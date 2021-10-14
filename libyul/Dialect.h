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
 * Yul dialect.
 */

#pragma once

#include <libyul/YulString.h>
#include <libyul/ControlFlowSideEffects.h>
#include <libyul/SideEffects.h>

#include <vector>
#include <set>
#include <optional>

namespace solidity::yul
{

class YulString;
using Type = YulString;
enum class LiteralKind;
struct Literal;

struct BuiltinFunction
{
	YulString name;
	std::vector<Type> parameters;
	std::vector<Type> returns;
	SideEffects sideEffects;
	ControlFlowSideEffects controlFlowSideEffects;
	/// If true, this is the msize instruction or might contain it.
	bool isMSize = false;
	/// Must be empty or the same length as the arguments.
	/// If set at index i, the i'th argument has to be a literal which means it can't be moved to variables.
	std::vector<std::optional<LiteralKind>> literalArguments{};
	std::optional<LiteralKind> literalArgument(size_t i) const
	{
		return literalArguments.empty() ? std::nullopt : literalArguments.at(i);
	}
};

struct Dialect
{
	/// Noncopiable.
	Dialect(Dialect const&) = delete;
	Dialect& operator=(Dialect const&) = delete;

	/// Default type, can be omitted.
	YulString defaultType;
	/// Type used for the literals "true" and "false".
	YulString boolType;
	std::set<YulString> types = {{}};

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* builtin(YulString /*_name*/) const { return nullptr; }

	/// @returns true if the identifier is reserved. This includes the builtins too.
	virtual bool reservedIdentifier(YulString _name) const { return builtin(_name) != nullptr; }

	virtual BuiltinFunction const* discardFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	virtual BuiltinFunction const* memoryStoreFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* memoryLoadFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* storageStoreFunction(YulString /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* storageLoadFunction(YulString /* _type */) const { return nullptr; }
	virtual YulString hashFunction(YulString /* _type */ ) const { return YulString{}; }

	/// Check whether the given type is legal for the given literal value.
	/// Should only be called if the type exists in the dialect at all.
	virtual bool validTypeForLiteral(LiteralKind _kind, YulString _value, YulString _type) const;

	virtual Literal zeroLiteralForType(YulString _type) const;
	virtual Literal trueLiteral() const;

	virtual std::set<YulString> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;

	/// Old "yul" dialect. This is only used for testing.
	static Dialect const& yulDeprecated();
};

}
