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
 * Interactive yul optimizer
 */

#include <libsolutil/CommonIO.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libsolidity/parsing/Parser.h>
#include <libyul/AsmData.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Object.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/Suite.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libsolutil/JSON.h>

#include <boost/program_options.hpp>

#include <cassert>
#include <string>
#include <sstream>
#include <iostream>
#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::yul;

namespace po = boost::program_options;

class YulOpti
{
public:
	void printErrors()
	{
		SourceReferenceFormatter formatter(cout);

		for (auto const& error: m_errors)
			formatter.printErrorInformation(*error);
	}

	bool parse(string const& _input)
	{
		ErrorReporter errorReporter(m_errors);
		shared_ptr<Scanner> scanner = make_shared<Scanner>(CharStream(_input, ""));
		m_ast = yul::Parser(errorReporter, m_dialect).parse(scanner, false);
		if (!m_ast || !errorReporter.errors().empty())
		{
			cout << "Error parsing source." << endl;
			printErrors();
			return false;
		}
		m_analysisInfo = make_shared<yul::AsmAnalysisInfo>();
		AsmAnalyzer analyzer(
			*m_analysisInfo,
			errorReporter,
			m_dialect
		);
		if (!analyzer.analyze(*m_ast) || !errorReporter.errors().empty())
		{
			cout << "Error analyzing source." << endl;
			printErrors();
			return false;
		}
		return true;
	}

	void runInteractive(string source)
	{
		bool disambiguated = false;
		while (true)
		{
			cout << "----------------------" << endl;
			cout << source << endl;
			if (!parse(source))
				return;
			set<YulString> reservedIdentifiers;
			if (!disambiguated)
			{
				*m_ast = std::get<yul::Block>(Disambiguator(m_dialect, *m_analysisInfo)(*m_ast));
				m_analysisInfo.reset();
				m_nameDispenser = make_shared<NameDispenser>(m_dialect, *m_ast, reservedIdentifiers);
				disambiguated = true;
			}
			cout << "(q)uit/(f)latten/(c)se/initialize var(d)ecls/(x)plit/(j)oin/(g)rouper/(h)oister/" << endl;
			cout << "  (e)xpr inline/(i)nline/(s)implify/varname c(l)eaner/(u)nusedprune/ss(a) transform/" << endl;
			cout << "  (r)edundant assign elim./re(m)aterializer/f(o)r-loop-init-rewriter/for-loop-condition-(I)nto-body/" << endl;
			cout << "  for-loop-condition-(O)ut-of-body/s(t)ructural simplifier/equi(v)alent function combiner/ssa re(V)erser/" << endl;
			cout << "  co(n)trol flow simplifier/stack com(p)ressor/(D)ead code eliminator/(L)oad resolver/" << endl;
			cout << "  (C)onditional simplifier/conditional (U)nsimplifier/loop-invariant code (M)otion?" << endl;
			cout.flush();
			int option = readStandardInputChar();
			cout << ' ' << char(option) << endl;

			OptimiserStepContext context{m_dialect, *m_nameDispenser, reservedIdentifiers};

			map<char, string> const& abbreviationMap = OptimiserSuite::stepAbbreviationToNameMap();
			assert(abbreviationMap.count('q') == 0 && "We have chosen 'q' for quit");
			assert(abbreviationMap.count('p') == 0 && "We have chosen 'p' for StackCompressor");

			auto abbreviationAndName = abbreviationMap.find(option);
			if (abbreviationAndName != abbreviationMap.end())
			{
				OptimiserStep const& step = *OptimiserSuite::allSteps().at(abbreviationAndName->second);
				step.run(context, *m_ast);
			}
			else switch (option)
			{
			case 'q':
				return;
			case 'l':
				VarNameCleaner::run(context, *m_ast);
				// VarNameCleaner destroys the unique names guarantee of the disambiguator.
				disambiguated = false;
				break;
			case 'p':
			{
				Object obj;
				obj.code = m_ast;
				StackCompressor::run(m_dialect, obj, true, 16);
				break;
			}
			default:
				cout << "Unknown option." << endl;
			}
			source = AsmPrinter{m_dialect}(*m_ast);
		}
	}

private:
	ErrorList m_errors;
	shared_ptr<yul::Block> m_ast;
	Dialect const& m_dialect{EVMDialect::strictAssemblyForEVMObjects(EVMVersion{})};
	shared_ptr<AsmAnalysisInfo> m_analysisInfo;
	shared_ptr<NameDispenser> m_nameDispenser;
};

int main(int argc, char** argv)
{
	po::options_description options(
		R"(yulopti, yul optimizer exploration tool.
Usage: yulopti [Options] <file>
Reads <file> as yul code and applies optimizer steps to it,
interactively read from stdin.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		(
			"input-file",
			po::value<string>(),
			"input file"
		)
		("help", "Show this help screen.");

	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add("input-file", 1);

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

	string input;
	if (arguments.count("input-file"))
		YulOpti{}.runInteractive(readFileAsString(arguments["input-file"].as<string>()));
	else
		cout << options;

	return 0;
}
