// SPDX-License-Identifier: GPL-3.0
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format
 *
 * Can generally deal with JSON files
 */

#pragma once

#include <json/json.h>
#include <memory>
#include <string>
#include <libsolidity/ast/AST.h>

namespace solidity::frontend
{

// Forward declarations
class ContractDefinition;
struct DocTag;

class Natspec
{
public:
	static unsigned int constexpr c_natspecVersion = 1;

	/// Get the User documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation of the contract's user documentation
	static Json::Value userDocumentation(ContractDefinition const& _contractDef);
	/// Generates the Developer's documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation
	///                     of the contract's developer documentation
	static Json::Value devDocumentation(ContractDefinition const& _contractDef);

private:
	/// @returns concatenation of all content under the given tag name.
	static std::string extractDoc(std::multimap<std::string, DocTag> const& _tags, std::string const& _name);

	/// Helper-function that will create a json object with dev specific annotations, if present.
	/// @param _tags docTags that are used.
	/// @return      A JSON representation
	///              of the contract's developer documentation
	static Json::Value devDocumentation(std::multimap<std::string, DocTag> const& _tags);

	/// Helper-function that will create a json object for the "returns" field for a given function definition.
	/// @param _tags docTags that are used.
	/// @param _functionDef functionDefinition that is used to determine which return parameters are named.
	/// @return      A JSON representation
	///              of a method's return notice documentation
	static Json::Value extractReturnParameterDocs(std::multimap<std::string, DocTag> const& _tags, FunctionDefinition const& _functionDef);
};

}
