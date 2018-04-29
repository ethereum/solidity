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

#include <test/Options.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/JSON.h>

namespace dev
{
namespace solidity
{
namespace test
{

class JSONStorageChecker
{
public:
	JSONStorageChecker(): m_compilerStack() {}

	void checkInterface(std::string const& _code, std::string const& _expectedStorageString)
	{
		m_compilerStack.reset(false);
		m_compilerStack.addSource("", "pragma solidity >=0.0;\n" + _code);
		m_compilerStack.setEVMVersion(dev::test::Options::get().evmVersion());
		m_compilerStack.setOptimiserSettings(dev::test::Options::get().optimize);
		BOOST_REQUIRE_MESSAGE(m_compilerStack.compile(), "Compiling contract failed");

		Json::Value generatedStorage = m_compilerStack.storageInfo(m_compilerStack.lastContractName());
		Json::Value expectedStorage;
		BOOST_REQUIRE(jsonParseStrict(_expectedStorageString, expectedStorage));

		// Sort both expected and generated json arrays before comparison, as we don't care about the order
		std::vector<Json::Value> generated(generatedStorage.begin(), generatedStorage.end());
		std::vector<Json::Value> expected(expectedStorage.begin(), expectedStorage.end());
		std::sort(generated.begin(), generated.end());
		std::sort(expected.begin(), expected.end());

		BOOST_CHECK_MESSAGE(
			expected == generated,
			"Expected:\n" << expectedStorage.toStyledString() <<
			"\n but got:\n" << generatedStorage.toStyledString()
		);
	}

protected:
	CompilerStack m_compilerStack;
};

BOOST_FIXTURE_TEST_SUITE(SolidityStorageInfoJSON, JSONStorageChecker)

BOOST_AUTO_TEST_CASE(single_var_test)
{
	char const* sourceCode = R"(
		contract test {
			uint a;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "test", "offset": "0", "slot": "0", "type": "uint256", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_var_test)
{
	char const* sourceCode = R"(
		contract test {
			uint a;
			uint b;
			uint c;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "test", "offset": "0", "slot": "0", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "b", "contract": "test", "offset": "0", "slot": "1", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "c", "contract": "test", "offset": "0", "slot": "2", "type": "uint256", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(packing_test)
{
	char const* sourceCode = R"(
		contract test {
			uint64  a;
			uint64  b;
			uint128 c;
			uint256 d;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "test", "offset": "0",  "slot": "0", "type": "uint64",  "size": "1", "bytes": "8" },
		{ "name": "b", "contract": "test", "offset": "8",  "slot": "0", "type": "uint64",  "size": "1", "bytes": "8" },
		{ "name": "c", "contract": "test", "offset": "16", "slot": "0", "type": "uint128", "size": "1", "bytes": "16" },
		{ "name": "d", "contract": "test", "offset": "0",  "slot": "1", "type": "uint256", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(bool_test)
{
	char const* sourceCode = R"(
		contract test {
			bool a;
			bool b;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "test", "offset": "0",  "slot": "0", "type": "bool", "size": "1", "bytes": "1" },
		{ "name": "b", "contract": "test", "offset": "1",  "slot": "0", "type": "bool", "size": "1", "bytes": "1" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(string_test)
{
	char const* sourceCode = R"(
		contract test {
			string a;
			string b;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "test", "offset": "0",  "slot": "0", "type": "string", "size": "1", "bytes": "32" },
		{ "name": "b", "contract": "test", "offset": "0",  "slot": "1", "type": "string", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(array_test)
{
	char const* sourceCode = R"(
		contract test {
			uint[10] arr;
		}
	)";

	char const* interface = R"JSON([
		{ "name": "arr", "contract": "test", "offset": "0", "slot": "0", "type": "uint256[10]", "size": "10" }
	])JSON";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(dynamic_array_test)
{
	char const* sourceCode = R"(
		contract test {
			uint[] arr;
		}
	)";

	char const* interface = R"JSON([
		{ "name": "arr", "contract": "test", "offset": "0", "slot": "0", "type": "uint256[]", "size": "1", "bytes": "32" }
	])JSON";

	checkInterface(sourceCode, interface);
}


BOOST_AUTO_TEST_CASE(mapping_test)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint => uint) m;
		}
	)";

	char const* interface = R"JSON([
		{ "name": "m", "contract": "test", "offset": "0", "slot": "0", "type": "mapping(uint256 => uint256)", "size": "1", "bytes": "32" }
	])JSON";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(struct_test)
{
	char const* sourceCode = R"(
		contract test {
			struct foo {
				uint a;
				uint b;
			}
			foo f;
		}
	)";

	char const* interface = R"JSON([
		{ "name": "f", "contract": "test", "offset": "0", "slot": "0", "type": "test.foo", "size": "2" }
	])JSON";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(inheritance_test)
{
	char const* sourceCode = R"(
		contract base {
			uint a;
		}

		contract test is base {
			uint b;
		}
	)";

	char const* interface = R"([
		{ "name": "a", "contract": "base", "offset": "0", "slot": "0", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "b", "contract": "test", "offset": "0", "slot": "1", "type": "uint256", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_inheritance_test)
{
	char const* sourceCode = R"(
		contract base {
			uint b;
		}

		contract child1 is base {
			uint c1;
		}

		contract child2 is base {
			uint c2;
		}
		
		contract test is child1, child2 {
			uint t;
		}
	)";

	char const* interface = R"([
		{ "name": "b",  "contract": "base",   "offset": "0", "slot": "0", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "c1", "contract": "child1", "offset": "0", "slot": "1", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "c2", "contract": "child2", "offset": "0", "slot": "2", "type": "uint256", "size": "1", "bytes": "32" },
		{ "name": "t",  "contract": "test",   "offset": "0", "slot": "3", "type": "uint256", "size": "1", "bytes": "32" }
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
