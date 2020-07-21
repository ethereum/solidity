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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Versioning.
 */

#include <libsolidity/interface/Version.h>

#include <liblangutil/Exceptions.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Common.h>
#include <solidity/BuildInfo.h>
#include <string>

using namespace std;

char const* solidity::frontend::VersionNumber = ETH_PROJECT_VERSION;

string const solidity::frontend::VersionString =
	string(solidity::frontend::VersionNumber) +
	(string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + string(SOL_VERSION_PRERELEASE)) +
	(string(SOL_VERSION_BUILDINFO).empty() ? "" : "+" + string(SOL_VERSION_BUILDINFO));

string const solidity::frontend::VersionStringStrict =
	string(solidity::frontend::VersionNumber) +
	(string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + string(SOL_VERSION_PRERELEASE)) +
	(string(SOL_VERSION_COMMIT).empty() ? "" : "+" + string(SOL_VERSION_COMMIT));

solidity::bytes const solidity::frontend::VersionCompactBytes = {
	ETH_PROJECT_VERSION_MAJOR,
	ETH_PROJECT_VERSION_MINOR,
	ETH_PROJECT_VERSION_PATCH
};

bool const solidity::frontend::VersionIsRelease = string(SOL_VERSION_PRERELEASE).empty();
