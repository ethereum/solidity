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

/// Unit tests for solc/CommandLineParser.h

#include <solc/CommandLineParser.h>

#include <test/solc/Common.h>

#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolutil/CommonData.h>
#include <liblangutil/EVMVersion.h>
#include <libsmtutil/SolverInterface.h>
#include <libsolidity/interface/Version.h>

#include <boost/algorithm/string.hpp>

#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{

optional<CommandLineOptions> parseCommandLine(vector<string> const& _commandLine, ostream& _stdout, ostream& _stderr)
{
	vector<char const*> argv = test::makeArgv(_commandLine);

	CommandLineParser cliParser(_stdout, _stderr);
	bool success = cliParser.parse(
		static_cast<int>(_commandLine.size()),
		argv.data(),
		false // interactiveTerminal
	);

	if (!success)
		return nullopt;
	else
		return cliParser.options();
}

} // namespace

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(CommandLineParserTest)

BOOST_AUTO_TEST_CASE(no_options)
{
	vector<string> commandLine = {"solc", "contract.sol"};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {"contract.sol"};
	expectedOptions.modelChecker.initialize = true;
	expectedOptions.modelChecker.settings = {};

	stringstream sout, serr;
	optional<CommandLineOptions> parsedOptions = parseCommandLine(commandLine, sout, serr);

	BOOST_TEST(sout.str() == "");
	BOOST_TEST(serr.str() == "");
	BOOST_REQUIRE(parsedOptions.has_value());
	BOOST_TEST(parsedOptions.value() == expectedOptions);
}

BOOST_AUTO_TEST_CASE(help)
{
	stringstream sout, serr;
	optional<CommandLineOptions> parsedOptions = parseCommandLine({"solc", "--help"}, sout, serr);

	BOOST_TEST(serr.str() == "");
	BOOST_TEST(boost::starts_with(sout.str(), "solc, the Solidity commandline compiler."));
	BOOST_TEST(sout.str().find("Usage: solc [options] [input_file...]") != string::npos);
	BOOST_TEST(!parsedOptions.has_value());
}

BOOST_AUTO_TEST_CASE(cli_mode_options)
{
	for (InputMode inputMode: {InputMode::Compiler, InputMode::CompilerWithASTImport})
	{
		vector<string> commandLine = {
			"solc",
			"contract.sol",             // Both modes do not care about file names, just about
			"/tmp/projects/token.sol",  // their content. They also both support stdin.
			"/home/user/lib/dex.sol",
			"file",
			"input.json",
			"-",
			"/tmp=/usr/lib/",
			"a:b=c/d",
			":contract.sol=",
			"--base-path=/home/user/",
			"--allow-paths=/tmp,/home,project,../contracts",
			"--ignore-missing",
			"--error-recovery",
			"--output-dir=/tmp/out",
			"--overwrite",
			"--evm-version=spuriousDragon",
			"--experimental-via-ir",
			"--revert-strings=strip",
			"--pretty-json",
			"--json-indent=7",
			"--no-color",
			"--error-codes",
			"--libraries="
				"dir1/file1.sol:L=0x1234567890123456789012345678901234567890,"
				"dir2/file2.sol:L=0x1111122222333334444455555666667777788888",
			"--ast-compact-json", "--asm", "--asm-json", "--opcodes", "--bin", "--bin-runtime", "--abi",
			"--ir", "--ir-optimized", "--ewasm", "--hashes", "--userdoc", "--devdoc", "--metadata", "--storage-layout",
			"--gas",
			"--combined-json="
				"abi,metadata,bin,bin-runtime,opcodes,asm,storage-layout,generated-sources,generated-sources-runtime,"
				"srcmap,srcmap-runtime,function-debug,function-debug-runtime,hashes,devdoc,userdoc,ast",
			"--metadata-hash=swarm",
			"--metadata-literal",
			"--optimize",
			"--optimize-runs=1000",
			"--yul-optimizations=agf",
			"--model-checker-contracts=contract1.yul:A,contract2.yul:B",
			"--model-checker-div-mod-no-slacks",
			"--model-checker-engine=bmc",
			"--model-checker-show-unproved=true",
			"--model-checker-solvers=z3,smtlib2",
			"--model-checker-targets=underflow,divByZero",
			"--model-checker-timeout=5",
		};

		if (inputMode == InputMode::CompilerWithASTImport)
			commandLine += vector<string>{
				"--import-ast",
			};

		CommandLineOptions expectedOptions;
		expectedOptions.input.mode = inputMode;
		expectedOptions.input.paths = {"contract.sol", "/tmp/projects/token.sol", "/home/user/lib/dex.sol", "file", "input.json"};
		expectedOptions.input.remappings = {
			{"", "/tmp", "/usr/lib/"},
			{"a", "b", "c/d"},
			{"", "contract.sol", ""},
		};

		expectedOptions.input.addStdin = true;
		expectedOptions.input.basePath = "/home/user/";
		expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts", "", "c", "/usr/lib"};
		expectedOptions.input.ignoreMissingFiles = true;
		expectedOptions.input.errorRecovery = (inputMode == InputMode::Compiler);
		expectedOptions.output.dir = "/tmp/out";
		expectedOptions.output.overwriteFiles = true;
		expectedOptions.output.evmVersion = EVMVersion::spuriousDragon();
		expectedOptions.output.experimentalViaIR = true;
		expectedOptions.output.revertStrings = RevertStrings::Strip;
		expectedOptions.formatting.json = JsonFormat{JsonFormat::Pretty, 7};
		expectedOptions.linker.libraries = {
			{"dir1/file1.sol:L", h160("1234567890123456789012345678901234567890")},
			{"dir2/file2.sol:L", h160("1111122222333334444455555666667777788888")},
		};
		expectedOptions.formatting.coloredOutput = false;
		expectedOptions.formatting.withErrorIds = true;
		expectedOptions.compiler.outputs = {
			true, true, true, true, true,
			true, true, true, true, true,
			true, true, true, true, true,
		};
		expectedOptions.compiler.estimateGas = true;
		expectedOptions.compiler.combinedJsonRequests = {
			true, true, true, true, true,
			true, true, true, true, true,
			true, true, true, true, true,
			true, true,
		};
		expectedOptions.metadata.hash = CompilerStack::MetadataHash::Bzzr1;
		expectedOptions.metadata.literalSources = true;
		expectedOptions.optimizer.enabled = true;
		expectedOptions.optimizer.expectedExecutionsPerDeployment = 1000;
		expectedOptions.optimizer.yulSteps = "agf";

		expectedOptions.modelChecker.initialize = true;
		expectedOptions.modelChecker.settings = {
			{{{"contract1.yul", {"A"}}, {"contract2.yul", {"B"}}}},
			true,
			{true, false},
			true,
			{false, true, true},
			{{VerificationTargetType::Underflow, VerificationTargetType::DivByZero}},
			5,
		};

		stringstream sout, serr;
		optional<CommandLineOptions> parsedOptions = parseCommandLine(commandLine, sout, serr);

		BOOST_TEST(sout.str() == "");
		BOOST_TEST(serr.str() == "");
		BOOST_REQUIRE(parsedOptions.has_value());
		BOOST_TEST(parsedOptions.value() == expectedOptions);
	}
}

BOOST_AUTO_TEST_CASE(assembly_mode_options)
{
	static vector<tuple<vector<string>, AssemblyStack::Machine, AssemblyStack::Language>> const allowedCombinations = {
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--assemble"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--yul"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--strict-assembly"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=evm", "--assemble"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--yul-dialect=evm", "--yul"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--yul-dialect=evm", "--strict-assembly"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--strict-assembly"}, AssemblyStack::Machine::Ewasm, AssemblyStack::Language::Ewasm},
		{{"--machine=evm", "--yul-dialect=evm", "--assemble"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::StrictAssembly},
		{{"--machine=evm", "--yul-dialect=evm", "--yul"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::StrictAssembly},
		{{"--machine=evm", "--yul-dialect=evm", "--strict-assembly"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::StrictAssembly},
		{{"--machine=evm", "--assemble"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::Assembly},
		{{"--machine=evm", "--yul"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::Yul},
		{{"--machine=evm", "--strict-assembly"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::StrictAssembly},
		{{"--assemble"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::Assembly},
		{{"--yul"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::Yul},
		{{"--strict-assembly"}, AssemblyStack::Machine::EVM, AssemblyStack::Language::StrictAssembly},
	};

	for (auto const& [assemblyOptions, expectedMachine, expectedLanguage]: allowedCombinations)
	{
		vector<string> commandLine = {
			"solc",
			"contract.yul",
			"/tmp/projects/token.yul",
			"/home/user/lib/dex.yul",
			"file",
			"input.json",
			"-",
			"/tmp=/usr/lib/",
			"a:b=c/d",
			":contract.yul=",
			"--base-path=/home/user/",
			"--allow-paths=/tmp,/home,project,../contracts",
			"--ignore-missing",
			"--error-recovery",            // Ignored in assembly mode
			"--overwrite",
			"--evm-version=spuriousDragon",
			"--experimental-via-ir",       // Ignored in assembly mode
			"--revert-strings=strip",      // Accepted but has no effect in assembly mode
			"--pretty-json",
			"--json-indent=1",
			"--no-color",
			"--error-codes",
			"--libraries="
				"dir1/file1.sol:L=0x1234567890123456789012345678901234567890,"
				"dir2/file2.sol:L=0x1111122222333334444455555666667777788888",
			"--metadata-hash=swarm",       // Ignored in assembly mode
			"--metadata-literal",          // Ignored in assembly mode
			"--model-checker-contracts="   // Ignored in assembly mode
				"contract1.yul:A,"
				"contract2.yul:B",
			"--model-checker-div-mod-no-slacks", // Ignored in assembly mode
			"--model-checker-engine=bmc",  // Ignored in assembly mode
			"--model-checker-show-unproved=true", // Ignored in assembly mode
			"--model-checker-solvers=z3,smtlib2", // Ignored in assembly mode
			"--model-checker-targets="     // Ignored in assembly mode
				"underflow,"
				"divByZero",
			"--model-checker-timeout=5",   // Ignored in assembly mode

			// Accepted but has no effect in assembly mode
			"--ast-compact-json", "--asm", "--asm-json", "--opcodes", "--bin", "--bin-runtime", "--abi",
			"--ir", "--ir-optimized", "--ewasm", "--hashes", "--userdoc", "--devdoc", "--metadata", "--storage-layout",
		};
		commandLine += assemblyOptions;
		if (expectedLanguage == AssemblyStack::Language::StrictAssembly || expectedLanguage == AssemblyStack::Language::Ewasm)
			commandLine += vector<string>{
				"--optimize",
				"--optimize-runs=1000",
				"--yul-optimizations=agf",
			};

		CommandLineOptions expectedOptions;
		expectedOptions.input.mode = InputMode::Assembler;

		expectedOptions.input.paths = {"contract.yul", "/tmp/projects/token.yul", "/home/user/lib/dex.yul", "file", "input.json"};
		expectedOptions.input.remappings = {
			{"", "/tmp", "/usr/lib/"},
			{"a", "b", "c/d"},
			{"", "contract.yul", ""},
		};
		expectedOptions.input.addStdin = true;
		expectedOptions.input.basePath = "/home/user/";
		expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts", "", "c", "/usr/lib"};
		expectedOptions.input.ignoreMissingFiles = true;
		expectedOptions.output.overwriteFiles = true;
		expectedOptions.output.evmVersion = EVMVersion::spuriousDragon();
		expectedOptions.output.revertStrings = RevertStrings::Strip;
		expectedOptions.formatting.json = JsonFormat {JsonFormat::Pretty, 1};
		expectedOptions.assembly.targetMachine = expectedMachine;
		expectedOptions.assembly.inputLanguage = expectedLanguage;
		expectedOptions.linker.libraries = {
			{"dir1/file1.sol:L", h160("1234567890123456789012345678901234567890")},
			{"dir2/file2.sol:L", h160("1111122222333334444455555666667777788888")},
		};
		expectedOptions.formatting.coloredOutput = false;
		expectedOptions.formatting.withErrorIds = true;
		expectedOptions.compiler.outputs = {
			true, true, true, true, true,
			true, true, true, true, true,
			true, true, true, true, true,
		};
		if (expectedLanguage == AssemblyStack::Language::StrictAssembly || expectedLanguage == AssemblyStack::Language::Ewasm)
		{
			expectedOptions.optimizer.enabled = true;
			expectedOptions.optimizer.yulSteps = "agf";
			expectedOptions.optimizer.expectedExecutionsPerDeployment = 1000;
		}

		stringstream sout, serr;
		optional<CommandLineOptions> parsedOptions = parseCommandLine(commandLine, sout, serr);

		BOOST_TEST(sout.str() == "");
		BOOST_TEST(serr.str() == "Warning: Yul is still experimental. Please use the output with care.\n");
		BOOST_REQUIRE(parsedOptions.has_value());
		BOOST_TEST(parsedOptions.value() == expectedOptions);
	}
}

BOOST_AUTO_TEST_CASE(standard_json_mode_options)
{
	vector<string> commandLine = {
		"solc",
		"input.json",
		"--standard-json",
		"--base-path=/home/user/",
		"--allow-paths=/tmp,/home,project,../contracts",
		"--ignore-missing",
		"--error-recovery",                // Ignored in Standard JSON mode
		"--output-dir=/tmp/out",           // Accepted but has no effect in Standard JSON mode
		"--overwrite",                     // Accepted but has no effect in Standard JSON mode
		"--evm-version=spuriousDragon",    // Ignored in Standard JSON mode
		"--experimental-via-ir",           // Ignored in Standard JSON mode
		"--revert-strings=strip",          // Accepted but has no effect in Standard JSON mode
		"--pretty-json",
		"--json-indent=1",
		"--no-color",                      // Accepted but has no effect in Standard JSON mode
		"--error-codes",                   // Accepted but has no effect in Standard JSON mode
		"--libraries="                     // Ignored in Standard JSON mode
			"dir1/file1.sol:L=0x1234567890123456789012345678901234567890,"
			"dir2/file2.sol:L=0x1111122222333334444455555666667777788888",
		"--gas",                           // Accepted but has no effect in Standard JSON mode
		"--combined-json=abi,bin",         // Accepted but has no effect in Standard JSON mode
		"--metadata-hash=swarm",           // Ignored in Standard JSON mode
		"--metadata-literal",              // Ignored in Standard JSON mode
		"--optimize",                      // Ignored in Standard JSON mode
		"--optimize-runs=1000",            // Ignored in Standard JSON mode
		"--yul-optimizations=agf",
		"--model-checker-contracts="       // Ignored in Standard JSON mode
			"contract1.yul:A,"
			"contract2.yul:B",
		"--model-checker-div-mod-no-slacks", // Ignored in Standard JSON mode
		"--model-checker-engine=bmc",      // Ignored in Standard JSON mode
		"--model-checker-show-unproved=true",      // Ignored in Standard JSON mode
		"--model-checker-solvers=z3,smtlib2", // Ignored in Standard JSON mode
		"--model-checker-targets="         // Ignored in Standard JSON mode
			"underflow,"
			"divByZero",
		"--model-checker-timeout=5",       // Ignored in Standard JSON mode

		// Accepted but has no effect in Standard JSON mode
		"--ast-compact-json", "--asm", "--asm-json", "--opcodes", "--bin", "--bin-runtime", "--abi",
		"--ir", "--ir-optimized", "--ewasm", "--hashes", "--userdoc", "--devdoc", "--metadata", "--storage-layout",
	};

	CommandLineOptions expectedOptions;

	expectedOptions.input.mode = InputMode::StandardJson;
	expectedOptions.input.paths = {"input.json"};
	expectedOptions.input.basePath = "/home/user/";
	expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts"};
	expectedOptions.input.ignoreMissingFiles = true;
	expectedOptions.output.dir = "/tmp/out";
	expectedOptions.output.overwriteFiles = true;
	expectedOptions.output.revertStrings = RevertStrings::Strip;
	expectedOptions.formatting.json = JsonFormat {JsonFormat::Pretty, 1};
	expectedOptions.formatting.coloredOutput = false;
	expectedOptions.formatting.withErrorIds = true;
	expectedOptions.compiler.outputs = {
		true, true, true, true, true,
		true, true, true, true, true,
		true, true, true, true, true,
	};
	expectedOptions.compiler.estimateGas = true;
	expectedOptions.compiler.combinedJsonRequests = CombinedJsonRequests{};
	expectedOptions.compiler.combinedJsonRequests->abi = true;
	expectedOptions.compiler.combinedJsonRequests->binary = true;

	stringstream sout, serr;
	optional<CommandLineOptions> parsedOptions = parseCommandLine(commandLine, sout, serr);

	BOOST_TEST(sout.str() == "");
	BOOST_TEST(serr.str() == "");
	BOOST_REQUIRE(parsedOptions.has_value());
	BOOST_TEST(parsedOptions.value() == expectedOptions);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
