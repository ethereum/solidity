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
 * Settings to aid debugging.
 */

#pragma once

#include <cstddef>
#include <string>
#include <optional>

namespace dev
{
namespace solidity
{

enum class RevertStrings
{

	Default, // no compiler-generated strings, keep user-supplied strings
	Strip, // no compiler-generated strings, remove user-supplied strings (if possible)
	Debug, // add strings for internal reverts, keep user-supplied strings
	VerboseDebug // add strings for internal reverts, add user-supplied strings if not provided
};

inline std::string revertStringsToString(RevertStrings _str)
{
	switch (_str)
	{
	case RevertStrings::Default: return "default";
	case RevertStrings::Strip: return "strip";
	case RevertStrings::Debug: return "debug";
	case RevertStrings::VerboseDebug: return "verboseDebug";
	}
	// Cannot reach this.
	return "INVALID";
}

inline std::optional<RevertStrings> revertStringsFromString(std::string const& _str)
{
	for (auto i: {RevertStrings::Default, RevertStrings::Strip, RevertStrings::Debug, RevertStrings::VerboseDebug})
		if (revertStringsToString(i) == _str)
			return i;
	return {};
}

}
}
