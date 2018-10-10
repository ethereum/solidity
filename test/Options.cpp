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
/** @file TestHelper.h
* @author Marko Simovic <markobarko@gmail.com>
* @date 2014
*/

#include <test/Options.h>

#include <test/Common.h>

#include <libsolidity/interface/EVMVersion.h>
#include <libsolidity/interface/Exceptions.h>

#include <boost/test/framework.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace dev::test;
namespace fs = boost::filesystem;

Options const& Options::get()
{
	static Options instance;
	return instance;
}

Options::Options()
{
	auto const& suite = boost::unit_test::framework::master_test_suite();
	for (auto i = 0; i < suite.argc; i++)
		if (string(suite.argv[i]) == "--ipcpath" && i + 1 < suite.argc)
		{
			ipcPath = suite.argv[i + 1];
			i++;
		}
		else if (string(suite.argv[i]) == "--testpath" && i + 1 < suite.argc)
		{
			testPath = suite.argv[i + 1];
			i++;
		}
		else if (string(suite.argv[i]) == "--optimize")
			optimize = true;
		else if (string(suite.argv[i]) == "--evm-version")
		{
			evmVersionString = i + 1 < suite.argc ? suite.argv[i + 1] : "INVALID";
			++i;
		}
		else if (string(suite.argv[i]) == "--show-messages")
			showMessages = true;
		else if (string(suite.argv[i]) == "--no-ipc")
			disableIPC = true;
		else if (string(suite.argv[i]) == "--no-smt")
			disableSMT = true;

	if (!disableIPC && ipcPath.empty())
		if (auto path = getenv("ETH_TEST_IPC"))
			ipcPath = path;

	if (testPath.empty())
		if (auto path = getenv("ETH_TEST_PATH"))
			testPath = path;

	if (testPath.empty())
		testPath = discoverTestPath();
}

void Options::validate() const
{
	solAssert(
		!dev::test::Options::get().testPath.empty(),
		"No test path specified. The --testpath argument is required."
	);
	if (!disableIPC)
		solAssert(
			!dev::test::Options::get().ipcPath.empty(),
			"No ipc path specified. The --ipcpath argument is required, unless --no-ipc is used."
		);
}

dev::solidity::EVMVersion Options::evmVersion() const
{
	if (!evmVersionString.empty())
	{
		// We do this check as opposed to in the constructor because the BOOST_REQUIRE
		// macros cannot yet be used in the constructor.
		auto version = solidity::EVMVersion::fromString(evmVersionString);
		BOOST_REQUIRE_MESSAGE(version, "Invalid EVM version: " + evmVersionString);
		return *version;
	}
	else
		return dev::solidity::EVMVersion();
}
