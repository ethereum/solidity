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
// SPDX-License-Identifier: GPL-3.0
/**
 * @date 2017
 * Unit tests for interface/StandardCompiler.h.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>
#include <libsolutil/JSON.h>
#include <libsolutil/CommonData.h>
#include <test/Metadata.h>
#include <test/Common.h>

#include <algorithm>
#include <set>
#include <utility>

using namespace solidity::evmasm;
using namespace std::string_literals;

namespace solidity::frontend::test
{

namespace
{

langutil::Error::Severity str2Severity(std::string const& _cat)
{
	std::map<std::string, langutil::Error::Severity> cats{
		{"info", langutil::Error::Severity::Info},
		{"Info", langutil::Error::Severity::Info},
		{"warning", langutil::Error::Severity::Warning},
		{"Warning", langutil::Error::Severity::Warning},
		{"error", langutil::Error::Severity::Error},
		{"Error", langutil::Error::Severity::Error}
	};
	return cats.at(_cat);
}

/// Helper to match a specific error type and message
bool containsError(Json const& _compilerResult, std::string const& _type, std::string const& _message)
{
	if (!_compilerResult.contains("errors"))
		return false;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.is_object());
		BOOST_REQUIRE(error["type"].is_string());
		BOOST_REQUIRE(error["message"].is_string());
		if ((error["type"].get<std::string>() == _type) && (error["message"].get<std::string>() == _message))
			return true;
	}

	return false;
}

bool containsAtMostWarnings(Json const& _compilerResult)
{
	if (!_compilerResult.contains("errors"))
		return true;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.is_object());
		BOOST_REQUIRE(error["severity"].is_string());
		if (langutil::Error::isError(str2Severity(error["severity"].get<std::string>())))
			return false;
	}

	return true;
}

Json getContractResult(Json const& _compilerResult, std::string const& _file, std::string const& _name)
{
	if (!_compilerResult.contains("contracts") ||
		!_compilerResult["contracts"].is_object() ||
		!_compilerResult["contracts"][_file].is_object() ||
		!_compilerResult["contracts"][_file][_name].is_object()
	)
		return Json();
	return _compilerResult["contracts"][_file][_name];
}

void checkLinkReferencesSchema(Json const& _contractResult)
{
	BOOST_TEST_REQUIRE(_contractResult.is_object());
	BOOST_TEST_REQUIRE(_contractResult["evm"]["bytecode"].is_object());

	Json const& linkReferenceResult = _contractResult["evm"]["bytecode"]["linkReferences"];
	BOOST_TEST_REQUIRE(linkReferenceResult.is_object());

	for (auto const& [fileName, references]: linkReferenceResult.items())
	{
		BOOST_TEST_REQUIRE(references.is_object());
		for (auto const& [libraryName, libraryValue]: references.items())
		{
			BOOST_TEST_REQUIRE(libraryValue.is_array());
			BOOST_TEST_REQUIRE(!libraryValue.empty());
			for (size_t i = 0; i < static_cast<size_t>(linkReferenceResult.size()); ++i)
			{
				BOOST_TEST_REQUIRE(libraryValue[i].is_object());
				BOOST_TEST_REQUIRE(libraryValue[i].size() == 2);
				BOOST_TEST_REQUIRE(libraryValue[i]["length"].is_number_unsigned());
				BOOST_TEST_REQUIRE(libraryValue[i]["start"].is_number_unsigned());
			}
		}
	}
}

void expectLinkReferences(Json const& _contractResult, std::map<std::string, std::set<std::string>> const& _expectedLinkReferences)
{
	checkLinkReferencesSchema(_contractResult);

	Json const& linkReferenceResult = _contractResult["evm"]["bytecode"]["linkReferences"];
	BOOST_TEST(linkReferenceResult.size() == _expectedLinkReferences.size());

	for (auto const& [fileName, libraries]: _expectedLinkReferences)
	{
		BOOST_TEST(linkReferenceResult.contains(fileName));
		BOOST_TEST(linkReferenceResult[fileName].size() == libraries.size());
		for (std::string const& libraryName: libraries)
			BOOST_TEST(linkReferenceResult[fileName].contains(libraryName));
	}
}

Json compile(std::string _input)
{
	StandardCompiler compiler;
	std::string output = compiler.compile(std::move(_input));
	Json ret;
	BOOST_REQUIRE(util::jsonParseStrict(output, ret));
	return ret;
}

Json createLanguageAndSourcesSection(std::string const& _language, std::map<std::string, Json> const& _sources, bool _contentNode = true)
{
	Json result = Json::object();
	result["language"] = _language;
	result["sources"] = Json::object();
	for (auto const& source: _sources)
	{
		result["sources"][source.first] = Json::object();
		if (_contentNode)
			result["sources"][source.first]["content"] = source.second;
		else
			result["sources"][source.first] = source.second;
	}
	return result;
}

class Code
{
public:
	virtual ~Code() = default;
	explicit Code(std::map<std::string, Json> _code = {}) : m_code(std::move(_code)) {}
	[[nodiscard]] virtual Json json() const = 0;
protected:
	std::map<std::string, Json> m_code;
};

class SolidityCode: public Code
{
public:
	explicit SolidityCode(std::map<std::string, Json> _code = {
		{"fileA", "pragma solidity >=0.0; contract C { function f() public pure {} }"}
	}) : Code(std::move(_code)) {}
	[[nodiscard]] Json json() const override
	{
		return createLanguageAndSourcesSection("Solidity", m_code);
	}
};

class YulCode: public Code
{
public:
	explicit YulCode(std::map<std::string, Json> _code = {
		{"fileA", "{}"}
	}) : Code(std::move(_code)) {}
	[[nodiscard]] Json json() const override
	{
		return createLanguageAndSourcesSection("Yul", m_code);
	}
};

class EvmAssemblyCode: public Code
{
public:
	explicit EvmAssemblyCode(std::map<std::string, Json> _code = {
		{"fileA", Json::parse(R"(
			{
				"assemblyJson": {
					".code": [
						{
							"begin": 36,
							"end": 51,
							"name": "PUSH",
							"source": 0,
							"value": "0"
						}
					],
					"sourceList": [
						"<stdin>"
					]
				}
			}
			)")}
	}) : Code(std::move(_code)) {}
	[[nodiscard]] Json json() const override
	{
		return createLanguageAndSourcesSection("EVMAssembly", m_code, false);
	}
};

class SolidityAstCode: public Code
{
public:
	explicit SolidityAstCode(std::map<std::string, Json> _code = {
		{"fileA", Json::parse(R"(
		{
			"ast": {
				"absolutePath": "empty_contract.sol",
				"exportedSymbols": {
					"test": [
						1
					]
				},
				"id": 2,
				"nodeType": "SourceUnit",
				"nodes": [
				{
					"abstract": false,
					"baseContracts": [],
					"canonicalName": "test",
					"contractDependencies": [],
					"contractKind": "contract",
					"fullyImplemented": true,
					"id": 1,
					"linearizedBaseContracts": [
						1
					],
					"name": "test",
					"nameLocation": "9:4:0",
					"nodeType": "ContractDefinition",
					"nodes": [],
					"scope": 2,
					"src": "0:17:0",
					"usedErrors": []
				}
				],
			"src": "0:124:0"
			},
			"id": 0
		}
		)")}
	}) : Code(std::move(_code)) {}
	[[nodiscard]] Json json() const override
	{
		return createLanguageAndSourcesSection("SolidityAST", m_code);
	}
};

Json generateStandardJson(bool _viaIr, Json const& _debugInfoSelection, Json const& _outputSelection, Code const& _code = SolidityCode(), bool _advancedOutputSelection = false)
{
	Json result = _code.json();
	result["settings"] = Json::object();
	result["settings"]["viaIR"] = _viaIr;
	if (!_debugInfoSelection.empty())
		result["settings"]["debug"]["debugInfo"] = _debugInfoSelection;
	if (_advancedOutputSelection)
		result["settings"]["outputSelection"] = _outputSelection;
	else
		result["settings"]["outputSelection"]["*"]["*"] = _outputSelection;
	return result;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(StandardCompiler)

BOOST_AUTO_TEST_CASE(assume_object_input)
{
	Json result;

	/// Use the native JSON interface of StandardCompiler to trigger these
	frontend::StandardCompiler compiler;
	result = compiler.compile(Json());
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));
	result = compiler.compile(Json("INVALID"));
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));

	/// Use the string interface of StandardCompiler to trigger these
	result = compile("");
	BOOST_CHECK(containsError(result, "JSONError", "parse error at line 1, column 1: attempting to parse an empty input; check that your input string or stream contains the expected JSON"));
	result = compile("invalid");
	BOOST_CHECK(containsError(result, "JSONError", "parse error at line 1, column 1: syntax error while parsing value - invalid literal; last read: 'i'"));
	result = compile("\"invalid\"");
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));
	result = compile("{}");
	BOOST_CHECK(containsError(result, "JSONError", "No input sources specified."));
	BOOST_CHECK(!containsAtMostWarnings(result));
}

BOOST_AUTO_TEST_CASE(invalid_language)
{
	char const* input = R"(
	{
		"language": "INVALID",
		"sources": { "name": { "content": "abc" } }
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Only \"Solidity\", \"Yul\", \"SolidityAST\" or \"EVMAssembly\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(valid_language)
{
	char const* input = R"(
	{
		"language": "Solidity"
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(!containsError(result, "JSONError", "Only \"Solidity\" or \"Yul\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(no_sources)
{
	char const* input = R"(
	{
		"language": "Solidity"
	}
	)";
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "parse error at line 10, column 2: syntax error while parsing value - unexpected '}'; expected end of input"));
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
					"": [ "ast" ]
				}
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
	BOOST_CHECK(contract["devdoc"].is_object());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["devdoc"]), R"({"kind":"dev","methods":{},"version":1})");
	BOOST_CHECK(contract["userdoc"].is_object());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["userdoc"]), R"({"kind":"user","methods":{},"version":1})");
	BOOST_CHECK(contract["evm"].is_object());
	/// @TODO check evm.methodIdentifiers, legacyAssembly, bytecode, deployedBytecode
	BOOST_CHECK(contract["evm"]["bytecode"].is_object());
	BOOST_CHECK(contract["evm"]["bytecode"]["object"].is_string());
	BOOST_CHECK_EQUAL(
		solidity::test::bytecodeSansMetadata(contract["evm"]["bytecode"]["object"].get<std::string>()),
		std::string("6080604052348015600e575f5ffd5b5060") +
		(VersionIsRelease ? "3e" : util::toHex(bytes{uint8_t(60 + VersionStringStrict.size())})) +
		"80601a5f395ff3fe60806040525f5ffdfe"
	);
	BOOST_CHECK(contract["evm"]["assembly"].is_string());
	BOOST_CHECK(contract["evm"]["assembly"].get<std::string>().find(
		"    /* \"fileA\":0:14  contract A { } */\n  mstore(0x40, 0x80)\n  "
		"callvalue\n  dup1\n  "
		"iszero\n  tag_1\n  jumpi\n  "
		"revert(0x00, 0x00)\n"
		"tag_1:\n  pop\n  dataSize(sub_0)\n  dup1\n  "
		"dataOffset(sub_0)\n  0x00\n  codecopy\n  0x00\n  return\nstop\n\nsub_0: assembly {\n        "
		"/* \"fileA\":0:14  contract A { } */\n      mstore(0x40, 0x80)\n      "
		"revert(0x00, 0x00)\n\n    auxdata: 0xa26469706673582212"
	) == 0);
	BOOST_CHECK(contract["evm"]["gasEstimates"].is_object());
	BOOST_CHECK_EQUAL(contract["evm"]["gasEstimates"].size(), 1);
	BOOST_CHECK(contract["evm"]["gasEstimates"]["creation"].is_object());
	BOOST_CHECK_EQUAL(contract["evm"]["gasEstimates"]["creation"].size(), 3);
	BOOST_CHECK(contract["evm"]["gasEstimates"]["creation"]["codeDepositCost"].is_string());
	BOOST_CHECK(contract["evm"]["gasEstimates"]["creation"]["executionCost"].is_string());
	BOOST_CHECK(contract["evm"]["gasEstimates"]["creation"]["totalCost"].is_string());
	BOOST_CHECK_EQUAL(
		u256(contract["evm"]["gasEstimates"]["creation"]["codeDepositCost"].get<std::string>()) +
		u256(contract["evm"]["gasEstimates"]["creation"]["executionCost"].get<std::string>()),
		u256(contract["evm"]["gasEstimates"]["creation"]["totalCost"].get<std::string>())
	);
	// Lets take the top level `.code` section (the "deployer code"), that should expose most of the features of
	// the assembly JSON. What we want to check here is Operation, Push, PushTag, PushSub, PushSubSize and Tag.
	BOOST_CHECK(contract["evm"]["legacyAssembly"].is_object());
	BOOST_CHECK(contract["evm"]["legacyAssembly"][".code"].is_array());
	BOOST_CHECK_EQUAL(
		util::jsonCompactPrint(contract["evm"]["legacyAssembly"][".code"]),
		"[{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"80\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"40\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"MSTORE\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"CALLVALUE\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"DUP1\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"ISZERO\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH [tag]\",\"source\":0,\"value\":\"1\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"JUMPI\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"REVERT\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"tag\",\"source\":0,\"value\":\"1\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"JUMPDEST\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"POP\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH #[$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"DUP1\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH [$]\",\"source\":0,\"value\":\"0000000000000000000000000000000000000000000000000000000000000000\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"CODECOPY\",\"source\":0},"
		"{\"begin\":0,\"end\":14,\"name\":\"PUSH\",\"source\":0,\"value\":\"0\"},"
		"{\"begin\":0,\"end\":14,\"name\":\"RETURN\",\"source\":0}]"
	);
	BOOST_CHECK(contract["metadata"].is_string());
	BOOST_CHECK(solidity::test::isValidMetadata(contract["metadata"].get<std::string>()));
	BOOST_CHECK(result["sources"].is_object());
	BOOST_CHECK(result["sources"]["fileA"].is_object());
	BOOST_CHECK(result["sources"]["fileA"]["ast"].is_object());
	BOOST_CHECK_EQUAL(
		util::jsonCompactPrint(result["sources"]["fileA"]["ast"]),
		"{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]},\"id\":2,\"nodeType\":\"SourceUnit\",\"nodes\":[{\"abstract\":false,"
		"\"baseContracts\":[],\"canonicalName\":\"A\",\"contractDependencies\":[],"
		"\"contractKind\":\"contract\",\"fullyImplemented\":true,\"id\":1,"
		"\"linearizedBaseContracts\":[1],\"name\":\"A\",\"nameLocation\":\"9:1:0\",\"nodeType\":\"ContractDefinition\",\"nodes\":[],\"scope\":2,"
		"\"src\":\"0:14:0\",\"usedErrors\":[],\"usedEvents\":[]}],\"src\":\"0:14:0\"}"
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
	Json result = compile(input);
	BOOST_CHECK(result.contains("errors"));
	BOOST_CHECK(result["errors"].size() >= 1);
	for (auto const& error: result["errors"])
	{
		BOOST_REQUIRE(error.is_object());
		BOOST_REQUIRE(error["message"].is_string());
		if (error["message"].get<std::string>().find("pre-release compiler") == std::string::npos)
		{
			BOOST_CHECK_EQUAL(
				util::jsonCompactPrint(error),
				"{\"component\":\"general\",\"errorCode\":\"2314\",\"formattedMessage\":\"ParserError: Expected identifier but got '}'\\n"
				" --> fileA:1:23:\\n  |\\n1 | contract A { function }\\n  |                       ^\\n\\n\",\"message\":\"Expected identifier but got '}'\","
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[{\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[{\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "http://github.com/ethereum/solidity/std/StandardToken.sol", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["abi"].is_array());
	BOOST_CHECK_EQUAL(util::jsonCompactPrint(contract["abi"]), "[]");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	expectLinkReferences(contract, {{"git:library.sol", {"L"}}});
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
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
	Json result = compile(input);
	BOOST_TEST(containsAtMostWarnings(result));
	Json contractResult = getContractResult(result, "fileA", "A");
	expectLinkReferences(contractResult, {{"library2.sol", {"L2"}}});
}

BOOST_AUTO_TEST_CASE(linking_yul)
{
	char const* input = R"(
	{
		"language": "Yul",
		"settings": {
			"libraries": {
				"fileB": {
					"L": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"*": [
						"evm.bytecode.linkReferences"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "object \"a\" { code { let addr := linkersymbol(\"fileB:L\") sstore(0, addr) } }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_TEST(containsAtMostWarnings(result));
	Json contractResult = getContractResult(result, "fileA", "a");
	expectLinkReferences(contractResult, {});
}

BOOST_AUTO_TEST_CASE(linking_yul_empty_link_reference)
{
	char const* input = R"(
	{
		"language": "Yul",
		"settings": {
			"libraries": {
				"": {
					"": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"*": [
						"evm.bytecode.linkReferences"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "object \"a\" { code { let addr := linkersymbol(\"\") sstore(0, addr) } }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_TEST(containsAtMostWarnings(result));
	Json contractResult = getContractResult(result, "fileA", "a");
	expectLinkReferences(contractResult, {{"", {""}}});
}

BOOST_AUTO_TEST_CASE(linking_yul_no_filename_in_link_reference)
{
	char const* input = R"(
	{
		"language": "Yul",
		"settings": {
			"libraries": {
				"": {
					"L": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"*": [
						"evm.bytecode.linkReferences"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "object \"a\" { code { let addr := linkersymbol(\"L\") sstore(0, addr) } }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_TEST(containsAtMostWarnings(result));
	Json contractResult = getContractResult(result, "fileA", "a");
	expectLinkReferences(contractResult, {{"", {"L"}}});
}

BOOST_AUTO_TEST_CASE(linking_yul_same_library_name_different_files)
{
	char const* input = R"(
	{
		"language": "Yul",
		"settings": {
			"libraries": {
				"fileB": {
					"L": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"*": [
						"evm.bytecode.linkReferences"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "object \"a\" { code { let addr := linkersymbol(\"fileC:L\") sstore(0, addr) } }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_TEST(containsAtMostWarnings(result));
	Json contractResult = getContractResult(result, "fileA", "a");
	expectLinkReferences(contractResult, {{"fileC", {"L"}}});
}

BOOST_AUTO_TEST_CASE(evm_version)
{
	auto inputForVersion = [](std::string const& _version)
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
	Json result;
	result = compile(inputForVersion("\"evmVersion\": \"homestead\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"homestead\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"tangerineWhistle\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"tangerineWhistle\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"spuriousDragon\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"spuriousDragon\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"byzantium\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"byzantium\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"constantinople\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"constantinople\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"petersburg\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"petersburg\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"istanbul\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"istanbul\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"berlin\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"berlin\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"london\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"london\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"paris\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"paris\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"shanghai\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"shanghai\"") != std::string::npos);
	result = compile(inputForVersion("\"evmVersion\": \"cancun\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"cancun\"") != std::string::npos);
	// test default
	result = compile(inputForVersion(""));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].get<std::string>().find("\"evmVersion\":\"cancun\"") != std::string::npos);
	// test invalid
	result = compile(inputForVersion("\"evmVersion\": \"invalid\","));
	BOOST_CHECK(result["errors"][0]["message"].get<std::string>() == "Invalid EVM version requested.");
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	Json metadata;
	BOOST_CHECK(util::jsonParseStrict(contract["metadata"].get<std::string>(), metadata));

	Json const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.contains("enabled"));
	BOOST_CHECK(optimizer["enabled"].get<bool>() == false);
	BOOST_CHECK(!optimizer.contains("details"));
	BOOST_CHECK(optimizer["runs"].get<unsigned>() == 200);
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	Json metadata;
	BOOST_CHECK(util::jsonParseStrict(contract["metadata"].get<std::string>(), metadata));

	Json const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.contains("enabled"));
	BOOST_CHECK(optimizer["enabled"].get<bool>() == true);
	BOOST_CHECK(!optimizer.contains("details"));
	BOOST_CHECK(optimizer["runs"].get<unsigned>() == 200);
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
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	Json metadata;
	BOOST_CHECK(util::jsonParseStrict(contract["metadata"].get<std::string>(), metadata));

	Json const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(optimizer.contains("enabled"));
	// enabled is switched to false instead!
	BOOST_CHECK(optimizer["enabled"].get<bool>() == false);
	BOOST_CHECK(!optimizer.contains("details"));
	BOOST_CHECK(optimizer["runs"].get<unsigned>() == 200);
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
				"yul": true,
				"inliner": true
			} }
		},
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	Json metadata;
	BOOST_CHECK(util::jsonParseStrict(contract["metadata"].get<std::string>(), metadata));

	Json const& optimizer = metadata["settings"]["optimizer"];
	BOOST_CHECK(!optimizer.contains("enabled"));
	BOOST_CHECK(optimizer.contains("details"));
	BOOST_CHECK(optimizer["details"]["constantOptimizer"].get<bool>() == true);
	BOOST_CHECK(optimizer["details"]["cse"].get<bool>() == false);
	BOOST_CHECK(optimizer["details"]["deduplicate"].get<bool>() == true);
	BOOST_CHECK(optimizer["details"]["jumpdestRemover"].get<bool>() == true);
	BOOST_CHECK(optimizer["details"]["orderLiterals"].get<bool>() == false);
	BOOST_CHECK(optimizer["details"]["peephole"].get<bool>() == true);
	BOOST_CHECK(optimizer["details"]["yul"].get<bool>() == true);
	BOOST_CHECK(optimizer["details"]["yulDetails"].is_object());
//	BOOST_CHECK(
//		util::convertContainer<std::set<std::string>>(optimizer["details"]["yulDetails"].getMemberNames()) ==
//		(std::set<std::string>{"stackAllocation", "optimizerSteps"})
//	);
	BOOST_CHECK(optimizer["details"]["yulDetails"]["stackAllocation"].get<bool>() == true);
	BOOST_CHECK(
		optimizer["details"]["yulDetails"]["optimizerSteps"].get<std::string>() ==
		OptimiserSettings::DefaultYulOptimiserSteps + ":"s + OptimiserSettings::DefaultYulOptimiserCleanupSteps
 	);
	BOOST_CHECK_EQUAL(optimizer["details"].size(), 10);
	BOOST_CHECK(optimizer["runs"].get<unsigned>() == 600);
}

BOOST_AUTO_TEST_CASE(metadata_without_compilation)
{
	// NOTE: the contract code here should fail to compile due to "out of stack"
	// If the metadata is successfully returned, that means no compilation was attempted.
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
				"content": "contract A {
  function x(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h, uint i, uint j, uint k, uint l, uint m, uint n, uint o, uint p) pure public {}
  function y() pure public {
    uint a; uint b; uint c; uint d; uint e; uint f; uint g; uint h; uint i; uint j; uint k; uint l; uint m; uint n; uint o; uint p;
    x(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
  }
}"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	BOOST_CHECK(solidity::test::isValidMetadata(contract["metadata"].get<std::string>()));
}


BOOST_AUTO_TEST_CASE(license_in_metadata)
{
	std::string const input = R"(
			{
				"language": "Solidity",
				"sources": {
					"fileA": { "content": "import \"fileB\"; contract A { } // SPDX-License-Identifier: GPL-3.0 \n" },
					"fileB": { "content": "import \"fileC\"; /* SPDX-License-Identifier: MIT */ contract B { }" },
					"fileC": { "content": "import \"fileD\"; /* SPDX-License-Identifier: MIT AND GPL-3.0 */ contract C { }" },
					"fileD": { "content": "// SPDX-License-Identifier: (GPL-3.0+ OR MIT) AND MIT \n import \"fileE\"; contract D { }" },
					"fileE": { "content": "import \"fileF\"; /// SPDX-License-Identifier: MIT   \n contract E { }" },
					"fileF": { "content": "/*\n * SPDX-License-Identifier: MIT\n */ contract F { }" }
				},
				"settings": {
					"outputSelection": {
						"fileA": {
							"*": [ "metadata" ]
						}
					}
				}
			}
		)";
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	Json metadata;
	BOOST_REQUIRE(util::jsonParseStrict(contract["metadata"].get<std::string>(), metadata));
	BOOST_CHECK_EQUAL(metadata["sources"]["fileA"]["license"], "GPL-3.0");
	BOOST_CHECK_EQUAL(metadata["sources"]["fileB"]["license"], "MIT");
	BOOST_CHECK_EQUAL(metadata["sources"]["fileC"]["license"], "MIT AND GPL-3.0");
	BOOST_CHECK_EQUAL(metadata["sources"]["fileD"]["license"], "(GPL-3.0+ OR MIT) AND MIT");
	// This is actually part of the docstring, but still picked up
	// because the source location of the contract does not cover the docstring.
	BOOST_CHECK_EQUAL(metadata["sources"]["fileE"]["license"], "MIT");
	BOOST_CHECK_EQUAL(metadata["sources"]["fileF"]["license"], "MIT");
}

BOOST_AUTO_TEST_CASE(common_pattern)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"outputSelection": {
				"*": {
					"*": [ "evm.bytecode.object", "metadata" ]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { function f() pure public {} }"
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.is_object());
	BOOST_CHECK(contract["metadata"].is_string());
	BOOST_CHECK(solidity::test::isValidMetadata(contract["metadata"].get<std::string>()));
	BOOST_CHECK(contract["evm"]["bytecode"].is_object());
	BOOST_CHECK(contract["evm"]["bytecode"]["object"].is_string());
}

BOOST_AUTO_TEST_CASE(use_stack_optimization)
{
	// NOTE: the contract code here should fail to compile due to "out of stack"
	// If we enable stack optimization, though, it will compile.
	char const* input = R"(
	{
		"language": "Solidity",
		"settings": {
			"optimizer": { "enabled": true, "details": { "yul": true } },
			"outputSelection": {
				"fileA": { "A": [ "evm.bytecode.object" ] }
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A {
					function y() public {
						assembly {
							function fun() -> a3, b3, c3, d3, e3, f3, g3, h3, i3, j3, k3, l3, m3, n3, o3, p3
							{
								let a := 1
								let b := 1
								let z3 := 1
								sstore(a, b)
								sstore(add(a, 1), b)
								sstore(add(a, 2), b)
								sstore(add(a, 3), b)
								sstore(add(a, 4), b)
								sstore(add(a, 5), b)
								sstore(add(a, 6), b)
								sstore(add(a, 7), b)
								sstore(add(a, 8), b)
								sstore(add(a, 9), b)
								sstore(add(a, 10), b)
								sstore(add(a, 11), b)
								sstore(add(a, 12), b)
							}
							let a1, b1, c1, d1, e1, f1, g1, h1, i1, j1, k1, l1, m1, n1, o1, p1 := fun()
							let a2, b2, c2, d2, e2, f2, g2, h2, i2, j2, k2, l2, m2, n2, o2, p2 := fun()
							sstore(a1, a2)
						}
					}
				}"
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_CHECK(containsAtMostWarnings(result));
	Json contract = getContractResult(result, "fileA", "A");
	BOOST_REQUIRE(contract.is_object());
	BOOST_REQUIRE(contract["evm"]["bytecode"]["object"].is_string());
	BOOST_CHECK(contract["evm"]["bytecode"]["object"].get<std::string>().length() > 20);

	// Now disable stack optimizations and UnusedFunctionParameterPruner (p)
	// results in "stack too deep"
	std::string optimiserSteps = OptimiserSettings::DefaultYulOptimiserSteps;
	optimiserSteps.erase(
		remove_if(optimiserSteps.begin(), optimiserSteps.end(), [](char ch) { return ch == 'p'; }),
		optimiserSteps.end()
	);
	parsedInput["settings"]["optimizer"]["details"]["yulDetails"]["stackAllocation"] = false;
	parsedInput["settings"]["optimizer"]["details"]["yulDetails"]["optimizerSteps"] = optimiserSteps;

	result = compiler.compile(parsedInput);
	BOOST_REQUIRE(result["errors"].is_array());
	BOOST_CHECK(result["errors"][0]["severity"] == "error");
	BOOST_REQUIRE(result["errors"][0]["message"].is_string());
	BOOST_CHECK(result["errors"][0]["message"].get<std::string>().find("When compiling inline assembly") != std::string::npos);
	BOOST_CHECK(result["errors"][0]["type"] == "CompilerError");
}

BOOST_AUTO_TEST_CASE(standard_output_selection_wildcard)
{
	char const* input = R"(
	{
		"language": "Solidity",
			"sources":
		{
			"A":
			{
				"content": "pragma solidity >=0.0; contract C { function f() public pure {} }"
			}
		},
		"settings":
		{
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["A"].is_object());
	BOOST_REQUIRE(result["contracts"]["A"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["A"]["C"].is_object());
	BOOST_REQUIRE(result["contracts"]["A"]["C"]["evm"].is_object());
	BOOST_REQUIRE(result["contracts"]["A"]["C"]["evm"]["bytecode"].is_object());
	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 1);
	BOOST_REQUIRE(result["sources"]["A"].is_object());

}

BOOST_AUTO_TEST_CASE(standard_output_selection_wildcard_colon_source)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{
			":A":
			{
				"content": "pragma solidity >=0.0; contract C { function f() public pure {} }"
			}
		},
		"settings":
		{
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"][":A"].is_object());
	BOOST_REQUIRE(result["contracts"][":A"].size() == 1);
	BOOST_REQUIRE(result["contracts"][":A"]["C"].is_object());
	BOOST_REQUIRE(result["contracts"][":A"]["C"]["evm"].is_object());
	BOOST_REQUIRE(result["contracts"][":A"]["C"]["evm"]["bytecode"].is_object());
	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 1);
	BOOST_REQUIRE(result["sources"][":A"].is_object());
}

BOOST_AUTO_TEST_CASE(standard_output_selection_wildcard_empty_source)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{
			"":
			{
				"content": "pragma solidity >=0.0; contract C { function f() public pure {} }"
			}
		},
		"settings":
		{
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"][""].is_object());
	BOOST_REQUIRE(result["contracts"][""].size() == 1);
	BOOST_REQUIRE(result["contracts"][""]["C"].is_object());
	BOOST_REQUIRE(result["contracts"][""]["C"]["evm"].is_object());
	BOOST_REQUIRE(result["contracts"][""]["C"]["evm"]["bytecode"].is_object());
	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 1);
	BOOST_REQUIRE(result["sources"][""].is_object());
}

BOOST_AUTO_TEST_CASE(standard_output_selection_wildcard_multiple_sources)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{
			"A":
			{
				"content": "pragma solidity >=0.0; contract C { function f() public pure {} }"
			},
			"B":
			{
				"content": "pragma solidity >=0.0; contract D { function f() public pure {} }"
			}
		},
		"settings":
		{
			"outputSelection":
			{
				"*": { "D": ["evm.bytecode"] }
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["B"].is_object());
	BOOST_REQUIRE(result["contracts"]["B"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["B"]["D"].is_object());
	BOOST_REQUIRE(result["contracts"]["B"]["D"]["evm"].is_object());
	BOOST_REQUIRE(result["contracts"]["B"]["D"]["evm"]["bytecode"].is_object());
	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 2);
	BOOST_REQUIRE(result["sources"]["A"].is_object());
	BOOST_REQUIRE(result["sources"]["B"].is_object());
}

BOOST_AUTO_TEST_CASE(stopAfter_invalid_value)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{ "": { "content": "pragma solidity >=0.0; contract C { function f() public pure {} }" } },
		"settings":
		{
			"stopAfter": "rrr",
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Invalid value for \"settings.stopAfter\". Only valid value is \"parsing\"."));
}

BOOST_AUTO_TEST_CASE(stopAfter_invalid_type)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{ "": { "content": "pragma solidity >=0.0; contract C { function f() public pure {} }" } },
		"settings":
		{
			"stopAfter": 3,
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"settings.stopAfter\" must be a string."));
}

BOOST_AUTO_TEST_CASE(stopAfter_bin_conflict)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources":
		{ "": { "content": "pragma solidity >=0.0; contract C { function f() public pure {} }" } },
		"settings":
		{
			"stopAfter": "parsing",
			"outputSelection":
			{
				"*": { "C": ["evm.bytecode"] }
			}
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Requested output selection conflicts with \"settings.stopAfter\"."));
}

BOOST_AUTO_TEST_CASE(stopAfter_ast_output)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"a.sol": {
				"content": "// SPDX-License-Identifier: GPL-3.0\nimport \"tes32.sol\";\n contract C is X { constructor() {} }"
			}
		},
		"settings": {
			"stopAfter": "parsing",
			"outputSelection": { "*": { "": [ "ast" ] } }
		}
	}
	)";
	Json result = compile(input);
	BOOST_CHECK(result["sources"].is_object());
	BOOST_CHECK(result["sources"]["a.sol"].is_object());
	BOOST_CHECK(result["sources"]["a.sol"]["ast"].is_object());
}

BOOST_AUTO_TEST_CASE(dependency_tracking_of_abstract_contract)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"BlockRewardAuRaBase.sol": {
				"content": " contract Sacrifice { constructor() payable {} } abstract contract BlockRewardAuRaBase { function _transferNativeReward() internal { new Sacrifice(); } function _distributeTokenRewards() internal virtual; } "
			},
			"BlockRewardAuRaCoins.sol": {
				"content": " import \"./BlockRewardAuRaBase.sol\"; contract BlockRewardAuRaCoins is BlockRewardAuRaBase { function transferReward() public { _transferNativeReward(); } function _distributeTokenRewards() internal override {} } "
			}
		},
		"settings": {
			"outputSelection": {
				"BlockRewardAuRaCoins.sol": {
					"BlockRewardAuRaCoins": ["ir", "evm.bytecode.sourceMap"]
				}
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"].is_object());
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"]["BlockRewardAuRaCoins"].is_object());
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"]["BlockRewardAuRaCoins"]["evm"].is_object());
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"]["BlockRewardAuRaCoins"]["ir"].is_string());
	BOOST_REQUIRE(result["contracts"]["BlockRewardAuRaCoins.sol"]["BlockRewardAuRaCoins"]["evm"]["bytecode"].is_object());
	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 2);
}

BOOST_AUTO_TEST_CASE(dependency_tracking_of_abstract_contract_yul)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"A.sol": {
				"content": "contract A {} contract B {} contract C { constructor() { new B(); } } contract D {}"
			}
		},
		"settings": {
			"outputSelection": {
				"A.sol": {
					"C": ["ir"]
				}
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	BOOST_REQUIRE(result["contracts"].is_object());
	BOOST_REQUIRE(result["contracts"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["A.sol"].is_object());
	BOOST_REQUIRE(result["contracts"]["A.sol"].size() == 1);
	BOOST_REQUIRE(result["contracts"]["A.sol"]["C"].is_object());
	BOOST_REQUIRE(result["contracts"]["A.sol"]["C"]["ir"].is_string());

	const std::string& irCode = result["contracts"]["A.sol"]["C"]["ir"].get<std::string>();

	// Make sure C and B contracts are deployed
	BOOST_REQUIRE(irCode.find("object \"C") != std::string::npos);
	BOOST_REQUIRE(irCode.find("object \"B") != std::string::npos);

	// Make sure A and D are NOT deployed as they were not requested and are not
	// in any dependency
	BOOST_REQUIRE(irCode.find("object \"A") == std::string::npos);
	BOOST_REQUIRE(irCode.find("object \"D") == std::string::npos);


	BOOST_REQUIRE(result["sources"].is_object());
	BOOST_REQUIRE(result["sources"].size() == 1);
}

BOOST_AUTO_TEST_CASE(source_location_of_bare_block)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"A.sol": {
				"content": "contract A { constructor() { uint x = 2; { uint y = 3; } } }"
			}
		},
		"settings": {
			"outputSelection": {
				"A.sol": {
					"A": ["evm.bytecode.sourceMap"]
				}
			}
		}
	}
	)";

	Json parsedInput;
	BOOST_REQUIRE(util::jsonParseStrict(input, parsedInput));

	solidity::frontend::StandardCompiler compiler;
	Json result = compiler.compile(parsedInput);

	std::string sourceMap = result["contracts"]["A.sol"]["A"]["evm"]["bytecode"]["sourceMap"].get<std::string>();

	// Check that the bare block's source location is referenced.
	std::string sourceRef =
		";" +
		std::to_string(std::string{"contract A { constructor() { uint x = 2; "}.size()) +
		":" +
		std::to_string(std::string{"{ uint y = 3; }"}.size());
	BOOST_REQUIRE(sourceMap.find(sourceRef) != std::string::npos);
}

BOOST_AUTO_TEST_CASE(ethdebug_excluded_from_wildcards)
{
	frontend::StandardCompiler compiler;
	// excluded from output selection wildcard
	Json result = compiler.compile(generateStandardJson(true, {}, Json::array({"*"})));
	BOOST_REQUIRE(result.dump().find("ethdebug") == std::string::npos);
	// excluded from debug info selection wildcard
	result = compiler.compile(generateStandardJson(true, {"*"}, Json::array({"ir"})));
	BOOST_REQUIRE(result.dump().find("ethdebug") == std::string::npos);
	// excluded from both - just in case ;)
	result = compiler.compile(generateStandardJson(true, {"*"}, Json::array({"*"})));
	BOOST_REQUIRE(result.dump().find("ethdebug") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(ethdebug_debug_info_ethdebug)
{
	static std::vector<std::tuple<Json, std::string, std::optional<std::function<bool(Json)>>>> tests{
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"*"})),
			"'settings.debug.debugInfo' can only include 'ethdebug', if output 'ir', 'irOptimized', 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected.",
			std::nullopt,
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"*"})),
			"'settings.debug.debugInfo' can only include 'ethdebug', if output 'ir', 'irOptimized', 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected.",
			std::nullopt,
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt,
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt,
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt,
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"ir"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"irOptimized"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"ir", "evm.bytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"ir", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"ir", "evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"irOptimized", "evm.bytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"irOptimized", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"irOptimized", "evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"ir"}), YulCode()),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"irOptimized"}), YulCode()),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos;
			}
		},
		{
			generateStandardJson(true, Json::array({"ethdebugs"}), Json::array({"irOptimized"}), YulCode()),
			"Invalid value in settings.debug.debugInfo.",
			{}
		},
		{
			generateStandardJson(
				true, Json::array({"ethdebug"}), {
					{"fileA", {{"contractA", Json::array({"evm.deployedBytecode.bin"})}}},
					{"fileB", {{"contractB", Json::array({"evm.bytecode.bin"})}}}
				},
				SolidityCode({
					{"fileA", "pragma solidity >=0.0; contract contractA { function f() public pure {} }"},
					{"fileB", "pragma solidity >=0.0; contract contractB { function f() public pure {} }"}
				}), true
			),
			"'settings.debug.debugInfo' can only include 'ethdebug', if output 'ir', 'irOptimized', 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected.",
			std::nullopt,
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"*"}), EvmAssemblyCode()),
			"'settings.debug.debugInfo' 'ethdebug' is only supported for languages 'Solidity' and 'Yul'.",
			std::nullopt,
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"*"}), SolidityAstCode()),
			"'settings.debug.debugInfo' 'ethdebug' is only supported for languages 'Solidity' and 'Yul'.",
			std::nullopt,
		},
	};
	frontend::StandardCompiler compiler;
	for (auto const& test: tests)
	{
		Json result = compiler.compile(std::get<0>(test));
		BOOST_REQUIRE(!std::get<1>(test).empty() ? result.contains("errors") : result.contains("contracts"));
		if (!std::get<1>(test).empty())
			for (auto const& e: result["errors"])
				BOOST_REQUIRE(e["message"] == std::get<1>(test));
		if (std::get<2>(test).has_value())
			BOOST_REQUIRE((*std::get<2>(test))(result));
	}
}

BOOST_AUTO_TEST_CASE(ethdebug_ethdebug_output)
{
	static std::vector<std::tuple<Json, std::string, std::optional<std::function<bool(Json)>>>> tests{
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(false, {}, Json::array({"evm.bytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(false, {}, Json::array({"evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(false, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(false, {}, Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			"'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' can only be selected as output, if 'viaIR' was set.",
			std::nullopt
		},
		{
			generateStandardJson(true, Json::array({"location"}), Json::array({"evm.bytecode.ethdebug"})),
			"'ethdebug' needs to be enabled in 'settings.debug.debugInfo', if 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected as output.",
			std::nullopt
		},
		{
			generateStandardJson(true, Json::array({"location"}), Json::array({"evm.deployedBytecode.ethdebug"})),
			"'ethdebug' needs to be enabled in 'settings.debug.debugInfo', if 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected as output.",
			std::nullopt
		},
		{
			generateStandardJson(true, Json::array({"location"}), Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			"'ethdebug' needs to be enabled in 'settings.debug.debugInfo', if 'evm.bytecode.ethdebug' or 'evm.deployedBytecode.ethdebug' was selected as output.",
			std::nullopt
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, Json::array({"ethdebug"}), Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug") &&
					 result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"})),
			{},
			[](const Json& result)
			{
				return result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug") &&
					 result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug", "ir"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos && result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.deployedBytecode.ethdebug", "ir"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos && result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebugs"})),
			{},
			[](const Json& result)
			{
				return !result.contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.deployedBytecode.ethdebugs"})),
			{},
			[](const Json& result)
			{
				return !result.contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug", "ir"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos && result.contains("ethdebug") && result["contracts"]["fileA"]["C"]["evm"]["deployedBytecode"].contains("ethdebug") &&
					 result["contracts"]["fileA"]["C"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug", "ir"}), YulCode()),
			{},
			[](const Json& result)
			{
				return result.dump().find("/// ethdebug: enabled") != std::string::npos && result["contracts"]["fileA"]["object"]["evm"]["bytecode"].contains("ethdebug");
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.deployedBytecode.ethdebug", "ir"}), YulCode()),
			{"\"evm.deployedBytecode.ethdebug\" cannot be used for Yul."},
			std::nullopt
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug", "ir"}), YulCode()),
			{"\"evm.deployedBytecode.ethdebug\" cannot be used for Yul."},
			std::nullopt
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.bytecode"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("ethdebug") == std::string::npos;
			}
		},
		{
			generateStandardJson(true, {}, Json::array({"evm.deployedBytecode"})),
			{},
			[](const Json& result)
			{
				return result.dump().find("ethdebug") == std::string::npos;
			}
		},
		{
			generateStandardJson(
				true, {}, {
					{"fileA", {{"contractA", Json::array({"evm.deployedBytecode.ethdebug"})}}},
					{"fileB", {{"contractB", Json::array({"evm.bytecode.ethdebug"})}}}
				},
				SolidityCode({
					{"fileA", "pragma solidity >=0.0; contract contractA { function f() public pure {} }"},
					{"fileB", "pragma solidity >=0.0; contract contractB { function f() public pure {} }"}
				}), true
			),
			{},
			[](const Json& result)
			{
				return result["contracts"]["fileA"]["contractA"]["evm"]["deployedBytecode"].contains("ethdebug") &&
					result["contracts"]["fileB"]["contractB"]["evm"]["bytecode"].contains("ethdebug") ;
			}
		}
	};
	frontend::StandardCompiler compiler;
	for (auto const& test: tests)
	{
		Json result = compiler.compile(std::get<0>(test));
		if (!std::get<1>(test).empty())
			for (auto const& e: result["errors"])
				BOOST_REQUIRE(e["message"] == std::get<1>(test));
		if (std::get<2>(test).has_value())
			BOOST_REQUIRE((*std::get<2>(test))(result));
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
