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
 * Component that computes information relevant during decoding an external function
 * call's return values.
 */
#pragma once

#include <liblangutil/EVMVersion.h>
#include <libsolidity/ast/Types.h>

namespace solidity::frontend
{

/**
 * Computes and holds information relevant during decoding an external function
 * call's return values.
 */
struct ReturnInfo
{
	ReturnInfo(langutil::EVMVersion const& _evmVersion, FunctionType const& _functionType);

	/// Vector of TypePointer, for each return variable. Dynamic types are already replaced if required.
	TypePointers returnTypes = {};

	/// Boolean, indicating whether or not return size is only known at runtime.
	bool dynamicReturnSize = false;

	/// Contains the at compile time estimated return size.
	unsigned estimatedReturnSize = 0;
};

}
