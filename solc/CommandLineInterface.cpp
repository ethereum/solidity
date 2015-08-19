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

static string const g_argAbiStr = "json-abi";
static string const g_argSolAbiStr = "sol-abi";
static string const g_argSignatureHashes = "hashes";
static string const g_argGas = "gas";
static string const g_argAsmStr = "asm";
static string const g_argAsmJsonStr = "asm-json";
static string const g_argAstStr = "ast";
static string const g_argAstJson = "ast-json";
static string const g_argBinaryStr = "binary";
static string const g_argCloneBinaryStr = "clone-binary";
static string const g_argOpcodesStr = "opcodes";
static string const g_argNatspecDevStr = "natspec-dev";
static string const g_argNatspecUserStr = "natspec-user";
static string const g_argAddStandard = "add-std";

/// Possible arguments to for --combined-json
static set<string> const g_combinedJsonArgs{
	"binary",
	"clone-binary",
	"opcodes",
	"json-abi",
	"sol-abi",
	"asm",
	"ast",
	"natspec-user",
	"natspec-dev"
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
	return _args.count(_name) && _args[_name].as<OutputType>() != OutputType::FILE;
}

static bool needsHumanTargetedStdout(po::variables_map const& _args)
{

	return
		_args.count(g_argGas) ||
		humanTargetedStdout(_args, g_argAbiStr) ||
		humanTargetedStdout(_args, g_argSolAbiStr) ||
		humanTargetedStdout(_args, g_argSignatureHashes) ||
		humanTargetedStdout(_args, g_argNatspecUserStr) ||
		humanTargetedStdout(_args, g_argAstJson) ||
		humanTargetedStdout(_args, g_argNatspecDevStr) ||
		humanTargetedStdout(_args, g_argAsmStr) ||
		humanTargetedStdout(_args, g_argAsmJsonStr) ||
		humanTargetedStdout(_args, g_argOpcodesStr) ||
		humanTargetedStdout(_args, g_argBinaryStr) ||
		humanTargetedStdout(_args, g_argCloneBinaryStr);
}

static inline bool outputToFile(OutputType type)
{
	return type == OutputType::FILE || type == OutputType::BOTH;
}

static inline bool outputToStdout(OutputType type)
{
	return type == OutputType::STDOUT || type == OutputType::BOTH;
}

static std::istream& operator>>(std::istream& _in, OutputType& io_output)
{
	std::string token;
	_in >> token;
	if (token == "stdout")
		io_output = OutputType::STDOUT;
	else if (token == "file")
		io_output = OutputType::FILE;
	else if (token == "both")
		io_output = OutputType::BOTH;
	else
		throw boost::program_options::invalid_option_value(token);
	return _in;
}

void CommandLineInterface::handleBinary(string const& _contract)
{
	if (m_args.count(g_argBinaryStr))
	{
		if (outputToStdout(m_args[g_argBinaryStr].as<OutputType>()))
		{
			cout << "Binary: " << endl;
			cout << toHex(m_compiler->getBytecode(_contract)) << endl;
		}
		if (outputToFile(m_args[g_argBinaryStr].as<OutputType>()))
		{
			ofstream outFile(_contract + ".binary");
			outFile << toHex(m_compiler->getBytecode(_contract));
			outFile.close();
		}
	}
	if (m_args.count(g_argCloneBinaryStr))
	{
		if (outputToStdout(m_args[g_argCloneBinaryStr].as<OutputType>()))
		{
			cout << "Clone Binary: " << endl;
			cout << toHex(m_compiler->getCloneBytecode(_contract)) << endl;
		}
		if (outputToFile(m_args[g_argCloneBinaryStr].as<OutputType>()))
		{
			ofstream outFile(_contract + ".clone_binary");
			outFile << toHex(m_compiler->getCloneBytecode(_contract));
			outFile.close();
		}
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	auto choice = m_args[g_argOpcodesStr].as<OutputType>();
	if (outputToStdout(choice))
	{
		cout << "Opcodes: " << endl;
		cout << eth::disassemble(m_compiler->getBytecode(_contract));
		cout << endl;
	}

	if (outputToFile(choice))
	{
		ofstream outFile(_contract + ".opcode");
		outFile << eth::disassemble(m_compiler->getBytecode(_contract));
		outFile.close();
	}
}

void CommandLineInterface::handleBytecode(string const& _contract)
{
	if (m_args.count(g_argOpcodesStr))
		handleOpcode(_contract);
	if (m_args.count(g_argBinaryStr) || m_args.count(g_argCloneBinaryStr))
		handleBinary(_contract);
}

void CommandLineInterface::handleSignatureHashes(string const& _contract)
{
	if (!m_args.count(g_argSignatureHashes))
		return;

	string out;
	for (auto const& it: m_compiler->getContractDefinition(_contract).getInterfaceFunctions())
		out += toHex(it.first.ref()) + ": " + it.second->externalSignature() + "\n";

	auto choice = m_args[g_argSignatureHashes].as<OutputType>();
	if (outputToStdout(choice))
		cout << "Function signatures: " << endl << out;

	if (outputToFile(choice))
	{
		ofstream outFile(_contract + ".signatures");
		outFile << out;
		outFile.close();
	}
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
		argName = g_argSolAbiStr;
		suffix = ".sol";
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
		auto choice = m_args[argName].as<OutputType>();
		if (outputToStdout(choice))
		{
			cout << title << endl;
			cout << m_compiler->getMetadata(_contract, _type) << endl;
		}

		if (outputToFile(choice))
		{
			ofstream outFile(_contract + suffix);
			outFile << m_compiler->getMetadata(_contract, _type);
			outFile.close();
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

bool CommandLineInterface::parseArguments(int argc, char** argv)
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show help message and exit")
		("version", "Show version and exit")
		("optimize", po::value<bool>()->default_value(false), "Optimize bytecode")
		("optimize-runs", po::value<unsigned>()->default_value(200), "Estimated number of contract runs for optimizer.")
		("add-std", po::value<bool>()->default_value(false), "Add standard contracts")
		("input-file", po::value<vector<string>>(), "input file")
		(
			"combined-json",
			po::value<string>()->value_name(boost::join(g_combinedJsonArgs, ",")),
			"Output a single json document containing the specified information, can be combined."
		)
		(g_argAstStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the AST of the contract.")
		(g_argAstJson.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the AST of the contract in JSON format.")
		(g_argAsmStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the EVM assembly of the contract.")
		(g_argAsmJsonStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the EVM assembly of the contract in JSON format.")
		(g_argOpcodesStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the Opcodes of the contract.")
		(g_argBinaryStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract in binary (hexadecimal).")
		(g_argCloneBinaryStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the clone contract in binary (hexadecimal).")
		(g_argAbiStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract's JSON ABI interface.")
		(g_argSolAbiStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract's Solidity ABI interface.")
		(g_argSignatureHashes.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract's functions' signature hashes.")
		(g_argGas.c_str(),
			"Request to output an estimate for each function's maximal gas usage.")
		(g_argNatspecUserStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract's Natspec user documentation.")
		(g_argNatspecDevStr.c_str(), po::value<OutputType>()->value_name("stdout|file|both"),
			"Request to output the contract's Natspec developer documentation.");

	// All positional options should be interpreted as input files
	po::positional_options_description p;
	p.add("input-file", -1);

	// parse the compiler arguments
	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).allow_unregistered().run(), m_args);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
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

	m_compiler.reset(new CompilerStack(m_args["add-std"].as<bool>()));
	try
	{
		for (auto const& sourceCode: m_sourceCodes)
			m_compiler->addSource(sourceCode.first, sourceCode.second);
		// TODO: Perhaps we should not compile unless requested
		bool optimize = m_args["optimize"].as<bool>();
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
		if (requests.count("sol-abi"))
			contractData["sol-abi"] = m_compiler->getSolidityInterface(contractName);
		if (requests.count("json-abi"))
			contractData["json-abi"] = m_compiler->getInterface(contractName);
		if (requests.count("binary"))
			contractData["binary"] = toHex(m_compiler->getBytecode(contractName));
		if (requests.count("clone-binary"))
			contractData["clone-binary"] = toHex(m_compiler->getCloneBytecode(contractName));
		if (requests.count("opcodes"))
			contractData["opcodes"] = eth::disassemble(m_compiler->getBytecode(contractName));
		if (requests.count("asm"))
		{
			ostringstream unused;
			contractData["asm"] = m_compiler->streamAssembly(unused, contractName, m_sourceCodes, true);
		}
		if (requests.count("natspec-dev"))
			contractData["natspec-dev"] = m_compiler->getMetadata(contractName, DocumentationType::NatspecDev);
		if (requests.count("natspec-user"))
			contractData["natspec-user"] = m_compiler->getMetadata(contractName, DocumentationType::NatspecUser);
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

		auto choice = m_args[_argStr].as<OutputType>();
		if (outputToStdout(choice))
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

		if (outputToFile(choice))
		{
			for (auto const& sourceCode: m_sourceCodes)
			{
				boost::filesystem::path p(sourceCode.first);
				ofstream outFile(p.stem().string() + ".ast");
				if (_argStr == g_argAstStr)
				{
					ASTPrinter printer(m_compiler->getAST(sourceCode.first), sourceCode.second);
					printer.print(outFile);
				}
				else
				{
					ASTJsonConverter converter(m_compiler->getAST(sourceCode.first));
					converter.print(outFile);
				}
				outFile.close();
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
			auto choice = m_args.count(g_argAsmStr) ? m_args[g_argAsmStr].as<OutputType>() : m_args[g_argAsmJsonStr].as<OutputType>();
			if (outputToStdout(choice))
			{
				cout << "EVM assembly:" << endl;
				m_compiler->streamAssembly(cout, contract, m_sourceCodes, m_args.count(g_argAsmJsonStr));
			}

			if (outputToFile(choice))
			{
				ofstream outFile(contract + (m_args.count(g_argAsmJsonStr) ? "_evm.json" : ".evm"));
				m_compiler->streamAssembly(outFile, contract, m_sourceCodes, m_args.count(g_argAsmJsonStr));
				outFile.close();
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
