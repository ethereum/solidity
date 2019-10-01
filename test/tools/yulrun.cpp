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
 * Yul interpreter.
 */

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>

#include <boost/program_options.hpp>

#include <string>
#include <memory>
#include <iostream>

using namespace std;
using namespace langutil;
using namespace yul;
using namespace dev;
using namespace yul::test;

namespace po = boost::program_options;

namespace
{

void printErrors(ErrorList const& _errors)
{
	for (auto const& error: _errors)
		SourceReferenceFormatter(cout).printErrorInformation(*error);
}

pair<shared_ptr<Block>, shared_ptr<AsmAnalysisInfo>> parse(string const& _source)
{
	AssemblyStack stack(
		langutil::EVMVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::OptimiserSettings::none()
	);
	if (stack.parseAndAnalyze("--INPUT--", _source))
	{
		yulAssert(stack.errors().empty(), "Parsed successfully but had errors.");
		return make_pair(stack.parserResult()->code, stack.parserResult()->analysisInfo);
	}
	else
	{
		printErrors(stack.errors());
		return {};
	}
}

void interpret(string const& _source)
{
	shared_ptr<Block> ast;
	shared_ptr<AsmAnalysisInfo> analysisInfo;
	tie(ast, analysisInfo) = parse(_source);
	if (!ast || !analysisInfo)
		return;

	InterpreterState state;
	state.maxTraceSize = 10000;
	Dialect const& dialect(EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion{}));
	Interpreter interpreter(state, dialect);
	try
	{
		interpreter(*ast);
	}
	catch (InterpreterTerminatedGeneric const&)
	{
	}

	state.dumpTraceAndState(cout);
}

}

int main(int argc, char** argv)
{
	po::options_description options(
		R"(yulrun, the Yul interpreter.
Usage: yulrun [Options] < input
Reads a single source from stdin, runs it and prints a trace of all side-effects.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		("help", "Show this help screen.")
		("input-file", po::value<vector<string>>(), "input file");
	po::positional_options_description filesPositions;
	filesPositions.add("input-file", -1);

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options).positional(filesPositions);
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
	{
		string input;

		if (arguments.count("input-file"))
			for (string path: arguments["input-file"].as<vector<string>>())
				input += readFileAsString(path);
		else
			input = readStandardInput();

		interpret(input);
	}

	return 0;
}
