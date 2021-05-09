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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://docs.soliditylang.org/en/develop/natspec-format.html
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

	/// Extract all custom tags from @a _tags.
	static Json::Value extractCustomDoc(std::multimap<std::string, DocTag> const& _tags);

	/// Helper-function that will create a json object with dev specific annotations, if present.
	/// @param _tags docTags that are used.
	/// @return      A JSON representation
	///              of the contract's developer documentation
	static Json::Value devDocumentation(std::multimap<std::string, DocTag> const& _tags);

	/// Helper-function that will create a json object for the "returns" field for a given function definition.
	/// @param _tags docTags that are used.
	/// @param _returnParameterNames names of the return parameters
	/// @return      A JSON representation
	///              of a method's return notice documentation
	static Json::Value extractReturnParameterDocs(std::multimap<std::string, DocTag> const& _tags, std::vector<std::string> const& _returnParameterNames);
};

}
