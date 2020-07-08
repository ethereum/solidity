// SPDX-License-Identifier: GPL-3.0
/**
 * Generates the storage layout of a contract.
 */

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

#include <json/json.h>

namespace solidity::frontend
{

class StorageLayout
{
public:
	/// Generates the storage layout of the contract
	/// @param _contractDef The contract definition
	/// @return A JSON representation of the contract's storage layout.
	Json::Value generate(ContractDefinition const& _contractDef);

private:
	/// Generates the JSON information for a variable and its storage location.
	Json::Value generate(VariableDeclaration const& _var, u256 const& _slot, unsigned _offset);

	/// Generates the JSON information for @param _type
	void generate(TypePointer _type);

	/// The key for the JSON object describing a type.
	std::string typeKeyName(TypePointer _type);

	Json::Value m_types;

	/// Current analyzed contract
	ContractDefinition const* m_contract = nullptr;
};

}
