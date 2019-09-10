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

#include <libsolidity/interface/Version.h>

#include <liblangutil/Exceptions.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Common.h>
#include <solidity/BuildInfo.h>
#include <string>

using namespace dev;
using namespace dev::solidity;
using namespace std;

char const* dev::solidity::VersionNumber = ETH_PROJECT_VERSION;

string const dev::solidity::VersionString =
	string(dev::solidity::VersionNumber) +
	(string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + string(SOL_VERSION_PRERELEASE)) +
	(string(SOL_VERSION_BUILDINFO).empty() ? "" : "+" + string(SOL_VERSION_BUILDINFO));

string const dev::solidity::VersionStringStrict =
	string(dev::solidity::VersionNumber) +
	(string(SOL_VERSION_PRERELEASE).empty() ? "" : "-" + string(SOL_VERSION_PRERELEASE)) +
	(string(SOL_VERSION_COMMIT).empty() ? "" : "+" + string(SOL_VERSION_COMMIT));

bytes const dev::solidity::VersionCompactBytes = {
	ETH_PROJECT_VERSION_MAJOR,
	ETH_PROJECT_VERSION_MINOR,
	ETH_PROJECT_VERSION_PATCH
};

bool const dev::solidity::VersionIsRelease = string(SOL_VERSION_PRERELEASE).empty();
