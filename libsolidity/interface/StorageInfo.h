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
 * @author Santiago Palladino <spalladino@gmail.com>
 * @date 2018
 * Outputs contract storage layout information
 */

#pragma once

#include <string>
#include <memory>
#include <json/json.h>
#include <libsolidity/codegen/Compiler.h>

namespace dev
{
namespace solidity
{

// Forward declarations
class Compiler;

class StorageInfo
{
public:
	/// Get the storage layout of a contract
	/// @param _compiler The compiler used for the contract
	/// @return          A JSON representation of the contract's storage layout
	static Json::Value generate(Compiler const* _compiler);
};
}
}
