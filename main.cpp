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

#include <boost/program_options.hpp>

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

int main(int argc, char** argv)
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "Show help message and exit")
	("version", "Show version and exit")
	("optimize", po::value<bool>()->default_value(false), "Optimize bytecode for size")
	("input-file", po::value<vector<string>>(), "input file");

	// All positional options should be interpreted as input files
	po::positional_options_description p;
	p.add("input-file", -1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc;
		return 0;
	}

	if (vm.count("version")) {
		version();
		return 0;
	}
	map<string, string> sourceCodes;
	if (!vm.count("input-file"))
	{
		string s;
		while (!cin.eof())
		{
			getline(cin, s);
			sourceCodes["<stdin>"].append(s);
		}
	}
	else
		for (string const& infile: vm["input-file"].as<vector<string>>())
			sourceCodes[infile] = asString(dev::contents(infile));

	CompilerStack compiler;
	try
	{
		for (auto const& sourceCode: sourceCodes)
			compiler.addSource(sourceCode.first, sourceCode.second);
		compiler.compile( vm["optimize"].as<bool>());
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

	cout << "Syntax trees:" << endl << endl;
	for (auto const& sourceCode: sourceCodes)
	{
		cout << endl << "======= " << sourceCode.first << " =======" << endl;
		ASTPrinter printer(compiler.getAST(sourceCode.first), sourceCode.second);
		printer.print(cout);
	}
	vector<string> contracts = compiler.getContractNames();
	cout << endl << "Contracts:" << endl;
	for (string const& contract: contracts)
	{
		cout << endl << "======= " << contract << " =======" << endl
			 << "EVM assembly:" << endl;
		compiler.streamAssembly(cout, contract);
		cout << "Opcodes:" << endl
			 << eth::disassemble(compiler.getBytecode(contract)) << endl
			 << "Binary: " << toHex(compiler.getBytecode(contract)) << endl
			 << "Interface specification: " << compiler.getJsonDocumentation(contract, DocumentationType::ABI_INTERFACE) << endl
			 << "Natspec user documentation: " << compiler.getJsonDocumentation(contract, DocumentationType::NATSPEC_USER) << endl
			 << "Natspec developer documentation: " << compiler.getJsonDocumentation(contract, DocumentationType::NATSPEC_DEV) << endl;
	}

	return 0;
}
