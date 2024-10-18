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

#include <libyul/Builtins.h>
#include <libyul/ControlFlowSideEffects.h>
#include <libyul/Exceptions.h>
#include <libyul/SideEffects.h>
#include <libyul/YulString.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace solidity::yul
{

enum class LiteralKind;
class LiteralValue;
struct Literal;

struct BuiltinFunction
{
	std::string name;
	size_t numParameters;
	size_t numReturns;
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

    /// Finds a builtin by name and returns the corresponding handle.
	/// @returns Builtin handle or null if the name does not match any builtin in the dialect.
	virtual std::optional<BuiltinHandle> findBuiltin(std::string_view /*_name*/) const { return std::nullopt; }

    /// Retrieves the description of a builtin function by its handle.
    /// Note that handles are dialect-specific and can be used only with a dialect that created them.
	virtual BuiltinFunction const& builtin(BuiltinHandle const&) const { yulAssert(false); }

	/// @returns true if the identifier is reserved. This includes the builtins too.
	virtual bool reservedIdentifier(std::string_view _name) const { return findBuiltin(_name).has_value(); }

	virtual std::optional<BuiltinHandle> discardFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> equalityFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> booleanNegationFunctionHandle() const { return std::nullopt; }

	virtual std::optional<BuiltinHandle> memoryStoreFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> memoryLoadFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> storageStoreFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> storageLoadFunctionHandle() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> hashFunctionHandle() const { return std::nullopt; }

	Literal zeroLiteral() const;

	Dialect() = default;
	virtual ~Dialect() = default;
};

}
