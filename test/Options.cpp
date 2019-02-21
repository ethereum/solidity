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

	options.add_options()
		("optimize", po::bool_switch(&optimize), "enables optimization")
		("abiencoderv2", po::bool_switch(&useABIEncoderV2), "enables abi encoder v2")
		("evm-version", po::value(&evmVersionString), "which evm version to use")
		("show-messages", po::bool_switch(&showMessages), "enables message output");

	parse(suite.argc, suite.argv);
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
