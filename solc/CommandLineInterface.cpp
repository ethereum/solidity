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
#include "CommandLineInterface.h"

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

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#include "solidity/BuildInfo.h"
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/JSON.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/GasMeter.h>
#include <libsolidity/interface/Version.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/ast/ASTPrinter.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/formal/Why3Translator.h>

using namespace std;
namespace po = boost::program_options;

namespace dev
{
namespace solidity
{

static string const g_argAbiStr = "abi";
static string const g_argSignatureHashes = "hashes";
static string const g_argGas = "gas";
static string const g_argAsmStr = "asm";
static string const g_argAsmJsonStr = "asm-json";
static string const g_argAstStr = "ast";
static string const g_argAstJson = "ast-json";
static string const g_argBinaryStr = "bin";
static string const g_argRuntimeBinaryStr = "bin-runtime";
static string const g_argCloneBinaryStr = "clone-bin";
static string const g_argOpcodesStr = "opcodes";
static string const g_argNatspecDevStr = "devdoc";
static string const g_argNatspecUserStr = "userdoc";
static string const g_argMetadata = "metadata";
static string const g_argAddStandard = "add-std";
static string const g_stdinFileName = "<stdin>";

/// Possible arguments to for --combined-json
static set<string> const g_combinedJsonArgs{
	"bin",
	"bin-runtime",
	"clone-bin",
	"srcmap",
	"srcmap-runtime",
	"opcodes",
	"abi",
	"interface",
	"metadata",
	"asm",
	"ast",
	"userdoc",
	"devdoc"
};

static void version()
{
	cout <<
		"solc, the solidity compiler commandline interface" <<
		endl <<
		"Version: " <<
		dev::solidity::VersionString <<
		endl;
	exit(0);
}

static bool needsHumanTargetedStdout(po::variables_map const& _args)
{
	if (_args.count(g_argGas))
		return true;
	if (_args.count("output-dir"))
		return false;
	for (string const& arg: {
		g_argAbiStr,
		g_argSignatureHashes,
		g_argMetadata,
		g_argNatspecUserStr,
		g_argAstJson,
		g_argNatspecDevStr,
		g_argAsmStr,
		g_argAsmJsonStr,
		g_argOpcodesStr,
		g_argBinaryStr,
		g_argRuntimeBinaryStr,
		g_argCloneBinaryStr,
		string("formal")
	})
		if (_args.count(arg))
			return true;
	return false;
}

void CommandLineInterface::handleBinary(string const& _contract)
{
	if (m_args.count(g_argBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".bin", m_compiler->object(_contract).toHex());
		else
		{
			cout << "Binary: " << endl;
			cout << m_compiler->object(_contract).toHex() << endl;
		}
	}
	if (m_args.count(g_argCloneBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".clone_bin", m_compiler->cloneObject(_contract).toHex());
		else
		{
			cout << "Clone Binary: " << endl;
			cout << m_compiler->cloneObject(_contract).toHex() << endl;
		}
	}
	if (m_args.count(g_argRuntimeBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".bin-runtime", m_compiler->runtimeObject(_contract).toHex());
		else
		{
			cout << "Binary of the runtime part: " << endl;
			cout << m_compiler->runtimeObject(_contract).toHex() << endl;
		}
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	if (m_args.count("output-dir"))
		createFile(_contract + ".opcode", solidity::disassemble(m_compiler->object(_contract).bytecode));
	else
	{
		cout << "Opcodes: " << endl;
		cout << solidity::disassemble(m_compiler->object(_contract).bytecode);
		cout << endl;
	}
}

void CommandLineInterface::handleBytecode(string const& _contract)
{
	if (m_args.count(g_argOpcodesStr))
		handleOpcode(_contract);
	if (m_args.count(g_argBinaryStr) || m_args.count(g_argCloneBinaryStr) || m_args.count(g_argRuntimeBinaryStr))
		handleBinary(_contract);
}

void CommandLineInterface::handleSignatureHashes(string const& _contract)
{
	if (!m_args.count(g_argSignatureHashes))
		return;

	string out;
	for (auto const& it: m_compiler->contractDefinition(_contract).interfaceFunctions())
		out += toHex(it.first.ref()) + ": " + it.second->externalSignature() + "\n";

	if (m_args.count("output-dir"))
		createFile(_contract + ".signatures", out);
	else
		cout << "Function signatures: " << endl << out;
}

void CommandLineInterface::handleOnChainMetadata(string const& _contract)
{
	if (!m_args.count(g_argMetadata))
		return;

	string data = m_compiler->onChainMetadata(_contract);
	if (m_args.count("output-dir"))
		createFile(_contract + ".meta", data);
	else
		cout << "Metadata: " << endl << data << endl;
}

void CommandLineInterface::handleMeta(DocumentationType _type, string const& _contract)
{
	std::string argName;
	std::string suffix;
	std::string title;
	switch(_type)
	{
	case DocumentationType::ABIInterface:
		argName = g_argAbiStr;
		suffix = ".abi";
		title = "Contract JSON ABI";
		break;
	case DocumentationType::NatspecUser:
		argName = g_argNatspecUserStr;
		suffix = ".docuser";
		title = "User Documentation";
		break;
	case DocumentationType::NatspecDev:
		argName = g_argNatspecDevStr;
		suffix = ".docdev";
		title = "Developer Documentation";
		break;
	default:
		// should never happen
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown documentation _type"));
	}

	if (m_args.count(argName))
	{
		std::string output;
		if (_type == DocumentationType::ABIInterface)
			output = dev::jsonCompactPrint(m_compiler->metadata(_contract, _type));
		else
			output = dev::jsonPrettyPrint(m_compiler->metadata(_contract, _type));

		if (m_args.count("output-dir"))
			createFile(_contract + suffix, output);
		else
		{
			cout << title << endl;
			cout << output << endl;
		}

	}
}

void CommandLineInterface::handleGasEstimation(string const& _contract)
{
	using Gas = GasEstimator::GasConsumption;
	if (!m_compiler->assemblyItems(_contract) && !m_compiler->runtimeAssemblyItems(_contract))
		return;
	cout << "Gas estimation:" << endl;
	if (eth::AssemblyItems const* items = m_compiler->assemblyItems(_contract))
	{
		Gas gas = GasEstimator::functionalEstimation(*items);
		u256 bytecodeSize(m_compiler->runtimeObject(_contract).bytecode.size());
		cout << "construction:" << endl;
		cout << "   " << gas << " + " << (bytecodeSize * eth::GasCosts::createDataGas) << " = ";
		gas += bytecodeSize * eth::GasCosts::createDataGas;
		cout << gas << endl;
	}
	if (eth::AssemblyItems const* items = m_compiler->runtimeAssemblyItems(_contract))
	{
		ContractDefinition const& contract = m_compiler->contractDefinition(_contract);
		cout << "external:" << endl;
		for (auto it: contract.interfaceFunctions())
		{
			string sig = it.second->externalSignature();
			GasEstimator::GasConsumption gas = GasEstimator::functionalEstimation(*items, sig);
			cout << "   " << sig << ":\t" << gas << endl;
		}
		if (contract.fallbackFunction())
		{
			GasEstimator::GasConsumption gas = GasEstimator::functionalEstimation(*items, "INVALID");
			cout << "   fallback:\t" << gas << endl;
		}
		cout << "internal:" << endl;
		for (auto const& it: contract.definedFunctions())
		{
			if (it->isPartOfExternalInterface() || it->isConstructor())
				continue;
			size_t entry = m_compiler->functionEntryPoint(_contract, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = GasEstimator::functionalEstimation(*items, entry, *it);
			FunctionType type(*it);
			cout << "   " << it->name() << "(";
			auto paramTypes = type.parameterTypes();
			for (auto it = paramTypes.begin(); it != paramTypes.end(); ++it)
				cout << (*it)->toString() << (it + 1 == paramTypes.end() ? "" : ",");
			cout << "):\t" << gas << endl;
		}
	}
}

void CommandLineInterface::handleFormal()
{
	if (!m_args.count("formal"))
		return;

	if (m_args.count("output-dir"))
		createFile("solidity.mlw", m_compiler->formalTranslation());
	else
		cout << "Formal version:" << endl << m_compiler->formalTranslation() << endl;
}

void CommandLineInterface::readInputFilesAndConfigureRemappings()
{
	vector<string> inputFiles;
	bool addStdin = false;
	if (!m_args.count("input-file"))
		addStdin = true;
	else
		for (string path: m_args["input-file"].as<vector<string>>())
		{
			auto eq = find(path.begin(), path.end(), '=');
			if (eq != path.end())
				path = string(eq + 1, path.end());
			else if (path == "-")
				addStdin = true;
			else
			{
				auto infile = boost::filesystem::path(path);
				if (!boost::filesystem::exists(infile))
				{
					cerr << "Skipping non-existent input file \"" << infile << "\"" << endl;
					continue;
				}

				if (!boost::filesystem::is_regular_file(infile))
				{
					cerr << "\"" << infile << "\" is not a valid file. Skipping" << endl;
					continue;
				}

				m_sourceCodes[infile.string()] = dev::contentsString(infile.string());
				path = boost::filesystem::canonical(infile).string();
			}
			m_allowedDirectories.push_back(boost::filesystem::path(path).remove_filename());
		}
	if (addStdin)
	{
		string s;
		while (!cin.eof())
		{
			getline(cin, s);
			m_sourceCodes[g_stdinFileName].append(s + '\n');
		}
	}
}

bool CommandLineInterface::parseLibraryOption(string const& _input)
{
	namespace fs = boost::filesystem;
	string data = fs::is_regular_file(_input) ? contentsString(_input) : _input;

	vector<string> libraries;
	boost::split(libraries, data, boost::is_space() || boost::is_any_of(","), boost::token_compress_on);
	for (string const& lib: libraries)
		if (!lib.empty())
		{
			auto colon = lib.find(':');
			if (colon == string::npos)
			{
				cerr << "Colon separator missing in library address specifier \"" << lib << "\"" << endl;
				return false;
			}
			string libName(lib.begin(), lib.begin() + colon);
			string addrString(lib.begin() + colon + 1, lib.end());
			boost::trim(libName);
			boost::trim(addrString);
			bytes binAddr = fromHex(addrString);
			h160 address(binAddr, h160::AlignRight);
			if (binAddr.size() > 20 || address == h160())
			{
				cerr << "Invalid address for library \"" << libName << "\": " << addrString << endl;
				return false;
			}
			m_libraries[libName] = address;
		}

	return true;
}

void CommandLineInterface::createFile(string const& _fileName, string const& _data)
{
	namespace fs = boost::filesystem;
	// create directory if not existent
	fs::path p(m_args.at("output-dir").as<string>());
	fs::create_directories(p);
	string pathName = (p / _fileName).string();
	ofstream outFile(pathName);
	outFile << _data;
	if (!outFile)
		BOOST_THROW_EXCEPTION(FileError() << errinfo_comment("Could not write to file: " + pathName));
}

bool CommandLineInterface::parseArguments(int _argc, char** _argv)
{
	// Declare the supported options.
	po::options_description desc(
		R"(solc, the Solidity commandline compiler.
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
		po::options_description::m_default_line_length - 23);
	desc.add_options()
		("help", "Show help message and exit.")
		("version", "Show version and exit.")
		("optimize", "Enable bytecode optimizer.")
		(
			"optimize-runs",
			po::value<unsigned>()->value_name("n")->default_value(200),
			"Estimated number of contract runs for optimizer tuning."
		)
		(g_argAddStandard.c_str(), "Add standard contracts.")
		(
			"libraries",
			po::value<vector<string>>()->value_name("libs"),
			"Direct string or file containing library addresses. Syntax: "
			"<libraryName>: <address> [, or whitespace] ...\n"
			"Address is interpreted as a hex string optionally prefixed by 0x."
		)
		(
			"output-dir,o",
			po::value<string>()->value_name("path"),
			"If given, creates one file per component and contract/file at the specified directory."
		)
		(
			"combined-json",
			po::value<string>()->value_name(boost::join(g_combinedJsonArgs, ",")),
			"Output a single json document containing the specified information."
		)
		(g_argGas.c_str(), "Print an estimate of the maximal gas usage for each function.")
		(
			"assemble",
			"Switch to assembly mode, ignoring all options and assumes input is assembly."
		)
		(
			"link",
			"Switch to linker mode, ignoring all options apart from --libraries "
			"and modify binaries in place."
		);
	po::options_description outputComponents("Output Components");
	outputComponents.add_options()
		(g_argAstStr.c_str(), "AST of all source files.")
		(g_argAstJson.c_str(), "AST of all source files in JSON format.")
		(g_argAsmStr.c_str(), "EVM assembly of the contracts.")
		(g_argAsmJsonStr.c_str(), "EVM assembly of the contracts in JSON format.")
		(g_argOpcodesStr.c_str(), "Opcodes of the contracts.")
		(g_argBinaryStr.c_str(), "Binary of the contracts in hex.")
		(g_argRuntimeBinaryStr.c_str(), "Binary of the runtime part of the contracts in hex.")
		(g_argCloneBinaryStr.c_str(), "Binary of the clone contracts in hex.")
		(g_argAbiStr.c_str(), "ABI specification of the contracts.")
		(g_argSignatureHashes.c_str(), "Function signature hashes of the contracts.")
		(g_argNatspecUserStr.c_str(), "Natspec user documentation of all contracts.")
		(g_argNatspecDevStr.c_str(), "Natspec developer documentation of all contracts.")
		(g_argMetadata.c_str(), "Combined metadata JSON whose swarm hash is stored on-chain.")
		("formal", "Translated source suitable for formal analysis.");
	desc.add(outputComponents);

	po::options_description allOptions = desc;
	allOptions.add_options()("input-file", po::value<vector<string>>(), "input file");

	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add("input-file", -1);

	// parse the compiler arguments
	try
	{
		po::command_line_parser cmdLineParser(_argc, _argv);
		cmdLineParser.options(allOptions).positional(filesPositions);
		po::store(cmdLineParser.run(), m_args);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return false;
	}

	if (m_args.count("help") || (isatty(fileno(stdin)) && _argc == 1))
	{
		cout << desc;
		return false;
	}

	if (m_args.count("version"))
	{
		version();
		return false;
	}

	if (m_args.count("combined-json"))
	{
		vector<string> requests;
		for (string const& item: boost::split(requests, m_args["combined-json"].as<string>(), boost::is_any_of(",")))
			if (!g_combinedJsonArgs.count(item))
			{
				cerr << "Invalid option to --combined-json: " << item << endl;
				return false;
			}
	}
	po::notify(m_args);

	return true;
}

bool CommandLineInterface::processInput()
{
	readInputFilesAndConfigureRemappings();

	if (m_args.count("libraries"))
		for (string const& library: m_args["libraries"].as<vector<string>>())
			if (!parseLibraryOption(library))
				return false;

	if (m_args.count("assemble"))
	{
		// switch to assembly mode
		m_onlyAssemble = true;
		return assemble();
	}
	if (m_args.count("link"))
	{
		// switch to linker mode
		m_onlyLink = true;
		return link();
	}

	CompilerStack::ReadFileCallback fileReader = [this](string const& _path)
	{
		auto path = boost::filesystem::path(_path);
		if (!boost::filesystem::exists(path))
			return CompilerStack::ReadFileResult{false, "File not found."};
		auto canonicalPath = boost::filesystem::canonical(path);
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
			return CompilerStack::ReadFileResult{false, "File outside of allowed directories."};
		else if (!boost::filesystem::is_regular_file(canonicalPath))
			return CompilerStack::ReadFileResult{false, "Not a valid file."};
		else
		{
			auto contents = dev::contentsString(canonicalPath.string());
			m_sourceCodes[path.string()] = contents;
			return CompilerStack::ReadFileResult{true, contents};
		}
	};

	m_compiler.reset(new CompilerStack(fileReader));
	auto scannerFromSourceName = [&](string const& _sourceName) -> solidity::Scanner const& { return m_compiler->scanner(_sourceName); };
	try
	{
		if (m_args.count("input-file"))
			m_compiler->setRemappings(m_args["input-file"].as<vector<string>>());
		for (auto const& sourceCode: m_sourceCodes)
			m_compiler->addSource(sourceCode.first, sourceCode.second);
		// TODO: Perhaps we should not compile unless requested
		bool optimize = m_args.count("optimize") > 0;
		unsigned runs = m_args["optimize-runs"].as<unsigned>();
		bool successful = m_compiler->compile(optimize, runs, m_libraries);

		if (successful && m_args.count("formal"))
			if (!m_compiler->prepareFormalAnalysis())
				successful = false;

		for (auto const& error: m_compiler->errors())
			SourceReferenceFormatter::printExceptionInformation(
				cerr,
				*error,
				(error->type() == Error::Type::Warning) ? "Warning" : "Error",
				scannerFromSourceName
			);

		if (!successful)
			return false;
	}
	catch (CompilerError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Compiler error", scannerFromSourceName);
		return false;
	}
	catch (InternalCompilerError const& _exception)
	{
		cerr << "Internal compiler error during compilation:" << endl
			 << boost::diagnostic_information(_exception);
		return false;
	}
	catch (UnimplementedFeatureError const& _exception)
	{
		cerr << "Unimplemented feature:" << endl
			 << boost::diagnostic_information(_exception);
		return false;
	}
	catch (Error const& _error)
	{
		if (_error.type() == Error::Type::DocstringParsingError)
			cerr << "Documentation parsing error: " << *boost::get_error_info<errinfo_comment>(_error) << endl;
		else
			SourceReferenceFormatter::printExceptionInformation(cerr, _error, _error.typeName(), scannerFromSourceName);

		return false;
	}
	catch (Exception const& _exception)
	{
		cerr << "Exception during compilation: " << boost::diagnostic_information(_exception) << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Unknown exception during compilation." << endl;
		return false;
	}

	return true;
}

void CommandLineInterface::handleCombinedJSON()
{
	if (!m_args.count("combined-json"))
		return;

	Json::Value output(Json::objectValue);

	output["version"] = ::dev::solidity::VersionString;
	set<string> requests;
	boost::split(requests, m_args["combined-json"].as<string>(), boost::is_any_of(","));
	vector<string> contracts = m_compiler->contractNames();

	if (!contracts.empty())
		output["contracts"] = Json::Value(Json::objectValue);
	for (string const& contractName: contracts)
	{
		Json::Value contractData(Json::objectValue);
		if (requests.count("abi"))
			contractData["abi"] = dev::jsonCompactPrint(m_compiler->interface(contractName));
		if (requests.count("metadata"))
			contractData["metadata"] = m_compiler->onChainMetadata(contractName);
		if (requests.count("bin"))
			contractData["bin"] = m_compiler->object(contractName).toHex();
		if (requests.count("bin-runtime"))
			contractData["bin-runtime"] = m_compiler->runtimeObject(contractName).toHex();
		if (requests.count("clone-bin"))
			contractData["clone-bin"] = m_compiler->cloneObject(contractName).toHex();
		if (requests.count("opcodes"))
			contractData["opcodes"] = solidity::disassemble(m_compiler->object(contractName).bytecode);
		if (requests.count("asm"))
		{
			ostringstream unused;
			contractData["asm"] = m_compiler->streamAssembly(unused, contractName, m_sourceCodes, true);
		}
		if (requests.count("srcmap"))
		{
			auto map = m_compiler->sourceMapping(contractName);
			contractData["srcmap"] = map ? *map : "";
		}
		if (requests.count("srcmap-runtime"))
		{
			auto map = m_compiler->runtimeSourceMapping(contractName);
			contractData["srcmap-runtime"] = map ? *map : "";
		}
		if (requests.count("devdoc"))
			contractData["devdoc"] = dev::jsonCompactPrint(m_compiler->metadata(contractName, DocumentationType::NatspecDev));
		if (requests.count("userdoc"))
			contractData["userdoc"] = dev::jsonCompactPrint(m_compiler->metadata(contractName, DocumentationType::NatspecUser));
		output["contracts"][contractName] = contractData;
	}

	bool needsSourceList = requests.count("ast") || requests.count("srcmap") || requests.count("srcmap-runtime");
	if (needsSourceList)
	{
		// Indices into this array are used to abbreviate source names in source locations.
		output["sourceList"] = Json::Value(Json::arrayValue);

		for (auto const& source: m_compiler->sourceNames())
			output["sourceList"].append(source);
	}

	if (requests.count("ast"))
	{
		output["sources"] = Json::Value(Json::objectValue);
		for (auto const& sourceCode: m_sourceCodes)
		{
			ASTJsonConverter converter(m_compiler->ast(sourceCode.first), m_compiler->sourceIndices());
			output["sources"][sourceCode.first] = Json::Value(Json::objectValue);
			output["sources"][sourceCode.first]["AST"] = converter.json();
		}
	}
	cout << dev::jsonCompactPrint(output) << endl;
}

void CommandLineInterface::handleAst(string const& _argStr)
{
	string title;

	if (_argStr == g_argAstStr)
		title = "Syntax trees:";
	else if (_argStr == g_argAstJson)
		title = "JSON AST:";
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Illegal argStr for AST"));

	// do we need AST output?
	if (m_args.count(_argStr))
	{
		vector<ASTNode const*> asts;
		for (auto const& sourceCode: m_sourceCodes)
			asts.push_back(&m_compiler->ast(sourceCode.first));
		map<ASTNode const*, eth::GasMeter::GasConsumption> gasCosts;
		if (m_compiler->runtimeAssemblyItems())
			gasCosts = GasEstimator::breakToStatementLevel(
				GasEstimator::structuralEstimation(*m_compiler->runtimeAssemblyItems(), asts),
				asts
			);

		if (m_args.count("output-dir"))
		{
			for (auto const& sourceCode: m_sourceCodes)
			{
				stringstream data;
				string postfix = "";
				if (_argStr == g_argAstStr)
				{
					ASTPrinter printer(m_compiler->ast(sourceCode.first), sourceCode.second);
					printer.print(data);
				}
				else
				{
					ASTJsonConverter converter(m_compiler->ast(sourceCode.first));
					converter.print(data);
					postfix += "_json";
				}
				boost::filesystem::path path(sourceCode.first);
				createFile(path.filename().string() + postfix + ".ast", data.str());
			}
		}
		else
		{
			cout << title << endl << endl;
			for (auto const& sourceCode: m_sourceCodes)
			{
				cout << endl << "======= " << sourceCode.first << " =======" << endl;
				if (_argStr == g_argAstStr)
				{
					ASTPrinter printer(
						m_compiler->ast(sourceCode.first),
						sourceCode.second,
						gasCosts
					);
					printer.print(cout);
				}
				else
				{
					ASTJsonConverter converter(m_compiler->ast(sourceCode.first));
					converter.print(cout);
				}
			}
		}
	}
}

void CommandLineInterface::actOnInput()
{
	if (m_onlyAssemble)
		outputAssembly();
	else if (m_onlyLink)
		writeLinkedFiles();
	else
		outputCompilationResults();
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
		// This leaves 36 characters for the library name, while too short library names are
		// padded on the right with '_' and too long names are truncated.
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
				cerr << "Error in binary object file " << src.first << " at position " << (end - src.second.begin()) << endl;
				return false;
			}

			string name(it, it + placeholderSize);
			if (librariesReplacements.count(name))
			{
				string hexStr(toHex(librariesReplacements.at(name).asBytes()));
				copy(hexStr.begin(), hexStr.end(), it);
			}
			else
				cerr << "Reference \"" << name << "\" in file \"" << src.first << "\" still unresolved." << endl;
			it += placeholderSize;
		}
	}
	return true;
}

void CommandLineInterface::writeLinkedFiles()
{
	for (auto const& src: m_sourceCodes)
		if (src.first == g_stdinFileName)
			cout << src.second << endl;
		else
			writeFile(src.first, src.second);
}

bool CommandLineInterface::assemble()
{
	//@TODO later, we will use the convenience interface and should also remove the include above
	bool successful = true;
	map<string, shared_ptr<Scanner>> scanners;
	for (auto const& src: m_sourceCodes)
	{
		auto scanner = make_shared<Scanner>(CharStream(src.second), src.first);
		scanners[src.first] = scanner;
		if (!m_assemblyStacks[src.first].parse(scanner))
			successful = false;
		else
			//@TODO we should not just throw away the result here
			m_assemblyStacks[src.first].assemble();
	}
	for (auto const& stack: m_assemblyStacks)
		for (auto const& error: stack.second.errors())
			SourceReferenceFormatter::printExceptionInformation(
				cerr,
				*error,
				(error->type() == Error::Type::Warning) ? "Warning" : "Error",
				[&](string const& _source) -> Scanner const& { return *scanners.at(_source); }
			);

	return successful;
}

void CommandLineInterface::outputAssembly()
{
	for (auto const& src: m_sourceCodes)
	{
		cout << endl << "======= " << src.first << " =======" << endl;
		eth::Assembly assembly = m_assemblyStacks[src.first].assemble();
		cout << assembly.assemble().toHex() << endl;
		assembly.stream(cout, "", m_sourceCodes);
	}
}

void CommandLineInterface::outputCompilationResults()
{
	handleCombinedJSON();

	// do we need AST output?
	handleAst(g_argAstStr);
	handleAst(g_argAstJson);

	vector<string> contracts = m_compiler->contractNames();
	for (string const& contract: contracts)
	{
		if (needsHumanTargetedStdout(m_args))
			cout << endl << "======= " << contract << " =======" << endl;

		// do we need EVM assembly?
		if (m_args.count(g_argAsmStr) || m_args.count(g_argAsmJsonStr))
		{
			if (m_args.count("output-dir"))
			{
				stringstream data;
				m_compiler->streamAssembly(data, contract, m_sourceCodes, m_args.count(g_argAsmJsonStr));
				createFile(contract + (m_args.count(g_argAsmJsonStr) ? "_evm.json" : ".evm"), data.str());
			}
			else
			{
				cout << "EVM assembly:" << endl;
				m_compiler->streamAssembly(cout, contract, m_sourceCodes, m_args.count(g_argAsmJsonStr));
			}
		}

		if (m_args.count(g_argGas))
			handleGasEstimation(contract);

		handleBytecode(contract);
		handleSignatureHashes(contract);
		handleOnChainMetadata(contract);
		handleMeta(DocumentationType::ABIInterface, contract);
		handleMeta(DocumentationType::NatspecDev, contract);
		handleMeta(DocumentationType::NatspecUser, contract);
	} // end of contracts iteration

	handleFormal();
}

}
}
