// SPDX-License-Identifier: GPL-3.0
/**
 * Utilities to handle the Contract ABI (https://solidity.readthedocs.io/en/develop/abi-spec.html)
 */

#pragma once

#include <json/json.h>
#include <memory>
#include <string>

namespace solidity::frontend
{

// Forward declarations
class ContractDefinition;
class Type;
using TypePointer = Type const*;

class ABI
{
public:
	/// Get the ABI Interface of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSONrepresentation of the contract's ABI Interface
	static Json::Value generate(ContractDefinition const& _contractDef);
private:
	/// @returns a json value suitable for a list of types in function input or output
	/// parameters or other places. If @a _forLibrary is true, complex types are referenced
	/// by name, otherwise they are anonymously expanded.
	/// @a _solidityTypes is the list of original Solidity types where @a _encodingTypes is the list of
	/// ABI types used for the actual encoding.
	static Json::Value formatTypeList(
		std::vector<std::string> const& _names,
		std::vector<TypePointer> const& _encodingTypes,
		std::vector<TypePointer> const& _solidityTypes,
		bool _forLibrary
	);
	/// @returns a Json object with "name", "type", "internalType" and potentially
	/// "components" keys, according to the ABI specification.
	/// If it is possible to express the type as a single string, it is allowed to return a single string.
	/// @a _solidityType is the original Solidity type and @a _encodingTypes is the
	/// ABI type used for the actual encoding.
	static Json::Value formatType(
		std::string const& _name,
		Type const& _encodingType,
		Type const& _solidityType,
		bool _forLibrary
	);
};

}
