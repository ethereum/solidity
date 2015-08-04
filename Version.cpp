/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Versioning.
 */

#include <libsolidity/Version.h>
#include <string>
#include <BuildInfo.h>
#include <libdevcore/Common.h>

using namespace dev;
using namespace dev::solidity;
using namespace std;

char const* dev::solidity::VersionNumber = "0.1.1";
extern string const dev::solidity::VersionString =
	string(dev::solidity::VersionNumber) +
	"-" +
	string(DEV_QUOTED(ETH_COMMIT_HASH)).substr(0, 8) +
	(ETH_CLEAN_REPO ? "" : "*") +
	"/" DEV_QUOTED(ETH_BUILD_TYPE) "-" DEV_QUOTED(ETH_BUILD_PLATFORM);

