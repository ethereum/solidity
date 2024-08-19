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
#include <libyul/SideEffects.h>
#include <libyul/YulName.h>

#include <optional>
#include <set>
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

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	virtual std::optional<BuiltinHandle> builtin(std::string_view /*_name*/) const { return std::nullopt; }
	virtual std::optional<VerbatimHandle> verbatim(std::string_view /*_name*/) const { return std::nullopt; }

	virtual BuiltinFunction const& builtinFunction(BuiltinHandle const&) const;
	virtual BuiltinFunction const& verbatimFunction(VerbatimHandle const&) const;

	/// @returns true if the identifier is reserved. This includes the builtins too.
	virtual bool reservedIdentifier(std::string_view _name) const { return builtin(_name).has_value(); }

	virtual std::optional<BuiltinHandle> discardFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> equalityFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> booleanNegationFunction() const { return std::nullopt; }

	virtual std::optional<BuiltinHandle> memoryStoreFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> memoryLoadFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> storageStoreFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> storageLoadFunction() const { return std::nullopt; }
	virtual std::optional<BuiltinHandle> hashFunction() const { return std::nullopt; }

	Literal zeroLiteral() const;

	virtual std::set<YulName> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;
};

}
