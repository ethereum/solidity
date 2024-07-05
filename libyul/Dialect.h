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

#include <libyul/YulName.h>
#include <libyul/ControlFlowSideEffects.h>
#include <libyul/SideEffects.h>

#include <vector>
#include <set>
#include <optional>

namespace solidity::yul
{

using Type = YulName;
enum class LiteralKind;
class LiteralValue;
struct Literal;

struct BuiltinFunction
{
	YulName name;
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
	YulName defaultType;
	/// Type used for the literals "true" and "false".
	YulName boolType;
	std::set<YulName> types = {{}};

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual BuiltinFunction const* builtin(YulName /*_name*/) const { return nullptr; }

	/// @returns true if the identifier is reserved. This includes the builtins too.
	virtual bool reservedIdentifier(YulName _name) const { return builtin(_name) != nullptr; }

	virtual BuiltinFunction const* discardFunction(YulName /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction(YulName /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	virtual BuiltinFunction const* memoryStoreFunction(YulName /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* memoryLoadFunction(YulName /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* storageStoreFunction(YulName /* _type */) const { return nullptr; }
	virtual BuiltinFunction const* storageLoadFunction(YulName /* _type */) const { return nullptr; }
	virtual YulName hashFunction(YulName /* _type */ ) const { return YulName{}; }

	/// Check whether the given type is legal for the given literal value.
	/// Should only be called if the type exists in the dialect at all.
	virtual bool validTypeForLiteral(LiteralKind _kind, LiteralValue const& _value, YulName _type) const;

	virtual Literal zeroLiteralForType(YulName _type) const;
	virtual Literal trueLiteral() const;

	virtual std::set<std::string> builtinNames() const { return {}; }

	virtual std::set<YulName> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;

	/// Old "yul" dialect. This is only used for testing.
	static Dialect const& yulDeprecated();
};

}
