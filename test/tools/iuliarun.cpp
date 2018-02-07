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
 * IULIA interpreter.
 */

#include <libjulia/interpreter/Interpreter.h>

#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmParser.h>

#include <libsolidity/parsing/Scanner.h>

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>

#include <libdevcore/CommonIO.h>

#include <boost/program_options.hpp>

#include <string>
#include <iostream>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;

namespace po = boost::program_options;

namespace
{

void printErrors(ErrorList const& _errors, Scanner const& _scanner)
{
	for (auto const& error: _errors)
		SourceReferenceFormatter(cout, [&](std::string const&) -> Scanner const& { return _scanner; })
			.printExceptionInformation(
				*error,
				(error->type() == Error::Type::Warning) ? "Warning" : "Error"
			);
}

bool parse(string const& _source, shared_ptr<Block>& parserResult, shared_ptr<assembly::AsmAnalysisInfo>& analysisInfo, bool _julia)
{
	auto flavour = _julia ? assembly::AsmFlavour::IULIA : assembly::AsmFlavour::Strict;
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = make_shared<Scanner>(CharStream(_source), "");
	parserResult = assembly::Parser(errorReporter, flavour).parse(scanner, false);
	if (parserResult)
	{
		solAssert(errorReporter.errors().empty(), "");
		analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
		assembly::AsmAnalyzer analyzer(*analysisInfo, errorReporter, EVMVersion(), boost::none, flavour);
		if (analyzer.analyze(*parserResult))
		{
			solAssert(errorReporter.errors().empty(), "");
			return true;
		}
	}
	printErrors(errors, *scanner);
	return false;
}

void interpret(bool /*_optimize*/)
{
	string source = readStandardInput();
	shared_ptr<Block> ast;
	shared_ptr<assembly::AsmAnalysisInfo> analysisInfo;
	if (!parse(source, ast, analysisInfo, false))
		return;

	InterpreterState state;
	Interpreter interpreter(state);
	try
	{
		interpreter(*ast);
	}
	catch (InterpreterTerminated const&)
	{
	}

	cout << "Trace:" << endl;
	for (auto const& line: interpreter.trace())
		cout << "  " << line << endl;
	cout << "Memory dump:" << endl;
	for (size_t i = 0; i < state.memory.size(); i += 0x20)
		cout << "  " << std::hex << std::setw(4) << i << ": " << toHex(bytesConstRef(state.memory.data() + i, 0x20)) << endl;
	cout << "Storage dump:" << endl;
	for (auto const& slot: state.storage)
		cout << "  " << slot.first.hex() << ": " << slot.second.hex() << endl;
}

}

int main(int argc, char** argv)
{
	po::options_description options(
		R"(iuliarun, the IULIA interpreter.
Usage: iuliarun [Options] < input
Reads a single source from stdin, runs it and prints a trace of all side-effects.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		("help", "Show this help screen.")
		/*(
			"optimize",
			"Apply optimizations before interpretation."
		)*/;

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options);
		po::store(cmdLineParser.run(), arguments);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return 1;
	}

	if (arguments.count("help"))
		cout << options;
	else
		interpret(arguments.count("optimize"));

	return 0;
}
