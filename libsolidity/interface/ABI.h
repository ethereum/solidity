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
 * Utilities to handle the Contract ABI (https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI)
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
	static Json::Value formatTypeList(
		std::vector<std::string> const& _names,
		std::vector<TypePointer> const& _types,
		bool _forLibrary
	);
};

}
}
