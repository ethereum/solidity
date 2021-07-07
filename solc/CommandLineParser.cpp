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

#include "license.h"

#include <solc/CommandLineParser.h>
#include <libyul/optimiser/Suite.h>
#include <liblangutil/EVMVersion.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity::langutil;

namespace po = boost::program_options;

namespace solidity::frontend
{

ostream& CommandLineParser::sout()
{
	m_hasOutput = true;
	return m_sout;
}

ostream& CommandLineParser::serr()
{
	m_hasOutput = true;
	return m_serr;
}

#define cout
#define cerr

static string const g_strAbi = "abi";
static string const g_strAllowPaths = "allow-paths";
static string const g_strBasePath = "base-path";
static string const g_strAsm = "asm";
static string const g_strAsmJson = "asm-json";
static string const g_strAssemble = "assemble";
static string const g_strAst = "ast";
static string const g_strAstCompactJson = "ast-compact-json";
static string const g_strBinary = "bin";
static string const g_strBinaryRuntime = "bin-runtime";
static string const g_strCombinedJson = "combined-json";
static string const g_strCompactJSON = "compact-format";
static string const g_strErrorRecovery = "error-recovery";
static string const g_strEVM = "evm";
static string const g_strEVMVersion = "evm-version";
static string const g_strEwasm = "ewasm";
static string const g_strExperimentalViaIR = "experimental-via-ir";
static string const g_strGeneratedSources = "generated-sources";
static string const g_strGeneratedSourcesRuntime = "generated-sources-runtime";
static string const g_strGas = "gas";
static string const g_strHelp = "help";
static string const g_strImportAst = "import-ast";
static string const g_strInputFile = "input-file";
static string const g_strInterface = "interface";
static string const g_strYul = "yul";
static string const g_strYulDialect = "yul-dialect";
static string const g_strIR = "ir";
static string const g_strIROptimized = "ir-optimized";
static string const g_strIPFS = "ipfs";
static string const g_strLicense = "license";
static string const g_strLibraries = "libraries";
static string const g_strLink = "link";
static string const g_strMachine = "machine";
static string const g_strMetadata = "metadata";
static string const g_strMetadataHash = "metadata-hash";
static string const g_strMetadataLiteral = "metadata-literal";
static string const g_strModelCheckerContracts = "model-checker-contracts";
static string const g_strModelCheckerEngine = "model-checker-engine";
static string const g_strModelCheckerTargets = "model-checker-targets";
static string const g_strModelCheckerTimeout = "model-checker-timeout";
static string const g_strNatspecDev = "devdoc";
static string const g_strNatspecUser = "userdoc";
static string const g_strNone = "none";
static string const g_strNoOptimizeYul = "no-optimize-yul";
static string const g_strOpcodes = "opcodes";
static string const g_strOptimize = "optimize";
static string const g_strOptimizeRuns = "optimize-runs";
static string const g_strOptimizeYul = "optimize-yul";
static string const g_strYulOptimizations = "yul-optimizations";
static string const g_strOutputDir = "output-dir";
static string const g_strOverwrite = "overwrite";
static string const g_strRevertStrings = "revert-strings";
static string const g_strStorageLayout = "storage-layout";
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

static string const g_strSignatureHashes = "hashes";
static string const g_strSources = "sources";
static string const g_strSourceList = "sourceList";
static string const g_strSrcMap = "srcmap";
static string const g_strSrcMapRuntime = "srcmap-runtime";
static string const g_strFunDebug = "function-debug";
static string const g_strFunDebugRuntime = "function-debug-runtime";
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

/// Possible arguments to for --combined-json
static set<string> const g_combinedJsonArgs
{
	g_strAbi,
	g_strAsm,
	g_strAst,
	g_strBinary,
	g_strBinaryRuntime,
	g_strCompactJSON,
	g_strFunDebug,
	g_strFunDebugRuntime,
	g_strGeneratedSources,
	g_strGeneratedSourcesRuntime,
	g_strInterface,
	g_strMetadata,
	g_strNatspecUser,
	g_strNatspecDev,
	g_strOpcodes,
	g_strSignatureHashes,
	g_strSrcMap,
	g_strSrcMapRuntime,
	g_strStorageLayout
};

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

void CommandLineParser::printVersionAndExit()
{
	sout() <<
		"solc, the solidity compiler commandline interface" <<
		endl <<
		"Version: " <<
		solidity::frontend::VersionString <<
		endl;
	exit(EXIT_SUCCESS);
}

void CommandLineParser::printLicenseAndExit()
{
	sout() << otherLicenses << endl;
	// This is a static variable generated by cmake from LICENSE.txt
	sout() << licenseText << endl;
	exit(EXIT_SUCCESS);
}


bool CommandLineParser::checkMutuallyExclusive(boost::program_options::variables_map const& args, string const& _optionA, string const& _optionB)
{
	if (args.count(_optionA) && args.count(_optionB))
	{
		serr() << "Option " << _optionA << " and " << _optionB << " are mutually exclusive." << endl;
		return false;
	}

	return true;
}

bool CompilerOutputs::operator==(CompilerOutputs const& _other) const noexcept
{
	static_assert(
		sizeof(*this) == 15 * sizeof(bool),
		"Remember to update code below if you add/remove fields."
	);

	return
		astCompactJson == _other.astCompactJson &&
		asm_ == _other.asm_ &&
		asmJson == _other.asmJson &&
		opcodes == _other.opcodes &&
		binary == _other.binary &&
		binaryRuntime == _other.binaryRuntime &&
		abi == _other.abi &&
		ir == _other.ir &&
		irOptimized == _other.irOptimized &&
		ewasm == _other.ewasm &&
		signatureHashes == _other.signatureHashes &&
		natspecUser == _other.natspecUser &&
		natspecDev == _other.natspecDev &&
		metadata == _other.metadata &&
		storageLayout == _other.storageLayout;
}

bool CombinedJsonRequests::operator==(CombinedJsonRequests const& _other) const noexcept
{
	static_assert(
		sizeof(*this) == 17 * sizeof(bool),
		"Remember to update code below if you add/remove fields."
	);

	return
		abi == _other.abi &&
		metadata == _other.metadata &&
		binary == _other.binary &&
		binaryRuntime == _other.binaryRuntime &&
		opcodes == _other.opcodes &&
		asm_ == _other.asm_ &&
		storageLayout == _other.storageLayout &&
		generatedSources == _other.generatedSources &&
		generatedSourcesRuntime == _other.generatedSourcesRuntime &&
		srcMap == _other.srcMap &&
		srcMapRuntime == _other.srcMapRuntime &&
		funDebug == _other.funDebug &&
		funDebugRuntime == _other.funDebugRuntime &&
		signatureHashes == _other.signatureHashes &&
		natspecDev == _other.natspecDev &&
		natspecUser == _other.natspecUser &&
		ast == _other.ast;
}

bool CommandLineOptions::operator==(CommandLineOptions const& _other) const noexcept
{
	return
		input.paths == _other.input.paths &&
		input.standardJsonFile == _other.input.standardJsonFile &&
		input.remappings == _other.input.remappings &&
		input.addStdin == _other.input.addStdin &&
		input.basePath == _other.input.basePath &&
		input.allowedDirectories == _other.input.allowedDirectories &&
		input.ignoreMissingFiles == _other.input.ignoreMissingFiles &&
		input.errorRecovery == _other.input.errorRecovery &&
		output.dir == _other.output.dir &&
		output.overwriteFiles == _other.output.overwriteFiles &&
		output.evmVersion == _other.output.evmVersion &&
		output.experimentalViaIR == _other.output.experimentalViaIR &&
		output.revertStrings == _other.output.revertStrings &&
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

bool CommandLineParser::parseInputPathsAndRemappings()
{
	m_options.input.ignoreMissingFiles = (m_args.count(g_strIgnoreMissingFiles) > 0);
	if (m_args.count(g_strInputFile))
		for (string path: m_args[g_strInputFile].as<vector<string>>())
		{
			auto eq = find(path.begin(), path.end(), '=');
			if (eq != path.end())
			{
				if (auto r = ImportRemapper::parseRemapping(path))
					m_options.input.remappings.emplace_back(std::move(*r));
				else
				{
					serr() << "Invalid remapping: \"" << path << "\"." << endl;
					return false;
				}

				string remappingTarget(eq + 1, path.end());
				m_options.input.allowedDirectories.insert(boost::filesystem::path(remappingTarget).remove_filename());
			}
			else if (path == "-")
				m_options.input.addStdin = true;
			else
				m_options.input.paths.insert(path);
		}

	return true;
}

bool CommandLineParser::parseLibraryOption(string const& _input)
{
	namespace fs = boost::filesystem;
	string data = _input;
	try
	{
		if (fs::is_regular_file(_input))
			data = readFileAsString(_input);
	}
	catch (fs::filesystem_error const&)
	{
		// Thrown e.g. if path is too long.
	}
	catch (FileNotFound const&)
	{
		// Should not happen if `fs::is_regular_file` is correct.
	}
	catch (NotAFile const&)
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
				{
					serr() << "Equal sign separator missing in library address specifier \"" << lib << "\"" << endl;
					return false;
				}
				else
					isSeparatorEqualSign = false; // separator is colon
			}
			else
				if (lib.rfind('=') != lib.find('='))
				{
					serr() << "Only one equal sign \"=\" is allowed in the address string \"" << lib << "\"." << endl;
					return false;
				}

			string libName(lib.begin(), lib.begin() + static_cast<ptrdiff_t>(separator));
			boost::trim(libName);
			if (m_options.linker.libraries.count(libName))
			{
				serr() << "Address specified more than once for library \"" << libName << "\"." << endl;
				return false;
			}

			string addrString(lib.begin() + static_cast<ptrdiff_t>(separator) + 1, lib.end());
			boost::trim(addrString);
			if (addrString.empty())
			{
				serr() << "Empty address provided for library \"" << libName << "\"." << endl;
				serr() << "Note that there should not be any whitespace after the " << (isSeparatorEqualSign ? "equal sign" : "colon") << "." << endl;
				return false;
			}

			if (addrString.substr(0, 2) == "0x")
				addrString = addrString.substr(2);
			else
			{
				serr() << "The address " << addrString << " is not prefixed with \"0x\"." << endl;
				serr() << "Note that the address must be prefixed with \"0x\"." << endl;
				return false;
			}

			if (addrString.length() != 40)
			{
				serr() << "Invalid length for address for library \"" << libName << "\": " << addrString.length() << " instead of 40 characters." << endl;
				return false;
			}
			if (!passesAddressChecksum(addrString, false))
			{
				serr() << "Invalid checksum on address for library \"" << libName << "\": " << addrString << endl;
				serr() << "The correct checksum is " << getChecksummedAddress(addrString) << endl;
				return false;
			}
			bytes binAddr = fromHex(addrString);
			h160 address(binAddr, h160::AlignRight);
			if (binAddr.size() > 20 || address == h160())
			{
				serr() << "Invalid address for library \"" << libName << "\": " << addrString << endl;
				return false;
			}
			m_options.linker.libraries[libName] = address;
		}

	return true;
}

bool CommandLineParser::parse(int _argc, char const* const* _argv, bool interactiveTerminal)
{
	m_hasOutput = false;

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
solc --)" + g_strBinary + R"( -o /tmp/solcoutput dapp-bin=/usr/local/lib/dapp-bin contract.sol

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
			"byzantium, constantinople, petersburg, istanbul or berlin."
		)
		(
			g_strExperimentalViaIR.c_str(),
			"Turn on experimental compilation mode via the IR (EXPERIMENTAL)."
		)
		(
			g_strRevertStrings.c_str(),
			po::value<string>()->value_name(boost::join(g_revertStringsArgs, ",")),
			"Strip revert (and require) reason strings or add additional debugging information."
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
			"--" + g_strCombinedJson + " " + g_strAst + "," + g_strCompactJSON).c_str()
		)
	;
	desc.add(alternativeInputModes);

	po::options_description assemblyModeOptions("Assembly Mode Options");
	assemblyModeOptions.add_options()
		(
			g_strMachine.c_str(),
			po::value<string>()->value_name(boost::join(g_machineArgs, ",")),
			"Target machine in assembly or Yul mode."
		)
		(
			g_strYulDialect.c_str(),
			po::value<string>()->value_name(boost::join(g_yulDialectArgs, ",")),
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
		(g_strAstCompactJson.c_str(), "AST of all source files in a compact JSON format.")
		(g_strAsm.c_str(), "EVM assembly of the contracts.")
		(g_strAsmJson.c_str(), "EVM assembly of the contracts in JSON format.")
		(g_strOpcodes.c_str(), "Opcodes of the contracts.")
		(g_strBinary.c_str(), "Binary of the contracts in hex.")
		(g_strBinaryRuntime.c_str(), "Binary of the runtime part of the contracts in hex.")
		(g_strAbi.c_str(), "ABI specification of the contracts.")
		(g_strIR.c_str(), "Intermediate Representation (IR) of all contracts (EXPERIMENTAL).")
		(g_strIROptimized.c_str(), "Optimized intermediate Representation (IR) of all contracts (EXPERIMENTAL).")
		(g_strEwasm.c_str(), "Ewasm text representation of all contracts (EXPERIMENTAL).")
		(g_strSignatureHashes.c_str(), "Function signature hashes of the contracts.")
		(g_strNatspecUser.c_str(), "Natspec user documentation of all contracts.")
		(g_strNatspecDev.c_str(), "Natspec developer documentation of all contracts.")
		(g_strMetadata.c_str(), "Combined Metadata JSON whose Swarm hash is stored on-chain.")
		(g_strStorageLayout.c_str(), "Slots, offsets and types of the contract's state variables.")
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
			po::value<string>()->value_name(boost::join(g_combinedJsonArgs, ",")),
			"Output a single json document containing the specified information."
		)
	;
	desc.add(extraOutput);

	po::options_description metadataOptions("Metadata Options");
	metadataOptions.add_options()
		(
			g_strMetadataHash.c_str(),
			po::value<string>()->value_name(boost::join(g_metadataHashArgs, ",")),
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
			"Set for how many contract runs to optimize. "
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
			g_strModelCheckerEngine.c_str(),
			po::value<string>()->value_name("all,bmc,chc,none")->default_value("none"),
			"Select model checker engine."
		)
		(
			g_strModelCheckerTargets.c_str(),
			po::value<string>()->value_name("default,constantCondition,underflow,overflow,divByZero,balance,assert,popEmptyArray,outOfBounds")->default_value("default"),
			"Select model checker verification targets. "
			"Multiple targets can be selected at the same time, separated by a comma "
			"and no spaces."
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

	po::options_description allOptions = desc;
	allOptions.add_options()(g_strInputFile.c_str(), po::value<vector<string>>(), "input file");

	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add(g_strInputFile.c_str(), -1);

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
		serr() << _exception.what() << endl;
		return false;
	}

	if (!checkMutuallyExclusive(m_args, g_strColor, g_strNoColor))
		return false;

	array<string, 8> const conflictingWithStopAfter{
		g_strBinary,
		g_strIR,
		g_strIROptimized,
		g_strEwasm,
		g_strGas,
		g_strAsm,
		g_strAsmJson,
		g_strOpcodes
	};

	for (auto& option: conflictingWithStopAfter)
		if (!checkMutuallyExclusive(m_args, g_strStopAfter, option))
			return false;

	if (m_args.count(g_strColor) > 0)
		m_options.formatting.coloredOutput = true;
	else if (m_args.count(g_strNoColor) > 0)
		m_options.formatting.coloredOutput = false;

	m_options.formatting.withErrorIds = m_args.count(g_strErrorIds);

	if (m_args.count(g_strHelp) || (interactiveTerminal && _argc == 1))
	{
		sout() << desc;
		return false;
	}

	if (m_args.count(g_strVersion))
		printVersionAndExit();

	if (m_args.count(g_strLicense))
		printLicenseAndExit();

	if (m_args.count(g_strRevertStrings))
	{
		string revertStringsString = m_args[g_strRevertStrings].as<string>();
		std::optional<RevertStrings> revertStrings = revertStringsFromString(revertStringsString);
		if (!revertStrings)
		{
			serr() << "Invalid option for --" << g_strRevertStrings << ": " << revertStringsString << endl;
			return false;
		}
		if (*revertStrings == RevertStrings::VerboseDebug)
		{
			serr() << "Only \"default\", \"strip\" and \"debug\" are implemented for --" << g_strRevertStrings << " for now." << endl;
			return false;
		}
		m_options.output.revertStrings = *revertStrings;
	}

	if (!parseCombinedJsonOption())
		return false;

	if (m_args.count(g_strOutputDir))
		m_options.output.dir = m_args.at(g_strOutputDir).as<string>();

	m_options.output.overwriteFiles = (m_args.count(g_strOverwrite) > 0);

	if (m_args.count(g_strPrettyJson) > 0)
	{
		m_options.formatting.json.format = JsonFormat::Pretty;
	}
	if (!m_args[g_strJsonIndent].defaulted())
	{
		m_options.formatting.json.format = JsonFormat::Pretty;
		m_options.formatting.json.indent = m_args[g_strJsonIndent].as<uint32_t>();
	}

	static_assert(
		sizeof(m_options.compiler.outputs) == 15 * sizeof(bool),
		"Remember to update code below if you add/remove fields."
	);
	m_options.compiler.outputs.astCompactJson = (m_args.count(g_strAstCompactJson) > 0);
	m_options.compiler.outputs.asm_ = (m_args.count(g_strAsm) > 0);
	m_options.compiler.outputs.asmJson = (m_args.count(g_strAsmJson) > 0);
	m_options.compiler.outputs.opcodes = (m_args.count(g_strOpcodes) > 0);
	m_options.compiler.outputs.binary = (m_args.count(g_strBinary) > 0);
	m_options.compiler.outputs.binaryRuntime = (m_args.count(g_strBinaryRuntime) > 0);
	m_options.compiler.outputs.abi = (m_args.count(g_strAbi) > 0);
	m_options.compiler.outputs.ir = (m_args.count(g_strIR) > 0);
	m_options.compiler.outputs.irOptimized = (m_args.count(g_strIROptimized) > 0);
	m_options.compiler.outputs.ewasm = (m_args.count(g_strEwasm) > 0);
	m_options.compiler.outputs.signatureHashes = (m_args.count(g_strSignatureHashes) > 0);
	m_options.compiler.outputs.natspecUser = (m_args.count(g_strNatspecUser) > 0);
	m_options.compiler.outputs.natspecDev = (m_args.count(g_strNatspecDev) > 0);
	m_options.compiler.outputs.metadata = (m_args.count(g_strMetadata) > 0);
	m_options.compiler.outputs.storageLayout = (m_args.count(g_strStorageLayout) > 0);

	m_options.compiler.estimateGas = (m_args.count(g_strGas) > 0);

	po::notify(m_args);

	if (m_args.count(g_strBasePath))
		m_options.input.basePath = m_args[g_strBasePath].as<string>();

	if (m_args.count(g_strAllowPaths))
	{
		vector<string> paths;
		for (string const& path: boost::split(paths, m_args[g_strAllowPaths].as<string>(), boost::is_any_of(",")))
		{
			auto filesystem_path = boost::filesystem::path(path);
			// If the given path had a trailing slash, the Boost filesystem
			// path will have it's last component set to '.'. This breaks
			// path comparison in later parts of the code, so we need to strip
			// it.
			if (filesystem_path.filename() == ".")
				filesystem_path.remove_filename();
			m_options.input.allowedDirectories.insert(filesystem_path);
		}
	}

	if (m_args.count(g_strStopAfter))
	{
		if (m_args[g_strStopAfter].as<string>() != "parsing")
		{
			serr() << "Valid options for --" << g_strStopAfter << " are: \"parsing\".\n";
			return false;
		}
		else
			m_options.output.stopAfter = CompilerStack::State::Parsed;
	}

	vector<string> const exclusiveModes = {
		g_strStandardJSON,
		g_strLink,
		g_strAssemble,
		g_strStrictAssembly,
		g_strYul,
		g_strImportAst,
	};
	if (countEnabledOptions(exclusiveModes) > 1)
	{
		serr() << "The following options are mutually exclusive: " << joinOptionNames(exclusiveModes) << ". ";
		serr() << "Select at most one." << endl;
		return false;
	}

	if (m_args.count(g_strStandardJSON))
	{
		m_options.input.mode = InputMode::StandardJson;

		vector<string> inputFiles;
		if (m_args.count(g_strInputFile))
			inputFiles = m_args[g_strInputFile].as<vector<string>>();
		if (inputFiles.size() == 1)
			m_options.input.standardJsonFile = inputFiles[0];
		else if (inputFiles.size() > 1)
		{
			serr() << "If --" << g_strStandardJSON << " is used, only zero or one input files are supported." << endl;
			return false;
		}

		return true;
	}

	if (!parseInputPathsAndRemappings())
		return false;

	if (m_args.count(g_strLibraries))
		for (string const& library: m_args[g_strLibraries].as<vector<string>>())
			if (!parseLibraryOption(library))
				return false;

	if (m_args.count(g_strEVMVersion))
	{
		string versionOptionStr = m_args[g_strEVMVersion].as<string>();
		std::optional<langutil::EVMVersion> versionOption = langutil::EVMVersion::fromString(versionOptionStr);
		if (!versionOption)
		{
			serr() << "Invalid option for --" << g_strEVMVersion << ": " << versionOptionStr << endl;
			return false;
		}
		m_options.output.evmVersion = *versionOption;
	}

	if (m_args.count(g_strAssemble) || m_args.count(g_strStrictAssembly) || m_args.count(g_strYul))
	{
		m_options.input.mode = InputMode::Assembler;

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
			auto enabledOptions = boost::copy_range<vector<string>>(nonAssemblyModeOptions | boost::adaptors::filtered(optionEnabled));

			serr() << "The following options are invalid in assembly mode: ";
			serr() << joinOptionNames(enabledOptions) << ".";
			if (m_args.count(g_strOptimizeYul) || m_args.count(g_strNoOptimizeYul))
				serr() << " Optimization is disabled by default and can be enabled with --" << g_strOptimize << "." << endl;
			serr() << endl;
			return false;
		}

		// switch to assembly mode
		using Input = yul::AssemblyStack::Language;
		using Machine = yul::AssemblyStack::Machine;
		m_options.assembly.inputLanguage = m_args.count(g_strYul) ? Input::Yul : (m_args.count(g_strStrictAssembly) ? Input::StrictAssembly : Input::Assembly);
		m_options.optimizer.enabled = (m_args.count(g_strOptimize) > 0);
		m_options.optimizer.noOptimizeYul = (m_args.count(g_strNoOptimizeYul) > 0);

		if (m_args.count(g_strYulOptimizations))
		{
			if (!m_options.optimizer.enabled)
			{
				serr() << "--" << g_strYulOptimizations << " is invalid if Yul optimizer is disabled" << endl;
				return false;
			}

			try
			{
				yul::OptimiserSuite::validateSequence(m_args[g_strYulOptimizations].as<string>());
			}
			catch (yul::OptimizerException const& _exception)
			{
				serr() << "Invalid optimizer step sequence in --" << g_strYulOptimizations << ": " << _exception.what() << endl;
				return false;
			}

			m_options.optimizer.yulSteps = m_args[g_strYulOptimizations].as<string>();
		}

		if (m_args.count(g_strMachine))
		{
			string machine = m_args[g_strMachine].as<string>();
			if (machine == g_strEVM)
				m_options.assembly.targetMachine = Machine::EVM;
			else if (machine == g_strEwasm)
				m_options.assembly.targetMachine = Machine::Ewasm;
			else
			{
				serr() << "Invalid option for --" << g_strMachine << ": " << machine << endl;
				return false;
			}
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
				{
					serr() << "If you select Ewasm as --" << g_strYulDialect << ", ";
					serr() << "--" << g_strMachine << " has to be Ewasm as well." << endl;
					return false;
				}
			}
			else
			{
				serr() << "Invalid option for --" << g_strYulDialect << ": " << dialect << endl;
				return false;
			}
		}
		if (m_options.optimizer.enabled && (m_options.assembly.inputLanguage != Input::StrictAssembly && m_options.assembly.inputLanguage != Input::Ewasm))
		{
			serr() <<
				"Optimizer can only be used for strict assembly. Use --" <<
				g_strStrictAssembly <<
				"." <<
				endl;
			return false;
		}
		if (m_options.assembly.targetMachine == Machine::Ewasm && m_options.assembly.inputLanguage != Input::StrictAssembly && m_options.assembly.inputLanguage != Input::Ewasm)
		{
			serr() << "The selected input language is not directly supported when targeting the Ewasm machine ";
			serr() << "and automatic translation is not available." << endl;
			return false;
		}
		serr() <<
			"Warning: Yul is still experimental. Please use the output with care." <<
			endl;

		return true;
	}
	else if (countEnabledOptions({g_strYulDialect, g_strMachine}) >= 1)
	{
		serr() << "--" << g_strYulDialect << " and --" << g_strMachine << " ";
		serr() << "are only valid in assembly mode." << endl;
		return false;
	}

	if (m_args.count(g_strLink))
	{
		m_options.input.mode = InputMode::Linker;
		return true;
	}

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
		{
			serr() << "Invalid option for --" << g_strMetadataHash << ": " << hashStr << endl;
			return false;
		}
	}

	if (m_args.count(g_strModelCheckerContracts))
	{
		string contractsStr = m_args[g_strModelCheckerContracts].as<string>();
		optional<ModelCheckerContracts> contracts = ModelCheckerContracts::fromString(contractsStr);
		if (!contracts)
		{
			serr() << "Invalid option for --" << g_strModelCheckerContracts << ": " << contractsStr << endl;
			return false;
		}
		m_options.modelChecker.settings.contracts = move(*contracts);
	}

	if (m_args.count(g_strModelCheckerEngine))
	{
		string engineStr = m_args[g_strModelCheckerEngine].as<string>();
		optional<ModelCheckerEngine> engine = ModelCheckerEngine::fromString(engineStr);
		if (!engine)
		{
			serr() << "Invalid option for --" << g_strModelCheckerEngine << ": " << engineStr << endl;
			return false;
		}
		m_options.modelChecker.settings.engine = *engine;
	}

	if (m_args.count(g_strModelCheckerTargets))
	{
		string targetsStr = m_args[g_strModelCheckerTargets].as<string>();
		optional<ModelCheckerTargets> targets = ModelCheckerTargets::fromString(targetsStr);
		if (!targets)
		{
			serr() << "Invalid option for --" << g_strModelCheckerTargets << ": " << targetsStr << endl;
			return false;
		}
		m_options.modelChecker.settings.targets = *targets;
	}

	if (m_args.count(g_strModelCheckerTimeout))
		m_options.modelChecker.settings.timeout = m_args[g_strModelCheckerTimeout].as<unsigned>();

	m_options.metadata.literalSources = (m_args.count(g_strMetadataLiteral) > 0);
	m_options.modelChecker.initialize =
		m_args.count(g_strModelCheckerContracts) ||
		m_args.count(g_strModelCheckerEngine) ||
		m_args.count(g_strModelCheckerTargets) ||
		m_args.count(g_strModelCheckerTimeout);
	m_options.output.experimentalViaIR = (m_args.count(g_strExperimentalViaIR) > 0);
	m_options.optimizer.expectedExecutionsPerDeployment = m_args[g_strOptimizeRuns].as<unsigned>();

	m_options.optimizer.enabled = (m_args.count(g_strOptimize) > 0);
	m_options.optimizer.noOptimizeYul = (m_args.count(g_strNoOptimizeYul) > 0);

	OptimiserSettings settings = m_options.optimizer.enabled ? OptimiserSettings::standard() : OptimiserSettings::minimal();
	if (m_options.optimizer.noOptimizeYul)
		settings.runYulOptimiser = false;
	if (m_args.count(g_strYulOptimizations))
	{
		if (!settings.runYulOptimiser)
		{
			serr() << "--" << g_strYulOptimizations << " is invalid if Yul optimizer is disabled" << endl;
			return false;
		}

		try
		{
			yul::OptimiserSuite::validateSequence(m_args[g_strYulOptimizations].as<string>());
		}
		catch (yul::OptimizerException const& _exception)
		{
			serr() << "Invalid optimizer step sequence in --" << g_strYulOptimizations << ": " << _exception.what() << endl;
			return false;
		}

		m_options.optimizer.yulSteps = m_args[g_strYulOptimizations].as<string>();
	}

	if (m_args.count(g_strImportAst) > 0)
		m_options.input.mode = InputMode::CompilerWithASTImport;
	else
		m_options.input.errorRecovery = (m_args.count(g_strErrorRecovery) > 0);

	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");
	return true;
}

bool CommandLineParser::parseCombinedJsonOption()
{
	if (!m_args.count(g_strCombinedJson))
		return true;

	set<string> requests;
	for (string const& item: boost::split(requests, m_args[g_strCombinedJson].as<string>(), boost::is_any_of(",")))
		if (!g_combinedJsonArgs.count(item))
		{
			serr() << "Invalid option to --" << g_strCombinedJson << ": " << item << endl;
			return false;
		}

	m_options.compiler.combinedJsonRequests = CombinedJsonRequests{};
	m_options.compiler.combinedJsonRequests->abi = (requests.count(g_strAbi) > 0);
	m_options.compiler.combinedJsonRequests->metadata = (requests.count("metadata") > 0);
	m_options.compiler.combinedJsonRequests->binary = (requests.count(g_strBinary) > 0);
	m_options.compiler.combinedJsonRequests->binaryRuntime = (requests.count(g_strBinaryRuntime) > 0);
	m_options.compiler.combinedJsonRequests->opcodes = (requests.count(g_strOpcodes) > 0);
	m_options.compiler.combinedJsonRequests->asm_ = (requests.count(g_strAsm) > 0);
	m_options.compiler.combinedJsonRequests->storageLayout = (requests.count(g_strStorageLayout) > 0);
	m_options.compiler.combinedJsonRequests->generatedSources = (requests.count(g_strGeneratedSources) > 0);
	m_options.compiler.combinedJsonRequests->generatedSourcesRuntime = (requests.count(g_strGeneratedSourcesRuntime) > 0);
	m_options.compiler.combinedJsonRequests->srcMap = (requests.count(g_strSrcMap) > 0);
	m_options.compiler.combinedJsonRequests->srcMapRuntime = (requests.count(g_strSrcMapRuntime) > 0);
	m_options.compiler.combinedJsonRequests->funDebug = (requests.count(g_strFunDebug) > 0);
	m_options.compiler.combinedJsonRequests->funDebugRuntime = (requests.count(g_strFunDebugRuntime) > 0);
	m_options.compiler.combinedJsonRequests->signatureHashes = (requests.count(g_strSignatureHashes) > 0);
	m_options.compiler.combinedJsonRequests->natspecDev = (requests.count(g_strNatspecDev) > 0);
	m_options.compiler.combinedJsonRequests->natspecUser = (requests.count(g_strNatspecUser) > 0);
	m_options.compiler.combinedJsonRequests->ast = (requests.count(g_strAst) > 0);

	return true;
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
	return joinHumanReadable(
		_optionNames | ranges::views::transform([](string const& _option){ return "--" + _option; }),
		_separator
	);
}

} // namespace solidity::frontend
