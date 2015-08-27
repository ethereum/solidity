/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#include "CommandLineInterface.h"

#include <string>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "BuildInfo.h"
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libevmcore/Instruction.h>
#include <libevmcore/Params.h>
#include <libsolidity/Version.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/ASTJsonConverter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/SourceReferenceFormatter.h>
#include <libsolidity/GasEstimator.h>

using namespace std;
namespace po = boost::program_options;

namespace dev
{
namespace solidity
{

static string const g_argAbiStr = "abi";
static string const g_argSolInterfaceStr = "interface";
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
static string const g_argAddStandard = "add-std";

/// Possible arguments to for --combined-json
static set<string> const g_combinedJsonArgs{
	"bin",
	"clone-bin",
	"opcodes",
	"abi",
	"interface",
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

static inline bool humanTargetedStdout(po::variables_map const& _args, string const& _name)
{
	return _args.count(_name) && !(_args.count("output-dir"));
}

static bool needsHumanTargetedStdout(po::variables_map const& _args)
{

	return
		_args.count(g_argGas) ||
		humanTargetedStdout(_args, g_argAbiStr) ||
		humanTargetedStdout(_args, g_argSolInterfaceStr) ||
		humanTargetedStdout(_args, g_argSignatureHashes) ||
		humanTargetedStdout(_args, g_argNatspecUserStr) ||
		humanTargetedStdout(_args, g_argAstJson) ||
		humanTargetedStdout(_args, g_argNatspecDevStr) ||
		humanTargetedStdout(_args, g_argAsmStr) ||
		humanTargetedStdout(_args, g_argAsmJsonStr) ||
		humanTargetedStdout(_args, g_argOpcodesStr) ||
		humanTargetedStdout(_args, g_argBinaryStr) ||
		humanTargetedStdout(_args, g_argRuntimeBinaryStr) ||
		humanTargetedStdout(_args, g_argCloneBinaryStr);
}

void CommandLineInterface::handleBinary(string const& _contract)
{
	if (m_args.count(g_argBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".bin", toHex(m_compiler->getBytecode(_contract)));
		else
		{
			cout << "Binary: " << endl;
			cout << toHex(m_compiler->getBytecode(_contract)) << endl;
		}
	}
	if (m_args.count(g_argCloneBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".clone_bin", toHex(m_compiler->getCloneBytecode(_contract)));
		else
		{
			cout << "Clone Binary: " << endl;
			cout << toHex(m_compiler->getCloneBytecode(_contract)) << endl;
		}
	}
	if (m_args.count(g_argRuntimeBinaryStr))
	{
		if (m_args.count("output-dir"))
			createFile(_contract + ".bin", toHex(m_compiler->getRuntimeBytecode(_contract)));
		else
		{
			cout << "Binary of the runtime part: " << endl;
			cout << toHex(m_compiler->getRuntimeBytecode(_contract)) << endl;
		}
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	if (m_args.count("output-dir"))
		createFile(_contract + ".opcode", eth::disassemble(m_compiler->getBytecode(_contract)));
	else
	{
		cout << "Opcodes: " << endl;
		cout << eth::disassemble(m_compiler->getBytecode(_contract));
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
	for (auto const& it: m_compiler->getContractDefinition(_contract).getInterfaceFunctions())
		out += toHex(it.first.ref()) + ": " + it.second->externalSignature() + "\n";

	if (m_args.count("output-dir"))
		createFile(_contract + ".signatures", out);
	else
		cout << "Function signatures: " << endl << out;
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
	case DocumentationType::ABISolidityInterface:
		argName = g_argSolInterfaceStr;
		suffix = "_interface.sol";
		title = "Contract Solidity ABI";
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
		if (m_args.count("output-dir"))
			createFile(_contract + suffix, m_compiler->getMetadata(_contract, _type));
		else
		{
			cout << title << endl;
			cout << m_compiler->getMetadata(_contract, _type) << endl;
		}

	}
}

void CommandLineInterface::handleGasEstimation(string const& _contract)
{
	using Gas = GasEstimator::GasConsumption;
	if (!m_compiler->getAssemblyItems(_contract) && !m_compiler->getRuntimeAssemblyItems(_contract))
		return;
	cout << "Gas estimation:" << endl;
	if (eth::AssemblyItems const* items = m_compiler->getAssemblyItems(_contract))
	{
		Gas gas = GasEstimator::functionalEstimation(*items);
		u256 bytecodeSize(m_compiler->getRuntimeBytecode(_contract).size());
		cout << "construction:" << endl;
		cout << "   " << gas << " + " << (bytecodeSize * eth::c_createDataGas) << " = ";
		gas += bytecodeSize * eth::c_createDataGas;
		cout << gas << endl;
	}
	if (eth::AssemblyItems const* items = m_compiler->getRuntimeAssemblyItems(_contract))
	{
		ContractDefinition const& contract = m_compiler->getContractDefinition(_contract);
		cout << "external:" << endl;
		for (auto it: contract.getInterfaceFunctions())
		{
			string sig = it.second->externalSignature();
			GasEstimator::GasConsumption gas = GasEstimator::functionalEstimation(*items, sig);
			cout << "   " << sig << ":\t" << gas << endl;
		}
		if (contract.getFallbackFunction())
		{
			GasEstimator::GasConsumption gas = GasEstimator::functionalEstimation(*items, "INVALID");
			cout << "   fallback:\t" << gas << endl;
		}
		cout << "internal:" << endl;
		for (auto const& it: contract.getDefinedFunctions())
		{
			if (it->isPartOfExternalInterface() || it->isConstructor())
				continue;
			size_t entry = m_compiler->getFunctionEntryPoint(_contract, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = GasEstimator::functionalEstimation(*items, entry, *it);
			FunctionType type(*it);
			cout << "   " << it->getName() << "(";
			auto end = type.getParameterTypes().end();
			for (auto it = type.getParameterTypes().begin(); it != end; ++it)
				cout << (*it)->toString() << (it + 1 == end ? "" : ",");
			cout << "):\t" << gas << endl;
		}
	}
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
Compiles the given Solidity input files (or the standard input if none given) and
outputs the components specified in the options at standard output or in files in
the output directory, if specified.
Example: solc --bin -o /tmp/solcoutput contract.sol

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
			"output-dir,o",
			po::value<string>()->value_name("path"),
			"If given, creates one file per component and contract/file at the specified directory."
		)
		(
			"combined-json",
			po::value<string>()->value_name(boost::join(g_combinedJsonArgs, ",")),
			"Output a single json document containing the specified information."
		)
		(g_argGas.c_str(), "Print an estimate of the maximal gas usage for each function.");
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
		(g_argSolInterfaceStr.c_str(), "Solidity interface of the contracts.")
		(g_argSignatureHashes.c_str(), "Function signature hashes of the contracts.")
		(g_argNatspecUserStr.c_str(), "Natspec user documentation of all contracts.")
		(g_argNatspecDevStr.c_str(), "Natspec developer documentation of all contracts.");
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
		cmdLineParser.options(allOptions).positional(filesPositions).allow_unregistered();
		po::store(cmdLineParser.run(), m_args);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return false;
	}

	if (m_args.count("help"))
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
	if (!m_args.count("input-file"))
	{
		string s;
		while (!cin.eof())
		{
			getline(cin, s);
			m_sourceCodes["<stdin>"].append(s + '\n');
		}
	}
	else
		for (string const& infile: m_args["input-file"].as<vector<string>>())
		{
			auto path = boost::filesystem::path(infile);
			if (!boost::filesystem::exists(path))
			{
				cerr << "Skipping non existant input file \"" << infile << "\"" << endl;
				continue;
			}

			if (!boost::filesystem::is_regular_file(path))
			{
				cerr << "\"" << infile << "\" is not a valid file. Skipping" << endl;
				continue;
			}

			m_sourceCodes[infile] = dev::contentsString(infile);
		}

	m_compiler.reset(new CompilerStack(m_args.count(g_argAddStandard) > 0));
	try
	{
		for (auto const& sourceCode: m_sourceCodes)
			m_compiler->addSource(sourceCode.first, sourceCode.second);
		// TODO: Perhaps we should not compile unless requested
		bool optimize = m_args.count("optimize") > 0;
		unsigned runs = m_args["optimize-runs"].as<unsigned>();
		m_compiler->compile(optimize, runs);
	}
	catch (ParserError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Parser error", *m_compiler);
		return false;
	}
	catch (DeclarationError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Declaration error", *m_compiler);
		return false;
	}
	catch (TypeError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Type error", *m_compiler);
		return false;
	}
	catch (CompilerError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Compiler error", *m_compiler);
		return false;
	}
	catch (InternalCompilerError const& _exception)
	{
		cerr << "Internal compiler error during compilation:" << endl
			 << boost::diagnostic_information(_exception);
		return false;
	}
	catch (DocstringParsingError const& _exception)
	{
		cerr << "Documentation parsing error: " << *boost::get_error_info<errinfo_comment>(_exception) << endl;
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

	set<string> requests;
	boost::split(requests, m_args["combined-json"].as<string>(), boost::is_any_of(","));
	vector<string> contracts = m_compiler->getContractNames();

	if (!contracts.empty())
		output["contracts"] = Json::Value(Json::objectValue);
	for (string const& contractName: contracts)
	{
		Json::Value contractData(Json::objectValue);
		if (requests.count("interface"))
			contractData["interface"] = m_compiler->getSolidityInterface(contractName);
		if (requests.count("abi"))
			contractData["abi"] = m_compiler->getInterface(contractName);
		if (requests.count("bin"))
			contractData["bin"] = toHex(m_compiler->getBytecode(contractName));
		if (requests.count("clone-bin"))
			contractData["clone-bin"] = toHex(m_compiler->getCloneBytecode(contractName));
		if (requests.count("opcodes"))
			contractData["opcodes"] = eth::disassemble(m_compiler->getBytecode(contractName));
		if (requests.count("asm"))
		{
			ostringstream unused;
			contractData["asm"] = m_compiler->streamAssembly(unused, contractName, m_sourceCodes, true);
		}
		if (requests.count("devdoc"))
			contractData["devdoc"] = m_compiler->getMetadata(contractName, DocumentationType::NatspecDev);
		if (requests.count("userdoc"))
			contractData["userdoc"] = m_compiler->getMetadata(contractName, DocumentationType::NatspecUser);
		output["contracts"][contractName] = contractData;
	}

	if (requests.count("ast"))
	{
		output["sources"] = Json::Value(Json::objectValue);
		for (auto const& sourceCode: m_sourceCodes)
		{
			ASTJsonConverter converter(m_compiler->getAST(sourceCode.first));
			output["sources"][sourceCode.first] = Json::Value(Json::objectValue);
			output["sources"][sourceCode.first]["AST"] = converter.json();
		}
	}
	cout << Json::FastWriter().write(output) << endl;
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
			asts.push_back(&m_compiler->getAST(sourceCode.first));
		map<ASTNode const*, eth::GasMeter::GasConsumption> gasCosts;
		if (m_compiler->getRuntimeAssemblyItems())
			gasCosts = GasEstimator::breakToStatementLevel(
				GasEstimator::structuralEstimation(*m_compiler->getRuntimeAssemblyItems(), asts),
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
					ASTPrinter printer(m_compiler->getAST(sourceCode.first), sourceCode.second);
					printer.print(data);
				}
				else
				{
					ASTJsonConverter converter(m_compiler->getAST(sourceCode.first));
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
						m_compiler->getAST(sourceCode.first),
						sourceCode.second,
						gasCosts
					);
					printer.print(cout);
				}
				else
				{
					ASTJsonConverter converter(m_compiler->getAST(sourceCode.first));
					converter.print(cout);
				}
			}
		}
	}
}

void CommandLineInterface::actOnInput()
{
	handleCombinedJSON();

	// do we need AST output?
	handleAst(g_argAstStr);
	handleAst(g_argAstJson);

	vector<string> contracts = m_compiler->getContractNames();
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
		handleMeta(DocumentationType::ABIInterface, contract);
		handleMeta(DocumentationType::ABISolidityInterface, contract);
		handleMeta(DocumentationType::NatspecDev, contract);
		handleMeta(DocumentationType::NatspecUser, contract);
	} // end of contracts iteration
}

}
}
