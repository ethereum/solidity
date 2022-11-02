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

#include <solc/CommandLineParser.h>

#include <solc/Exceptions.h>

#include <libyul/optimiser/Suite.h>

#include <liblangutil/EVMVersion.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range/conversion.hpp>

#include <fmt/format.h>

using namespace solidity::langutil;

namespace po = boost::program_options;

namespace solidity::frontend
{

static std::string const g_strAllowPaths = "allow-paths";
static std::string const g_strBasePath = "base-path";
static std::string const g_strIncludePath = "include-path";
static std::string const g_strAssemble = "assemble";
static std::string const g_strCombinedJson = "combined-json";
static std::string const g_strEVM = "evm";
static std::string const g_strEVMVersion = "evm-version";
static std::string const g_strEOFVersion = "experimental-eof-version";
static std::string const g_strViaIR = "via-ir";
static std::string const g_strExperimentalViaIR = "experimental-via-ir";
static std::string const g_strGas = "gas";
static std::string const g_strHelp = "help";
static std::string const g_strImportAst = "import-ast";
static std::string const g_strImportEvmAssemblerJson = "import-asm-json";
static std::string const g_strInputFile = "input-file";
static std::string const g_strYul = "yul";
static std::string const g_strYulDialect = "yul-dialect";
static std::string const g_strDebugInfo = "debug-info";
static std::string const g_strIPFS = "ipfs";
static std::string const g_strLicense = "license";
static std::string const g_strLibraries = "libraries";
static std::string const g_strLink = "link";
static std::string const g_strLSP = "lsp";
static std::string const g_strMachine = "machine";
static std::string const g_strNoCBORMetadata = "no-cbor-metadata";
static std::string const g_strMetadataHash = "metadata-hash";
static std::string const g_strMetadataLiteral = "metadata-literal";
static std::string const g_strModelCheckerContracts = "model-checker-contracts";
static std::string const g_strModelCheckerDivModNoSlacks = "model-checker-div-mod-no-slacks";
static std::string const g_strModelCheckerEngine = "model-checker-engine";
static std::string const g_strModelCheckerExtCalls = "model-checker-ext-calls";
static std::string const g_strModelCheckerInvariants = "model-checker-invariants";
static std::string const g_strModelCheckerPrintQuery = "model-checker-print-query";
static std::string const g_strModelCheckerShowProvedSafe = "model-checker-show-proved-safe";
static std::string const g_strModelCheckerShowUnproved = "model-checker-show-unproved";
static std::string const g_strModelCheckerShowUnsupported = "model-checker-show-unsupported";
static std::string const g_strModelCheckerSolvers = "model-checker-solvers";
static std::string const g_strModelCheckerTargets = "model-checker-targets";
static std::string const g_strModelCheckerTimeout = "model-checker-timeout";
static std::string const g_strModelCheckerBMCLoopIterations = "model-checker-bmc-loop-iterations";
static std::string const g_strNone = "none";
static std::string const g_strNoOptimizeYul = "no-optimize-yul";
static std::string const g_strOptimize = "optimize";
static std::string const g_strOptimizeRuns = "optimize-runs";
static std::string const g_strOptimizeYul = "optimize-yul";
static std::string const g_strYulOptimizations = "yul-optimizations";
static std::string const g_strOutputDir = "output-dir";
static std::string const g_strOverwrite = "overwrite";
static std::string const g_strRevertStrings = "revert-strings";
static std::string const g_strStopAfter = "stop-after";
static std::string const g_strParsing = "parsing";

/// Possible arguments to for --revert-strings
static std::set<std::string> const g_revertStringsArgs
{
	revertStringsToString(RevertStrings::Default),
	revertStringsToString(RevertStrings::Strip),
	revertStringsToString(RevertStrings::Debug),
	revertStringsToString(RevertStrings::VerboseDebug)
};

static std::string const g_strSources = "sources";
static std::string const g_strSourceList = "sourceList";
static std::string const g_strStandardJSON = "standard-json";
static std::string const g_strStrictAssembly = "strict-assembly";
static std::string const g_strSwarm = "swarm";
static std::string const g_strPrettyJson = "pretty-json";
static std::string const g_strJsonIndent = "json-indent";
static std::string const g_strVersion = "version";
static std::string const g_strIgnoreMissingFiles = "ignore-missing";
static std::string const g_strColor = "color";
static std::string const g_strNoColor = "no-color";
static std::string const g_strErrorIds = "error-codes";

/// Possible arguments to for --machine
static std::set<std::string> const g_machineArgs
{
	g_strEVM
};

/// Possible arguments to for --yul-dialect
static std::set<std::string> const g_yulDialectArgs
{
	g_strEVM
};

/// Possible arguments to for --metadata-hash
static std::set<std::string> const g_metadataHashArgs
{
	g_strIPFS,
	g_strSwarm,
	g_strNone
};

static std::map<InputMode, std::string> const g_inputModeName = {
	{InputMode::Help, "help"},
	{InputMode::License, "license"},
	{InputMode::Version, "version"},
	{InputMode::Compiler, "compiler"},
	{InputMode::CompilerWithASTImport, "compiler (AST import)"},
	{InputMode::Assembler, "assembler"},
	{InputMode::StandardJson, "standard JSON"},
	{InputMode::Linker, "linker"},
	{InputMode::LanguageServer, "language server (LSP)"},
	{InputMode::EVMAssemblerJSON, "EVM assembler (JSON format)"},
};

void CommandLineParser::checkMutuallyExclusive(std::vector<std::string> const& _optionNames)
{
	if (countEnabledOptions(_optionNames) > 1)
	{
		solThrow(
			CommandLineValidationError,
			"The following options are mutually exclusive: " + joinOptionNames(_optionNames) + ". " +
			"Select at most one."
		);
	}
}

bool CompilerOutputs::operator==(CompilerOutputs const& _other) const noexcept
{
	for (bool CompilerOutputs::* member: componentMap() | ranges::views::values)
		if (this->*member != _other.*member)
			return false;
	return true;
}

std::ostream& operator<<(std::ostream& _out, CompilerOutputs const& _selection)
{
	std::vector<std::string> serializedSelection;
	for (auto&& [componentName, component]: CompilerOutputs::componentMap())
		if (_selection.*component)
			serializedSelection.push_back(CompilerOutputs::componentName(component));

	return _out << util::joinHumanReadable(serializedSelection, ",");
}

std::string const& CompilerOutputs::componentName(bool CompilerOutputs::* _component)
{
	solAssert(_component, "");

	// NOTE: Linear search is not optimal but it's simpler than getting pointers-to-members to work as map keys.
	for (auto const& [componentName, component]: CompilerOutputs::componentMap())
		if (component == _component)
			return componentName;

	solAssert(false, "");
}

bool CombinedJsonRequests::operator==(CombinedJsonRequests const& _other) const noexcept
{
	for (bool CombinedJsonRequests::* member: componentMap() | ranges::views::values)
		if (this->*member != _other.*member)
			return false;
	return true;
}


std::ostream& operator<<(std::ostream& _out, CombinedJsonRequests const& _requests)
{
	std::vector<std::string> serializedRequests;
	for (auto&& [componentName, component]: CombinedJsonRequests::componentMap())
		if (_requests.*component)
			serializedRequests.push_back(CombinedJsonRequests::componentName(component));

	return _out << util::joinHumanReadable(serializedRequests, ",");
}

std::string const& CombinedJsonRequests::componentName(bool CombinedJsonRequests::* _component)
{
	solAssert(_component, "");

	for (auto const& [componentName, component]: CombinedJsonRequests::componentMap())
		if (component == _component)
			return componentName;

	solAssert(false, "");
}

bool CommandLineOptions::operator==(CommandLineOptions const& _other) const noexcept
{
	return
		input.paths == _other.input.paths &&
		input.remappings == _other.input.remappings &&
		input.addStdin == _other.input.addStdin &&
		input.basePath == _other.input.basePath &&
		input.includePaths == _other.input.includePaths &&
		input.allowedDirectories == _other.input.allowedDirectories &&
		input.ignoreMissingFiles == _other.input.ignoreMissingFiles &&
		output.dir == _other.output.dir &&
		output.overwriteFiles == _other.output.overwriteFiles &&
		output.evmVersion == _other.output.evmVersion &&
		output.viaIR == _other.output.viaIR &&
		output.revertStrings == _other.output.revertStrings &&
		output.debugInfoSelection == _other.output.debugInfoSelection &&
		output.stopAfter == _other.output.stopAfter &&
		output.eofVersion == _other.output.eofVersion &&
		input.mode == _other.input.mode &&
		assembly.targetMachine == _other.assembly.targetMachine &&
		assembly.inputLanguage == _other.assembly.inputLanguage &&
		linker.libraries == _other.linker.libraries &&
		formatting.json == _other.formatting.json &&
		formatting.coloredOutput == _other.formatting.coloredOutput &&
		formatting.withErrorIds == _other.formatting.withErrorIds &&
		compiler.outputs == _other.compiler.outputs &&
		compiler.estimateGas == _other.compiler.estimateGas &&
		compiler.combinedJsonRequests == _other.compiler.combinedJsonRequests &&
		metadata.format == _other.metadata.format &&
		metadata.hash == _other.metadata.hash &&
		metadata.literalSources == _other.metadata.literalSources &&
		optimizer.optimizeEvmasm == _other.optimizer.optimizeEvmasm &&
		optimizer.optimizeYul == _other.optimizer.optimizeYul &&
		optimizer.expectedExecutionsPerDeployment == _other.optimizer.expectedExecutionsPerDeployment &&
		optimizer.yulSteps == _other.optimizer.yulSteps &&
		modelChecker.initialize == _other.modelChecker.initialize &&
		modelChecker.settings == _other.modelChecker.settings;
}

OptimiserSettings CommandLineOptions::optimiserSettings() const
{
	OptimiserSettings settings;

	if (optimizer.optimizeEvmasm)
		settings = OptimiserSettings::standard();
	else
		settings = OptimiserSettings::minimal();

	settings.runYulOptimiser = optimizer.optimizeYul;
	if (optimizer.optimizeYul)
		// NOTE: Standard JSON disables optimizeStackAllocation by default when yul optimizer is disabled.
		// --optimize --no-optimize-yul on the CLI does not have that effect.
		settings.optimizeStackAllocation = true;

	if (optimizer.expectedExecutionsPerDeployment.has_value())
		settings.expectedExecutionsPerDeployment = optimizer.expectedExecutionsPerDeployment.value();

	if (optimizer.yulSteps.has_value())
	{
		std::string const fullSequence = optimizer.yulSteps.value();
		auto const delimiterPos = fullSequence.find(":");
		settings.yulOptimiserSteps = fullSequence.substr(0, delimiterPos);

		if (delimiterPos != std::string::npos)
			settings.yulOptimiserCleanupSteps = fullSequence.substr(delimiterPos + 1);
		else
			solAssert(settings.yulOptimiserCleanupSteps == OptimiserSettings::DefaultYulOptimiserCleanupSteps);
	}

	return settings;
}

void CommandLineParser::parse(int _argc, char const* const* _argv)
{
	parseArgs(_argc, _argv);
	processArgs();
}

void CommandLineParser::parseInputPathsAndRemappings()
{
	m_options.input.ignoreMissingFiles = (m_args.count(g_strIgnoreMissingFiles) > 0);

	if (m_args.count(g_strInputFile))
		for (std::string const& positionalArg: m_args[g_strInputFile].as<std::vector<std::string>>())
		{
			if (ImportRemapper::isRemapping(positionalArg))
			{
				std::optional<ImportRemapper::Remapping> remapping = ImportRemapper::parseRemapping(positionalArg);
				if (!remapping.has_value())
					solThrow(CommandLineValidationError, "Invalid remapping: \"" + positionalArg + "\".");

				if (m_options.input.mode == InputMode::StandardJson)
					solThrow(
						CommandLineValidationError,
						"Import remappings are not accepted on the command line in Standard JSON mode.\n"
						"Please put them under 'settings.remappings' in the JSON input."
					);

				if (!remapping->target.empty())
				{
					// If the target is a directory, whitelist it. Otherwise whitelist containing dir.
					// NOTE: /a/b/c/ is a directory while /a/b/c is not.
					boost::filesystem::path remappingDir = remapping->target;
					if (remappingDir.filename() != "..")
						// As an exception we'll treat /a/b/c/.. as a directory too. It would be
						// unintuitive to whitelist /a/b/c when the target is equivalent to /a/b/.
						remappingDir.remove_filename();
					m_options.input.allowedDirectories.insert(remappingDir.empty() ? "." : remappingDir);
				}

				m_options.input.remappings.emplace_back(std::move(remapping.value()));
			}
			else if (positionalArg == "-")
				m_options.input.addStdin = true;
			else
				m_options.input.paths.insert(positionalArg);
		}

	if (m_options.input.mode == InputMode::StandardJson)
	{
		if (m_options.input.paths.size() > 1 || (m_options.input.paths.size() == 1 && m_options.input.addStdin))
			solThrow(
				CommandLineValidationError,
				"Too many input files for --" + g_strStandardJSON + ".\n"
				"Please either specify a single file name or provide its content on standard input."
			);
		else if (m_options.input.paths.size() == 0)
			// Standard JSON mode input used to be handled separately and zero files meant "read from stdin".
			// Keep it working that way for backwards-compatibility.
			m_options.input.addStdin = true;
	}
	else if (m_options.input.paths.size() == 0 && !m_options.input.addStdin)
		solThrow(
			CommandLineValidationError,
			"No input files given. If you wish to use the standard input please specify \"-\" explicitly."
		);
}

void CommandLineParser::parseLibraryOption(std::string const& _input)
{
	namespace fs = boost::filesystem;
	std::string data = _input;
	try
	{
		if (fs::is_regular_file(_input))
			data = util::readFileAsString(_input);
	}
	catch (fs::filesystem_error const&)
	{
		// Thrown e.g. if path is too long.
	}
	catch (util::FileNotFound const&)
	{
		// Should not happen if `fs::is_regular_file` is correct.
	}
	catch (util::NotAFile const&)
	{
		// Should not happen if `fs::is_regular_file` is correct.
	}

	std::vector<std::string> libraries;
	boost::split(libraries, data, boost::is_space() || boost::is_any_of(","), boost::token_compress_on);
	for (std::string const& lib: libraries)
		if (!lib.empty())
		{
			//search for equal sign or last colon in string as our binaries output placeholders in the form of file=Name or file:Name
			//so we need to search for `=` or `:` in the string
			auto separator = lib.rfind('=');
			bool isSeparatorEqualSign = true;
			if (separator == std::string::npos)
			{
				separator = lib.rfind(':');
				if (separator == std::string::npos)
					solThrow(
						CommandLineValidationError,
						"Equal sign separator missing in library address specifier \"" + lib + "\""
					);
				else
					isSeparatorEqualSign = false; // separator is colon
			}
			else
				if (lib.rfind('=') != lib.find('='))
					solThrow(
						CommandLineValidationError,
						"Only one equal sign \"=\" is allowed in the address string \"" + lib + "\"."
					);

			std::string libName(lib.begin(), lib.begin() + static_cast<ptrdiff_t>(separator));
			boost::trim(libName);
			if (m_options.linker.libraries.count(libName))
				solThrow(
					CommandLineValidationError,
					"Address specified more than once for library \"" + libName + "\"."
				);

			std::string addrString(lib.begin() + static_cast<ptrdiff_t>(separator) + 1, lib.end());
			boost::trim(addrString);
			if (addrString.empty())
				solThrow(
					CommandLineValidationError,
					"Empty address provided for library \"" + libName + "\".\n"
					"Note that there should not be any whitespace after the " +
					(isSeparatorEqualSign ? "equal sign" : "colon") + "."
				);

			if (addrString.substr(0, 2) == "0x")
				addrString = addrString.substr(2);
			else
				solThrow(
					CommandLineValidationError,
					"The address " + addrString + " is not prefixed with \"0x\".\n"
					"Note that the address must be prefixed with \"0x\"."
				);

			if (addrString.length() != 40)
				solThrow(
					CommandLineValidationError,
					"Invalid length for address for library \"" + libName + "\": " +
					std::to_string(addrString.length()) + " instead of 40 characters."
				);
			if (!util::passesAddressChecksum(addrString, false))
				solThrow(
					CommandLineValidationError,
					"Invalid checksum on address for library \"" + libName + "\": " + addrString + "\n"
					"The correct checksum is " + util::getChecksummedAddress(addrString)
				);
			bytes binAddr = util::fromHex(addrString);
			util::h160 address(binAddr, util::h160::AlignRight);
			if (binAddr.size() > 20 || address == util::h160())
				solThrow(
					CommandLineValidationError,
					"Invalid address for library \"" + libName + "\": " + addrString
				);
			m_options.linker.libraries[libName] = address;
		}
}

void CommandLineParser::parseOutputSelection()
{
	static auto outputSupported = [](InputMode _mode, std::string_view _outputName)
	{
		static std::set<std::string> const compilerModeOutputs = (
			CompilerOutputs::componentMap() |
			ranges::views::keys |
			ranges::to<std::set>()
		);
		static std::set<std::string> const assemblerModeOutputs = {
			CompilerOutputs::componentName(&CompilerOutputs::asm_),
			CompilerOutputs::componentName(&CompilerOutputs::binary),
			CompilerOutputs::componentName(&CompilerOutputs::irOptimized),
			CompilerOutputs::componentName(&CompilerOutputs::astCompactJson),
			CompilerOutputs::componentName(&CompilerOutputs::asmJson),
		};
		static std::set<std::string> const evmAssemblyJsonImportModeOutputs = {
			CompilerOutputs::componentName(&CompilerOutputs::asm_),
			CompilerOutputs::componentName(&CompilerOutputs::binary),
			CompilerOutputs::componentName(&CompilerOutputs::binaryRuntime),
			CompilerOutputs::componentName(&CompilerOutputs::opcodes),
			CompilerOutputs::componentName(&CompilerOutputs::asmJson),
		};
		switch (_mode)
		{
		case InputMode::Help:
		case InputMode::License:
		case InputMode::Version:
		case InputMode::LanguageServer:
			solAssert(false);
		case InputMode::Compiler:
		case InputMode::CompilerWithASTImport:
			return util::contains(compilerModeOutputs, _outputName);
		case InputMode::EVMAssemblerJSON:
			return util::contains(evmAssemblyJsonImportModeOutputs, _outputName);
		case InputMode::Assembler:
			return util::contains(assemblerModeOutputs, _outputName);
		case InputMode::StandardJson:
		case InputMode::Linker:
			return false;
		}

		solAssert(false, "");
	};

	for (auto&& [optionName, outputComponent]: CompilerOutputs::componentMap())
		m_options.compiler.outputs.*outputComponent = (m_args.count(optionName) > 0);

	if (m_options.input.mode == InputMode::Assembler && m_options.compiler.outputs == CompilerOutputs{})
	{
		// In assembly mode keep the default outputs enabled for backwards-compatibility.
		// TODO: Remove this (must be done in a breaking release).
		m_options.compiler.outputs.asm_ = true;
		m_options.compiler.outputs.binary = true;
		m_options.compiler.outputs.irOptimized = true;
	}

	std::vector<std::string> unsupportedOutputs;
	for (auto&& [optionName, outputComponent]: CompilerOutputs::componentMap())
		if (m_options.compiler.outputs.*outputComponent && !outputSupported(m_options.input.mode, optionName))
			unsupportedOutputs.push_back(optionName);

	if (!unsupportedOutputs.empty())
		solThrow(
			CommandLineValidationError,
			"The following outputs are not supported in " + g_inputModeName.at(m_options.input.mode) + " mode: " +
			joinOptionNames(unsupportedOutputs) + "."
		);

	// TODO: restrict EOF version to correct EVM version.
}

po::options_description CommandLineParser::optionsDescription(bool _forHelp)
{
	// Declare the supported options.
	po::options_description desc((R"(solc, the Solidity commandline compiler.

This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you
are welcome to redistribute it under certain conditions. See 'solc --)" + g_strLicense + R"('
for details.

Usage: solc [options] [input_file...]
Compiles the given Solidity input files (or the standard input if "-" is
used as a file name) and outputs the components specified in the options
at standard output or in files in the output directory, if specified.
Imports are automatically read from the filesystem, but it is also possible to
remap paths using the context:prefix=path syntax.
Example:
solc --)" + CompilerOutputs::componentName(&CompilerOutputs::binary) + R"( -o /tmp/solcoutput dapp-bin=/usr/local/lib/dapp-bin contract.sol

General Information)").c_str(),
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	);
	desc.add_options()
		(g_strHelp.c_str(), "Show help message and exit.")
		(g_strVersion.c_str(), "Show version and exit.")
		(g_strLicense.c_str(), "Show licensing information and exit.")
	;

	po::options_description inputOptions("Input Options");
	inputOptions.add_options()
		(
			g_strBasePath.c_str(),
			po::value<std::string>()->value_name("path"),
			"Use the given path as the root of the source tree instead of the root of the filesystem."
		)
		(
			g_strIncludePath.c_str(),
			po::value<std::vector<std::string>>()->value_name("path"),
			"Make an additional source directory available to the default import callback. "
			"Use this option if you want to import contracts whose location is not fixed in relation "
			"to your main source tree, e.g. third-party libraries installed using a package manager. "
			"Can be used multiple times. "
			"Can only be used if base path has a non-empty value."
		)
		(
			g_strAllowPaths.c_str(),
			po::value<std::string>()->value_name("path(s)"),
			"Allow a given path for imports. A list of paths can be supplied by separating them with a comma."
		)
		(
			g_strIgnoreMissingFiles.c_str(),
			"Ignore missing files."
		)
	;
	desc.add(inputOptions);

	po::options_description outputOptions("Output Options");
	outputOptions.add_options()
		(
			(g_strOutputDir + ",o").c_str(),
			po::value<std::string>()->value_name("path"),
			"If given, creates one file per output component and contract/file at the specified directory."
		)
		(
			g_strOverwrite.c_str(),
			"Overwrite existing files (used together with -o)."
		)
		(
			g_strEVMVersion.c_str(),
			po::value<std::string>()->value_name("version")->default_value(EVMVersion{}.name()),
			"Select desired EVM version. Either homestead, tangerineWhistle, spuriousDragon, "
			"byzantium, constantinople, petersburg, istanbul, berlin, london, paris or shanghai."
		)
	;
	if (!_forHelp) // Note: We intentionally keep this undocumented for now.
		outputOptions.add_options()
			(
				g_strEOFVersion.c_str(),
				// Declared as uint64_t, since uint8_t will be parsed as character by boost.
				po::value<uint64_t>()->value_name("version")->implicit_value(1),
				"Select desired EOF version. Currently the only valid value is 1. "
				"If not specified, legacy non-EOF bytecode will be generated."
			)
		;
	outputOptions.add_options()
		(
			g_strExperimentalViaIR.c_str(),
			"Deprecated synonym of --via-ir."
		)
		(
			g_strViaIR.c_str(),
			"Turn on compilation mode via the IR."
		)
		(
			g_strRevertStrings.c_str(),
			po::value<std::string>()->value_name(util::joinHumanReadable(g_revertStringsArgs, ",")),
			"Strip revert (and require) reason strings or add additional debugging information."
		)
		(
			g_strDebugInfo.c_str(),
			po::value<std::string>()->default_value(util::toString(DebugInfoSelection::Default())),
			("Debug info components to be included in the produced EVM assembly and Yul code. "
			"Value can be all, none or a comma-separated list containing one or more of the "
			"following components: " + util::joinHumanReadable(DebugInfoSelection::componentMap() | ranges::views::keys) + ".").c_str()
		)
		(
			g_strStopAfter.c_str(),
			po::value<std::string>()->value_name("stage"),
			"Stop execution after the given compiler stage. Valid options: \"parsing\"."
		)
	;
	desc.add(outputOptions);

	po::options_description alternativeInputModes("Alternative Input Modes");
	alternativeInputModes.add_options()
		(
			g_strStandardJSON.c_str(),
			"Switch to Standard JSON input / output mode, ignoring all options. "
			"It reads from standard input, if no input file was given, otherwise it reads from the provided input file. The result will be written to standard output."
		)
		(
			g_strLink.c_str(),
			("Switch to linker mode, ignoring all options apart from --" + g_strLibraries + " "
			"and modify binaries in place.").c_str()
		)
		(
			g_strAssemble.c_str(),
			"Switch to assembly mode and assume input is assembly."
		)
		(
			g_strYul.c_str(),
			"Switch to Yul mode and assume input is Yul."
		)
		(
			g_strStrictAssembly.c_str(),
			"Switch to strict assembly mode and assume input is strict assembly."
		)
		(
			g_strImportAst.c_str(),
			("Import ASTs to be compiled, assumes input holds the AST in compact JSON format. "
			"Supported Inputs is the output of the --" + g_strStandardJSON + " or the one produced by "
			"--" + g_strCombinedJson + " " + CombinedJsonRequests::componentName(&CombinedJsonRequests::ast)).c_str()
		)
		(
			g_strImportEvmAssemblerJson.c_str(),
			"Import EVM assembly from JSON. Assumes input is in the format used by --asm-json."
		)
		(
			g_strLSP.c_str(),
			"Switch to language server mode (\"LSP\"). Allows the compiler to be used as an analysis backend "
			"for your favourite IDE."
		)
	;
	desc.add(alternativeInputModes);

	po::options_description assemblyModeOptions("Assembly Mode Options");
	assemblyModeOptions.add_options()
		(
			g_strMachine.c_str(),
			po::value<std::string>()->value_name(util::joinHumanReadable(g_machineArgs, ",")),
			"Target machine in assembly or Yul mode."
		)
		(
			g_strYulDialect.c_str(),
			po::value<std::string>()->value_name(util::joinHumanReadable(g_yulDialectArgs, ",")),
			"Input dialect to use in assembly or yul mode."
		)
	;
	desc.add(assemblyModeOptions);

	po::options_description linkerModeOptions("Linker Mode Options");
	linkerModeOptions.add_options()
		(
			g_strLibraries.c_str(),
			po::value<std::vector<std::string>>()->value_name("libs"),
			"Direct string or file containing library addresses. Syntax: "
			"<libraryName>=<address> [, or whitespace] ...\n"
			"Address is interpreted as a hex string prefixed by 0x."
		)
	;
	desc.add(linkerModeOptions);

	po::options_description outputFormatting("Output Formatting");
	outputFormatting.add_options()
		(
			g_strPrettyJson.c_str(),
			"Output JSON in pretty format."
		)
		(
			g_strJsonIndent.c_str(),
			po::value<uint32_t>()->value_name("N")->default_value(util::JsonFormat::defaultIndent),
			"Indent pretty-printed JSON with N spaces. Enables '--pretty-json' automatically."
		)
		(
			g_strColor.c_str(),
			"Force colored output."
		)
		(
			g_strNoColor.c_str(),
			"Explicitly disable colored output, disabling terminal auto-detection."
		)
		(
			g_strErrorIds.c_str(),
			"Output error codes."
		)
	;
	desc.add(outputFormatting);

	po::options_description outputComponents("Output Components");
	outputComponents.add_options()
		(CompilerOutputs::componentName(&CompilerOutputs::astCompactJson).c_str(), "AST of all source files in a compact JSON format.")
		(CompilerOutputs::componentName(&CompilerOutputs::asm_).c_str(), "EVM assembly of the contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::asmJson).c_str(), "EVM assembly of the contracts in JSON format.")
		(CompilerOutputs::componentName(&CompilerOutputs::opcodes).c_str(), "Opcodes of the contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::binary).c_str(), "Binary of the contracts in hex.")
		(CompilerOutputs::componentName(&CompilerOutputs::binaryRuntime).c_str(), "Binary of the runtime part of the contracts in hex.")
		(CompilerOutputs::componentName(&CompilerOutputs::abi).c_str(), "ABI specification of the contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::ir).c_str(), "Intermediate Representation (IR) of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::irAstJson).c_str(), "AST of Intermediate Representation (IR) of all contracts in a compact JSON format.")
		(CompilerOutputs::componentName(&CompilerOutputs::irOptimized).c_str(), "Optimized Intermediate Representation (IR) of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::irOptimizedAstJson).c_str(), "AST of optimized Intermediate Representation (IR) of all contracts in a compact JSON format.")
		(CompilerOutputs::componentName(&CompilerOutputs::signatureHashes).c_str(), "Function signature hashes of the contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::natspecUser).c_str(), "Natspec user documentation of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::natspecDev).c_str(), "Natspec developer documentation of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::metadata).c_str(), "Combined Metadata JSON whose IPFS hash is stored on-chain.")
		(CompilerOutputs::componentName(&CompilerOutputs::storageLayout).c_str(), "Slots, offsets and types of the contract's state variables.")
	;
	desc.add(outputComponents);

	po::options_description extraOutput("Extra Output");
	extraOutput.add_options()
		(
			g_strGas.c_str(),
			"Print an estimate of the maximal gas usage for each function."
		)
		(
			g_strCombinedJson.c_str(),
			po::value<std::string>()->value_name(util::joinHumanReadable(CombinedJsonRequests::componentMap() | ranges::views::keys, ",")),
			"Output a single json document containing the specified information."
		)
	;
	desc.add(extraOutput);

	po::options_description metadataOptions("Metadata Options");
	metadataOptions.add_options()
		(
			g_strNoCBORMetadata.c_str(),
			"Do not append CBOR metadata to the end of the bytecode."
		)
		(
			g_strMetadataHash.c_str(),
			po::value<std::string>()->value_name(util::joinHumanReadable(g_metadataHashArgs, ",")),
			"Choose hash method for the bytecode metadata or disable it."
		)
		(
			g_strMetadataLiteral.c_str(),
			"Store referenced sources as literal data in the metadata output."
		)
	;
	desc.add(metadataOptions);

	po::options_description optimizerOptions("Optimizer Options");
	optimizerOptions.add_options()
		(
			g_strOptimize.c_str(),
			"Enable optimizer."
		)
		(
			g_strOptimizeRuns.c_str(),
			// TODO: The type in OptimiserSettings is size_t but we only accept values up to 2**32-1
			// on the CLI and in Standard JSON. We should just switch to uint32_t everywhere.
			po::value<unsigned>()->value_name("n")->default_value(static_cast<unsigned>(OptimiserSettings{}.expectedExecutionsPerDeployment)),
			"The number of runs specifies roughly how often each opcode of the deployed code will be executed across the lifetime of the contract. "
			"Lower values will optimize more for initial deployment cost, higher values will optimize more for high-frequency usage."
		)
		(
			g_strOptimizeYul.c_str(),
			("Enable Yul optimizer (independently of the EVM assembly optimizer). "
			"The general --" + g_strOptimize + " option automatically enables this unless --" +
			g_strNoOptimizeYul + " is specified.").c_str()
		)
		(
			g_strNoOptimizeYul.c_str(),
			"Disable Yul optimizer (independently of the EVM assembly optimizer)."
		)
		(
			g_strYulOptimizations.c_str(),
			po::value<std::string>()->value_name("steps"),
			"Forces Yul optimizer to use the specified sequence of optimization steps instead of the built-in one."
		)
	;
	desc.add(optimizerOptions);

	po::options_description smtCheckerOptions("Model Checker Options");
	smtCheckerOptions.add_options()
		(
			g_strModelCheckerContracts.c_str(),
			po::value<std::string>()->value_name("default,<source>:<contract>")->default_value("default"),
			"Select which contracts should be analyzed using the form <source>:<contract>."
			"Multiple pairs <source>:<contract> can be selected at the same time, separated by a comma "
			"and no spaces."
		)
		(
			g_strModelCheckerDivModNoSlacks.c_str(),
			"Encode division and modulo operations with their precise operators"
			" instead of multiplication with slack variables."
		)
		(
			g_strModelCheckerEngine.c_str(),
			po::value<std::string>()->value_name("all,bmc,chc,none")->default_value("none"),
			"Select model checker engine."
		)
		(
			g_strModelCheckerExtCalls.c_str(),
			po::value<std::string>()->value_name("untrusted,trusted")->default_value("untrusted"),
			"Select whether to assume (trusted) that external calls always invoke"
			" the code given by the type of the contract, if that code is available."
		)
		(
			g_strModelCheckerInvariants.c_str(),
			po::value<std::string>()->value_name("default,all,contract,reentrancy")->default_value("default"),
			"Select whether to report inferred contract inductive invariants."
			" Multiple types of invariants can be selected at the same time, separated by a comma and no spaces."
			" By default no invariants are reported."
		)
		(
			g_strModelCheckerPrintQuery.c_str(),
			"Print the queries created by the SMTChecker in the SMTLIB2 format."
		)
		(
			g_strModelCheckerShowProvedSafe.c_str(),
			"Show all targets that were proved safe separately."
		)
		(
			g_strModelCheckerShowUnproved.c_str(),
			"Show all unproved targets separately."
		)
		(
			g_strModelCheckerShowUnsupported.c_str(),
			"Show all unsupported language features separately."
		)
		(
			g_strModelCheckerSolvers.c_str(),
			po::value<std::string>()->value_name("cvc4,eld,z3,smtlib2")->default_value("z3"),
			"Select model checker solvers."
		)
		(
			g_strModelCheckerTargets.c_str(),
			po::value<std::string>()->value_name("default,all,constantCondition,underflow,overflow,divByZero,balance,assert,popEmptyArray,outOfBounds")->default_value("default"),
			"Select model checker verification targets."
			"Multiple targets can be selected at the same time, separated by a comma and no spaces."
			" By default all targets except underflow and overflow are selected."
		)
		(
			g_strModelCheckerTimeout.c_str(),
			po::value<unsigned>()->value_name("ms"),
			"Set model checker timeout per query in milliseconds."
			"The default is a deterministic resource limit."
			"A timeout of 0 means no resource/time restrictions for any query."
		)
		(
			g_strModelCheckerBMCLoopIterations.c_str(),
			po::value<unsigned>(),
			"Set loop unrolling depth for BMC engine."
			"Default is 1."
		)
	;
	desc.add(smtCheckerOptions);

	desc.add_options()(g_strInputFile.c_str(), po::value<std::vector<std::string>>(), "input file");
	return desc;
}

po::positional_options_description CommandLineParser::positionalOptionsDescription()
{
	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add(g_strInputFile.c_str(), -1);
	return filesPositions;
}

void CommandLineParser::parseArgs(int _argc, char const* const* _argv)
{
	po::options_description allOptions = optionsDescription();
	po::positional_options_description filesPositions = positionalOptionsDescription();

	// parse the compiler arguments
	try
	{
		po::command_line_parser cmdLineParser(_argc, _argv);
		cmdLineParser.style(po::command_line_style::default_style & (~po::command_line_style::allow_guessing));
		cmdLineParser.options(allOptions).positional(filesPositions);
		po::store(cmdLineParser.run(), m_args);
	}
	catch (po::error const& _exception)
	{
		solThrow(CommandLineValidationError, _exception.what());
	}

	po::notify(m_args);
}

void CommandLineParser::processArgs()
{
	checkMutuallyExclusive({
		g_strHelp,
		g_strLicense,
		g_strVersion,
		g_strStandardJSON,
		g_strLink,
		g_strAssemble,
		g_strStrictAssembly,
		g_strYul,
		g_strImportAst,
		g_strLSP,
		g_strImportEvmAssemblerJson,
	});

	if (m_args.count(g_strHelp) > 0)
		m_options.input.mode = InputMode::Help;
	else if (m_args.count(g_strLicense) > 0)
		m_options.input.mode = InputMode::License;
	else if (m_args.count(g_strVersion) > 0)
		m_options.input.mode = InputMode::Version;
	else if (m_args.count(g_strStandardJSON) > 0)
		m_options.input.mode = InputMode::StandardJson;
	else if (m_args.count(g_strLSP))
		m_options.input.mode = InputMode::LanguageServer;
	else if (m_args.count(g_strAssemble) > 0 || m_args.count(g_strStrictAssembly) > 0 || m_args.count(g_strYul) > 0)
		m_options.input.mode = InputMode::Assembler;
	else if (m_args.count(g_strLink) > 0)
		m_options.input.mode = InputMode::Linker;
	else if (m_args.count(g_strImportAst) > 0)
		m_options.input.mode = InputMode::CompilerWithASTImport;
	else if (m_args.count(g_strImportEvmAssemblerJson) > 0)
		m_options.input.mode = InputMode::EVMAssemblerJSON;
	else
		m_options.input.mode = InputMode::Compiler;

	if (
		m_options.input.mode == InputMode::Help ||
		m_options.input.mode == InputMode::License ||
		m_options.input.mode == InputMode::Version
	)
		return;

	std::map<std::string, std::set<InputMode>> validOptionInputModeCombinations = {
		// TODO: This should eventually contain all options.
		{g_strExperimentalViaIR, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strViaIR, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strMetadataLiteral, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strNoCBORMetadata, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strMetadataHash, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerContracts, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerDivModNoSlacks, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerEngine, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerInvariants, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerPrintQuery, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerShowProvedSafe, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerShowUnproved, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerShowUnsupported, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerSolvers, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerTimeout, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerBMCLoopIterations, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerContracts, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strModelCheckerTargets, {InputMode::Compiler, InputMode::CompilerWithASTImport}}
	};
	std::vector<std::string> invalidOptionsForCurrentInputMode;
	for (auto const& [optionName, inputModes]: validOptionInputModeCombinations)
	{
		if (
			m_args.count(optionName) > 0 &&
			inputModes.count(m_options.input.mode) == 0 &&
			!m_args[optionName].defaulted()
		)
			invalidOptionsForCurrentInputMode.push_back(optionName);
	}

	if (!invalidOptionsForCurrentInputMode.empty())
		solThrow(
			CommandLineValidationError,
			"The following options are not supported in the current input mode: " +
			joinOptionNames(invalidOptionsForCurrentInputMode)
		);

	if (m_options.input.mode == InputMode::LanguageServer)
		return;

	checkMutuallyExclusive({g_strColor, g_strNoColor});
	checkMutuallyExclusive({g_strStopAfter, g_strGas});

	for (std::string const& option: CompilerOutputs::componentMap() | ranges::views::keys)
		if (option != CompilerOutputs::componentName(&CompilerOutputs::astCompactJson))
			checkMutuallyExclusive({g_strStopAfter, option});

	if (m_options.input.mode == InputMode::EVMAssemblerJSON)
	{
		static std::set<std::string> const supportedByEvmAsmJsonImport{
			g_strImportEvmAssemblerJson,
			CompilerOutputs::componentName(&CompilerOutputs::asm_),
			CompilerOutputs::componentName(&CompilerOutputs::binary),
			CompilerOutputs::componentName(&CompilerOutputs::binaryRuntime),
			CompilerOutputs::componentName(&CompilerOutputs::asmJson),
			CompilerOutputs::componentName(&CompilerOutputs::opcodes),
			g_strCombinedJson,
			g_strInputFile,
			g_strJsonIndent,
			g_strPrettyJson,
			"srcmap",
			"srcmap-runtime",
		};

		for (auto const& option: m_args)
			if (!option.second.defaulted() && !supportedByEvmAsmJsonImport.count(option.first))
				solThrow(
					CommandLineValidationError,
					fmt::format(
						"Option --{} is not supported with --{}.",
						option.first,
						g_strImportEvmAssemblerJson
					)
				);
	}

	if (
		m_options.input.mode != InputMode::Compiler &&
		m_options.input.mode != InputMode::CompilerWithASTImport &&
		m_options.input.mode != InputMode::EVMAssemblerJSON &&
		m_options.input.mode != InputMode::Assembler
	)
	{
		if (!m_args[g_strOptimizeRuns].defaulted())
			solThrow(
				CommandLineValidationError,
				"Option --" + g_strOptimizeRuns + " is only valid in compiler and assembler modes."
			);

		for (std::string const& option: {g_strOptimize, g_strNoOptimizeYul, g_strOptimizeYul, g_strYulOptimizations})
			if (m_args.count(option) > 0)
				solThrow(
					CommandLineValidationError,
					"Option --" + option + " is only valid in compiler and assembler modes."
				);

		if (!m_args[g_strDebugInfo].defaulted())
			solThrow(
				CommandLineValidationError,
				"Option --" + g_strDebugInfo + " is only valid in compiler and assembler modes."
			);
	}

	if (m_args.count(g_strColor) > 0)
		m_options.formatting.coloredOutput = true;
	else if (m_args.count(g_strNoColor) > 0)
		m_options.formatting.coloredOutput = false;

	m_options.formatting.withErrorIds = m_args.count(g_strErrorIds);

	if (m_args.count(g_strRevertStrings))
	{
		std::string revertStringsString = m_args[g_strRevertStrings].as<std::string>();
		std::optional<RevertStrings> revertStrings = revertStringsFromString(revertStringsString);
		if (!revertStrings)
			solThrow(
				CommandLineValidationError,
				"Invalid option for --" + g_strRevertStrings + ": " + revertStringsString
			);
		if (*revertStrings == RevertStrings::VerboseDebug)
			solThrow(
				CommandLineValidationError,
				"Only \"default\", \"strip\" and \"debug\" are implemented for --" + g_strRevertStrings + " for now."
			);
		m_options.output.revertStrings = *revertStrings;
	}

	if (!m_args[g_strDebugInfo].defaulted())
	{
		std::string optionValue = m_args[g_strDebugInfo].as<std::string>();
		m_options.output.debugInfoSelection = DebugInfoSelection::fromString(optionValue);
		if (!m_options.output.debugInfoSelection.has_value())
			solThrow(CommandLineValidationError, "Invalid value for --" + g_strDebugInfo + " option: " + optionValue);

		if (m_options.output.debugInfoSelection->snippet && !m_options.output.debugInfoSelection->location)
			solThrow(CommandLineValidationError, "To use 'snippet' with --" + g_strDebugInfo + " you must select also 'location'.");
	}

	parseCombinedJsonOption();

	if (m_args.count(g_strOutputDir))
		m_options.output.dir = m_args.at(g_strOutputDir).as<std::string>();

	m_options.output.overwriteFiles = (m_args.count(g_strOverwrite) > 0);

	if (m_args.count(g_strPrettyJson) > 0)
	{
		m_options.formatting.json.format = util::JsonFormat::Pretty;
	}
	if (!m_args[g_strJsonIndent].defaulted())
	{
		m_options.formatting.json.format = util::JsonFormat::Pretty;
		m_options.formatting.json.indent = m_args[g_strJsonIndent].as<uint32_t>();
	}

	parseOutputSelection();

	m_options.compiler.estimateGas = (m_args.count(g_strGas) > 0);

	if (m_args.count(g_strBasePath))
		m_options.input.basePath = m_args[g_strBasePath].as<std::string>();

	if (m_args.count(g_strIncludePath) > 0)
	{
		if (m_options.input.basePath.empty())
			solThrow(CommandLineValidationError, "--" + g_strIncludePath + " option requires a non-empty base path.");

		for (std::string const& includePath: m_args[g_strIncludePath].as<std::vector<std::string>>())
		{
			if (includePath.empty())
				solThrow(CommandLineValidationError, "Empty values are not allowed in --" + g_strIncludePath + ".");

			m_options.input.includePaths.push_back(includePath);
		}
	}

	if (m_args.count(g_strAllowPaths))
	{
		std::vector<std::string> paths;
		for (std::string const& allowedPath: boost::split(paths, m_args[g_strAllowPaths].as<std::string>(), boost::is_any_of(",")))
			if (!allowedPath.empty())
				m_options.input.allowedDirectories.insert(allowedPath);
	}

	if (m_args.count(g_strStopAfter))
	{
		if (m_args[g_strStopAfter].as<std::string>() != "parsing")
			solThrow(CommandLineValidationError, "Valid options for --" + g_strStopAfter + " are: \"parsing\".\n");
		else
			m_options.output.stopAfter = CompilerStack::State::Parsed;
	}

	parseInputPathsAndRemappings();

	if (m_options.input.mode == InputMode::StandardJson)
		return;

	if (m_args.count(g_strLibraries))
		for (std::string const& library: m_args[g_strLibraries].as<std::vector<std::string>>())
			parseLibraryOption(library);

	if (m_options.input.mode == InputMode::Linker)
		return;

	if (m_args.count(g_strEVMVersion))
	{
		std::string versionOptionStr = m_args[g_strEVMVersion].as<std::string>();
		std::optional<langutil::EVMVersion> versionOption = langutil::EVMVersion::fromString(versionOptionStr);
		if (!versionOption)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strEVMVersion + ": " + versionOptionStr);
		m_options.output.evmVersion = *versionOption;
	}

	if (m_args.count(g_strEOFVersion))
	{
		// Request as uint64_t, since uint8_t will be parsed as character by boost.
		uint64_t versionOption = m_args[g_strEOFVersion].as<uint64_t>();
		if (versionOption != 1)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strEOFVersion + ": " + std::to_string(versionOption));
		m_options.output.eofVersion = 1;
	}

	if (m_args.count(g_strNoOptimizeYul) > 0 && m_args.count(g_strOptimizeYul) > 0)
		solThrow(
			CommandLineValidationError,
			"Options --" + g_strOptimizeYul + " and --" + g_strNoOptimizeYul + " cannot be used together."
		);

	m_options.optimizer.optimizeEvmasm = (m_args.count(g_strOptimize) > 0);
	m_options.optimizer.optimizeYul = (
		(m_args.count(g_strOptimize) > 0 && m_args.count(g_strNoOptimizeYul) == 0) ||
		m_args.count(g_strOptimizeYul) > 0
	);
	if (!m_args[g_strOptimizeRuns].defaulted())
		m_options.optimizer.expectedExecutionsPerDeployment = m_args.at(g_strOptimizeRuns).as<unsigned>();

	if (m_args.count(g_strYulOptimizations))
	{
		OptimiserSettings optimiserSettings = m_options.optimiserSettings();
		if (!optimiserSettings.runYulOptimiser)
			solThrow(CommandLineValidationError, "--" + g_strYulOptimizations + " is invalid if Yul optimizer is disabled");

		try
		{
			yul::OptimiserSuite::validateSequence(m_args[g_strYulOptimizations].as<std::string>());
		}
		catch (yul::OptimizerException const& _exception)
		{
			solThrow(
				CommandLineValidationError,
				"Invalid optimizer step sequence in --" + g_strYulOptimizations + ": " + _exception.what()
			);
		}

		m_options.optimizer.yulSteps = m_args[g_strYulOptimizations].as<std::string>();
	}

	if (m_options.input.mode == InputMode::Assembler)
	{
		std::vector<std::string> const nonAssemblyModeOptions = {
			// TODO: The list is not complete. Add more.
			g_strOutputDir,
			g_strGas,
			g_strCombinedJson,
		};
		if (countEnabledOptions(nonAssemblyModeOptions) >= 1)
		{
			auto optionEnabled = [&](std::string const& name){ return m_args.count(name) > 0; };
			auto enabledOptions = nonAssemblyModeOptions | ranges::views::filter(optionEnabled) | ranges::to_vector;

			std::string message = "The following options are invalid in assembly mode: " + joinOptionNames(enabledOptions) + ".";
			solThrow(CommandLineValidationError, message);
		}

		// switch to assembly mode
		using Input = yul::YulStack::Language;
		using Machine = yul::YulStack::Machine;
		m_options.assembly.inputLanguage = m_args.count(g_strYul) ? Input::Yul : (m_args.count(g_strStrictAssembly) ? Input::StrictAssembly : Input::Assembly);

		if (m_args.count(g_strMachine))
		{
			std::string machine = m_args[g_strMachine].as<std::string>();
			if (machine == g_strEVM)
				m_options.assembly.targetMachine = Machine::EVM;
			else
				solThrow(CommandLineValidationError, "Invalid option for --" + g_strMachine + ": " + machine);
		}
		if (m_args.count(g_strYulDialect))
		{
			std::string dialect = m_args[g_strYulDialect].as<std::string>();
			if (dialect == g_strEVM)
				m_options.assembly.inputLanguage = Input::StrictAssembly;
			else
				solThrow(CommandLineValidationError, "Invalid option for --" + g_strYulDialect + ": " + dialect);
		}
		if (
				(m_options.optimizer.optimizeEvmasm || m_options.optimizer.optimizeYul) &&
				m_options.assembly.inputLanguage != Input::StrictAssembly
			)
			solThrow(
				CommandLineValidationError,
				"Optimizer can only be used for strict assembly. Use --"  + g_strStrictAssembly + "."
			);
		return;
	}
	else if (countEnabledOptions({g_strYulDialect, g_strMachine}) >= 1)
		solThrow(
			CommandLineValidationError,
			"--" + g_strYulDialect + " and --" + g_strMachine + " are only valid in assembly mode."
		);

	if (m_args.count(g_strMetadataHash))
	{
		std::string hashStr = m_args[g_strMetadataHash].as<std::string>();
		if (hashStr == g_strIPFS)
			m_options.metadata.hash = CompilerStack::MetadataHash::IPFS;
		else if (hashStr == g_strSwarm)
			m_options.metadata.hash = CompilerStack::MetadataHash::Bzzr1;
		else if (hashStr == g_strNone)
			m_options.metadata.hash = CompilerStack::MetadataHash::None;
		else
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strMetadataHash + ": " + hashStr);
	}

	if (m_args.count(g_strNoCBORMetadata))
	{
		if (
			m_args.count(g_strMetadataHash) &&
			m_options.metadata.hash != CompilerStack::MetadataHash::None
		)
			solThrow(
				CommandLineValidationError,
				"Cannot specify a metadata hashing method when --" +
				g_strNoCBORMetadata + " is set."
			);

		m_options.metadata.format = CompilerStack::MetadataFormat::NoMetadata;
	}

	if (m_args.count(g_strModelCheckerContracts))
	{
		std::string contractsStr = m_args[g_strModelCheckerContracts].as<std::string>();
		std::optional<ModelCheckerContracts> contracts = ModelCheckerContracts::fromString(contractsStr);
		if (!contracts)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerContracts + ": " + contractsStr);
		m_options.modelChecker.settings.contracts = std::move(*contracts);
	}

	if (m_args.count(g_strModelCheckerDivModNoSlacks))
		m_options.modelChecker.settings.divModNoSlacks = true;

	if (m_args.count(g_strModelCheckerEngine))
	{
		std::string engineStr = m_args[g_strModelCheckerEngine].as<std::string>();
		std::optional<ModelCheckerEngine> engine = ModelCheckerEngine::fromString(engineStr);
		if (!engine)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerEngine + ": " + engineStr);
		m_options.modelChecker.settings.engine = *engine;
	}

	if (m_args.count(g_strModelCheckerExtCalls))
	{
		std::string mode = m_args[g_strModelCheckerExtCalls].as<std::string>();
		std::optional<ModelCheckerExtCalls> extCallsMode = ModelCheckerExtCalls::fromString(mode);
		if (!extCallsMode)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerExtCalls + ": " + mode);
		m_options.modelChecker.settings.externalCalls = *extCallsMode;
	}

	if (m_args.count(g_strModelCheckerInvariants))
	{
		std::string invsStr = m_args[g_strModelCheckerInvariants].as<std::string>();
		std::optional<ModelCheckerInvariants> invs = ModelCheckerInvariants::fromString(invsStr);
		if (!invs)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerInvariants + ": " + invsStr);
		m_options.modelChecker.settings.invariants = *invs;
	}

	if (m_args.count(g_strModelCheckerShowProvedSafe))
		m_options.modelChecker.settings.showProvedSafe = true;

	if (m_args.count(g_strModelCheckerShowUnproved))
		m_options.modelChecker.settings.showUnproved = true;

	if (m_args.count(g_strModelCheckerShowUnsupported))
		m_options.modelChecker.settings.showUnsupported = true;

	if (m_args.count(g_strModelCheckerSolvers))
	{
		std::string solversStr = m_args[g_strModelCheckerSolvers].as<std::string>();
		std::optional<smtutil::SMTSolverChoice> solvers = smtutil::SMTSolverChoice::fromString(solversStr);
		if (!solvers)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerSolvers + ": " + solversStr);
		m_options.modelChecker.settings.solvers = *solvers;
	}

	if (m_args.count(g_strModelCheckerPrintQuery))
	{
		if (!(m_options.modelChecker.settings.solvers == smtutil::SMTSolverChoice::SMTLIB2()))
			solThrow(CommandLineValidationError, "Only SMTLib2 solver can be enabled to print queries");
		m_options.modelChecker.settings.printQuery = true;
	}

	if (m_args.count(g_strModelCheckerTargets))
	{
		std::string targetsStr = m_args[g_strModelCheckerTargets].as<std::string>();
		std::optional<ModelCheckerTargets> targets = ModelCheckerTargets::fromString(targetsStr);
		if (!targets)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerTargets + ": " + targetsStr);
		m_options.modelChecker.settings.targets = *targets;
	}

	if (m_args.count(g_strModelCheckerTimeout))
		m_options.modelChecker.settings.timeout = m_args[g_strModelCheckerTimeout].as<unsigned>();

	if (m_args.count(g_strModelCheckerBMCLoopIterations))
	{
		if (!m_options.modelChecker.settings.engine.bmc)
			solThrow(CommandLineValidationError, "BMC loop unrolling requires the BMC engine to be enabled");
		m_options.modelChecker.settings.bmcLoopIterations = m_args[g_strModelCheckerBMCLoopIterations].as<unsigned>();
	}

	m_options.metadata.literalSources = (m_args.count(g_strMetadataLiteral) > 0);
	m_options.modelChecker.initialize =
		m_args.count(g_strModelCheckerContracts) ||
		m_args.count(g_strModelCheckerDivModNoSlacks) ||
		m_args.count(g_strModelCheckerEngine) ||
		m_args.count(g_strModelCheckerExtCalls) ||
		m_args.count(g_strModelCheckerInvariants) ||
		m_args.count(g_strModelCheckerShowProvedSafe) ||
		m_args.count(g_strModelCheckerShowUnproved) ||
		m_args.count(g_strModelCheckerShowUnsupported) ||
		m_args.count(g_strModelCheckerSolvers) ||
		m_args.count(g_strModelCheckerTargets) ||
		m_args.count(g_strModelCheckerTimeout);
	m_options.output.viaIR = (m_args.count(g_strExperimentalViaIR) > 0 || m_args.count(g_strViaIR) > 0);

	solAssert(
		m_options.input.mode == InputMode::Compiler ||
		m_options.input.mode == InputMode::CompilerWithASTImport ||
		m_options.input.mode == InputMode::EVMAssemblerJSON
	);
}

void CommandLineParser::parseCombinedJsonOption()
{
	if (!m_args.count(g_strCombinedJson))
		return;

	std::set<std::string> requests;
	for (std::string const& item: boost::split(requests, m_args[g_strCombinedJson].as<std::string>(), boost::is_any_of(",")))
		if (CombinedJsonRequests::componentMap().count(item) == 0)
			solThrow(CommandLineValidationError, "Invalid option to --" + g_strCombinedJson + ": " + item);

	m_options.compiler.combinedJsonRequests = CombinedJsonRequests{};
	for (auto&& [componentName, component]: CombinedJsonRequests::componentMap())
		m_options.compiler.combinedJsonRequests.value().*component = (requests.count(componentName) > 0);

	if (m_options.input.mode == InputMode::EVMAssemblerJSON && m_options.compiler.combinedJsonRequests.has_value())
	{
		static bool CombinedJsonRequests::* invalidOptions[]{
			&CombinedJsonRequests::abi,
			&CombinedJsonRequests::ast,
			&CombinedJsonRequests::funDebug,
			&CombinedJsonRequests::funDebugRuntime,
			&CombinedJsonRequests::generatedSources,
			&CombinedJsonRequests::generatedSourcesRuntime,
			&CombinedJsonRequests::metadata,
			&CombinedJsonRequests::natspecDev,
			&CombinedJsonRequests::natspecUser,
			&CombinedJsonRequests::signatureHashes,
			&CombinedJsonRequests::storageLayout
		};

		for (auto const invalidOption: invalidOptions)
			if (m_options.compiler.combinedJsonRequests.value().*invalidOption)
				solThrow(
					CommandLineValidationError,
					fmt::format(
						"Invalid option to --{}: {} for --{}",
						g_strCombinedJson,
						CombinedJsonRequests::componentName(invalidOption),
						g_strImportEvmAssemblerJson
					)
				);
	}
}

size_t CommandLineParser::countEnabledOptions(std::vector<std::string> const& _optionNames) const
{
	size_t count = 0;
	for (std::string const& _option: _optionNames)
		count += m_args.count(_option);

	return count;
}

std::string CommandLineParser::joinOptionNames(std::vector<std::string> const& _optionNames, std::string _separator)
{
	return util::joinHumanReadable(
		_optionNames | ranges::views::transform([](std::string const& _option){ return "--" + _option; }),
		_separator
	);
}

} // namespace solidity::frontend
