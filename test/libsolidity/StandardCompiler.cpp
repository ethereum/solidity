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
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/StandardCompiler.h>
#include <libdevcore/JSON.h>
#include <test/Metadata.h>

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
	BOOST_REQUIRE(jsonParseStrict(output, ret));
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
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n* Line 1, Column 1\n  A valid JSON document must be either an array or an object value.\n"));
	result = compile("invalid");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n* Line 1, Column 2\n  Extra non-whitespace after JSON value.\n"));
	result = compile("\"invalid\"");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  A valid JSON document must be either an array or an object value.\n"));
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

BOOST_AUTO_TEST_CASE(no_sources_empty_object)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "No input sources specified."));
}

BOOST_AUTO_TEST_CASE(no_sources_empty_array)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": []
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"sources\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(sources_is_array)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": ["aa", "bb"]
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"sources\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(unexpected_trailing_test)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"A": {
				"content": "contract A { function f() {} }"
			}
		}
	}
	}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "* Line 10, Column 2\n  Extra non-whitespace after JSON value.\n"));
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

BOOST_AUTO_TEST_CASE(optimizer_enabled_not_boolean)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"optimizer": {
				"enabled": "wrong"
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "The \"enabled\" setting must be a Boolean."));
}

BOOST_AUTO_TEST_CASE(optimizer_runs_not_a_number)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"optimizer": {
				"enabled": true,
				"runs": "not a number"
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "The \"runs\" setting must be an unsigned number."));
}

BOOST_AUTO_TEST_CASE(optimizer_runs_not_an_unsigned_number)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"optimizer": {
				"enabled": true,
				"runs": -1
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "The \"runs\" setting must be an unsigned number."));
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
		},
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [ "abi", "devdoc", "userdoc", "evm.bytecode", "evm.assembly", "evm.gasEstimates", "evm.legacyAssembly", "metadata" ],
					"": [ "legacyAST" ]
				}
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
	BOOST_CHECK(contract["devdoc"].isObject());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["devdoc"]), "{\"methods\":{}}");
	BOOST_CHECK(contract["userdoc"].isObject());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["userdoc"]), "{\"methods\":{}}");
	BOOST_CHECK(contract["evm"].isObject());
	/// @TODO check evm.methodIdentifiers, legacyAssembly, bytecode, deployedBytecode
	BOOST_CHECK(contract["evm"]["bytecode"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["object"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["evm"]["bytecode"]["object"].asString()),
		"6080604052348015600f57600080fd5b50603580601d6000396000f3fe6080604052600080fdfe"
	);
	BOOST_CHECK(contract["evm"]["assembly"].isString());
	BOOST_CHECK(contract["evm"]["assembly"].asString().find(
		"    /* \"fileA\":0:14  contract A { } */\n  mstore(0x40, 0x80)\n  "
		"callvalue\n    /* \"--CODEGEN--\":8:17   */\n  dup1\n    "
		"/* \"--CODEGEN--\":5:7   */\n  iszero\n  tag_1\n  jumpi\n    "
		"/* \"--CODEGEN--\":30:31   */\n  0x00\n    /* \"--CODEGEN--\":27:28   */\n  "
		"dup1\n    /* \"--CODEGEN--\":20:32   */\n  revert\n    /* \"--CODEGEN--\":5:7   */\n"
		"tag_1:\n    /* \"fileA\":0:14  contract A { } */\n  pop\n  dataSize(sub_0)\n  dup1\n  "
		"dataOffset(sub_0)\n  0x00\n  codecopy\n  0x00\n  return\nstop\n\nsub_0: assembly {\n        "
		"/* \"fileA\":0:14  contract A { } */\n      mstore(0x40, 0x80)\n      0x00\n      "
		"dup1\n      revert\n\n    auxdata: 0xa165627a7a72305820"
	) == 0);
	BOOST_CHECK(contract["evm"]["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["evm"]["gasEstimates"]),
		"{\"creation\":{\"codeDepositCost\":\"10600\",\"executionCost\":\"66\",\"totalCost\":\"10666\"}}"
	);
	// Lets take the top level `.code` section (the "deployer code"), that should expose most of the features of
	// the assembly JSON. What we want to check here is Operation, Push, PushTag, PushSub, PushSubSize and Tag.
	BOOST_CHECK(contract["evm"]["legacyAssembly"].isObject());
	BOOST_CHECK(contract["evm"]["legacyAssembly"][".code"].isArray());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["evm"]["legacyAssembly"][".code"]),
		"[{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"value\":\"80\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"value\":\"40\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"MSTORE\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"CALLVALUE\"},"
		"{\"begin\":8,\"end\":17,\"name\":\"DUP1\"},"
		"{\"begin\":5,\"end\":7,\"name\":\"ISZERO\"},"
		"{\"begin\":5,\"end\":7,\"name\":\"PUSH [tag]\",\"value\":\"1\"},"
		"{\"begin\":5,\"end\":7,\"name\":\"JUMPI\"},"
		"{\"begin\":30,\"end\":31,\"name\":\"PUSH\",\"value\":\"0\"},"
		"{\"begin\":27,\"end\":28,\"name\":\"DUP1\"},"
		"{\"begin\":20,\"end\":32,\"name\":\"REVERT\"},"
		"{\"begin\":5,\"end\":7,\"name\":\"tag\",\"value\":\"1\"},"
		"{\"begin\":5,\"end\":7,\"name\":\"JUMPDEST\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"POP\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH #[$]\",\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"DUP1\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH [$]\",\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"CODECOPY\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"RETURN\"}]"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["legacyAST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"]["fileA"]["legacyAST"]),
		"{\"attributes\":{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]}},\"children\":"
		"[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],\"contractKind\":\"contract\","
		"\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],\"name\":\"A\",\"nodes\":[null],\"scope\":2},"
		"\"id\":1,\"name\":\"ContractDefinition\",\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
}

BOOST_AUTO_TEST_CASE(compilation_error)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { function }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isMember("errors"));
	BOOST_CHECK(result["errors"].size() >= 1);
	for (auto const& error: result["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["message"].isString());
		if (error["message"].asString().find("pre-release compiler") == string::npos)
		{
			BOOST_CHECK_EQUAL(
				dev::jsonCompactPrint(error),
				"{\"component\":\"general\",\"formattedMessage\":\"fileA:1:23: ParserError: Expected identifier but got '}'\\n"
				"contract A { function }\\n                      ^\\n\",\"message\":\"Expected identifier but got '}'\","
				"\"severity\":\"error\",\"sourceLocation\":{\"end\":23,\"file\":\"fileA\",\"start\":22},\"type\":\"ParserError\"}"
			);
		}
	}
}

BOOST_AUTO_TEST_CASE(output_selection_explicit)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"abi"
					]
				}
			}
		},
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
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_contracts)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": {
					"*": [
						"abi"
					]
				}
			}
		},
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
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_files_single_contract)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
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
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_files_all_contracts)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"*": {
					"*": [
						"abi"
					]
				}
			}
		},
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
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_dependent_contract)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract B { } contract A { function f() public { new B(); } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[{\"constant\":false,\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
}

BOOST_AUTO_TEST_CASE(output_selection_dependent_contract_with_import)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"fileB\"; contract A { function f() public { new B(); } }"
			},
			"fileB": {
				"content": "contract B { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[{\"constant\":false,\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
}

BOOST_AUTO_TEST_CASE(filename_with_colon)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"http://github.com/ethereum/solidity/std/StandardToken.sol": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"http://github.com/ethereum/solidity/std/StandardToken.sol": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "http://github.com/ethereum/solidity/std/StandardToken.sol", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(library_filename_with_colon)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"evm.bytecode"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"git:library.sol\"; contract A { function f() public returns (uint) { return L.g(); } }"
			},
			"git:library.sol": {
				"content": "library L { function g() public returns (uint) { return 1; } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["evm"]["bytecode"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["git:library.sol"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["git:library.sol"]["L"].isArray());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["git:library.sol"]["L"][0].isObject());
}

BOOST_AUTO_TEST_CASE(libraries_invalid_top_level)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": "42"
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"libraries\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_entry)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": {
				"L": "42"
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Library entry is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_hex)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": {
				"library.sol": {
					"L": "0x4200000000000000000000000000000000000xx1"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Invalid library address (\"0x4200000000000000000000000000000000000xx1\") supplied."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_length)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": {
				"library.sol": {
					"L1": "0x42",
					"L2": "0x4200000000000000000000000000000000000001ff"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Library address is of invalid length."));
}

BOOST_AUTO_TEST_CASE(libraries_missing_hex_prefix)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": {
				"library.sol": {
					"L": "4200000000000000000000000000000000000001"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Library address is not prefixed with \"0x\"."));
}

BOOST_AUTO_TEST_CASE(library_linking)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"libraries": {
				"library.sol": {
					"L": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"A": [
						"evm.bytecode"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"library.sol\"; import \"library2.sol\"; contract A { function f() public returns (uint) { L2.g(); return L.g(); } }"
			},
			"library.sol": {
				"content": "library L { function g() public returns (uint) { return 1; } }"
			},
			"library2.sol": {
				"content": "library L2 { function g() public { } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["evm"]["bytecode"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"].isObject());
	BOOST_CHECK(!contract["evm"]["bytecode"]["linkReferences"]["library.sol"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["library2.sol"].isObject());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["library2.sol"]["L2"].isArray());
	BOOST_CHECK(contract["evm"]["bytecode"]["linkReferences"]["library2.sol"]["L2"][0].isObject());
}

BOOST_AUTO_TEST_CASE(evm_version)
{
	auto inputForVersion = [](string const& _version)
	{
		return R"(
			{
				"language": "Solidity",
				"sources": { "fileA": { "content": "contract A { }" } },
				"settings": {
					)" + _version + R"(
					"outputSelection": {
						"fileA": {
							"A": [ "metadata" ]
						}
					}
				}
			}
		)";
	};
	Json::Value result;
	result = compile(inputForVersion("\"evmVersion\": \"homestead\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"homestead\"") != string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"tangerineWhistle\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"tangerineWhistle\"") != string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"spuriousDragon\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"spuriousDragon\"") != string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"byzantium\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"byzantium\"") != string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"constantinople\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"constantinople\"") != string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"petersburg\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"petersburg\"") != string::npos);
	// test default
	result = compile(inputForVersion(""));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"evmVersion\":\"petersburg\"") != string::npos);
	// test invalid
	result = compile(inputForVersion("\"evmVersion\": \"invalid\","));
	BOOST_CHECK(result["errors"][0]["message"].asString() == "Invalid EVM version requested.");
}

BOOST_AUTO_TEST_CASE(optimizer_settings_default_disabled)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": { "A": [ "metadata" ] }
			}
		},
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
	BOOST_CHECK(contract["metadata"].isString());
	Json::Value metadata;
	BOOST_CHECK(jsonParseStrict(contract["metadata"].asString(), metadata));

	Json::Value const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.isMember("enabled"));
	BOOST_CHECK(optimizer["enabled"].asBool() == false);
	BOOST_CHECK(!optimizer.isMember("details"));
	BOOST_CHECK(optimizer["runs"].asUInt() == 200);
}

BOOST_AUTO_TEST_CASE(optimizer_settings_default_enabled)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": { "A": [ "metadata" ] }
			},
			"optimizer": { "enabled": true }
		},
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
	BOOST_CHECK(contract["metadata"].isString());
	Json::Value metadata;
	BOOST_CHECK(jsonParseStrict(contract["metadata"].asString(), metadata));

	Json::Value const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.isMember("enabled"));
	BOOST_CHECK(optimizer["enabled"].asBool() == true);
	BOOST_CHECK(!optimizer.isMember("details"));
	BOOST_CHECK(optimizer["runs"].asUInt() == 200);
}

BOOST_AUTO_TEST_CASE(optimizer_settings_details_exactly_as_default_disabled)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": { "A": [ "metadata" ] }
			},
			"optimizer": { "details": {
				"constantOptimizer" : false,
				"cse" : false,
				"deduplicate" : false,
				"jumpdestRemover" : true,
				"orderLiterals" : false,
				"peephole" : true
			} }
		},
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
	BOOST_CHECK(contract["metadata"].isString());
	Json::Value metadata;
	BOOST_CHECK(jsonParseStrict(contract["metadata"].asString(), metadata));

	Json::Value const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.isMember("enabled"));
	// enabled is switched to false instead!
	BOOST_CHECK(optimizer["enabled"].asBool() == false);
	BOOST_CHECK(!optimizer.isMember("details"));
	BOOST_CHECK(optimizer["runs"].asUInt() == 200);
}

BOOST_AUTO_TEST_CASE(optimizer_settings_details_different)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"fileA": { "A": [ "metadata" ] }
			},
			"optimizer": { "runs": 600, "details": {
				"constantOptimizer" : true,
				"cse" : false,
				"deduplicate" : true,
				"jumpdestRemover" : true,
				"orderLiterals" : false,
				"peephole" : true,
				"yul": true
			} }
		},
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
	BOOST_CHECK(contract["metadata"].isString());
	Json::Value metadata;
	BOOST_CHECK(jsonParseStrict(contract["metadata"].asString(), metadata));

	Json::Value const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(!optimizer.isMember("enabled"));
	BOOST_CHECK(optimizer.isMember("details"));
	BOOST_CHECK(optimizer["details"]["constantOptimizer"].asBool() == true);
	BOOST_CHECK(optimizer["details"]["cse"].asBool() == false);
	BOOST_CHECK(optimizer["details"]["deduplicate"].asBool() == true);
	BOOST_CHECK(optimizer["details"]["jumpdestRemover"].asBool() == true);
	BOOST_CHECK(optimizer["details"]["orderLiterals"].asBool() == false);
	BOOST_CHECK(optimizer["details"]["peephole"].asBool() == true);
	BOOST_CHECK(optimizer["details"]["yulDetails"].isObject());
	BOOST_CHECK_EQUAL(optimizer["details"].getMemberNames().size(), 8);
	BOOST_CHECK(optimizer["runs"].asUInt() == 600);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
