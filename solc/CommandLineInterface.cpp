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
 * @author Lefteris <lefteris@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#include <solc/CommandLineInterface.h>

#include "solidity/BuildInfo.h"
#include "license.h"

#include <libsolidity/interface/Version.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolidity/ast/ASTJsonImporter.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/interface/DebugSettings.h>

#include <libyul/AssemblyStack.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/GasMeter.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>
#include <liblangutil/SourceReferenceFormatterHuman.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/JSON.h>

#include <memory>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32 // windows
	#include <io.h>
	#define isatty _isatty
	#define fileno _fileno
#else // unix
	#include <unistd.h>
#endif

#include <string>
#include <iostream>
#include <fstream>

#if !defined(STDERR_FILENO)
	#define STDERR_FILENO 2
#endif

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

namespace po = boost::program_options;

namespace solidity::frontend
{

bool g_hasOutput = false;

std::ostream& sout()
{
	g_hasOutput = true;
	return cout;
}

std::ostream& serr(bool _used = true)
{
	if (_used)
		g_hasOutput = true;
	return cerr;
}

#define cout
#define cerr

static string const g_stdinFileNameStr = "<stdin>";
static string const g_strAbi = "abi";
static string const g_strAllowPaths = "allow-paths";
static string const g_strAsm = "asm";
static string const g_strAsmJson = "asm-json";
static string const g_strAssemble = "assemble";
static string const g_strAst = "ast";
static string const g_strAstJson = "ast-json";
static string const g_strAstCompactJson = "ast-compact-json";
static string const g_strBinary = "bin";
static string const g_strBinaryRuntime = "bin-runtime";
static string const g_strCombinedJson = "combined-json";
static string const g_strCompactJSON = "compact-format";
static string const g_strContracts = "contracts";
static string const g_strErrorRecovery = "error-recovery";
static string const g_strEVM = "evm";
static string const g_strEVM15 = "evm15";
static string const g_strEVMVersion = "evm-version";
static string const g_strEwasm = "ewasm";
static string const g_strGas = "gas";
static string const g_strHelp = "help";
static string const g_strImportAst = "import-ast";
static string const g_strInputFile = "input-file";
static string const g_strInterface = "interface";
static string const g_strYul = "yul";
static string const g_strYulDialect = "yul-dialect";
static string const g_strIR = "ir";
static string const g_strIPFS = "ipfs";
static string const g_strLicense = "license";
static string const g_strLibraries = "libraries";
static string const g_strLink = "link";
static string const g_strMachine = "machine";
static string const g_strMetadata = "metadata";
static string const g_strMetadataHash = "metadata-hash";
static string const g_strMetadataLiteral = "metadata-literal";
static string const g_strNatspecDev = "devdoc";
static string const g_strNatspecUser = "userdoc";
static string const g_strNone = "none";
static string const g_strNoOptimizeYul = "no-optimize-yul";
static string const g_strOpcodes = "opcodes";
static string const g_strOptimize = "optimize";
static string const g_strOptimizeRuns = "optimize-runs";
static string const g_strOptimizeYul = "optimize-yul";
static string const g_strOutputDir = "output-dir";
static string const g_strOverwrite = "overwrite";
static string const g_strRevertStrings = "revert-strings";

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
static string const g_strStandardJSON = "standard-json";
static string const g_strStrictAssembly = "strict-assembly";
static string const g_strSwarm = "swarm";
static string const g_strPrettyJson = "pretty-json";
static string const g_strVersion = "version";
static string const g_strIgnoreMissingFiles = "ignore-missing";
static string const g_strColor = "color";
static string const g_strNoColor = "no-color";
static string const g_strOldReporter = "old-reporter";

static string const g_argAbi = g_strAbi;
static string const g_argPrettyJson = g_strPrettyJson;
static string const g_argAllowPaths = g_strAllowPaths;
static string const g_argAsm = g_strAsm;
static string const g_argAsmJson = g_strAsmJson;
static string const g_argAssemble = g_strAssemble;
static string const g_argAstCompactJson = g_strAstCompactJson;
static string const g_argAstJson = g_strAstJson;
static string const g_argBinary = g_strBinary;
static string const g_argBinaryRuntime = g_strBinaryRuntime;
static string const g_argCombinedJson = g_strCombinedJson;
static string const g_argCompactJSON = g_strCompactJSON;
static string const g_argErrorRecovery = g_strErrorRecovery;
static string const g_argGas = g_strGas;
static string const g_argHelp = g_strHelp;
static string const g_argImportAst = g_strImportAst;
static string const g_argInputFile = g_strInputFile;
static string const g_argYul = g_strYul;
static string const g_argIR = g_strIR;
static string const g_argEwasm = g_strEwasm;
static string const g_argLibraries = g_strLibraries;
static string const g_argLink = g_strLink;
static string const g_argMachine = g_strMachine;
static string const g_argMetadata = g_strMetadata;
static string const g_argMetadataHash = g_strMetadataHash;
static string const g_argMetadataLiteral = g_strMetadataLiteral;
static string const g_argNatspecDev = g_strNatspecDev;
static string const g_argNatspecUser = g_strNatspecUser;
static string const g_argOpcodes = g_strOpcodes;
static string const g_argOptimize = g_strOptimize;
static string const g_argOptimizeRuns = g_strOptimizeRuns;
static string const g_argOutputDir = g_strOutputDir;
static string const g_argSignatureHashes = g_strSignatureHashes;
static string const g_argStandardJSON = g_strStandardJSON;
static string const g_argStrictAssembly = g_strStrictAssembly;
static string const g_argVersion = g_strVersion;
static string const g_stdinFileName = g_stdinFileNameStr;
static string const g_argIgnoreMissingFiles = g_strIgnoreMissingFiles;
static string const g_argColor = g_strColor;
static string const g_argNoColor = g_strNoColor;
static string const g_argOldReporter = g_strOldReporter;

/// Possible arguments to for --combined-json
static set<string> const g_combinedJsonArgs
{
	g_strAbi,
	g_strAsm,
	g_strAst,
	g_strBinary,
	g_strBinaryRuntime,
	g_strCompactJSON,
	g_strInterface,
	g_strMetadata,
	g_strNatspecUser,
	g_strNatspecDev,
	g_strOpcodes,
	g_strSignatureHashes,
	g_strSrcMap,
	g_strSrcMapRuntime
};

/// Possible arguments to for --machine
static set<string> const g_machineArgs
{
	g_strEVM,
	g_strEVM15,
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

static void version()
{
	sout() <<
		"solc, the solidity compiler commandline interface" <<
		endl <<
		"Version: " <<
		solidity::frontend::VersionString <<
		endl;
	exit(0);
}

static void license()
{
	sout() << otherLicenses << endl;
	// This is a static variable generated by cmake from LICENSE.txt
	sout() << licenseText << endl;
	exit(0);
}

static bool needsHumanTargetedStdout(po::variables_map const& _args)
{
	if (_args.count(g_argGas))
		return true;
	if (_args.count(g_argOutputDir))
		return false;
	for (string const& arg: {
		g_argAbi,
		g_argAsm,
		g_argAsmJson,
		g_argAstJson,
		g_argBinary,
		g_argBinaryRuntime,
		g_argMetadata,
		g_argNatspecUser,
		g_argNatspecDev,
		g_argOpcodes,
		g_argSignatureHashes
	})
		if (_args.count(arg))
			return true;
	return false;
}

void CommandLineInterface::handleBinary(string const& _contract)
{
	if (m_args.count(g_argBinary))
	{
		if (m_args.count(g_argOutputDir))
			createFile(m_compiler->filesystemFriendlyName(_contract) + ".bin", objectWithLinkRefsHex(m_compiler->object(_contract)));
		else
		{
			sout() << "Binary:" << endl;
			sout() << objectWithLinkRefsHex(m_compiler->object(_contract)) << endl;
		}
	}
	if (m_args.count(g_argBinaryRuntime))
	{
		if (m_args.count(g_argOutputDir))
			createFile(m_compiler->filesystemFriendlyName(_contract) + ".bin-runtime", objectWithLinkRefsHex(m_compiler->runtimeObject(_contract)));
		else
		{
			sout() << "Binary of the runtime part:" << endl;
			sout() << objectWithLinkRefsHex(m_compiler->runtimeObject(_contract)) << endl;
		}
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	if (m_args.count(g_argOutputDir))
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".opcode", evmasm::disassemble(m_compiler->object(_contract).bytecode));
	else
	{
		sout() << "Opcodes:" << endl;
		sout() << std::uppercase << evmasm::disassemble(m_compiler->object(_contract).bytecode);
		sout() << endl;
	}
}

void CommandLineInterface::handleIR(string const& _contractName)
{
	if (m_args.count(g_argIR))
	{
		if (m_args.count(g_argOutputDir))
			createFile(m_compiler->filesystemFriendlyName(_contractName) + ".yul", m_compiler->yulIR(_contractName));
		else
		{
			sout() << "IR:" << endl;
			sout() << m_compiler->yulIR(_contractName) << endl;
		}
	}
}

void CommandLineInterface::handleEwasm(string const& _contractName)
{
	if (m_args.count(g_argEwasm))
	{
		if (m_args.count(g_argOutputDir))
		{
			createFile(m_compiler->filesystemFriendlyName(_contractName) + ".wast", m_compiler->ewasm(_contractName));
			createFile(
				m_compiler->filesystemFriendlyName(_contractName) + ".wasm",
				asString(m_compiler->ewasmObject(_contractName).bytecode)
			);
		}
		else
		{
			sout() << "Ewasm text:" << endl;
			sout() << m_compiler->ewasm(_contractName) << endl;
			sout() << "Ewasm binary (hex): " << m_compiler->ewasmObject(_contractName).toHex() << endl;
		}
	}
}

void CommandLineInterface::handleBytecode(string const& _contract)
{
	if (m_args.count(g_argOpcodes))
		handleOpcode(_contract);
	if (m_args.count(g_argBinary) || m_args.count(g_argBinaryRuntime))
		handleBinary(_contract);
}

void CommandLineInterface::handleSignatureHashes(string const& _contract)
{
	if (!m_args.count(g_argSignatureHashes))
		return;

	Json::Value methodIdentifiers = m_compiler->methodIdentifiers(_contract);
	string out;
	for (auto const& name: methodIdentifiers.getMemberNames())
		out += methodIdentifiers[name].asString() + ": " + name + "\n";

	if (m_args.count(g_argOutputDir))
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".signatures", out);
	else
		sout() << "Function signatures:" << endl << out;
}

void CommandLineInterface::handleMetadata(string const& _contract)
{
	if (!m_args.count(g_argMetadata))
		return;

	string data = m_compiler->metadata(_contract);
	if (m_args.count(g_argOutputDir))
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_meta.json", data);
	else
		sout() << "Metadata:" << endl << data << endl;
}

void CommandLineInterface::handleABI(string const& _contract)
{
	if (!m_args.count(g_argAbi))
		return;

	string data = jsonCompactPrint(m_compiler->contractABI(_contract));
	if (m_args.count(g_argOutputDir))
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".abi", data);
	else
		sout() << "Contract JSON ABI" << endl << data << endl;
}

void CommandLineInterface::handleNatspec(bool _natspecDev, string const& _contract)
{
	std::string argName;
	std::string suffix;
	std::string title;

	if (_natspecDev)
	{
		argName = g_argNatspecDev;
		suffix = ".docdev";
		title = "Developer Documentation";
	}
	else
	{
		argName = g_argNatspecUser;
		suffix = ".docuser";
		title = "User Documentation";
	}

	if (m_args.count(argName))
	{
		std::string output = jsonPrettyPrint(
			_natspecDev ?
			m_compiler->natspecDev(_contract) :
			m_compiler->natspecUser(_contract)
		);

		if (m_args.count(g_argOutputDir))
			createFile(m_compiler->filesystemFriendlyName(_contract) + suffix, output);
		else
		{
			sout() << title << endl;
			sout() << output << endl;
		}

	}
}

void CommandLineInterface::handleGasEstimation(string const& _contract)
{
	Json::Value estimates = m_compiler->gasEstimates(_contract);
	sout() << "Gas estimation:" << endl;

	if (estimates["creation"].isObject())
	{
		Json::Value creation = estimates["creation"];
		sout() << "construction:" << endl;
		sout() << "   " << creation["executionCost"].asString();
		sout() << " + " << creation["codeDepositCost"].asString();
		sout() << " = " << creation["totalCost"].asString() << endl;
	}

	if (estimates["external"].isObject())
	{
		Json::Value externalFunctions = estimates["external"];
		sout() << "external:" << endl;
		for (auto const& name: externalFunctions.getMemberNames())
		{
			if (name.empty())
				sout() << "   fallback:\t";
			else
				sout() << "   " << name << ":\t";
			sout() << externalFunctions[name].asString() << endl;
		}
	}

	if (estimates["internal"].isObject())
	{
		Json::Value internalFunctions = estimates["internal"];
		sout() << "internal:" << endl;
		for (auto const& name: internalFunctions.getMemberNames())
		{
			sout() << "   " << name << ":\t";
			sout() << internalFunctions[name].asString() << endl;
		}
	}
}

bool CommandLineInterface::readInputFilesAndConfigureRemappings()
{
	bool ignoreMissing = m_args.count(g_argIgnoreMissingFiles);
	bool addStdin = false;
	if (m_args.count(g_argInputFile))
		for (string path: m_args[g_argInputFile].as<vector<string>>())
		{
			auto eq = find(path.begin(), path.end(), '=');
			if (eq != path.end())
			{
				if (auto r = CompilerStack::parseRemapping(path))
				{
					m_remappings.emplace_back(std::move(*r));
					path = string(eq + 1, path.end());
				}
				else
				{
					serr() << "Invalid remapping: \"" << path << "\"." << endl;
					return false;
				}
			}
			else if (path == "-")
				addStdin = true;
			else
			{
				auto infile = boost::filesystem::path(path);
				if (!boost::filesystem::exists(infile))
				{
					if (!ignoreMissing)
					{
						serr() << infile << " is not found." << endl;
						return false;
					}
					else
						serr() << infile << " is not found. Skipping." << endl;

					continue;
				}

				if (!boost::filesystem::is_regular_file(infile))
				{
					if (!ignoreMissing)
					{
						serr() << infile << " is not a valid file." << endl;
						return false;
					}
					else
						serr() << infile << " is not a valid file. Skipping." << endl;

					continue;
				}

				m_sourceCodes[infile.generic_string()] = readFileAsString(infile.string());
				path = boost::filesystem::canonical(infile).string();
			}
			m_allowedDirectories.push_back(boost::filesystem::path(path).remove_filename());
		}
	if (addStdin)
		m_sourceCodes[g_stdinFileName] = readStandardInput();
	if (m_sourceCodes.size() == 0)
	{
		serr() << "No input files given. If you wish to use the standard input please specify \"-\" explicitly." << endl;
		return false;
	}

	return true;
}

bool CommandLineInterface::parseLibraryOption(string const& _input)
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

	vector<string> libraries;
	boost::split(libraries, data, boost::is_space() || boost::is_any_of(","), boost::token_compress_on);
	for (string const& lib: libraries)
		if (!lib.empty())
		{
			//search for last colon in string as our binaries output placeholders in the form of file:Name
			//so we need to search for the second `:` in the string
			auto colon = lib.rfind(':');
			if (colon == string::npos)
			{
				serr() << "Colon separator missing in library address specifier \"" << lib << "\"" << endl;
				return false;
			}
			string libName(lib.begin(), lib.begin() + colon);
			string addrString(lib.begin() + colon + 1, lib.end());
			boost::trim(libName);
			boost::trim(addrString);
			if (addrString.substr(0, 2) == "0x")
				addrString = addrString.substr(2);
			if (addrString.empty())
			{
				serr() << "Empty address provided for library \"" << libName << "\":" << endl;
				serr() << "Note that there should not be any whitespace after the colon." << endl;
				return false;
			}
			else if (addrString.length() != 40)
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
			m_libraries[libName] = address;
		}

	return true;
}

map<string, Json::Value> CommandLineInterface::parseAstFromInput()
{
	map<string, Json::Value> sourceJsons;
	map<string, string> tmpSources;

	for (auto const& srcPair: m_sourceCodes)
	{
		Json::Value ast;
		astAssert(jsonParseStrict(srcPair.second, ast), "Input file could not be parsed to JSON");
		astAssert(ast.isMember("sources"), "Invalid Format for import-JSON: Must have 'sources'-object");

		for (auto& src: ast["sources"].getMemberNames())
		{
			std::string astKey = ast["sources"][src].isMember("ast") ? "ast" : "AST";

			astAssert(ast["sources"][src].isMember(astKey), "astkey is not member");
			astAssert(ast["sources"][src][astKey]["nodeType"].asString() == "SourceUnit",  "Top-level node should be a 'SourceUnit'");
			astAssert(sourceJsons.count(src) == 0, "All sources must have unique names");
			sourceJsons.emplace(src, move(ast["sources"][src][astKey]));
			tmpSources[src] = util::jsonCompactPrint(ast);
		}
	}

	m_sourceCodes = std::move(tmpSources);
	return sourceJsons;
}

void CommandLineInterface::createFile(string const& _fileName, string const& _data)
{
	namespace fs = boost::filesystem;
	// create directory if not existent
	fs::path p(m_args.at(g_argOutputDir).as<string>());
	// Do not try creating the directory if the first item is . or ..
	if (p.filename() != "." && p.filename() != "..")
		fs::create_directories(p);
	string pathName = (p / _fileName).string();
	if (fs::exists(pathName) && !m_args.count(g_strOverwrite))
	{
		serr() << "Refusing to overwrite existing file \"" << pathName << "\" (use --overwrite to force)." << endl;
		m_error = true;
		return;
	}
	ofstream outFile(pathName);
	outFile << _data;
	if (!outFile)
		BOOST_THROW_EXCEPTION(FileError() << errinfo_comment("Could not write to file: " + pathName));
}

void CommandLineInterface::createJson(string const& _fileName, string const& _json)
{
	createFile(boost::filesystem::basename(_fileName) + string(".json"), _json);
}

bool CommandLineInterface::parseArguments(int _argc, char** _argv)
{
	g_hasOutput = false;

	// Declare the supported options.
	po::options_description desc(R"(solc, the Solidity commandline compiler.

This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you
are welcome to redistribute it under certain conditions. See 'solc --license'
for details.

Usage: solc [options] [input_file...]
Compiles the given Solidity input files (or the standard input if none given or
"-" is used as a file name) and outputs the components specified in the options
at standard output or in files in the output directory, if specified.
Imports are automatically read from the filesystem, but it is also possible to
remap paths using the context:prefix=path syntax.
Example:
solc --bin -o /tmp/solcoutput dapp-bin=/usr/local/lib/dapp-bin contract.sol

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	);
	desc.add_options()
		(g_argHelp.c_str(), "Show help message and exit.")
		(g_argVersion.c_str(), "Show version and exit.")
		(g_strLicense.c_str(), "Show licensing information and exit.")
		(
			g_strEVMVersion.c_str(),
			po::value<string>()->value_name("version"),
			"Select desired EVM version. Either homestead, tangerineWhistle, spuriousDragon, "
			"byzantium, constantinople, petersburg, istanbul (default) or berlin."
		)
		(g_argOptimize.c_str(), "Enable bytecode optimizer.")
		(
			g_argOptimizeRuns.c_str(),
			po::value<unsigned>()->value_name("n")->default_value(200),
			"Set for how many contract runs to optimize."
			"Lower values will optimize more for initial deployment cost, higher values will optimize more for high-frequency usage."
		)
		(g_strOptimizeYul.c_str(), "Enable Yul optimizer in Solidity. Legacy option: the yul optimizer is enabled as part of the general --optimize option.")
		(g_strNoOptimizeYul.c_str(), "Disable Yul optimizer in Solidity.")
		(g_argPrettyJson.c_str(), "Output JSON in pretty format. Currently it only works with the combined JSON output.")
		(
			g_argLibraries.c_str(),
			po::value<vector<string>>()->value_name("libs"),
			"Direct string or file containing library addresses. Syntax: "
			"<libraryName>:<address> [, or whitespace] ...\n"
			"Address is interpreted as a hex string optionally prefixed by 0x."
		)
		(
			g_strRevertStrings.c_str(),
			po::value<string>()->value_name(boost::join(g_revertStringsArgs, ",")),
			"Strip revert (and require) reason strings or add additional debugging information."
		)
		(
			(g_argOutputDir + ",o").c_str(),
			po::value<string>()->value_name("path"),
			"If given, creates one file per component and contract/file at the specified directory."
		)
		(g_strOverwrite.c_str(), "Overwrite existing files (used together with -o).")
		(
			g_argCombinedJson.c_str(),
			po::value<string>()->value_name(boost::join(g_combinedJsonArgs, ",")),
			"Output a single json document containing the specified information."
		)
		(g_argGas.c_str(), "Print an estimate of the maximal gas usage for each function.")
		(
			g_argStandardJSON.c_str(),
			"Switch to Standard JSON input / output mode, ignoring all options. "
			"It reads from standard input, if no input file was given, otherwise it reads from the provided input file. The result will be written to standard output."
		)
		(
			g_argImportAst.c_str(),
			"Import ASTs to be compiled, assumes input holds the AST in compact JSON format."
			" Supported Inputs is the output of the standard-json or the one produced by --combined-json ast,compact-format"
		)

		(
			g_argAssemble.c_str(),
			"Switch to assembly mode, ignoring all options except --machine, --yul-dialect and --optimize and assumes input is assembly."
		)
		(
			g_argYul.c_str(),
			"Switch to Yul mode, ignoring all options except --machine, --yul-dialect and --optimize and assumes input is Yul."
		)
		(
			g_argStrictAssembly.c_str(),
			"Switch to strict assembly mode, ignoring all options except --machine, --yul-dialect and --optimize and assumes input is strict assembly."
		)
		(
			g_strYulDialect.c_str(),
			po::value<string>()->value_name(boost::join(g_yulDialectArgs, ",")),
			"Input dialect to use in assembly or yul mode."
		)
		(
			g_argMachine.c_str(),
			po::value<string>()->value_name(boost::join(g_machineArgs, ",")),
			"Target machine in assembly or Yul mode."
		)
		(
			g_argLink.c_str(),
			"Switch to linker mode, ignoring all options apart from --libraries "
			"and modify binaries in place."
		)
		(
			g_argMetadataHash.c_str(),
			po::value<string>()->value_name(boost::join(g_metadataHashArgs, ",")),
			"Choose hash method for the bytecode metadata or disable it."
		)
		(g_argMetadataLiteral.c_str(), "Store referenced sources as literal data in the metadata output.")
		(
			g_argAllowPaths.c_str(),
			po::value<string>()->value_name("path(s)"),
			"Allow a given path for imports. A list of paths can be supplied by separating them with a comma."
		)
		(g_argColor.c_str(), "Force colored output.")
		(g_argNoColor.c_str(), "Explicitly disable colored output, disabling terminal auto-detection.")
		(g_argOldReporter.c_str(), "Enables old diagnostics reporter.")
		(g_argErrorRecovery.c_str(), "Enables additional parser error recovery.")
		(g_argIgnoreMissingFiles.c_str(), "Ignore missing files.");
	po::options_description outputComponents("Output Components");
	outputComponents.add_options()
		(g_argAstJson.c_str(), "AST of all source files in JSON format.")
		(g_argAstCompactJson.c_str(), "AST of all source files in a compact JSON format.")
		(g_argAsm.c_str(), "EVM assembly of the contracts.")
		(g_argAsmJson.c_str(), "EVM assembly of the contracts in JSON format.")
		(g_argOpcodes.c_str(), "Opcodes of the contracts.")
		(g_argBinary.c_str(), "Binary of the contracts in hex.")
		(g_argBinaryRuntime.c_str(), "Binary of the runtime part of the contracts in hex.")
		(g_argAbi.c_str(), "ABI specification of the contracts.")
		(g_argIR.c_str(), "Intermediate Representation (IR) of all contracts (EXPERIMENTAL).")
		(g_argEwasm.c_str(), "Ewasm text representation of all contracts (EXPERIMENTAL).")
		(g_argSignatureHashes.c_str(), "Function signature hashes of the contracts.")
		(g_argNatspecUser.c_str(), "Natspec user documentation of all contracts.")
		(g_argNatspecDev.c_str(), "Natspec developer documentation of all contracts.")
		(g_argMetadata.c_str(), "Combined Metadata JSON whose Swarm hash is stored on-chain.");
	desc.add(outputComponents);

	po::options_description allOptions = desc;
	allOptions.add_options()(g_argInputFile.c_str(), po::value<vector<string>>(), "input file");

	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add(g_argInputFile.c_str(), -1);

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

	if (m_args.count(g_argColor) && m_args.count(g_argNoColor))
	{
		serr() << "Option " << g_argColor << " and " << g_argNoColor << " are mutualy exclusive." << endl;
		return false;
	}

	m_coloredOutput = !m_args.count(g_argNoColor) && (isatty(STDERR_FILENO) || m_args.count(g_argColor));

	if (m_args.count(g_argHelp) || (isatty(fileno(stdin)) && _argc == 1))
	{
		sout() << desc;
		return false;
	}

	if (m_args.count(g_argVersion))
	{
		version();
		return false;
	}

	if (m_args.count(g_strLicense))
	{
		license();
		return false;
	}

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
		m_revertStrings = *revertStrings;
	}

	if (m_args.count(g_argCombinedJson))
	{
		vector<string> requests;
		for (string const& item: boost::split(requests, m_args[g_argCombinedJson].as<string>(), boost::is_any_of(",")))
			if (!g_combinedJsonArgs.count(item))
			{
				serr() << "Invalid option to --combined-json: " << item << endl;
				return false;
			}
	}
	po::notify(m_args);

	return true;
}

bool CommandLineInterface::processInput()
{
	ReadCallback::Callback fileReader = [this](string const& _kind, string const& _path)
	{
		try
		{
			if (_kind != ReadCallback::kindString(ReadCallback::Kind::ReadFile))
				BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment(
					"ReadFile callback used as callback kind " +
					_kind
				));
			auto path = boost::filesystem::path(_path);
			auto canonicalPath = boost::filesystem::weakly_canonical(path);
			bool isAllowed = false;
			for (auto const& allowedDir: m_allowedDirectories)
			{
				// If dir is a prefix of boostPath, we are fine.
				if (
					std::distance(allowedDir.begin(), allowedDir.end()) <= std::distance(canonicalPath.begin(), canonicalPath.end()) &&
					std::equal(allowedDir.begin(), allowedDir.end(), canonicalPath.begin())
				)
				{
					isAllowed = true;
					break;
				}
			}
			if (!isAllowed)
				return ReadCallback::Result{false, "File outside of allowed directories."};

			if (!boost::filesystem::exists(canonicalPath))
				return ReadCallback::Result{false, "File not found."};

			if (!boost::filesystem::is_regular_file(canonicalPath))
				return ReadCallback::Result{false, "Not a valid file."};

			auto contents = readFileAsString(canonicalPath.string());
			m_sourceCodes[path.generic_string()] = contents;
			return ReadCallback::Result{true, contents};
		}
		catch (Exception const& _exception)
		{
			return ReadCallback::Result{false, "Exception in read callback: " + boost::diagnostic_information(_exception)};
		}
		catch (...)
		{
			return ReadCallback::Result{false, "Unknown exception in read callback."};
		}
	};

	if (m_args.count(g_argAllowPaths))
	{
		vector<string> paths;
		for (string const& path: boost::split(paths, m_args[g_argAllowPaths].as<string>(), boost::is_any_of(",")))
		{
			auto filesystem_path = boost::filesystem::path(path);
			// If the given path had a trailing slash, the Boost filesystem
			// path will have it's last component set to '.'. This breaks
			// path comparison in later parts of the code, so we need to strip
			// it.
			if (filesystem_path.filename() == ".")
				filesystem_path.remove_filename();
			m_allowedDirectories.push_back(filesystem_path);
		}
	}

	if (m_args.count(g_argStandardJSON))
	{
		vector<string> inputFiles;
		string jsonFile;
		if (m_args.count(g_argInputFile))
			inputFiles = m_args[g_argInputFile].as<vector<string>>();
		if (inputFiles.size() == 1)
			jsonFile = inputFiles[0];
		else if (inputFiles.size() > 1)
		{
			serr() << "If --" << g_argStandardJSON << " is used, only zero or one input files are supported." << endl;
			return false;
		}
		string input;
		if (jsonFile.empty())
			input = readStandardInput();
		else
			input = readFileAsString(jsonFile);
		StandardCompiler compiler(fileReader);
		sout() << compiler.compile(std::move(input)) << endl;
		return true;
	}

	if (!readInputFilesAndConfigureRemappings())
		return false;

	if (m_args.count(g_argLibraries))
		for (string const& library: m_args[g_argLibraries].as<vector<string>>())
			if (!parseLibraryOption(library))
				return false;

	if (m_args.count(g_strEVMVersion))
	{
		string versionOptionStr = m_args[g_strEVMVersion].as<string>();
		std::optional<langutil::EVMVersion> versionOption = langutil::EVMVersion::fromString(versionOptionStr);
		if (!versionOption)
		{
			serr() << "Invalid option for --evm-version: " << versionOptionStr << endl;
			return false;
		}
		m_evmVersion = *versionOption;
	}

	if (m_args.count(g_argAssemble) || m_args.count(g_argStrictAssembly) || m_args.count(g_argYul))
	{
		// switch to assembly mode
		m_onlyAssemble = true;
		using Input = yul::AssemblyStack::Language;
		using Machine = yul::AssemblyStack::Machine;
		Input inputLanguage = m_args.count(g_argYul) ? Input::Yul : (m_args.count(g_argStrictAssembly) ? Input::StrictAssembly : Input::Assembly);
		Machine targetMachine = Machine::EVM;
		bool optimize = m_args.count(g_argOptimize);
		if (m_args.count(g_strOptimizeYul))
		{
			serr() << "--optimize-yul is invalid in assembly mode. Use --optimize instead." << endl;
			return false;
		}
		if (m_args.count(g_strNoOptimizeYul))
		{
			serr() << "--no-optimize-yul is invalid in assembly mode. Optimization is disabled by default and can be enabled with --optimize." << endl;
			return false;
		}
		if (m_args.count(g_argMachine))
		{
			string machine = m_args[g_argMachine].as<string>();
			if (machine == g_strEVM)
				targetMachine = Machine::EVM;
			else if (machine == g_strEVM15)
				targetMachine = Machine::EVM15;
			else if (machine == g_strEwasm)
				targetMachine = Machine::Ewasm;
			else
			{
				serr() << "Invalid option for --machine: " << machine << endl;
				return false;
			}
		}
		if (targetMachine == Machine::Ewasm && inputLanguage == Input::StrictAssembly)
			inputLanguage = Input::Ewasm;
		if (m_args.count(g_strYulDialect))
		{
			string dialect = m_args[g_strYulDialect].as<string>();
			if (dialect == g_strEVM)
				inputLanguage = Input::StrictAssembly;
			else if (dialect == g_strEwasm)
			{
				inputLanguage = Input::Ewasm;
				if (targetMachine != Machine::Ewasm)
				{
					serr() << "If you select Ewasm as --yul-dialect, --machine has to be Ewasm as well." << endl;
					return false;
				}
			}
			else
			{
				serr() << "Invalid option for --yul-dialect: " << dialect << endl;
				return false;
			}
		}
		if (optimize && (inputLanguage != Input::StrictAssembly && inputLanguage != Input::Ewasm))
		{
			serr() <<
				"Optimizer can only be used for strict assembly. Use --" <<
				g_strStrictAssembly <<
				"." <<
				endl;
			return false;
		}
		serr() <<
			"Warning: Yul is still experimental. Please use the output with care." <<
			endl;

		return assemble(inputLanguage, targetMachine, optimize);
	}
	if (m_args.count(g_argLink))
	{
		// switch to linker mode
		m_onlyLink = true;
		return link();
	}

	if (m_args.count(g_argMetadataHash))
	{
		string hashStr = m_args[g_argMetadataHash].as<string>();
		if (hashStr == g_strIPFS)
			m_metadataHash = CompilerStack::MetadataHash::IPFS;
		else if (hashStr == g_strSwarm)
			m_metadataHash = CompilerStack::MetadataHash::Bzzr1;
		else if (hashStr == g_strNone)
			m_metadataHash = CompilerStack::MetadataHash::None;
		else
		{
			serr() << "Invalid option for --metadata-hash: " << hashStr << endl;
			return false;
		}
	}

	m_compiler = make_unique<CompilerStack>(fileReader);

	unique_ptr<SourceReferenceFormatter> formatter;
	if (m_args.count(g_argOldReporter))
		formatter = make_unique<SourceReferenceFormatter>(serr(false));
	else
		formatter = make_unique<SourceReferenceFormatterHuman>(serr(false), m_coloredOutput);

	try
	{
		if (m_args.count(g_argMetadataLiteral) > 0)
			m_compiler->useMetadataLiteralSources(true);
		if (m_args.count(g_argMetadataHash))
			m_compiler->setMetadataHash(m_metadataHash);
		if (m_args.count(g_argInputFile))
			m_compiler->setRemappings(m_remappings);

		if (m_args.count(g_argLibraries))
			m_compiler->setLibraries(m_libraries);
		m_compiler->setEVMVersion(m_evmVersion);
		m_compiler->setRevertStringBehaviour(m_revertStrings);
		// TODO: Perhaps we should not compile unless requested

		m_compiler->enableIRGeneration(m_args.count(g_argIR));
		m_compiler->enableEwasmGeneration(m_args.count(g_argEwasm));

		OptimiserSettings settings = m_args.count(g_argOptimize) ? OptimiserSettings::standard() : OptimiserSettings::minimal();
		settings.expectedExecutionsPerDeployment = m_args[g_argOptimizeRuns].as<unsigned>();
		if (m_args.count(g_strNoOptimizeYul))
			settings.runYulOptimiser = false;
		settings.optimizeStackAllocation = settings.runYulOptimiser;
		m_compiler->setOptimiserSettings(settings);

		if (m_args.count(g_argImportAst))
		{
			try
			{
				m_compiler->importASTs(parseAstFromInput());

				if (!m_compiler->analyze())
				{
					for (auto const& error: m_compiler->errors())
						formatter->printErrorInformation(*error);
					astAssert(false, "Analysis of the AST failed");
				}
			}
			catch (Exception const& _exc)
			{
				serr() << string("Failed to import AST: ") << _exc.what() << endl;
				return false;
			}
		}
		else
		{
			m_compiler->setSources(m_sourceCodes);
			if (m_args.count(g_argErrorRecovery))
				m_compiler->setParserErrorRecovery(true);
		}

		bool successful = m_compiler->compile();

		for (auto const& error: m_compiler->errors())
		{
			g_hasOutput = true;
			formatter->printErrorInformation(*error);
		}

		if (!successful)
		{
			if (m_args.count(g_argErrorRecovery))
				return true;
			else
				return false;
		}
	}
	catch (CompilerError const& _exception)
	{
		g_hasOutput = true;
		formatter->printExceptionInformation(_exception, "Compiler error");
		return false;
	}
	catch (InternalCompilerError const& _exception)
	{
		serr() <<
			"Internal compiler error during compilation:" <<
			endl <<
			boost::diagnostic_information(_exception);
		return false;
	}
	catch (UnimplementedFeatureError const& _exception)
	{
		serr() <<
			"Unimplemented feature:" <<
			endl <<
			boost::diagnostic_information(_exception);
		return false;
	}
	catch (Error const& _error)
	{
		if (_error.type() == Error::Type::DocstringParsingError)
			serr() << "Documentation parsing error: " << *boost::get_error_info<errinfo_comment>(_error) << endl;
		else
		{
			g_hasOutput = true;
			formatter->printExceptionInformation(_error, _error.typeName());
		}

		return false;
	}
	catch (Exception const& _exception)
	{
		serr() << "Exception during compilation: " << boost::diagnostic_information(_exception) << endl;
		return false;
	}
	catch (std::exception const& _e)
	{
		serr() << "Unknown exception during compilation" << (
			_e.what() ? ": " + string(_e.what()) : "."
		) << endl;
		return false;
	}
	catch (...)
	{
		serr() << "Unknown exception during compilation." << endl;
		return false;
	}

	return true;
}

void CommandLineInterface::handleCombinedJSON()
{
	if (!m_args.count(g_argCombinedJson))
		return;

	Json::Value output(Json::objectValue);

	output[g_strVersion] = frontend::VersionString;
	set<string> requests;
	boost::split(requests, m_args[g_argCombinedJson].as<string>(), boost::is_any_of(","));
	vector<string> contracts = m_compiler->contractNames();

	if (!contracts.empty())
		output[g_strContracts] = Json::Value(Json::objectValue);
	for (string const& contractName: contracts)
	{
		Json::Value& contractData = output[g_strContracts][contractName] = Json::objectValue;
		if (requests.count(g_strAbi))
			contractData[g_strAbi] = jsonCompactPrint(m_compiler->contractABI(contractName));
		if (requests.count("metadata"))
			contractData["metadata"] = m_compiler->metadata(contractName);
		if (requests.count(g_strBinary) && m_compiler->compilationSuccessful())
			contractData[g_strBinary] = m_compiler->object(contractName).toHex();
		if (requests.count(g_strBinaryRuntime) && m_compiler->compilationSuccessful())
			contractData[g_strBinaryRuntime] = m_compiler->runtimeObject(contractName).toHex();
		if (requests.count(g_strOpcodes) && m_compiler->compilationSuccessful())
			contractData[g_strOpcodes] = evmasm::disassemble(m_compiler->object(contractName).bytecode);
		if (requests.count(g_strAsm) && m_compiler->compilationSuccessful())
			contractData[g_strAsm] = m_compiler->assemblyJSON(contractName);
		if (requests.count(g_strSrcMap) && m_compiler->compilationSuccessful())
		{
			auto map = m_compiler->sourceMapping(contractName);
			contractData[g_strSrcMap] = map ? *map : "";
		}
		if (requests.count(g_strSrcMapRuntime) && m_compiler->compilationSuccessful())
		{
			auto map = m_compiler->runtimeSourceMapping(contractName);
			contractData[g_strSrcMapRuntime] = map ? *map : "";
		}
		if (requests.count(g_strSignatureHashes))
			contractData[g_strSignatureHashes] = m_compiler->methodIdentifiers(contractName);
		if (requests.count(g_strNatspecDev))
			contractData[g_strNatspecDev] = jsonCompactPrint(m_compiler->natspecDev(contractName));
		if (requests.count(g_strNatspecUser))
			contractData[g_strNatspecUser] = jsonCompactPrint(m_compiler->natspecUser(contractName));
	}

	bool needsSourceList = requests.count(g_strAst) || requests.count(g_strSrcMap) || requests.count(g_strSrcMapRuntime);
	if (needsSourceList)
	{
		// Indices into this array are used to abbreviate source names in source locations.
		output[g_strSourceList] = Json::Value(Json::arrayValue);

		for (auto const& source: m_compiler->sourceNames())
			output[g_strSourceList].append(source);
	}

	if (requests.count(g_strAst))
	{
		bool legacyFormat = !requests.count(g_strCompactJSON);
		output[g_strSources] = Json::Value(Json::objectValue);
		for (auto const& sourceCode: m_sourceCodes)
		{
			ASTJsonConverter converter(legacyFormat, m_compiler->sourceIndices());
			output[g_strSources][sourceCode.first] = Json::Value(Json::objectValue);
			output[g_strSources][sourceCode.first]["AST"] = converter.toJson(m_compiler->ast(sourceCode.first));
		}
	}

	string json = m_args.count(g_argPrettyJson) ? jsonPrettyPrint(output) : jsonCompactPrint(output);

	if (m_args.count(g_argOutputDir))
		createJson("combined", json);
	else
		sout() << json << endl;
}

void CommandLineInterface::handleAst(string const& _argStr)
{
	string title;

	if (_argStr == g_argAstJson)
		title = "JSON AST:";
	else if (_argStr == g_argAstCompactJson)
		title = "JSON AST (compact format):";
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Illegal argStr for AST"));

	// do we need AST output?
	if (m_args.count(_argStr))
	{
		vector<ASTNode const*> asts;
		for (auto const& sourceCode: m_sourceCodes)
			asts.push_back(&m_compiler->ast(sourceCode.first));
		map<ASTNode const*, evmasm::GasMeter::GasConsumption> gasCosts;
		for (auto const& contract: m_compiler->contractNames())
			if (m_compiler->compilationSuccessful())
				if (auto const* assemblyItems = m_compiler->runtimeAssemblyItems(contract))
				{
					auto ret = GasEstimator::breakToStatementLevel(
						GasEstimator(m_evmVersion).structuralEstimation(*assemblyItems, asts),
						asts
					);
					for (auto const& it: ret)
						gasCosts[it.first] += it.second;
				}

		bool legacyFormat = !m_args.count(g_argAstCompactJson);
		if (m_args.count(g_argOutputDir))
		{
			for (auto const& sourceCode: m_sourceCodes)
			{
				stringstream data;
				string postfix = "";
				ASTJsonConverter(legacyFormat, m_compiler->sourceIndices()).print(data, m_compiler->ast(sourceCode.first));
				postfix += "_json";
				boost::filesystem::path path(sourceCode.first);
				createFile(path.filename().string() + postfix + ".ast", data.str());
			}
		}
		else
		{
			sout() << title << endl << endl;
			for (auto const& sourceCode: m_sourceCodes)
			{
				sout() << endl << "======= " << sourceCode.first << " =======" << endl;
				ASTJsonConverter(legacyFormat, m_compiler->sourceIndices()).print(sout(), m_compiler->ast(sourceCode.first));
			}
		}
	}
}

bool CommandLineInterface::actOnInput()
{
	if (m_args.count(g_argStandardJSON) || m_onlyAssemble)
		// Already done in "processInput" phase.
		return true;
	else if (m_onlyLink)
		writeLinkedFiles();
	else
		outputCompilationResults();
	return !m_error;
}

bool CommandLineInterface::link()
{
	// Map from how the libraries will be named inside the bytecode to their addresses.
	map<string, h160> librariesReplacements;
	int const placeholderSize = 40; // 20 bytes or 40 hex characters
	for (auto const& library: m_libraries)
	{
		string const& name = library.first;
		// Library placeholders are 40 hex digits (20 bytes) that start and end with '__'.
		// This leaves 36 characters for the library identifier. The identifier used to
		// be just the cropped or '_'-padded library name, but this changed to
		// the cropped hex representation of the hash of the library name.
		// We support both ways of linking here.
		librariesReplacements["__" + evmasm::LinkerObject::libraryPlaceholder(name) + "__"] = library.second;

		string replacement = "__";
		for (size_t i = 0; i < placeholderSize - 4; ++i)
			replacement.push_back(i < name.size() ? name[i] : '_');
		replacement += "__";
		librariesReplacements[replacement] = library.second;
	}
	for (auto& src: m_sourceCodes)
	{
		auto end = src.second.end();
		for (auto it = src.second.begin(); it != end;)
		{
			while (it != end && *it != '_') ++it;
			if (it == end) break;
			if (end - it < placeholderSize)
			{
				serr() << "Error in binary object file " << src.first << " at position " << (end - src.second.begin()) << endl;
				return false;
			}

			string name(it, it + placeholderSize);
			if (librariesReplacements.count(name))
			{
				string hexStr(toHex(librariesReplacements.at(name).asBytes()));
				copy(hexStr.begin(), hexStr.end(), it);
			}
			else
				serr() << "Reference \"" << name << "\" in file \"" << src.first << "\" still unresolved." << endl;
			it += placeholderSize;
		}
		// Remove hints for resolved libraries.
		for (auto const& library: m_libraries)
			boost::algorithm::erase_all(src.second, "\n" + libraryPlaceholderHint(library.first));
		while (!src.second.empty() && *prev(src.second.end()) == '\n')
			src.second.resize(src.second.size() - 1);
	}
	return true;
}

void CommandLineInterface::writeLinkedFiles()
{
	for (auto const& src: m_sourceCodes)
		if (src.first == g_stdinFileName)
			sout() << src.second << endl;
		else
		{
			ofstream outFile(src.first);
			outFile << src.second;
			if (!outFile)
			{
				serr() << "Could not write to file " << src.first << ". Aborting." << endl;
				return;
			}
		}
	sout() << "Linking completed." << endl;
}

string CommandLineInterface::libraryPlaceholderHint(string const& _libraryName)
{
	return "// " + evmasm::LinkerObject::libraryPlaceholder(_libraryName) + " -> " + _libraryName;
}

string CommandLineInterface::objectWithLinkRefsHex(evmasm::LinkerObject const& _obj)
{
	string out = _obj.toHex();
	if (!_obj.linkReferences.empty())
	{
		out += "\n";
		for (auto const& linkRef: _obj.linkReferences)
			out += "\n" + libraryPlaceholderHint(linkRef.second);
	}
	return out;
}

bool CommandLineInterface::assemble(
	yul::AssemblyStack::Language _language,
	yul::AssemblyStack::Machine _targetMachine,
	bool _optimize
)
{
	bool successful = true;
	map<string, yul::AssemblyStack> assemblyStacks;
	for (auto const& src: m_sourceCodes)
	{
		auto& stack = assemblyStacks[src.first] = yul::AssemblyStack(
			m_evmVersion,
			_language,
			_optimize ? OptimiserSettings::full() : OptimiserSettings::minimal()
		);
		try
		{
			if (!stack.parseAndAnalyze(src.first, src.second))
				successful = false;
			else
				stack.optimize();
		}
		catch (Exception const& _exception)
		{
			serr() << "Exception in assembler: " << boost::diagnostic_information(_exception) << endl;
			return false;
		}
		catch (std::exception const& _e)
		{
			serr() <<
				"Unknown exception during compilation" <<
				(_e.what() ? ": " + string(_e.what()) : ".") <<
				endl;
			return false;
		}
		catch (...)
		{
			serr() << "Unknown exception in assembler." << endl;
			return false;
		}
	}

	for (auto const& sourceAndStack: assemblyStacks)
	{
		auto const& stack = sourceAndStack.second;
		unique_ptr<SourceReferenceFormatter> formatter;
		if (m_args.count(g_argOldReporter))
			formatter = make_unique<SourceReferenceFormatter>(serr(false));
		else
			formatter = make_unique<SourceReferenceFormatterHuman>(serr(false), m_coloredOutput);

		for (auto const& error: stack.errors())
		{
			g_hasOutput = true;
			formatter->printErrorInformation(*error);
		}
		if (!Error::containsOnlyWarnings(stack.errors()))
			successful = false;
	}

	if (!successful)
		return false;

	for (auto const& src: m_sourceCodes)
	{
		string machine =
			_targetMachine == yul::AssemblyStack::Machine::EVM ? "EVM" :
			_targetMachine == yul::AssemblyStack::Machine::EVM15 ? "EVM 1.5" :
			"Ewasm";
		sout() << endl << "======= " << src.first << " (" << machine << ") =======" << endl;

		yul::AssemblyStack& stack = assemblyStacks[src.first];

		sout() << endl << "Pretty printed source:" << endl;
		sout() << stack.print() << endl;

		if (_language != yul::AssemblyStack::Language::Ewasm && _targetMachine == yul::AssemblyStack::Machine::Ewasm)
		{
			stack.translate(yul::AssemblyStack::Language::Ewasm);
			stack.optimize();

			sout() << endl << "==========================" << endl;
			sout() << endl << "Translated source:" << endl;
			sout() << stack.print() << endl;
		}

		yul::MachineAssemblyObject object;
		try
		{
			object = stack.assemble(_targetMachine);
		}
		catch (Exception const& _exception)
		{
			serr() << "Exception while assembling: " << boost::diagnostic_information(_exception) << endl;
			return false;
		}
		catch (std::exception const& _e)
		{
			serr() << "Unknown exception during compilation" << (
				_e.what() ? ": " + string(_e.what()) : "."
			) << endl;
			return false;
		}
		catch (...)
		{
			serr() << "Unknown exception while assembling." << endl;
			return false;
		}

		sout() << endl << "Binary representation:" << endl;
		if (object.bytecode)
			sout() << object.bytecode->toHex() << endl;
		else
			serr() << "No binary representation found." << endl;

		sout() << endl << "Text representation:" << endl;
		if (!object.assembly.empty())
			sout() << object.assembly << endl;
		else
			serr() << "No text representation found." << endl;
	}

	return true;
}

void CommandLineInterface::outputCompilationResults()
{
	handleCombinedJSON();

	// do we need AST output?
	handleAst(g_argAstJson);
	handleAst(g_argAstCompactJson);

	if (!m_compiler->compilationSuccessful())
	{
		serr() << endl << "Compilation halted after AST generation due to errors." << endl;
		return;
	}

	vector<string> contracts = m_compiler->contractNames();
	for (string const& contract: contracts)
	{
		if (needsHumanTargetedStdout(m_args))
			sout() << endl << "======= " << contract << " =======" << endl;

		// do we need EVM assembly?
		if (m_args.count(g_argAsm) || m_args.count(g_argAsmJson))
		{
			string ret;
			if (m_args.count(g_argAsmJson))
				ret = jsonPrettyPrint(m_compiler->assemblyJSON(contract));
			else
				ret = m_compiler->assemblyString(contract, m_sourceCodes);

			if (m_args.count(g_argOutputDir))
			{
				createFile(m_compiler->filesystemFriendlyName(contract) + (m_args.count(g_argAsmJson) ? "_evm.json" : ".evm"), ret);
			}
			else
			{
				sout() << "EVM assembly:" << endl << ret << endl;
			}
		}

		if (m_args.count(g_argGas))
			handleGasEstimation(contract);

		handleBytecode(contract);
		handleIR(contract);
		handleEwasm(contract);
		handleSignatureHashes(contract);
		handleMetadata(contract);
		handleABI(contract);
		handleNatspec(true, contract);
		handleNatspec(false, contract);
	} // end of contracts iteration

	if (!g_hasOutput)
	{
		if (m_args.count(g_argOutputDir))
			sout() << "Compiler run successful. Artifact(s) can be found in directory " << m_args.at(g_argOutputDir).as<string>() << "." << endl;
		else
			serr() << "Compiler run successful, no output requested." << endl;
	}
}

}
