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
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/interface/Version.h>
#include <libsolc/libsolc.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

/// TODO: share this between StandardCompiler.cpp
/// Helper to match a specific error type and message
bool containsError(Json::Value const& _compilerResult, string const& _type, string const& _message)
{
	if (!_compilerResult.isMember("errors"))
		return false;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["type"].isString());
		BOOST_REQUIRE(error["message"].isString());
		if ((error["type"].asString() == _type) && (error["message"].asString() == _message))
			return true;
	}

	return false;
}

Json::Value compile(string const& _input, CStyleReadFileCallback _callback = nullptr)
{
	string output(solidity_compile(_input.c_str(), _callback, nullptr));
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

BOOST_AUTO_TEST_CASE(missing_callback)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"fileA": {
				"content": "import \"missing.sol\"; contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());

	BOOST_CHECK(containsError(result, "ParserError", "Source \"missing.sol\" not found: File not supplied initially."));
}

BOOST_AUTO_TEST_CASE(with_callback)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"fileA": {
				"content": "import \"found.sol\"; import \"notfound.sol\"; contract A { }"
			}
		}
	}
	)";

	CStyleReadFileCallback callback{
		[](void* _context, char const* _kind, char const* _path, char** o_contents, char** o_error)
		{
			// Passed in a nullptr in the compile() helper above.
			BOOST_CHECK(_context == nullptr);
			// Caller frees the pointers.
			BOOST_CHECK(string(_kind) == ReadCallback::kindString(ReadCallback::Kind::ReadFile));
			if (string(_path) == "found.sol")
			{
				static string content{"import \"missing.sol\"; contract B {}"};
				*o_contents = strdup(content.c_str());
				*o_error = nullptr;
			}
			else if (string(_path) == "missing.sol")
			{
				static string errorMsg{"Missing file."};
				*o_error = strdup(errorMsg.c_str());
				*o_contents = nullptr;
			}
			else
			{
				*o_error = nullptr;
				*o_contents = nullptr;
			}
		}
	};

	Json::Value result = compile(input, callback);
	BOOST_CHECK(result.isObject());

	// This ensures that "found.sol" was properly loaded which triggered the second import statement.
	BOOST_CHECK(containsError(result, "ParserError", "Source \"missing.sol\" not found: Missing file."));

	// This should be placed due to the missing "notfound.sol" which sets both pointers to null.
	BOOST_CHECK(containsError(result, "ParserError", "Source \"notfound.sol\" not found: Callback not supported."));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
