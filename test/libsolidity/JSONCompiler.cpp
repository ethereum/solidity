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
 * Unit tests for solc/jsonCompiler.cpp.
 */

#include <string>
#include <iostream>
#include <regex>
#include <boost/test/unit_test.hpp>
#include <libdevcore/JSON.h>

using namespace std;

extern "C"
{
extern char const* compileJSONMulti(char const* _input, bool _optimize);
}

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

string bytecodeSansMetadata(string const& _bytecode)
{
	/// The metadata hash takes up 43 bytes (or 86 characters in hex)
	/// /a165627a7a72305820([0-9a-f]{64})0029$/

	if (_bytecode.size() < 88)
		return _bytecode;

	if (_bytecode.substr(_bytecode.size() - 4, 4) != "0029")
		return _bytecode;

	if (_bytecode.substr(_bytecode.size() - 86, 18) != "a165627a7a72305820")
		return _bytecode;

	return _bytecode.substr(0, _bytecode.size() - 86);
}

bool isValidMetadata(string const& _metadata)
{
	Json::Value metadata;
	if (!Json::Reader().parse(_metadata, metadata, false))
		return false;

	if (
		!metadata.isObject() ||
		!metadata.isMember("version") ||
		!metadata.isMember("language") ||
		!metadata.isMember("compiler") ||
		!metadata.isMember("settings") ||
		!metadata.isMember("sources") ||
		!metadata.isMember("output")
	)
		return false;

	if (!metadata["version"].isNumeric() || metadata["version"] != 1)
		return false;

	if (!metadata["language"].isString() || metadata["language"].asString() != "Solidity")
		return false;

	/// @TODO add more strict checks

	return true;
}

Json::Value compile(string const& _input)
{
	string output(compileJSONMulti(_input.c_str(), false));
	Json::Value ret;
	BOOST_REQUIRE(Json::Reader().parse(output, ret, false));
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(JSONCompiler)

BOOST_AUTO_TEST_CASE(basic_compilation)
{
	char const* input = R"(
	{
		"sources": {
			"fileA": "contract A { }"
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());
	BOOST_CHECK(result["contracts"].isObject());
	BOOST_CHECK(result["contracts"]["fileA:A"].isObject());
	Json::Value contract = result["contracts"]["fileA:A"];
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["interface"].isString());
	BOOST_CHECK(contract["interface"].asString() == "[]");
	BOOST_CHECK(contract["bytecode"].isString());
	BOOST_CHECK(bytecodeSansMetadata(contract["bytecode"].asString()) ==
		"60606040523415600b57fe5b5b60338060196000396000f30060606040525bfe00");
	BOOST_CHECK(contract["runtimeBytecode"].isString());
	BOOST_CHECK(bytecodeSansMetadata(contract["runtimeBytecode"].asString()) ==
		"60606040525bfe00");
	BOOST_CHECK(contract["functionHashes"].isObject());
	BOOST_CHECK(contract["gasEstimates"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(contract["gasEstimates"]) ==
		"{\"creation\":[62,10200],\"external\":{},\"internal\":{}}");
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["AST"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(result["sources"]["fileA"]["AST"]) ==
		"{\"children\":[{\"attributes\":{\"fullyImplemented\":true,\"isLibrary\":false,\"linearizedBaseContracts\":[1],"
		"\"name\":\"A\"},\"children\":[],\"id\":1,\"name\":\"ContractDefinition\",\"src\":\"0:14:0\"}],\"name\":\"SourceUnit\"}");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
