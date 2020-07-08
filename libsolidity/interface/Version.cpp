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
