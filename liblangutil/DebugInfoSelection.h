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
 * Handles selections of debug info components.
 */

#pragma once

#include <map>
#include <libsolutil/FlagSet.h>

namespace solidity::langutil
{

/**
 * Represents a set of flags corresponding to components of debug info selected for some purpose.
 *
 * Provides extra functionality for enumerating the components and serializing/deserializing the
 * selection to/from a comma-separated string.
 */
struct DebugInfoSelection: public solidity::util::FlagSet<DebugInfoSelection>
{
	static auto const& flagMap()
	{
		static std::map<std::string, bool DebugInfoSelection::*> const flags = {
			{"location", &DebugInfoSelection::location},
			{"snippet", &DebugInfoSelection::snippet},
			{"ast-id", &DebugInfoSelection::astID},
		};
		return flags;
	}

	bool location = false; ///< Include source location. E.g. `@src 3:50:100`
	bool snippet = false;  ///< Include source code snippet next to location. E.g. `@src 3:50:100 "contract C {..."`
	bool astID = false;	   ///< Include ID of the Solidity AST node. E.g. `@ast-id 15`
};

}
