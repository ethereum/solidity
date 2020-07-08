// SPDX-License-Identifier: GPL-3.0
/**
 * Settings to aid debugging.
 */

#pragma once

#include <cstddef>
#include <string>
#include <optional>

namespace solidity::frontend
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
	return std::nullopt;
}

}
