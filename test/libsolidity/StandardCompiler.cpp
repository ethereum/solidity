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
 * Unit tests for interface/StandardCompiler.h.
 */

#include <string>
#include <iostream>
#include <regex>
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/StandardCompiler.h>
#include <libdevcore/JSON.h>

#include "../Metadata.h"

using namespace std;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

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

bool containsAtMostWarnings(Json::Value const& _compilerResult)
{
	if (!_compilerResult.isMember("errors"))
		return true;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["severity"].isString());
		if (error["severity"].asString() != "warning")
			return false;
	}

	return true;
}

Json::Value getContractResult(Json::Value const& _compilerResult, string const& _file, string const& _name)
{
	if (
		!_compilerResult["contracts"].isObject() ||
		!_compilerResult["contracts"][_file].isObject() ||
		!_compilerResult["contracts"][_file][_name].isObject()
	)
		return Json::Value();
	return _compilerResult["contracts"][_file][_name];
}

Json::Value compile(string const& _input)
{
	StandardCompiler compiler;
	string output = compiler.compile(_input);
	Json::Value ret;
	BOOST_REQUIRE(Json::Reader().parse(output, ret, false));
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(StandardCompiler)

BOOST_AUTO_TEST_CASE(assume_object_input)
{
	Json::Value result;

	/// Use the native JSON interface of StandardCompiler to trigger these
	solidity::StandardCompiler compiler;
	result = compiler.compile(Json::Value());
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));
	result = compiler.compile(Json::Value("INVALID"));
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));

	/// Use the string interface of StandardCompiler to trigger these
	result = compile("");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	result = compile("invalid");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	result = compile("\"invalid\"");
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));
	BOOST_CHECK(!containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	result = compile("{}");
	BOOST_CHECK(!containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	BOOST_CHECK(!containsAtMostWarnings(result));
}

BOOST_AUTO_TEST_CASE(invalid_language)
{
	char const* input = R"(
	{
		"language": "INVALID"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Only \"Solidity\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(valid_language)
{
	char const* input = R"(
	{
		"language": "Solidity"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(!containsError(result, "JSONError", "Only \"Solidity\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(no_sources)
{
	char const* input = R"(
	{
		"language": "Solidity"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "No input sources specified."));
}

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
}

BOOST_AUTO_TEST_CASE(basic_compilation)
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
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK(dev::jsonCompactPrint(contract["abi"]) == "[]");
	BOOST_CHECK(contract["devdoc"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(contract["devdoc"]) == "{\"methods\":{}}");
	BOOST_CHECK(contract["userdoc"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(contract["userdoc"]) == "{\"methods\":{}}");
	BOOST_CHECK(contract["evm"].isObject());
	/// @TODO check evm.methodIdentifiers, legacyAssembly, bytecode, deployedBytecode
	BOOST_CHECK(contract["evm"]["bytecode"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["object"].isString());
	BOOST_CHECK(dev::test::bytecodeSansMetadata(contract["evm"]["bytecode"]["object"].asString()) ==
		"60606040523415600b57fe5b5b60338060196000396000f30060606040525bfe00");
	BOOST_CHECK(contract["evm"]["assembly"].isString());
	BOOST_CHECK(contract["evm"]["assembly"].asString() ==
		"    /* \"fileA\":0:14  contract A { } */\n  mstore(0x40, 0x60)\n  jumpi(tag_1, iszero(callvalue))\n"
		"  invalid\ntag_1:\ntag_2:\n  dataSize(sub_0)\n  dup1\n  dataOffset(sub_0)\n  0x0\n  codecopy\n  0x0\n"
		"  return\nstop\n\nsub_0: assembly {\n        /* \"fileA\":0:14  contract A { } */\n"
		"      mstore(0x40, 0x60)\n    tag_1:\n      invalid\n}\n");
	BOOST_CHECK(contract["evm"]["gasEstimates"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(contract["evm"]["gasEstimates"]) ==
		"{\"creation\":{\"codeDepositCost\":\"10200\",\"executionCost\":\"62\",\"totalCost\":\"10262\"}}");
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["legacyAST"].isObject());
	BOOST_CHECK(dev::jsonCompactPrint(result["sources"]["fileA"]["legacyAST"]) ==
		"{\"attributes\":{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]}},\"children\":"
		"[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],\"contractKind\":\"contract\","
		"\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],\"name\":\"A\",\"nodes\":[null],\"scope\":2},"
		"\"id\":1,\"name\":\"ContractDefinition\",\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
