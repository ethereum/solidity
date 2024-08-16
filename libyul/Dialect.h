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

enum class LiteralKind;
class LiteralValue;
struct Literal;

struct BuiltinFunction
{
	YulName name;
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
	virtual BuiltinFunction const* builtin(YulName /*_name*/) const { return nullptr; }

	/// @returns true if the identifier is reserved. This includes the builtins too.
	virtual bool reservedIdentifier(YulName _name) const { return builtin(_name) != nullptr; }

	virtual BuiltinFunction const* discardFunction() const { return nullptr; }
	virtual BuiltinFunction const* equalityFunction() const { return nullptr; }
	virtual BuiltinFunction const* booleanNegationFunction() const { return nullptr; }

	virtual BuiltinFunction const* memoryStoreFunction() const { return nullptr; }
	virtual BuiltinFunction const* memoryLoadFunction() const { return nullptr; }
	virtual BuiltinFunction const* storageStoreFunction() const { return nullptr; }
	virtual BuiltinFunction const* storageLoadFunction() const { return nullptr; }
	virtual YulName hashFunction() const { return YulName{}; }

	Literal zeroLiteral() const;

	virtual std::set<YulName> fixedFunctionNames() const { return {}; }

	Dialect() = default;
	virtual ~Dialect() = default;
};

}
