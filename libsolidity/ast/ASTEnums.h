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
 * @date 2017
 * Enums for AST classes.
 */

#pragma once

#include <liblangutil/Exceptions.h>
#include <libsolidity/ast/ASTForward.h>

#include <string>

namespace solidity::frontend
{

/// Possible lookups for function resolving
enum class VirtualLookup { Static, Virtual, Super };

/// Visibility ordered from restricted to unrestricted.
enum class Visibility { Default, Private, Internal, Public, External };

enum class Arithmetic { Checked, Wrapping };

// How a function can mutate the EVM state.
enum class StateMutability { Pure, View, NonPayable, Payable };

/// Map from `StateMutability` enum values to their ABI JSON output representations.
inline std::map<StateMutability, std::string> const stateMutabilites = {
	{StateMutability::Pure, "pure"},
	{StateMutability::View, "view"},
	{StateMutability::NonPayable, "nonpayable"},
	{StateMutability::Payable, "payable"},
};

/// @param _stateMutability The state mutability string to convert.
/// @return The corresponding StateMutability enum value, or std::nullopt for unknown strings.
inline std::optional<StateMutability> stateMutabilityFromString(std::string const& _stateMutability)
{
	for (auto const& [value, name]: stateMutabilites)
		if (name == _stateMutability)
			return value;
	return std::nullopt;
}

/// @param _stateMutability The state mutability enum value to convert to string.
/// @return The string representation of the state mutability.
inline std::string stateMutabilityToString(StateMutability const& _stateMutability)
{
	return stateMutabilites.at(_stateMutability);
}

class Type;

/// Container for function call parameter types & names
struct FuncCallArguments
{
	/// Types of arguments
	std::vector<Type const*> types;
	/// Names of the arguments if given, otherwise unset
	std::vector<ASTPointer<ASTString>> names;

	size_t numArguments() const { return types.size(); }
	size_t numNames() const { return names.size(); }
	bool hasNamedArguments() const { return !names.empty(); }
};

enum class ContractKind { Interface, Contract, Library };

}
