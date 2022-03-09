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

using namespace std;
using namespace solidity::langutil;

namespace po = boost::program_options;

namespace solidity::frontend
{

static string const g_strAllowPaths = "allow-paths";
static string const g_strBasePath = "base-path";
static string const g_strIncludePath = "include-path";
static string const g_strAssemble = "assemble";
static string const g_strCombinedJson = "combined-json";
static string const g_strErrorRecovery = "error-recovery";
static string const g_strEVM = "evm";
static string const g_strEVMVersion = "evm-version";
static string const g_strEwasm = "ewasm";
static string const g_strViaIR = "via-ir";
static string const g_strExperimentalViaIR = "experimental-via-ir";
static string const g_strGas = "gas";
static string const g_strHelp = "help";
static string const g_strImportAst = "import-ast";
static string const g_strInputFile = "input-file";
static string const g_strYul = "yul";
static string const g_strYulDialect = "yul-dialect";
static string const g_strDebugInfo = "debug-info";
static string const g_strIPFS = "ipfs";
static string const g_strLicense = "license";
static string const g_strLibraries = "libraries";
static string const g_strLink = "link";
static string const g_strLSP = "lsp";
static string const g_strMachine = "machine";
static string const g_strMetadataHash = "metadata-hash";
static string const g_strMetadataLiteral = "metadata-literal";
static string const g_strModelCheckerContracts = "model-checker-contracts";
static string const g_strModelCheckerDivModNoSlacks = "model-checker-div-mod-no-slacks";
static string const g_strModelCheckerEngine = "model-checker-engine";
static string const g_strModelCheckerInvariants = "model-checker-invariants";
static string const g_strModelCheckerShowUnproved = "model-checker-show-unproved";
static string const g_strModelCheckerSolvers = "model-checker-solvers";
static string const g_strModelCheckerTargets = "model-checker-targets";
static string const g_strModelCheckerTimeout = "model-checker-timeout";
static string const g_strNone = "none";
static string const g_strNoOptimizeYul = "no-optimize-yul";
static string const g_strOptimize = "optimize";
static string const g_strOptimizeRuns = "optimize-runs";
static string const g_strOptimizeYul = "optimize-yul";
static string const g_strYulOptimizations = "yul-optimizations";
static string const g_strOutputDir = "output-dir";
static string const g_strOverwrite = "overwrite";
static string const g_strRevertStrings = "revert-strings";
static string const g_strStopAfter = "stop-after";
static string const g_strParsing = "parsing";

/// Possible arguments to for --revert-strings
static set<string> const g_revertStringsArgs
{
	revertStringsToString(RevertStrings::Default),
	revertStringsToString(RevertStrings::Strip),
	revertStringsToString(RevertStrings::Debug),
	revertStringsToString(RevertStrings::VerboseDebug)
};

static string const g_strSources = "sources";
static string const g_strSourceList = "sourceList";
static string const g_strStandardJSON = "standard-json";
static string const g_strStrictAssembly = "strict-assembly";
static string const g_strSwarm = "swarm";
static string const g_strPrettyJson = "pretty-json";
static string const g_strJsonIndent = "json-indent";
static string const g_strVersion = "version";
static string const g_strIgnoreMissingFiles = "ignore-missing";
static string const g_strColor = "color";
static string const g_strNoColor = "no-color";
static string const g_strErrorIds = "error-codes";

/// Possible arguments to for --machine
static set<string> const g_machineArgs
{
	g_strEVM,
	g_strEwasm
};

/// Possible arguments to for --yul-dialect
static set<string> const g_yulDialectArgs
{
	g_strEVM,
	g_strEwasm
};

/// Possible arguments to for --metadata-hash
static set<string> const g_metadataHashArgs
{
	g_strIPFS,
	g_strSwarm,
	g_strNone
};

static map<InputMode, string> const g_inputModeName = {
	{InputMode::Help, "help"},
	{InputMode::License, "license"},
	{InputMode::Version, "version"},
	{InputMode::Compiler, "compiler"},
	{InputMode::CompilerWithASTImport, "compiler (AST import)"},
	{InputMode::Assembler, "assembler"},
	{InputMode::StandardJson, "standard JSON"},
	{InputMode::Linker, "linker"},
	{InputMode::LanguageServer, "language server (LSP)"},
};

void CommandLineParser::checkMutuallyExclusive(vector<string> const& _optionNames)
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

ostream& operator<<(ostream& _out, CompilerOutputs const& _selection)
{
	vector<string> serializedSelection;
	for (auto&& [componentName, component]: CompilerOutputs::componentMap())
		if (_selection.*component)
			serializedSelection.push_back(CompilerOutputs::componentName(component));

	return _out << util::joinHumanReadable(serializedSelection, ",");
}

string const& CompilerOutputs::componentName(bool CompilerOutputs::* _component)
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


ostream& operator<<(ostream& _out, CombinedJsonRequests const& _requests)
{
	vector<string> serializedRequests;
	for (auto&& [componentName, component]: CombinedJsonRequests::componentMap())
		if (_requests.*component)
			serializedRequests.push_back(CombinedJsonRequests::componentName(component));

	return _out << util::joinHumanReadable(serializedRequests, ",");
}

string const& CombinedJsonRequests::componentName(bool CombinedJsonRequests::* _component)
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
		input.errorRecovery == _other.input.errorRecovery &&
		output.dir == _other.output.dir &&
		output.overwriteFiles == _other.output.overwriteFiles &&
		output.evmVersion == _other.output.evmVersion &&
		output.viaIR == _other.output.viaIR &&
		output.revertStrings == _other.output.revertStrings &&
		output.debugInfoSelection == _other.output.debugInfoSelection &&
		output.stopAfter == _other.output.stopAfter &&
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
		metadata.hash == _other.metadata.hash &&
		metadata.literalSources == _other.metadata.literalSources &&
		optimizer.enabled == _other.optimizer.enabled &&
		optimizer.expectedExecutionsPerDeployment == _other.optimizer.expectedExecutionsPerDeployment &&
		optimizer.noOptimizeYul == _other.optimizer.noOptimizeYul &&
		optimizer.yulSteps == _other.optimizer.yulSteps &&
		modelChecker.initialize == _other.modelChecker.initialize &&
		modelChecker.settings == _other.modelChecker.settings;
}

OptimiserSettings CommandLineOptions::optimiserSettings() const
{
	OptimiserSettings settings;

	if (optimizer.enabled)
		settings = OptimiserSettings::standard();
	else
		settings = OptimiserSettings::minimal();

	if (optimizer.noOptimizeYul)
		settings.runYulOptimiser = false;

	if (optimizer.expectedExecutionsPerDeployment.has_value())
		settings.expectedExecutionsPerDeployment = optimizer.expectedExecutionsPerDeployment.value();

	if (optimizer.yulSteps.has_value())
		settings.yulOptimiserSteps = optimizer.yulSteps.value();

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
		for (string const& positionalArg: m_args[g_strInputFile].as<vector<string>>())
		{
			if (ImportRemapper::isRemapping(positionalArg))
			{
				optional<ImportRemapper::Remapping> remapping = ImportRemapper::parseRemapping(positionalArg);
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

				m_options.input.remappings.emplace_back(move(remapping.value()));
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

void CommandLineParser::parseLibraryOption(string const& _input)
{
	namespace fs = boost::filesystem;
	string data = _input;
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

	vector<string> libraries;
	boost::split(libraries, data, boost::is_space() || boost::is_any_of(","), boost::token_compress_on);
	for (string const& lib: libraries)
		if (!lib.empty())
		{
			//search for equal sign or last colon in string as our binaries output placeholders in the form of file=Name or file:Name
			//so we need to search for `=` or `:` in the string
			auto separator = lib.rfind('=');
			bool isSeparatorEqualSign = true;
			if (separator == string::npos)
			{
				separator = lib.rfind(':');
				if (separator == string::npos)
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

			string libName(lib.begin(), lib.begin() + static_cast<ptrdiff_t>(separator));
			boost::trim(libName);
			if (m_options.linker.libraries.count(libName))
				solThrow(
					CommandLineValidationError,
					"Address specified more than once for library \"" + libName + "\"."
				);

			string addrString(lib.begin() + static_cast<ptrdiff_t>(separator) + 1, lib.end());
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
					to_string(addrString.length()) + " instead of 40 characters."
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
	static auto outputSupported = [](InputMode _mode, string_view _outputName)
	{
		static set<string> const compilerModeOutputs = (
			CompilerOutputs::componentMap() |
			ranges::views::keys |
			ranges::to<set>()
		) - set<string>{CompilerOutputs::componentName(&CompilerOutputs::ewasmIR)};
		static set<string> const assemblerModeOutputs = {
			CompilerOutputs::componentName(&CompilerOutputs::asm_),
			CompilerOutputs::componentName(&CompilerOutputs::binary),
			CompilerOutputs::componentName(&CompilerOutputs::irOptimized),
			CompilerOutputs::componentName(&CompilerOutputs::ewasm),
			CompilerOutputs::componentName(&CompilerOutputs::ewasmIR),
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
		m_options.compiler.outputs.ewasm = true;
		m_options.compiler.outputs.ewasmIR = true;
	}

	vector<string> unsupportedOutputs;
	for (auto&& [optionName, outputComponent]: CompilerOutputs::componentMap())
		if (m_options.compiler.outputs.*outputComponent && !outputSupported(m_options.input.mode, optionName))
			unsupportedOutputs.push_back(optionName);

	if (!unsupportedOutputs.empty())
		solThrow(
			CommandLineValidationError,
			"The following outputs are not supported in " + g_inputModeName.at(m_options.input.mode) + " mode: " +
			joinOptionNames(unsupportedOutputs) + "."
		);
}

po::options_description CommandLineParser::optionsDescription()
{
	// Declare the supported options.
	po::options_description desc((R"(solc, the Solidity commandline compiler.

This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you
are welcome to redistribute it under certain conditions. See 'solc --)" + g_strLicense + R"('
for details.

Usage: solc [options] [input_file...]
Compiles the given Solidity input files (or the standard input if none given or
"-" is used as a file name) and outputs the components specified in the options
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
			po::value<string>()->value_name("path"),
			"Use the given path as the root of the source tree instead of the root of the filesystem."
		)
		(
			g_strIncludePath.c_str(),
			po::value<vector<string>>()->value_name("path"),
			"Make an additional source directory available to the default import callback. "
			"Use this option if you want to import contracts whose location is not fixed in relation "
			"to your main source tree, e.g. third-party libraries installed using a package manager. "
			"Can be used multiple times. "
			"Can only be used if base path has a non-empty value."
		)
		(
			g_strAllowPaths.c_str(),
			po::value<string>()->value_name("path(s)"),
			"Allow a given path for imports. A list of paths can be supplied by separating them with a comma."
		)
		(
			g_strIgnoreMissingFiles.c_str(),
			"Ignore missing files."
		)
		(
			g_strErrorRecovery.c_str(),
			"Enables additional parser error recovery."
		)
	;
	desc.add(inputOptions);

	po::options_description outputOptions("Output Options");
	outputOptions.add_options()
		(
			(g_strOutputDir + ",o").c_str(),
			po::value<string>()->value_name("path"),
			"If given, creates one file per component and contract/file at the specified directory."
		)
		(
			g_strOverwrite.c_str(),
			"Overwrite existing files (used together with -o)."
		)
		(
			g_strEVMVersion.c_str(),
			po::value<string>()->value_name("version")->default_value(EVMVersion{}.name()),
			"Select desired EVM version. Either homestead, tangerineWhistle, spuriousDragon, "
			"byzantium, constantinople, petersburg, istanbul, berlin or london."
		)
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
			po::value<string>()->value_name(util::joinHumanReadable(g_revertStringsArgs, ",")),
			"Strip revert (and require) reason strings or add additional debugging information."
		)
		(
			g_strDebugInfo.c_str(),
			po::value<string>()->default_value(util::toString(DebugInfoSelection::Default())),
			("Debug info components to be included in the produced EVM assembly and Yul code. "
			"Value can be all, none or a comma-separated list containing one or more of the "
			"following components: " + util::joinHumanReadable(DebugInfoSelection::componentMap() | ranges::views::keys) + ".").c_str()
		)
		(
			g_strStopAfter.c_str(),
			po::value<string>()->value_name("stage"),
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
			("Switch to assembly mode, ignoring all options except "
			"--" + g_strMachine + ", --" + g_strYulDialect + ", --" + g_strOptimize + " and --" + g_strYulOptimizations + " "
			"and assumes input is assembly.").c_str()
		)
		(
			g_strYul.c_str(),
			("Switch to Yul mode, ignoring all options except "
			"--" + g_strMachine + ", --" + g_strYulDialect + ", --" + g_strOptimize + " and --" + g_strYulOptimizations + " "
			"and assumes input is Yul.").c_str()
		)
		(
			g_strStrictAssembly.c_str(),
			("Switch to strict assembly mode, ignoring all options except "
			"--" + g_strMachine + ", --" + g_strYulDialect + ", --" + g_strOptimize + " and --" + g_strYulOptimizations + " "
			"and assumes input is strict assembly.").c_str()
		)
		(
			g_strImportAst.c_str(),
			("Import ASTs to be compiled, assumes input holds the AST in compact JSON format. "
			"Supported Inputs is the output of the --" + g_strStandardJSON + " or the one produced by "
			"--" + g_strCombinedJson + " " + CombinedJsonRequests::componentName(&CombinedJsonRequests::ast)).c_str()
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
			po::value<string>()->value_name(util::joinHumanReadable(g_machineArgs, ",")),
			"Target machine in assembly or Yul mode."
		)
		(
			g_strYulDialect.c_str(),
			po::value<string>()->value_name(util::joinHumanReadable(g_yulDialectArgs, ",")),
			"Input dialect to use in assembly or yul mode."
		)
	;
	desc.add(assemblyModeOptions);

	po::options_description linkerModeOptions("Linker Mode Options");
	linkerModeOptions.add_options()
		(
			g_strLibraries.c_str(),
			po::value<vector<string>>()->value_name("libs"),
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
		(CompilerOutputs::componentName(&CompilerOutputs::irOptimized).c_str(), "Optimized intermediate Representation (IR) of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::ewasm).c_str(), "Ewasm text representation of all contracts (EXPERIMENTAL).")
		(CompilerOutputs::componentName(&CompilerOutputs::ewasmIR).c_str(), "Intermediate representation (IR) converted to a form that can be translated directly into Ewasm text representation (EXPERIMENTAL).")
		(CompilerOutputs::componentName(&CompilerOutputs::signatureHashes).c_str(), "Function signature hashes of the contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::natspecUser).c_str(), "Natspec user documentation of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::natspecDev).c_str(), "Natspec developer documentation of all contracts.")
		(CompilerOutputs::componentName(&CompilerOutputs::metadata).c_str(), "Combined Metadata JSON whose Swarm hash is stored on-chain.")
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
			po::value<string>()->value_name(util::joinHumanReadable(CombinedJsonRequests::componentMap() | ranges::views::keys, ",")),
			"Output a single json document containing the specified information."
		)
	;
	desc.add(extraOutput);

	po::options_description metadataOptions("Metadata Options");
	metadataOptions.add_options()
		(
			g_strMetadataHash.c_str(),
			po::value<string>()->value_name(util::joinHumanReadable(g_metadataHashArgs, ",")),
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
			"Enable bytecode optimizer."
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
			("Legacy option, ignored. Use the general --" + g_strOptimize + " to enable Yul optimizer.").c_str()
		)
		(
			g_strNoOptimizeYul.c_str(),
			"Disable Yul optimizer in Solidity."
		)
		(
			g_strYulOptimizations.c_str(),
			po::value<string>()->value_name("steps"),
			"Forces yul optimizer to use the specified sequence of optimization steps instead of the built-in one."
		)
	;
	desc.add(optimizerOptions);

	po::options_description smtCheckerOptions("Model Checker Options");
	smtCheckerOptions.add_options()
		(
			g_strModelCheckerContracts.c_str(),
			po::value<string>()->value_name("default,<source>:<contract>")->default_value("default"),
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
			po::value<string>()->value_name("all,bmc,chc,none")->default_value("none"),
			"Select model checker engine."
		)
		(
			g_strModelCheckerInvariants.c_str(),
			po::value<string>()->value_name("default,all,contract,reentrancy")->default_value("default"),
			"Select whether to report inferred contract inductive invariants."
			" Multiple types of invariants can be selected at the same time, separated by a comma and no spaces."
			" By default no invariants are reported."
		)
		(
			g_strModelCheckerShowUnproved.c_str(),
			"Show all unproved targets separately."
		)
		(
			g_strModelCheckerSolvers.c_str(),
			po::value<string>()->value_name("all,cvc4,z3,smtlib2")->default_value("all"),
			"Select model checker solvers."
		)
		(
			g_strModelCheckerTargets.c_str(),
			po::value<string>()->value_name("default,all,constantCondition,underflow,overflow,divByZero,balance,assert,popEmptyArray,outOfBounds")->default_value("default"),
			"Select model checker verification targets. "
			"Multiple targets can be selected at the same time, separated by a comma and no spaces."
			" By default all targets except underflow and overflow are selected."
		)
		(
			g_strModelCheckerTimeout.c_str(),
			po::value<unsigned>()->value_name("ms"),
			"Set model checker timeout per query in milliseconds. "
			"The default is a deterministic resource limit. "
			"A timeout of 0 means no resource/time restrictions for any query."
		)
	;
	desc.add(smtCheckerOptions);

	desc.add_options()(g_strInputFile.c_str(), po::value<vector<string>>(), "input file");
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
		g_strLSP
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
	else
		m_options.input.mode = InputMode::Compiler;

	if (
		m_options.input.mode == InputMode::Help ||
		m_options.input.mode == InputMode::License ||
		m_options.input.mode == InputMode::Version
	)
		return;

	map<string, set<InputMode>> validOptionInputModeCombinations = {
		// TODO: This should eventually contain all options.
		{g_strErrorRecovery, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strExperimentalViaIR, {InputMode::Compiler, InputMode::CompilerWithASTImport}},
		{g_strViaIR, {InputMode::Compiler, InputMode::CompilerWithASTImport}}
	};
	vector<string> invalidOptionsForCurrentInputMode;
	for (auto const& [optionName, inputModes]: validOptionInputModeCombinations)
	{
		if (m_args.count(optionName) > 0 && inputModes.count(m_options.input.mode) == 0)
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

	array<string, 9> const conflictingWithStopAfter{
		CompilerOutputs::componentName(&CompilerOutputs::binary),
		CompilerOutputs::componentName(&CompilerOutputs::ir),
		CompilerOutputs::componentName(&CompilerOutputs::irOptimized),
		CompilerOutputs::componentName(&CompilerOutputs::ewasm),
		CompilerOutputs::componentName(&CompilerOutputs::ewasmIR),
		g_strGas,
		CompilerOutputs::componentName(&CompilerOutputs::asm_),
		CompilerOutputs::componentName(&CompilerOutputs::asmJson),
		CompilerOutputs::componentName(&CompilerOutputs::opcodes),
	};

	for (auto& option: conflictingWithStopAfter)
		checkMutuallyExclusive({g_strStopAfter, option});

	if (
		m_options.input.mode != InputMode::Compiler &&
		m_options.input.mode != InputMode::CompilerWithASTImport &&
		m_options.input.mode != InputMode::Assembler
	)
	{
		if (!m_args[g_strOptimizeRuns].defaulted())
			solThrow(
				CommandLineValidationError,
				"Option --" + g_strOptimizeRuns + " is only valid in compiler and assembler modes."
			);

		for (string const& option: {g_strOptimize, g_strNoOptimizeYul, g_strOptimizeYul, g_strYulOptimizations})
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
		string revertStringsString = m_args[g_strRevertStrings].as<string>();
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
		string optionValue = m_args[g_strDebugInfo].as<string>();
		m_options.output.debugInfoSelection = DebugInfoSelection::fromString(optionValue);
		if (!m_options.output.debugInfoSelection.has_value())
			solThrow(CommandLineValidationError, "Invalid value for --" + g_strDebugInfo + " option: " + optionValue);

		if (m_options.output.debugInfoSelection->snippet && !m_options.output.debugInfoSelection->location)
			solThrow(CommandLineValidationError, "To use 'snippet' with --" + g_strDebugInfo + " you must select also 'location'.");
	}

	parseCombinedJsonOption();

	if (m_args.count(g_strOutputDir))
		m_options.output.dir = m_args.at(g_strOutputDir).as<string>();

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
		m_options.input.basePath = m_args[g_strBasePath].as<string>();

	if (m_args.count(g_strIncludePath) > 0)
	{
		if (m_options.input.basePath.empty())
			solThrow(CommandLineValidationError, "--" + g_strIncludePath + " option requires a non-empty base path.");

		for (string const& includePath: m_args[g_strIncludePath].as<vector<string>>())
		{
			if (includePath.empty())
				solThrow(CommandLineValidationError, "Empty values are not allowed in --" + g_strIncludePath + ".");

			m_options.input.includePaths.push_back(includePath);
		}
	}

	if (m_args.count(g_strAllowPaths))
	{
		vector<string> paths;
		for (string const& allowedPath: boost::split(paths, m_args[g_strAllowPaths].as<string>(), boost::is_any_of(",")))
			if (!allowedPath.empty())
				m_options.input.allowedDirectories.insert(allowedPath);
	}

	if (m_args.count(g_strStopAfter))
	{
		if (m_args[g_strStopAfter].as<string>() != "parsing")
			solThrow(CommandLineValidationError, "Valid options for --" + g_strStopAfter + " are: \"parsing\".\n");
		else
			m_options.output.stopAfter = CompilerStack::State::Parsed;
	}

	parseInputPathsAndRemappings();

	if (m_options.input.mode == InputMode::StandardJson)
		return;

	if (m_args.count(g_strLibraries))
		for (string const& library: m_args[g_strLibraries].as<vector<string>>())
			parseLibraryOption(library);

	if (m_options.input.mode == InputMode::Linker)
		return;

	if (m_args.count(g_strEVMVersion))
	{
		string versionOptionStr = m_args[g_strEVMVersion].as<string>();
		std::optional<langutil::EVMVersion> versionOption = langutil::EVMVersion::fromString(versionOptionStr);
		if (!versionOption)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strEVMVersion + ": " + versionOptionStr);
		m_options.output.evmVersion = *versionOption;
	}

	m_options.optimizer.enabled = (m_args.count(g_strOptimize) > 0);
	m_options.optimizer.noOptimizeYul = (m_args.count(g_strNoOptimizeYul) > 0);
	if (!m_args[g_strOptimizeRuns].defaulted())
		m_options.optimizer.expectedExecutionsPerDeployment = m_args.at(g_strOptimizeRuns).as<unsigned>();

	if (m_args.count(g_strYulOptimizations))
	{
		OptimiserSettings optimiserSettings = m_options.optimiserSettings();
		if (!optimiserSettings.runYulOptimiser)
			solThrow(CommandLineValidationError, "--" + g_strYulOptimizations + " is invalid if Yul optimizer is disabled");

		try
		{
			yul::OptimiserSuite::validateSequence(m_args[g_strYulOptimizations].as<string>());
		}
		catch (yul::OptimizerException const& _exception)
		{
			solThrow(
				CommandLineValidationError,
				"Invalid optimizer step sequence in --" + g_strYulOptimizations + ": " + _exception.what()
			);
		}

		m_options.optimizer.yulSteps = m_args[g_strYulOptimizations].as<string>();
	}

	if (m_options.input.mode == InputMode::Assembler)
	{
		vector<string> const nonAssemblyModeOptions = {
			// TODO: The list is not complete. Add more.
			g_strOutputDir,
			g_strGas,
			g_strCombinedJson,
			g_strOptimizeYul,
			g_strNoOptimizeYul,
		};
		if (countEnabledOptions(nonAssemblyModeOptions) >= 1)
		{
			auto optionEnabled = [&](string const& name){ return m_args.count(name) > 0; };
			auto enabledOptions = nonAssemblyModeOptions | ranges::views::filter(optionEnabled) | ranges::to_vector;

			string message = "The following options are invalid in assembly mode: " + joinOptionNames(enabledOptions) + ".";
			if (m_args.count(g_strOptimizeYul) || m_args.count(g_strNoOptimizeYul))
				message += " Optimization is disabled by default and can be enabled with --" + g_strOptimize + ".";

			solThrow(CommandLineValidationError, message);
		}

		// switch to assembly mode
		using Input = yul::AssemblyStack::Language;
		using Machine = yul::AssemblyStack::Machine;
		m_options.assembly.inputLanguage = m_args.count(g_strYul) ? Input::Yul : (m_args.count(g_strStrictAssembly) ? Input::StrictAssembly : Input::Assembly);

		if (m_args.count(g_strMachine))
		{
			string machine = m_args[g_strMachine].as<string>();
			if (machine == g_strEVM)
				m_options.assembly.targetMachine = Machine::EVM;
			else if (machine == g_strEwasm)
				m_options.assembly.targetMachine = Machine::Ewasm;
			else
				solThrow(CommandLineValidationError, "Invalid option for --" + g_strMachine + ": " + machine);
		}
		if (m_options.assembly.targetMachine == Machine::Ewasm && m_options.assembly.inputLanguage == Input::StrictAssembly)
			m_options.assembly.inputLanguage = Input::Ewasm;
		if (m_args.count(g_strYulDialect))
		{
			string dialect = m_args[g_strYulDialect].as<string>();
			if (dialect == g_strEVM)
				m_options.assembly.inputLanguage = Input::StrictAssembly;
			else if (dialect == g_strEwasm)
			{
				m_options.assembly.inputLanguage = Input::Ewasm;
				if (m_options.assembly.targetMachine != Machine::Ewasm)
					solThrow(
						CommandLineValidationError,
						"If you select Ewasm as --" + g_strYulDialect + ", "
						"--" + g_strMachine + " has to be Ewasm as well."
					);
			}
			else
				solThrow(CommandLineValidationError, "Invalid option for --" + g_strYulDialect + ": " + dialect);
		}
		if (m_options.optimizer.enabled && (m_options.assembly.inputLanguage != Input::StrictAssembly && m_options.assembly.inputLanguage != Input::Ewasm))
			solThrow(
				CommandLineValidationError,
				"Optimizer can only be used for strict assembly. Use --"  + g_strStrictAssembly + "."
			);
		if (m_options.assembly.targetMachine == Machine::Ewasm && m_options.assembly.inputLanguage != Input::StrictAssembly && m_options.assembly.inputLanguage != Input::Ewasm)
			solThrow(
				CommandLineValidationError,
				"The selected input language is not directly supported when targeting the Ewasm machine "
				"and automatic translation is not available."
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
		string hashStr = m_args[g_strMetadataHash].as<string>();
		if (hashStr == g_strIPFS)
			m_options.metadata.hash = CompilerStack::MetadataHash::IPFS;
		else if (hashStr == g_strSwarm)
			m_options.metadata.hash = CompilerStack::MetadataHash::Bzzr1;
		else if (hashStr == g_strNone)
			m_options.metadata.hash = CompilerStack::MetadataHash::None;
		else
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strMetadataHash + ": " + hashStr);
	}

	if (m_args.count(g_strModelCheckerContracts))
	{
		string contractsStr = m_args[g_strModelCheckerContracts].as<string>();
		optional<ModelCheckerContracts> contracts = ModelCheckerContracts::fromString(contractsStr);
		if (!contracts)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerContracts + ": " + contractsStr);
		m_options.modelChecker.settings.contracts = move(*contracts);
	}

	if (m_args.count(g_strModelCheckerDivModNoSlacks))
		m_options.modelChecker.settings.divModNoSlacks = true;

	if (m_args.count(g_strModelCheckerEngine))
	{
		string engineStr = m_args[g_strModelCheckerEngine].as<string>();
		optional<ModelCheckerEngine> engine = ModelCheckerEngine::fromString(engineStr);
		if (!engine)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerEngine + ": " + engineStr);
		m_options.modelChecker.settings.engine = *engine;
	}

	if (m_args.count(g_strModelCheckerInvariants))
	{
		string invsStr = m_args[g_strModelCheckerInvariants].as<string>();
		optional<ModelCheckerInvariants> invs = ModelCheckerInvariants::fromString(invsStr);
		if (!invs)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerInvariants + ": " + invsStr);
		m_options.modelChecker.settings.invariants = *invs;
	}

	if (m_args.count(g_strModelCheckerShowUnproved))
		m_options.modelChecker.settings.showUnproved = true;

	if (m_args.count(g_strModelCheckerSolvers))
	{
		string solversStr = m_args[g_strModelCheckerSolvers].as<string>();
		optional<smtutil::SMTSolverChoice> solvers = smtutil::SMTSolverChoice::fromString(solversStr);
		if (!solvers)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerSolvers + ": " + solversStr);
		m_options.modelChecker.settings.solvers = *solvers;
	}

	if (m_args.count(g_strModelCheckerTargets))
	{
		string targetsStr = m_args[g_strModelCheckerTargets].as<string>();
		optional<ModelCheckerTargets> targets = ModelCheckerTargets::fromString(targetsStr);
		if (!targets)
			solThrow(CommandLineValidationError, "Invalid option for --" + g_strModelCheckerTargets + ": " + targetsStr);
		m_options.modelChecker.settings.targets = *targets;
	}

	if (m_args.count(g_strModelCheckerTimeout))
		m_options.modelChecker.settings.timeout = m_args[g_strModelCheckerTimeout].as<unsigned>();

	m_options.metadata.literalSources = (m_args.count(g_strMetadataLiteral) > 0);
	m_options.modelChecker.initialize =
		m_args.count(g_strModelCheckerContracts) ||
		m_args.count(g_strModelCheckerDivModNoSlacks) ||
		m_args.count(g_strModelCheckerEngine) ||
		m_args.count(g_strModelCheckerInvariants) ||
		m_args.count(g_strModelCheckerShowUnproved) ||
		m_args.count(g_strModelCheckerSolvers) ||
		m_args.count(g_strModelCheckerTargets) ||
		m_args.count(g_strModelCheckerTimeout);
	m_options.output.viaIR = (m_args.count(g_strExperimentalViaIR) > 0 || m_args.count(g_strViaIR) > 0);
	if (m_options.input.mode == InputMode::Compiler)
		m_options.input.errorRecovery = (m_args.count(g_strErrorRecovery) > 0);

	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport);
}

void CommandLineParser::parseCombinedJsonOption()
{
	if (!m_args.count(g_strCombinedJson))
		return;

	set<string> requests;
	for (string const& item: boost::split(requests, m_args[g_strCombinedJson].as<string>(), boost::is_any_of(",")))
		if (CombinedJsonRequests::componentMap().count(item) == 0)
			solThrow(CommandLineValidationError, "Invalid option to --" + g_strCombinedJson + ": " + item);

	m_options.compiler.combinedJsonRequests = CombinedJsonRequests{};
	for (auto&& [componentName, component]: CombinedJsonRequests::componentMap())
		m_options.compiler.combinedJsonRequests.value().*component = (requests.count(componentName) > 0);
}

size_t CommandLineParser::countEnabledOptions(vector<string> const& _optionNames) const
{
	size_t count = 0;
	for (string const& _option: _optionNames)
		count += m_args.count(_option);

	return count;
}

string CommandLineParser::joinOptionNames(vector<string> const& _optionNames, string _separator)
{
	return util::joinHumanReadable(
		_optionNames | ranges::views::transform([](string const& _option){ return "--" + _option; }),
		_separator
	);
}

} // namespace solidity::frontend
