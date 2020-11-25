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
 * Yul util functions that are known by name and always included.
 */

#pragma once

#include <liblangutil/EVMVersion.h>
#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>
#include <libsolidity/interface/DebugSettings.h>

namespace solidity::frontend
{

/**
 * Yul util functions that are known by name and always included.
 */
class WellKnownUtilFunctions
{
public:
	explicit WellKnownUtilFunctions(
		langutil::EVMVersion _evmVersion,
		RevertStrings _revertStrings,
		MultiUseYulFunctionCollector& _functionCollector
	);
};

}
