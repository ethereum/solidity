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
 * @date 2014
 * Solidity command line interface.
 */
#include "CommandLineInterface.h"

#include <string>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>

#include "BuildInfo.h"
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/SourceReferenceFormatter.h>

using namespace std;
namespace po = boost::program_options;

namespace dev
{
namespace solidity
{

// LTODO: Maybe some argument class pairing names with
// extensions and other attributes would be a better choice here?
static string const g_argAbiStr         = "abi";
static string const g_argAsmStr         = "asm";
static string const g_argAstStr         = "ast";
static string const g_argBinaryStr      = "binary";
static string const g_argOpcodesStr     = "opcodes";
static string const g_argNatspecDevStr  = "natspec-dev";
static string const g_argNatspecUserStr = "natspec-user";

static void version()
{
	cout << "solc, the solidity complier commandline interface " << dev::Version << endl
		 << "  by Christian <c@ethdev.com> and Lefteris <lefteris@ethdev.com>, (c) 2014." << endl
		 << "Build: " << DEV_QUOTED(ETH_BUILD_PLATFORM) << "/" << DEV_QUOTED(ETH_BUILD_TYPE) << endl;
	exit(0);
}

static inline bool argToStdout(po::variables_map const& _args, string const& _name)
{
	return _args.count(_name) && _args[_name].as<OutputType>() != OutputType::FILE;
}

static bool needStdout(po::variables_map const& _args)
{
	return argToStdout(_args, g_argAbiStr) || argToStdout(_args, g_argNatspecUserStr) ||
		   argToStdout(_args, g_argNatspecDevStr) || argToStdout(_args, g_argAsmStr) ||
		   argToStdout(_args, g_argOpcodesStr) || argToStdout(_args, g_argBinaryStr);
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
	auto choice = m_args[g_argBinaryStr].as<OutputType>();
	if (outputToStdout(choice))
	{
		cout << "Binary: " << endl;
		cout << toHex(m_compiler.getBytecode(_contract)) << endl;
	}

	if (outputToFile(choice))
	{
		ofstream outFile(_contract + ".binary");
		outFile << toHex(m_compiler.getBytecode(_contract));
		outFile.close();
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	auto choice = m_args[g_argOpcodesStr].as<OutputType>();
	if (outputToStdout(choice))
	{
		cout << "Opcodes: " << endl;
		cout << eth::disassemble(m_compiler.getBytecode(_contract));
		cout << endl;
	}

	if (outputToFile(choice))
	{
		ofstream outFile(_contract + ".opcode");
		outFile << eth::disassemble(m_compiler.getBytecode(_contract));
		outFile.close();
	}
}

void CommandLineInterface::handleBytecode(string const& _contract)
{
	if (m_args.count(g_argOpcodesStr))
		handleOpcode(_contract);
	if (m_args.count(g_argBinaryStr))
		handleBinary(_contract);
}

void CommandLineInterface::handleJson(DocumentationType _type,
									  string const& _contract)
{
	std::string argName;
	std::string suffix;
	std::string title;
	switch(_type)
	{
	case DocumentationType::ABI_INTERFACE:
		argName = g_argAbiStr;
		suffix = ".abi";
		title = "Contract ABI";
		break;
	case DocumentationType::NATSPEC_USER:
		argName = "g_argNatspecUserStr";
		suffix = ".docuser";
		title = "User Documentation";
		break;
	case DocumentationType::NATSPEC_DEV:
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
			cout << m_compiler.getJsonDocumentation(_contract, _type);
		}

		if (outputToFile(choice))
		{
			ofstream outFile(_contract + suffix);
			outFile << m_compiler.getJsonDocumentation(_contract, _type);
			outFile.close();
		}
	}
}

bool CommandLineInterface::parseArguments(int argc, char** argv)
{
#define OUTPUT_TYPE_STR "Legal values:\n"				\
		"\tstdout: Print it to standard output\n"		\
		"\tfile: Print it to a file with same name\n"	\
		"\tboth: Print both to a file and the stdout\n"
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show help message and exit")
		("version", "Show version and exit")
		("optimize", po::value<bool>()->default_value(false), "Optimize bytecode for size")
		("input-file", po::value<vector<string>>(), "input file")
		(g_argAstStr.c_str(), po::value<OutputType>(),
		 "Request to output the AST of the contract. " OUTPUT_TYPE_STR)
		(g_argAsmStr.c_str(), po::value<OutputType>(),
		 "Request to output the EVM assembly of the contract. " OUTPUT_TYPE_STR)
		(g_argOpcodesStr.c_str(), po::value<OutputType>(),
		 "Request to output the Opcodes of the contract. " OUTPUT_TYPE_STR)
		(g_argBinaryStr.c_str(), po::value<OutputType>(),
		 "Request to output the contract in binary (hexadecimal). " OUTPUT_TYPE_STR)
		(g_argAbiStr.c_str(), po::value<OutputType>(),
		 "Request to output the contract's ABI interface. "  OUTPUT_TYPE_STR)
		(g_argNatspecUserStr.c_str(), po::value<OutputType>(),
		 "Request to output the contract's Natspec user documentation. " OUTPUT_TYPE_STR)
		(g_argNatspecDevStr.c_str(), po::value<OutputType>(),
		 "Request to output the contract's Natspec developer documentation. " OUTPUT_TYPE_STR);
#undef OUTPUT_TYPE_STR

	// All positional options should be interpreted as input files
	po::positional_options_description p;
	p.add("input-file", -1);

	// parse the compiler arguments
	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).allow_unregistered().run(), m_args);
	}
	catch (po::error const& exception)
	{
		cout << exception.what() << endl;
		return false;
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
			m_sourceCodes["<stdin>"].append(s);
		}
	}
	else
		for (string const& infile: m_args["input-file"].as<vector<string>>())
		{
			auto path = boost::filesystem::path(infile);
			if (!boost::filesystem::exists(path))
			{
				cout << "Skipping non existant input file \"" << infile << "\"" << endl;
				continue;
			}

			if (!boost::filesystem::is_regular_file(path))
			{
				cout << "\"" << infile << "\" is not a valid file. Skipping" << endl;
				continue;
			}

			m_sourceCodes[infile] = asString(dev::contents(infile));
		}

	try
	{
		for (auto const& sourceCode: m_sourceCodes)
			m_compiler.addSource(sourceCode.first, sourceCode.second);
		// TODO: Perhaps we should not compile unless requested
		m_compiler.compile(m_args["optimize"].as<bool>());
	}
	catch (ParserError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Parser error", m_compiler);
		return false;
	}
	catch (DeclarationError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Declaration error", m_compiler);
		return false;
	}
	catch (TypeError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Type error", m_compiler);
		return false;
	}
	catch (CompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Compiler error", m_compiler);
		return false;
	}
	catch (InternalCompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Internal compiler error", m_compiler);
		return false;
	}
	catch (Exception const& exception)
	{
		cerr << "Exception during compilation: " << boost::diagnostic_information(exception) << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Unknown exception during compilation." << endl;
		return false;
	}

	return true;
}

void CommandLineInterface::actOnInput()
{
	// do we need AST output?
	if (m_args.count(g_argAstStr))
	{
		auto choice = m_args[g_argAstStr].as<OutputType>();
		if (outputToStdout(choice))
		{
			cout << "Syntax trees:" << endl << endl;
			for (auto const& sourceCode: m_sourceCodes)
			{
				cout << endl << "======= " << sourceCode.first << " =======" << endl;
				ASTPrinter printer(m_compiler.getAST(sourceCode.first), sourceCode.second);
				printer.print(cout);
			}
		}

		if (outputToFile(choice))
		{
			for (auto const& sourceCode: m_sourceCodes)
			{
				boost::filesystem::path p(sourceCode.first);
				ofstream outFile(p.stem().string() + ".ast");
				ASTPrinter printer(m_compiler.getAST(sourceCode.first), sourceCode.second);
				printer.print(outFile);
				outFile.close();
			}
		}
	}

	vector<string> contracts = m_compiler.getContractNames();
	for (string const& contract: contracts)
	{
		if (needStdout(m_args))
			cout << endl << "======= " << contract << " =======" << endl;

		// do we need EVM assembly?
		if (m_args.count(g_argAsmStr))
		{
			auto choice = m_args[g_argAsmStr].as<OutputType>();
			if (outputToStdout(choice))
			{
				cout << "EVM assembly:" << endl;
				m_compiler.streamAssembly(cout, contract);
			}

			if (outputToFile(choice))
			{
				ofstream outFile(contract + ".evm");
				m_compiler.streamAssembly(outFile, contract);
				outFile.close();
			}
		}

		handleBytecode(contract);
		handleJson(DocumentationType::ABI_INTERFACE, contract);
		handleJson(DocumentationType::NATSPEC_DEV, contract);
		handleJson(DocumentationType::NATSPEC_USER, contract);
	} // end of contracts iteration
}

}
}
