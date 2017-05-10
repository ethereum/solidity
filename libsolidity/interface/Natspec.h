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
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format
 *
 * Can generally deal with JSON files
 */

#pragma once

#include <string>
#include <memory>
#include <json/json.h>

namespace dev
{
namespace solidity
{

// Forward declarations
class ContractDefinition;
class Type;
using TypePointer = std::shared_ptr<Type const>;
struct DocTag;
enum class DocumentationType: uint8_t;

enum class DocTagType: uint8_t
{
	None = 0,
	Dev,
	Notice,
	Param,
	Return,
	Author,
	Title
};

enum class CommentOwner
{
	Contract,
	Function
};

class Natspec
{
public:
	/// Get the given type of documentation
	/// @param _contractDef The contract definition
	/// @param _type        The type of the documentation. Can be one of the
	///                     types provided by @c DocumentationType
	/// @return             A JSON representation of provided type
	static Json::Value documentation(
		ContractDefinition const& _contractDef,
		DocumentationType _type
	);
	/// Get the User documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation of the contract's user documentation
	static Json::Value userDocumentation(ContractDefinition const& _contractDef);
	/// Genereates the Developer's documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A JSON representation
	///                     of the contract's developer documentation
	static Json::Value devDocumentation(ContractDefinition const& _contractDef);

private:
	/// @returns a json value suitable for a list of types in function input or output
	/// parameters or other places. If @a _forLibrary is true, complex types are referenced
	/// by name, otherwise they are anonymously expanded.
	static Json::Value formatTypeList(
		std::vector<std::string> const& _names,
		std::vector<TypePointer> const& _types,
		bool _forLibrary
	);
	/// @returns concatenation of all content under the given tag name.
	static std::string extractDoc(std::multimap<std::string, DocTag> const& _tags, std::string const& _name);
};

} //solidity NS
} // dev NS
