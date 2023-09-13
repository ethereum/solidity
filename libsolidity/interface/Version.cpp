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

#include <solidity/BuildInfo.h>

char const* solidity::frontend::VersionNumber = ETH_PROJECT_VERSION;

std::string const solidity::frontend::VersionString =
	std::string(solidity::frontend::VersionNumber) +
	(std::string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + std::string(SOL_VERSION_PRERELEASE)) +
	(std::string(SOL_VERSION_BUILDINFO).empty() ? "" : "+" + std::string(SOL_VERSION_BUILDINFO));

std::string const solidity::frontend::VersionStringStrict =
	std::string(solidity::frontend::VersionNumber) +
	(std::string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + std::string(SOL_VERSION_PRERELEASE)) +
	(std::string(SOL_VERSION_COMMIT).empty() ? "" : "+" + std::string(SOL_VERSION_COMMIT));

solidity::bytes const solidity::frontend::VersionCompactBytes = {
	ETH_PROJECT_VERSION_MAJOR,
	ETH_PROJECT_VERSION_MINOR,
	ETH_PROJECT_VERSION_PATCH
};

bool const solidity::frontend::VersionIsRelease = std::string(SOL_VERSION_PRERELEASE).empty();
