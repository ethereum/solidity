/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec
 * documentation and the ABI interface
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

class InterfaceHandler
{
public:
	InterfaceHandler();

	/// Get the given type of documentation
	/// @param _contractDef The contract definition
	/// @param _type        The type of the documentation. Can be one of the
	///                     types provided by @c DocumentationType
	/// @return             A string with the json representation of provided type
	std::string getDocumentation(
		ContractDefinition const& _contractDef,
		DocumentationType _type
	);
	/// Get the ABI Interface of the contract
	/// @param _contractDef The contract definition
	/// @return             A string with the json representation of the contract's ABI Interface
	std::string getABIInterface(ContractDefinition const& _contractDef);
	std::string getABISolidityInterface(ContractDefinition const& _contractDef);
	/// Get the User documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A string with the json representation of the contract's user documentation
	std::string userDocumentation(ContractDefinition const& _contractDef);
	/// Genereates the Developer's documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A string with the json representation
	///                     of the contract's developer documentation
	std::string devDocumentation(ContractDefinition const& _contractDef);

private:
	void resetUser();
	void resetDev();

	std::string::const_iterator parseDocTagLine(
		std::string::const_iterator _pos,
		std::string::const_iterator _end,
		std::string& _tagString,
		DocTagType _tagType,
		bool _appending
	);
	std::string::const_iterator parseDocTagParam(
		std::string::const_iterator _pos,
		std::string::const_iterator _end
	);
	std::string::const_iterator appendDocTagParam(
		std::string::const_iterator _pos,
		std::string::const_iterator _end
	);
	void parseDocString(std::string const& _string, CommentOwner _owner);
	std::string::const_iterator appendDocTag(
		std::string::const_iterator _pos,
		std::string::const_iterator _end,
		CommentOwner _owner
	);
	std::string::const_iterator parseDocTag(
		std::string::const_iterator _pos,
		std::string::const_iterator _end,
		std::string const& _tag,
		CommentOwner _owner
	);

	// internal state
	DocTagType m_lastTag;
	std::string m_notice;
	std::string m_dev;
	std::string m_return;
	std::string m_contractAuthor;
	std::string m_author;
	std::string m_title;
	std::vector<std::pair<std::string, std::string>> m_params;
};

} //solidity NS
} // dev NS
