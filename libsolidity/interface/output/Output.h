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

#pragma once


#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>

#include <libsolutil/CommonData.h>

#include <libsolidity/ast/ASTEnums.h>

#include <range/v3/algorithm.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace solidity::frontend::output
{

struct SourceLocation
{
	/// The name of the source file.
	std::string file;
	/// The start of the source position.
	int start;
	/// The end of the source position.
	int end;
	/// If this is a secondary source location, a message should exist.
	std::optional<std::string> message;

	/// @returns this source location converted to the compiler's internal type.
	langutil::SourceLocation toInternalSourceLocation() const
	{
		return langutil::SourceLocation{start, end, std::make_shared<std::string>(file)};
	}
};

struct Error
{
	/// Location within the source file.
	std::optional<SourceLocation> sourceLocation;
	/// Further locations (e.g. places of conflicting declarations).
	std::optional<std::vector<SourceLocation>> secondarySourceLocations;
	/// Unique code for the cause of the error.
	std::optional<langutil::ErrorId> errorCode;
	/// The error type.
	langutil::Error::Type type;
	/// The error message.
	std::optional<std::string> message;

	/// @returns this error converted to the compiler's internal type.
	langutil::Error toInternalError() const
	{
		auto locations = secondarySourceLocations.value_or(std::vector<SourceLocation>{});
		return {
			errorCode.value_or(langutil::ErrorId{}),
			type,
			message.value_or(std::string{}),
			sourceLocation.value_or(SourceLocation{}).toInternalSourceLocation(),
			langutil::SecondarySourceLocation{
				locations | ranges::views::filter([](auto const& s) {
					return s.message.has_value();
				}) | ranges::views::transform([](auto const& s) {
					return std::pair{s.message.value(), s.toInternalSourceLocation()};
				}) | ranges::to<std::vector>()
			}
		};
	}
};

struct Source
{
	/// Identifier of the source (used in source maps)
	size_t id;
};

struct ABIParameter
{
	/// The parameter name.
	std::string name;
	/// The ABI-level type, e.g. "address", "uint256", "tuple"
	std::string type;
	/// The Solidity-level type, may differ for e.g. enums or user-defined value types.
	std::optional<std::string> internalType;
	/// Whether this parameter is indexed. Only present for event inputs.
	std::optional<bool> indexed;
	/// Component parameters, only present when type == "tuple"
	std::optional<std::vector<ABIParameter>> components;
};

struct ABIConstructor
{
	/// The constructor's input parameters.
	std::vector<ABIParameter> inputs;
	/// The state mutability of the constructor.
	StateMutability stateMutability;
};

struct ABIFunction
{
	/// The name of the function.
	std::string name;
	/// The function's input parameters.
	std::vector<ABIParameter> inputs;
	/// The function's output parameters.
	std::vector<ABIParameter> outputs;
	/// The state mutability of the function.
	StateMutability stateMutability;
};

struct ABIFallback
{
	/// State mutability of the fallback function.
	StateMutability stateMutability;
};

struct ABIReceive
{
	/// State mutability of the receive function.
	StateMutability stateMutability;
};


struct ABIEvent
{
	/// The name of the event.
	std::string name;
	/// The event's parameters.
	std::vector<ABIParameter> inputs;
	/// Whether the event is anonymous. Anonymous events do not have their
	/// signature included in the topic list.
	bool isAnonymous;
};

struct ABIError
{
	/// The name of the error.
	std::string name;
	/// The error's parameters.
	std::vector<ABIParameter> inputs;
};

struct ByteOffset
{
	/// The start of the bytes to replace.
	size_t start;
	/// The length of the bytes to replace.
	size_t length;
};

using LinkReferences = std::map<std::string, std::vector<ByteOffset>>;

struct Bytecode
{
	/// The bytecode as a hex string.
	bytes object;
	/// The byte offsets per source and library. If not empty, this is an unlinked object.
	std::map<std::string, LinkReferences> linkReferences;
};

/// EVM-related outputs.
struct EVM
{
	/// The bytecode and link references.
	Bytecode bytecode;
	/// The list of function hashes, mapping function signatures to their selectors.
	std::map<std::string, std::string> methodIdentifiers;
};

/// A single entry in the contract ABI, either an event or a function.
using ABIEntry = std::variant<ABIConstructor, ABIFunction, ABIFallback, ABIReceive, ABIEvent, ABIError>;
using ABI = std::vector<ABIEntry>;

/**
 * Represents a compiled contract. Carries the contract-specific compiler output.
 */
struct Contract
{
	/// The contract name.
	std::string name;
	/// The Ethereum Contract ABI. If empty, it is represented as an empty array.
	ABI abi;
	/// EVM-related outputs (bytecode, function hashes, etc.).
	EVM evm;
	/// The contract metadata as a serialized JSON string.
	std::string metadata;
};

using Errors = std::vector<output::Error>;
using Sources = std::map<std::string, output::Source>;
using Contracts = std::map<std::string, std::map<std::string, output::Contract>>;

/**
 * Compilation output produced by `StandardCompiler` — mirrors solc's standard-JSON output contract (`--standard-json`): errors, sources and contracts.
 */
struct Output
{
	/// Compilation errors, warnings, and infos (optional in the JSON output; empty if none were encountered).
	output::Errors errors;
	/// File-level outputs, indexed by source file name.
	output::Sources sources;
	/// Contract-level outputs, indexed by source file name, then by contract name.
	output::Contracts contracts;
};

}
