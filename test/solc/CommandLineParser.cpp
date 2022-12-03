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
#include <solc/Exceptions.h>

#include <test/solc/Common.h>

#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolutil/CommonData.h>
#include <liblangutil/EVMVersion.h>
#include <libsmtutil/SolverInterface.h>
#include <libsolidity/interface/Version.h>

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

CommandLineOptions parseCommandLine(vector<string> const& _commandLine)
{
	vector<char const*> argv = test::makeArgv(_commandLine);

	CommandLineParser cliParser;
	cliParser.parse(static_cast<int>(_commandLine.size()), argv.data());
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

	CommandLineOptions parsedOptions = parseCommandLine(commandLine);

	BOOST_TEST(parsedOptions == expectedOptions);
}

BOOST_AUTO_TEST_CASE(help_license_version)
{
	map<string, InputMode> expectedModePerOption = {
		{"--help", InputMode::Help},
		{"--license", InputMode::License},
		{"--version", InputMode::Version},
	};

	for (auto const& [option, expectedMode]: expectedModePerOption)
	{
		CommandLineOptions parsedOptions = parseCommandLine({"solc", option});

		CommandLineOptions expectedOptions;
		expectedOptions.input.mode = expectedMode;

		BOOST_TEST(parsedOptions == expectedOptions);
	}
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
			"--include-path=/usr/lib/include/",
			"--include-path=/home/user/include",
			"--allow-paths=/tmp,/home,project,../contracts",
			"--ignore-missing",
			"--error-recovery",
			"--output-dir=/tmp/out",
			"--overwrite",
			"--evm-version=spuriousDragon",
			"--via-ir",
			"--experimental-via-ir",
			"--revert-strings=strip",
			"--debug-info=location",
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
			"--model-checker-invariants=contract,reentrancy",
			"--model-checker-show-unproved",
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
		expectedOptions.input.includePaths = {"/usr/lib/include/", "/home/user/include"};

		expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts", "c", "/usr/lib"};
		expectedOptions.input.ignoreMissingFiles = true;
		expectedOptions.input.errorRecovery = (inputMode == InputMode::Compiler);
		expectedOptions.output.dir = "/tmp/out";
		expectedOptions.output.overwriteFiles = true;
		expectedOptions.output.evmVersion = EVMVersion::spuriousDragon();
		expectedOptions.output.viaIR = true;
		expectedOptions.output.revertStrings = RevertStrings::Strip;
		expectedOptions.output.debugInfoSelection = DebugInfoSelection::fromString("location");
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
			true,
		};
		expectedOptions.compiler.outputs.ewasmIR = false;
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
			{{InvariantType::Contract, InvariantType::Reentrancy}},
			true,
			{false, false, true, true},
			{{VerificationTargetType::Underflow, VerificationTargetType::DivByZero}},
			5,
		};

		CommandLineOptions parsedOptions = parseCommandLine(commandLine);

		BOOST_TEST(parsedOptions == expectedOptions);
	}
}

BOOST_AUTO_TEST_CASE(no_cbor_metadata)
{
	vector<string> commandLine = {"solc", "--no-cbor-metadata", "contract.sol"};
	CommandLineOptions parsedOptions = parseCommandLine(commandLine);
	bool assert = parsedOptions.metadata.format == CompilerStack::MetadataFormat::NoMetadata;

	BOOST_TEST(assert);
}

BOOST_AUTO_TEST_CASE(via_ir_options)
{
	BOOST_TEST(!parseCommandLine({"solc", "contract.sol"}).output.viaIR);
	for (string viaIrOption: {"--via-ir", "--experimental-via-ir"})
		BOOST_TEST(parseCommandLine({"solc", viaIrOption, "contract.sol"}).output.viaIR);
}

BOOST_AUTO_TEST_CASE(assembly_mode_options)
{
	static vector<tuple<vector<string>, YulStack::Machine, YulStack::Language>> const allowedCombinations = {
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--assemble"}, YulStack::Machine::Ewasm, YulStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--yul"}, YulStack::Machine::Ewasm, YulStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=ewasm", "--strict-assembly"}, YulStack::Machine::Ewasm, YulStack::Language::Ewasm},
		{{"--machine=ewasm", "--yul-dialect=evm", "--assemble"}, YulStack::Machine::Ewasm, YulStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--yul-dialect=evm", "--yul"}, YulStack::Machine::Ewasm, YulStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--yul-dialect=evm", "--strict-assembly"}, YulStack::Machine::Ewasm, YulStack::Language::StrictAssembly},
		{{"--machine=ewasm", "--strict-assembly"}, YulStack::Machine::Ewasm, YulStack::Language::Ewasm},
		{{"--machine=evm", "--yul-dialect=evm", "--assemble"}, YulStack::Machine::EVM, YulStack::Language::StrictAssembly},
		{{"--machine=evm", "--yul-dialect=evm", "--yul"}, YulStack::Machine::EVM, YulStack::Language::StrictAssembly},
		{{"--machine=evm", "--yul-dialect=evm", "--strict-assembly"}, YulStack::Machine::EVM, YulStack::Language::StrictAssembly},
		{{"--machine=evm", "--assemble"}, YulStack::Machine::EVM, YulStack::Language::Assembly},
		{{"--machine=evm", "--yul"}, YulStack::Machine::EVM, YulStack::Language::Yul},
		{{"--machine=evm", "--strict-assembly"}, YulStack::Machine::EVM, YulStack::Language::StrictAssembly},
		{{"--assemble"}, YulStack::Machine::EVM, YulStack::Language::Assembly},
		{{"--yul"}, YulStack::Machine::EVM, YulStack::Language::Yul},
		{{"--strict-assembly"}, YulStack::Machine::EVM, YulStack::Language::StrictAssembly},
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
			"--include-path=/usr/lib/include/",
			"--include-path=/home/user/include",
			"--allow-paths=/tmp,/home,project,../contracts",
			"--ignore-missing",
			"--overwrite",
			"--evm-version=spuriousDragon",
			"--revert-strings=strip",      // Accepted but has no effect in assembly mode
			"--debug-info=location",
			"--pretty-json",
			"--json-indent=1",
			"--no-color",
			"--error-codes",
			"--libraries="
				"dir1/file1.sol:L=0x1234567890123456789012345678901234567890,"
				"dir2/file2.sol:L=0x1111122222333334444455555666667777788888",
			"--asm",
			"--bin",
			"--ir-optimized",
			"--ewasm",
			"--ewasm-ir",
		};
		commandLine += assemblyOptions;
		if (expectedLanguage == YulStack::Language::StrictAssembly || expectedLanguage == YulStack::Language::Ewasm)
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
		expectedOptions.input.includePaths = {"/usr/lib/include/", "/home/user/include"};
		expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts", "c", "/usr/lib"};
		expectedOptions.input.ignoreMissingFiles = true;
		expectedOptions.output.overwriteFiles = true;
		expectedOptions.output.evmVersion = EVMVersion::spuriousDragon();
		expectedOptions.output.revertStrings = RevertStrings::Strip;
		expectedOptions.output.debugInfoSelection = DebugInfoSelection::fromString("location");
		expectedOptions.formatting.json = JsonFormat {JsonFormat::Pretty, 1};
		expectedOptions.assembly.targetMachine = expectedMachine;
		expectedOptions.assembly.inputLanguage = expectedLanguage;
		expectedOptions.linker.libraries = {
			{"dir1/file1.sol:L", h160("1234567890123456789012345678901234567890")},
			{"dir2/file2.sol:L", h160("1111122222333334444455555666667777788888")},
		};
		expectedOptions.formatting.coloredOutput = false;
		expectedOptions.formatting.withErrorIds = true;
		expectedOptions.compiler.outputs.asm_ = true;
		expectedOptions.compiler.outputs.binary = true;
		expectedOptions.compiler.outputs.irOptimized = true;
		expectedOptions.compiler.outputs.ewasm = true;
		expectedOptions.compiler.outputs.ewasmIR = true;
		if (expectedLanguage == YulStack::Language::StrictAssembly || expectedLanguage == YulStack::Language::Ewasm)
		{
			expectedOptions.optimizer.enabled = true;
			expectedOptions.optimizer.yulSteps = "agf";
			expectedOptions.optimizer.expectedExecutionsPerDeployment = 1000;
		}

		CommandLineOptions parsedOptions = parseCommandLine(commandLine);

		BOOST_TEST(parsedOptions == expectedOptions);
	}
}

BOOST_AUTO_TEST_CASE(standard_json_mode_options)
{
	vector<string> commandLine = {
		"solc",
		"input.json",
		"--standard-json",
		"--base-path=/home/user/",
		"--include-path=/usr/lib/include/",
		"--include-path=/home/user/include",
		"--allow-paths=/tmp,/home,project,../contracts",
		"--ignore-missing",
		"--output-dir=/tmp/out",           // Accepted but has no effect in Standard JSON mode
		"--overwrite",                     // Accepted but has no effect in Standard JSON mode
		"--evm-version=spuriousDragon",    // Ignored in Standard JSON mode
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
	};

	CommandLineOptions expectedOptions;

	expectedOptions.input.mode = InputMode::StandardJson;
	expectedOptions.input.paths = {"input.json"};
	expectedOptions.input.basePath = "/home/user/";
	expectedOptions.input.includePaths = {"/usr/lib/include/", "/home/user/include"};
	expectedOptions.input.allowedDirectories = {"/tmp", "/home", "project", "../contracts"};
	expectedOptions.input.ignoreMissingFiles = true;
	expectedOptions.output.dir = "/tmp/out";
	expectedOptions.output.overwriteFiles = true;
	expectedOptions.output.revertStrings = RevertStrings::Strip;
	expectedOptions.formatting.json = JsonFormat {JsonFormat::Pretty, 1};
	expectedOptions.formatting.coloredOutput = false;
	expectedOptions.formatting.withErrorIds = true;
	expectedOptions.compiler.estimateGas = true;
	expectedOptions.compiler.combinedJsonRequests = CombinedJsonRequests{};
	expectedOptions.compiler.combinedJsonRequests->abi = true;
	expectedOptions.compiler.combinedJsonRequests->binary = true;

	CommandLineOptions parsedOptions = parseCommandLine(commandLine);

	BOOST_TEST(parsedOptions == expectedOptions);
}

BOOST_AUTO_TEST_CASE(invalid_options_input_modes_combinations)
{
	map<string, vector<string>> invalidOptionInputModeCombinations = {
		// TODO: This should eventually contain all options.
		{"--error-recovery", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--experimental-via-ir", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--via-ir", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--metadata-literal", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--metadata-hash=swarm", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-show-unproved", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-div-mod-no-slacks", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-engine=bmc", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-invariants=contract,reentrancy", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-solvers=z3,smtlib2", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-timeout=5", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-contracts=contract1.yul:A,contract2.yul:B", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}},
		{"--model-checker-targets=underflow,divByZero", {"--assemble", "--yul", "--strict-assembly", "--standard-json", "--link"}}
	};

	for (auto const& [optionName, inputModes]: invalidOptionInputModeCombinations)
		for (string const& inputMode: inputModes)
		{
			stringstream serr;
			size_t separatorPosition = optionName.find("=");
			string optionNameWithoutValue = optionName.substr(0, separatorPosition);
			soltestAssert(!optionNameWithoutValue.empty());

			vector<string> commandLine = {"solc", optionName, "file", inputMode};

			string expectedMessage = "The following options are not supported in the current input mode: " + optionNameWithoutValue;
			auto hasCorrectMessage = [&](CommandLineValidationError const& _exception) { return _exception.what() == expectedMessage; };

			BOOST_CHECK_EXCEPTION(parseCommandLine(commandLine), CommandLineValidationError, hasCorrectMessage);
		}
}

BOOST_AUTO_TEST_CASE(default_optimiser_sequence)
{
	CommandLineOptions const& commandLineOptions = parseCommandLine({"solc", "contract.sol", "--optimize"});
	BOOST_CHECK_EQUAL(commandLineOptions.optimiserSettings().yulOptimiserSteps, OptimiserSettings::DefaultYulOptimiserSteps);
	BOOST_CHECK_EQUAL(commandLineOptions.optimiserSettings().yulOptimiserCleanupSteps, OptimiserSettings::DefaultYulOptimiserCleanupSteps);
}

BOOST_AUTO_TEST_CASE(valid_optimiser_sequences)
{
	vector<string> validSequenceInputs {
		":",                         // Empty optimization sequence and empty cleanup sequence
		":fDn",                      // Empty optimization sequence and specified cleanup sequence
		"dhfoDgvulfnTUtnIf:",        // Specified optimization sequence and empty cleanup sequence
		"dhfoDgvulfnTUtnIf:fDn",     // Specified optimization sequence and cleanup sequence
		"dhfo[Dgvulfn]TUtnIf:f[D]n", // Specified and nested optimization and cleanup sequence
		"dhfoDgvulfnTUtnIf",         // Specified optimizer sequence only
		"iDu",                       // Short optimizer sequence
		"a[[a][[aa]aa[aa]][]]aaa[aa[aa[aa]]]a[a][a][a]a[a]" // Nested brackets
	};

	vector<tuple<string, string>> const expectedParsedSequences {
		{"", ""},
		{"", "fDn"},
		{"dhfoDgvulfnTUtnIf", ""},
		{"dhfoDgvulfnTUtnIf", "fDn"},
		{"dhfo[Dgvulfn]TUtnIf", "f[D]n"},
		{"dhfoDgvulfnTUtnIf", OptimiserSettings::DefaultYulOptimiserCleanupSteps},
		{"iDu", OptimiserSettings::DefaultYulOptimiserCleanupSteps},
		{"a[[a][[aa]aa[aa]][]]aaa[aa[aa[aa]]]a[a][a][a]a[a]", OptimiserSettings::DefaultYulOptimiserCleanupSteps}
	};

	BOOST_CHECK_EQUAL(validSequenceInputs.size(), expectedParsedSequences.size());

	for (size_t i = 0; i < validSequenceInputs.size(); ++i)
	{
		CommandLineOptions const& commandLineOptions = parseCommandLine({"solc", "contract.sol", "--optimize", "--yul-optimizations=" + validSequenceInputs[i]});
		auto const& [expectedYulOptimiserSteps, expectedYulCleanupSteps] = expectedParsedSequences[i];
		BOOST_CHECK_EQUAL(commandLineOptions.optimiserSettings().yulOptimiserSteps, expectedYulOptimiserSteps);
		BOOST_CHECK_EQUAL(commandLineOptions.optimiserSettings().yulOptimiserCleanupSteps, expectedYulCleanupSteps);
	}
}

BOOST_AUTO_TEST_CASE(invalid_optimiser_sequences)
{
	vector<string> const invalidSequenceInputs {
		"abcdefg{hijklmno}pqr[st]uvwxyz", // Invalid abbreviation
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
		"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[a]"
		"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
		"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
		"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
		"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
		"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]",  // Brackets nested too deep
		"a]a][",                          // Unbalanced closing bracket
		"a[a][",                          // Unbalanced opening bracket
		"dhfoDgvulfnTUt[nIf:fd]N",        // Nested cleanup sequence delimiter
		"dhfoDgvulfnTU:tnIf:fdN"          // Too many cleanup sequence delimiters
	};

	vector<string> const expectedErrorMessages {
		"'b' is not a valid step abbreviation",
		"Brackets nested too deep",
		"Unbalanced brackets",
		"Unbalanced brackets",
		"Cleanup sequence delimiter cannot be placed inside the brackets",
		"Too many cleanup sequence delimiters"
	};

	BOOST_CHECK_EQUAL(invalidSequenceInputs.size(), expectedErrorMessages.size());

	string const baseExpectedErrorMessage = "Invalid optimizer step sequence in --yul-optimizations: ";

	for (size_t i = 0; i < invalidSequenceInputs.size(); ++i)
	{
		vector<string> const commandLineOptions = {"solc", "contract.sol", "--optimize", "--yul-optimizations=" + invalidSequenceInputs[i]};
		string const expectedErrorMessage = baseExpectedErrorMessage + expectedErrorMessages[i];
		auto hasCorrectMessage = [&](CommandLineValidationError const& _exception) { return _exception.what() == expectedErrorMessage; };
		BOOST_CHECK_EXCEPTION(parseCommandLine(commandLineOptions), CommandLineValidationError, hasCorrectMessage);
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
