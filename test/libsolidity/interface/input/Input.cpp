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

#include <libsolidity/interface/input/Input.h>
#include <libsolidity/interface/input/JSONSerialization.h>

#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace solidity::util;
using namespace solidity::test;

#define TEST_CASE_NAME (boost::unit_test::framework::current_test_case().p_name)

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(InputTests)

BOOST_AUTO_TEST_CASE(default_creation)
{
	input::Input input;

	BOOST_CHECK_EQUAL(input.language, "Solidity");
	BOOST_CHECK(input.sources.empty());
	BOOST_CHECK(!input.settings.has_value());
}

BOOST_AUTO_TEST_CASE(default_to_json)
{
	Json json = input::Input{};

	BOOST_CHECK_EQUAL(json["language"], "Solidity");
	BOOST_CHECK(json["sources"].empty());
}

BOOST_AUTO_TEST_CASE(full_serialization)
{
	using namespace solidity::frontend::input;

	input::Input input;
	input.language = "Solidity";

	// Sources
	input.sources["file.sol"] = Source{"pragma solidity ^0.8.0;\ncontract C {}"};
	Source sourceWithOptionalFields;
	sourceWithOptionalFields.keccak256 = "0xabc123";
	sourceWithOptionalFields.urls = std::vector<std::string>{"ipfs://Qma...", "/tmp/file.sol"};
	input.sources["lib.sol"] = sourceWithOptionalFields;

	// Settings
	Settings settings;
	settings.stopAfter = "parsing";
	settings.remappings = std::vector<std::string>{"g=/dir/"};
	settings.experimental = true;

	// Optimizer
	Optimizer optimizer;
	optimizer.enable = true;
	optimizer.runs = 200;
	YulOptimizerDetails yulDetails;
	yulDetails.stackAllocation = true;
	yulDetails.optimizerSteps = "dfDvulfnTUtnIf";
	OptimizerDetails details;
	details.peephole = true;
	details.inliner = false;
	details.jumpdestRemover = true;
	details.orderLiterals = true;
	details.deduplicate = false;
	details.cse = false;
	details.constantOptimizer = false;
	details.simpleCounterForLoopUncheckedIncrement = true;
	details.yul = true;
	details.yulDetails = yulDetails;
	optimizer.details = details;
	settings.optimizer = optimizer;

	// EVM version
	settings.evmVersion = langutil::EVMVersion{};
	settings.viaIR = true;
	settings.viaSSACFG = false;

	// Debug
	Debug debug;
	debug.revertStrings = RevertStrings::Debug;
	debug.debugInfo = std::vector<std::string>{"location", "snippet", "ast-id"};
	settings.debug = debug;

	// Metadata
	Metadata metadata;
	metadata.appendCBOR = true;
	metadata.bytecodeHash = MetadataHash::IPFS;
	metadata.useLiteralContent = false;
	settings.metadata = metadata;

	// Libraries
	LibraryAddresses libraries;
	libraries["myFile.sol"]["MyLib"] = "0x1234567890123456789012345678901234567890";
	settings.libraries = libraries;

	// Model checker
	ModelChecker modelChecker;
	modelChecker.contracts = std::map<std::string, std::vector<std::string>>{
		{"source.sol", std::vector<std::string>{"Contract1"}}
	};
	modelChecker.divModNoSlacks = false;
	modelChecker.engine = "chc";
	modelChecker.extCalls = "trusted";
	modelChecker.invariants = std::vector<std::string>{"contract", "reentrancy"};
	modelChecker.showProvedSafe = true;
	modelChecker.showUnproved = true;
	modelChecker.showUnsupported = true;
	modelChecker.solvers = std::vector<std::string>{"z3", "smtlib2"};
	modelChecker.targets = std::vector<std::string>{"assert", "underflow", "overflow"};
	modelChecker.timeout = 10000;
	modelChecker.bmcLoopIterations = 5;
	settings.modelChecker = modelChecker;

	// Output selection
	OutputSelection outputSelection;
	outputSelection["*"]["*"] = std::vector<std::string>{"abi", "evm.bytecode"};
	outputSelection["*"][""] = std::vector<std::string>{"ast"};
	settings.outputSelection = outputSelection;

	input.settings = settings;

	// Serialize
	Json json = input;

	// Top-level
	BOOST_CHECK_EQUAL(json["language"], "Solidity");
	BOOST_CHECK(json["sources"].contains("file.sol"));
	BOOST_CHECK_EQUAL(json["sources"]["file.sol"]["content"], "pragma solidity ^0.8.0;\ncontract C {}");
	BOOST_CHECK(json["sources"].contains("lib.sol"));
	BOOST_CHECK_EQUAL(json["sources"]["lib.sol"]["keccak256"], "0xabc123");
	BOOST_CHECK_EQUAL(json["sources"]["lib.sol"]["urls"][0], "ipfs://Qma...");
	BOOST_CHECK_EQUAL(json["sources"]["lib.sol"]["urls"][1], "/tmp/file.sol");

	// Settings
	BOOST_CHECK(json["settings"].contains("stopAfter"));
	BOOST_CHECK_EQUAL(json["settings"]["stopAfter"], "parsing");
	BOOST_CHECK(json["settings"].contains("remappings"));
	BOOST_CHECK_EQUAL(json["settings"]["remappings"][0], "g=/dir/");
	BOOST_CHECK_EQUAL(json["settings"]["experimental"], true);

	// Optimizer
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["enabled"], true);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["runs"], 200);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["peephole"], true);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["inliner"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["yul"], true);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["yulDetails"]["stackAllocation"], true);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["yulDetails"]["optimizerSteps"], "dfDvulfnTUtnIf");

	// EVM version & IR
	BOOST_CHECK(json["settings"].contains("evmVersion"));
	BOOST_CHECK_EQUAL(json["settings"]["viaIR"], true);
	BOOST_CHECK_EQUAL(json["settings"]["viaSSACFG"], false);

	// Debug
	BOOST_CHECK_EQUAL(json["settings"]["debug"]["revertStrings"], "debug");
	BOOST_CHECK_EQUAL(json["settings"]["debug"]["debugInfo"][0], "location");
	BOOST_CHECK_EQUAL(json["settings"]["debug"]["debugInfo"][1], "snippet");
	BOOST_CHECK_EQUAL(json["settings"]["debug"]["debugInfo"][2], "ast-id");

	// Metadata
	BOOST_CHECK_EQUAL(json["settings"]["metadata"]["appendCBOR"], true);
	BOOST_CHECK_EQUAL(json["settings"]["metadata"]["bytecodeHash"], "ipfs");
	BOOST_CHECK_EQUAL(json["settings"]["metadata"]["useLiteralContent"], false);

	// Libraries
	BOOST_CHECK_EQUAL(
		json["settings"]["libraries"]["myFile.sol"]["MyLib"],
		"0x1234567890123456789012345678901234567890"
	);

	// Model checker
	BOOST_CHECK(json["settings"]["modelChecker"]["contracts"].contains("source.sol"));
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["contracts"]["source.sol"][0], "Contract1");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["divModNoSlacks"], false);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["engine"], "chc");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["extCalls"], "trusted");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["invariants"][0], "contract");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["invariants"][1], "reentrancy");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showProvedSafe"], true);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showUnproved"], true);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showUnsupported"], true);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["solvers"][0], "z3");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["solvers"][1], "smtlib2");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["targets"][0], "assert");
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["timeout"], 10000);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["bmcLoopIterations"], 5);

	// Output selection
	BOOST_CHECK_EQUAL(json["settings"]["outputSelection"]["*"]["*"][0], "abi");
	BOOST_CHECK_EQUAL(json["settings"]["outputSelection"]["*"]["*"][1], "evm.bytecode");
	BOOST_CHECK_EQUAL(json["settings"]["outputSelection"]["*"][""][0], "ast");
}

BOOST_AUTO_TEST_CASE(optional_fields_omitted)
{
	using namespace solidity::frontend::input;

	input::Input input;
	input.language = "Solidity";
	input.sources["test.sol"] = Source{"contract C {}"};

	Json json = input;

	// Only language and sources should be present; settings should be empty
	BOOST_CHECK_EQUAL(json["language"], "Solidity");
	BOOST_CHECK(json["sources"].contains("test.sol"));
	BOOST_CHECK(!json.contains("settings") || json["settings"].empty() || json["settings"].is_null());
}

BOOST_AUTO_TEST_CASE(yul_language)
{
	input::Input input;
	input.language = "Yul";

	Json json = input;

	BOOST_CHECK_EQUAL(json["language"], "Yul");
}

BOOST_AUTO_TEST_CASE(optional_bool_false_is_serialized)
{
	using namespace solidity::frontend::input;

	input::Input input;
	input.language = "Solidity";
	input.sources["test.sol"] = Source{"contract C {}"};

	Settings settings;

	Optimizer optimizer;
	optimizer.enable = false;
	OptimizerDetails details;
	details.peephole = false;
	details.inliner = false;
	details.jumpdestRemover = false;
	details.orderLiterals = false;
	details.deduplicate = false;
	details.cse = false;
	details.constantOptimizer = false;
	details.simpleCounterForLoopUncheckedIncrement = false;
	details.yul = false;
	optimizer.details = details;
	settings.optimizer = optimizer;

	settings.viaIR = false;
	settings.viaSSACFG = false;
	settings.experimental = false;

	Metadata metadata;
	metadata.appendCBOR = false;
	metadata.useLiteralContent = false;
	settings.metadata = metadata;

	ModelChecker modelChecker;
	modelChecker.divModNoSlacks = false;
	modelChecker.showProvedSafe = false;
	modelChecker.showUnproved = false;
	modelChecker.showUnsupported = false;
	settings.modelChecker = modelChecker;

	input.settings = settings;

	Json json = input;

	BOOST_CHECK(json["settings"].contains("optimizer"));
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["enabled"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["peephole"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["inliner"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["jumpdestRemover"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["orderLiterals"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["deduplicate"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["cse"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["constantOptimizer"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["simpleCounterForLoopUncheckedIncrement"], false);
	BOOST_CHECK_EQUAL(json["settings"]["optimizer"]["details"]["yul"], false);
	BOOST_CHECK_EQUAL(json["settings"]["viaIR"], false);
	BOOST_CHECK_EQUAL(json["settings"]["viaSSACFG"], false);
	BOOST_CHECK_EQUAL(json["settings"]["experimental"], false);
	BOOST_CHECK_EQUAL(json["settings"]["metadata"]["appendCBOR"], false);
	BOOST_CHECK_EQUAL(json["settings"]["metadata"]["useLiteralContent"], false);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["divModNoSlacks"], false);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showProvedSafe"], false);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showUnproved"], false);
	BOOST_CHECK_EQUAL(json["settings"]["modelChecker"]["showUnsupported"], false);

	YulOptimizerDetails yulDetails;
	yulDetails.stackAllocation = false;
	OptimizerDetails detailsWithYul;
	detailsWithYul.yul = true;
	detailsWithYul.yulDetails = yulDetails;
	Optimizer optimizerWithYul;
	optimizerWithYul.enable = true;
	optimizerWithYul.details = detailsWithYul;
	Settings settingsWithYul;
	settingsWithYul.optimizer = optimizerWithYul;
	input.settings = settingsWithYul;

	Json jsonWithYul = input;
	BOOST_CHECK_EQUAL(jsonWithYul["settings"]["optimizer"]["details"]["yulDetails"]["stackAllocation"], false);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
