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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Versioning.
 */

#pragma once

#include <libdevcore/Common.h>
#include <string>

namespace dev
{
namespace solidity
{

extern char const* VersionNumber;
extern std::string const VersionString;
extern std::string const VersionStringStrict;

/// @returns a binary form of the version string, where A.B.C-HASH is encoded such that
/// the first byte is zero, the following three bytes encode A B and C (interpreted as decimals)
/// and HASH is interpreted as 8 hex digits and encoded into the last four bytes.
bytes binaryVersion();

}
}
