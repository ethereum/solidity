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

#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>

#include <boost/test/framework.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

using namespace std;
using namespace dev::test;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

Options const& Options::get()
{
	static Options instance;
	return instance;
}

Options::Options()
{
	auto const& suite = boost::unit_test::framework::master_test_suite();

	if (suite.argc == 0)
		return;

	po::options_description options("",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);

	options.add_options()
		("testpath", po::value<fs::path>(&testPath)->default_value(getTestPath()), "path to test files")
		("ipcpath", po::value<string>(&ipcPath)->default_value(getenv("ETH_TEST_IPC")), "path to ipc socket")
		("no-ipc", po::bool_switch(&disableIPC), "disable semantic tests")
		("no-smt", po::bool_switch(&disableSMT), "disable SMT checker")
		("optimize", po::bool_switch(&optimize), "enables optimization")
		("abiencoderv2", po::bool_switch(&useABIEncoderV2), "enables abi encoder v2")
		("evm-version", po::value(&evmVersionString), "which evm version to use")
		("show-messages", po::bool_switch(&showMessages), "enables message output");

	po::variables_map arguments;

	po::command_line_parser cmdLineParser(suite.argc, suite.argv);
	cmdLineParser.options(options);
	po::store(cmdLineParser.run(), arguments);
	po::notify(arguments);

	if (!disableIPC)
	{
		solAssert(
			!ipcPath.empty(),
			"No ipc path specified. The --ipcpath argument is required, unless --no-ipc is used."
		);
		solAssert(
			fs::exists(ipcPath),
			"Invalid ipc path specified."
		);
	}
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
