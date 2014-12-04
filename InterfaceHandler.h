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
#include <jsonrpc/json/json.h>

namespace dev
{
namespace solidity
{

// Forward declarations
class ContractDefinition;
enum DocumentationType: unsigned short;

enum DocTagType
{
	DOCTAG_NONE = 0,
	DOCTAG_DEV,
	DOCTAG_NOTICE,
	DOCTAG_PARAM,
	DOCTAG_RETURN
};

class InterfaceHandler
{
public:
	InterfaceHandler();

	/// Get the given type of documentation
	/// @param _contractDef The contract definition
	/// @param _type        The type of the documentation. Can be one of the
	///                     types provided by @c DocumentationType
	/// @return             A unique pointer contained string with the json
	///                     representation of provided type
	std::unique_ptr<std::string> getDocumentation(std::shared_ptr<ContractDefinition> _contractDef,
												  enum DocumentationType _type);
	/// Get the ABI Interface of the contract
	/// @param _contractDef The contract definition
	/// @return             A unique pointer contained string with the json
	///                     representation of the contract's ABI Interface
	std::unique_ptr<std::string> getABIInterface(std::shared_ptr<ContractDefinition> _contractDef);
	/// Get the User documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A unique pointer contained string with the json
	///                     representation of the contract's user documentation
	std::unique_ptr<std::string> getUserDocumentation(std::shared_ptr<ContractDefinition> _contractDef);
	/// Get the Developer's documentation of the contract
	/// @param _contractDef The contract definition
	/// @return             A unique pointer contained string with the json
	///                     representation of the contract's developer documentation
	std::unique_ptr<std::string> getDevDocumentation(std::shared_ptr<ContractDefinition> _contractDef);

private:
	void resetUser();
	void resetDev();

	size_t parseDocTagLine(std::string const& _string, std::string& _tagString, size_t _pos, enum DocTagType _tagType);
	size_t parseDocTagParam(std::string const& _string, size_t _startPos);
	size_t appendDocTagParam(std::string const& _string, size_t _startPos);
	void parseDocString(std::string const& _string, size_t _startPos = 0);
	size_t appendDocTag(std::string const& _string, size_t _startPos);
	size_t parseDocTag(std::string const& _string, std::string const& _tag, size_t _pos);

	Json::StyledWriter m_writer;

	// internal state
	enum DocTagType m_lastTag;
	std::string m_notice;
	std::string m_dev;
	std::string m_return;
	std::vector<std::pair<std::string, std::string>> m_params;
};

} //solidity NS
} // dev NS
