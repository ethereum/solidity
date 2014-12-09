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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity commandline compiler.
 */

#include <string>
#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
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
using namespace dev;
using namespace solidity;
namespace po = boost::program_options;

void version()
{
	cout << "solc, the solidity complier commandline interface " << dev::Version << endl
		 << "  by Christian <c@ethdev.com> and Lefteris <lefteris@ethdev.com>, (c) 2014." << endl
		 << "Build: " << DEV_QUOTED(ETH_BUILD_PLATFORM) << "/" << DEV_QUOTED(ETH_BUILD_TYPE) << endl;
	exit(0);
}

enum class OutputType
{
	STDOUT,
	FILE,
	BOTH
};

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

static void handleBytecode(po::variables_map const& vm,
						   string const& _argName,
						   string const& _title,
						   string const& _contract,
						   CompilerStack& _compiler,
						   string const& _suffix)
{
	if (vm.count(_argName))
	{
		auto choice = vm[_argName].as<OutputType>();
		if (outputToStdout(choice))
		{
			cout << _title << endl;
			if (_suffix == "opcodes")
				cout << _compiler.getBytecode(_contract) << endl;
			else
				cout << toHex(_compiler.getBytecode(_contract)) << endl;
		}

		if (outputToFile(choice))
		{
			ofstream outFile(_contract + _suffix);
			if (_suffix == "opcodes")
				outFile << _compiler.getBytecode(_contract);
			else
				outFile << toHex(_compiler.getBytecode(_contract));
			outFile.close();
		}
	}
}

static void handleJson(po::variables_map const& _args,
					   DocumentationType _type,
					   string const& _contract,
					   CompilerStack&_compiler)
{
	std::string argName;
	std::string suffix;
	std::string title;
	switch(_type)
	{
	case DocumentationType::ABI_INTERFACE:
		argName = "abi";
		suffix = ".abi";
		title = "Contract ABI";
		break;
	case DocumentationType::NATSPEC_USER:
		argName = "natspec-user";
		suffix = ".docuser";
		title = "User Documentation";
		break;
	case DocumentationType::NATSPEC_DEV:
		argName = "natspec-dev";
		suffix = ".docdev";
		title = "Developer Documentation";
		break;
	default:
		// should never happen
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown documentation _type"));
	}

	if (_args.count(argName))
	{
		auto choice = _args[argName].as<OutputType>();
		if (outputToStdout(choice))
		{
			cout << title << endl;
			cout << _compiler.getJsonDocumentation(_contract, _type);
		}

		if (outputToFile(choice))
		{
			ofstream outFile(_contract + suffix);
			outFile << _compiler.getJsonDocumentation(_contract, _type);
			outFile.close();
		}
	}
}

static inline bool argToStdout(po::variables_map const& _args, const char* _name)
{
	return _args.count(_name) && _args[_name].as<OutputType>() != OutputType::FILE;
}

static bool needStdout(po::variables_map const& _args)
{
	return argToStdout(_args, "abi") || argToStdout(_args, "natspec-user") || argToStdout(_args, "natspec-dev") ||
		   argToStdout(_args, "asm") || argToStdout(_args, "opcodes") || argToStdout(_args, "binary");
}

int main(int argc, char** argv)
{
#define OUTPUT_TYPE_STR "Legal values:\n"               \
        "\tstdout: Print it to standard output\n"       \
        "\tfile: Print it to a file with same name\n"   \
        "\tboth: Print both to a file and the stdout\n"
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
        ("help", "Show help message and exit")
        ("version", "Show version and exit")
        ("optimize", po::value<bool>()->default_value(false), "Optimize bytecode for size")
        ("input-file", po::value<vector<string>>(), "input file")
        ("ast", po::value<OutputType>(),
         "Request to output the AST of the contract. " OUTPUT_TYPE_STR)
        ("asm", po::value<OutputType>(),
         "Request to output the EVM assembly of the contract. "  OUTPUT_TYPE_STR)
        ("opcodes", po::value<OutputType>(),
         "Request to output the Opcodes of the contract. "  OUTPUT_TYPE_STR)
        ("binary", po::value<OutputType>(),
         "Request to output the contract in binary (hexadecimal). "  OUTPUT_TYPE_STR)
        ("abi", po::value<OutputType>(),
         "Request to output the contract's ABI interface. "  OUTPUT_TYPE_STR)
        ("natspec-user", po::value<OutputType>(),
         "Request to output the contract's Natspec user documentation. "  OUTPUT_TYPE_STR)
        ("natspec-dev", po::value<OutputType>(),
         "Request to output the contract's Natspec developer documentation. "  OUTPUT_TYPE_STR);
#undef OUTPUT_TYPE_STR

	// All positional options should be interpreted as input files
	po::positional_options_description p;
	p.add("input-file", -1);

	// parse the compiler arguments
	po::variables_map args;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).allow_unregistered().run(), args);
	}
	catch (po::error const& exception)
	{
		cout << exception.what() << endl;
		return -1;
	}
	po::notify(args);

	if (args.count("help"))
    {
		cout << desc;
		return 0;
	}

	if (args.count("version"))
    {
		version();
		return 0;
	}

	// create a map of input files to source code strings
	map<string, string> sourceCodes;
	if (!args.count("input-file"))
	{
		string s;
		while (!cin.eof())
		{
			getline(cin, s);
			sourceCodes["<stdin>"].append(s);
		}
	}
	else
		for (string const& infile: args["input-file"].as<vector<string>>())
			sourceCodes[infile] = asString(dev::contents(infile));

	// parse the files
	CompilerStack compiler;
	try
	{
		for (auto const& sourceCode: sourceCodes)
			compiler.addSource(sourceCode.first, sourceCode.second);
        // TODO: Perhaps we should not compile unless requested
		compiler.compile(args["optimize"].as<bool>());
	}
	catch (ParserError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Parser error", compiler);
		return -1;
	}
	catch (DeclarationError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Declaration error", compiler);
		return -1;
	}
	catch (TypeError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Type error", compiler);
		return -1;
	}
	catch (CompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Compiler error", compiler);
		return -1;
	}
	catch (InternalCompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, exception, "Internal compiler error", compiler);
		return -1;
	}
	catch (Exception const& exception)
	{
		cerr << "Exception during compilation: " << boost::diagnostic_information(exception) << endl;
		return -1;
	}
	catch (...)
	{
		cerr << "Unknown exception during compilation." << endl;
		return -1;
	}


	/* -- act depending on the provided arguments -- */

	// do we need AST output?
	if (args.count("ast"))
	{
		auto choice = args["ast"].as<OutputType>();
		if (outputToStdout(choice))
		{
			cout << "Syntax trees:" << endl << endl;
			for (auto const& sourceCode: sourceCodes)
			{
				cout << endl << "======= " << sourceCode.first << " =======" << endl;
				ASTPrinter printer(compiler.getAST(sourceCode.first), sourceCode.second);
				printer.print(cout);
			}
		}

		if (outputToFile(choice))
		{
			for (auto const& sourceCode: sourceCodes)
			{
				boost::filesystem::path p(sourceCode.first);
				ofstream outFile(p.stem().string() + ".ast");
				ASTPrinter printer(compiler.getAST(sourceCode.first), sourceCode.second);
				printer.print(outFile);
				outFile.close();
			}
		}
	}

	vector<string> contracts = compiler.getContractNames();
	for (string const& contract: contracts)
	{
		if (needStdout(args))
			cout << endl << "======= " << contract << " =======" << endl;

		// do we need EVM assembly?
		if (args.count("asm"))
		{
			auto choice = args["asm"].as<OutputType>();
            if (outputToStdout(choice))
			{
				cout << "EVM assembly:" << endl;
				compiler.streamAssembly(cout, contract);
			}

            if (outputToFile(choice))
			{
				ofstream outFile(contract + ".evm");
				compiler.streamAssembly(outFile, contract);
				outFile.close();
			}
		}

		handleBytecode(args, "opcodes", "Opcodes:", contract, compiler, ".opcodes");
		handleBytecode(args, "binary", "Binary:", contract, compiler, ".binary");
		handleJson(args, DocumentationType::ABI_INTERFACE, contract, compiler);
		handleJson(args, DocumentationType::NATSPEC_DEV, contract, compiler);
		handleJson(args, DocumentationType::NATSPEC_USER, contract, compiler);
	} // end of contracts iteration

	return 0;
}
