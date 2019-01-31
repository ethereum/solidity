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
 * @date 2017
 * Unit tests for libsolc/libsolc.cpp.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libdevcore/JSON.h>
#include <libsolidity/interface/Version.h>
#include <libsolc/libsolc.h>

#include <test/Metadata.h>
#include <test/Options.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

Json::Value compile(string const& _input)
{
	string output(solidity_compile(_input.c_str(), nullptr));
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	solidity_free();
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(LibSolc)

BOOST_AUTO_TEST_CASE(read_version)
{
	string output(solidity_version());
	BOOST_CHECK(output.find(VersionString) == 0);
	solidity_free();
}

BOOST_AUTO_TEST_CASE(read_license)
{
	string output(solidity_license());
	BOOST_CHECK(output.find("GNU GENERAL PUBLIC LICENSE") != string::npos);
	solidity_free();
}

BOOST_AUTO_TEST_CASE(standard_compilation)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());

	// Only tests some assumptions. The StandardCompiler is tested properly in another suite.
	BOOST_CHECK(result.isMember("sources"));
	// This used to test that it is a member, but we did not actually request any output,
	// so there should not be a contract member.
	BOOST_CHECK(!result.isMember("contracts"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
