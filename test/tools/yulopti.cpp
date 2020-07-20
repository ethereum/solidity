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
// SPDX-License-Identifier: GPL-3.0
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
		SourceReferenceFormatter formatter(cerr);

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
			cerr << "Error parsing source." << endl;
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
			cerr << "Error analyzing source." << endl;
			printErrors();
			return false;
		}
		return true;
	}

	void printUsageBanner(
		map<char, string> const& _optimizationSteps,
		map<char, string> const& _extraOptions,
		size_t _columns
	)
	{
		auto hasShorterString = [](auto const& a, auto const& b){ return a.second.size() < b.second.size(); };
		size_t longestDescriptionLength = max(
			max_element(_optimizationSteps.begin(), _optimizationSteps.end(), hasShorterString)->second.size(),
			max_element(_extraOptions.begin(), _extraOptions.end(), hasShorterString)->second.size()
		);

		size_t index = 0;
		auto printPair = [&](auto const& optionAndDescription)
		{
			cout << optionAndDescription.first << ": ";
			cout << setw(longestDescriptionLength) << setiosflags(ios::left);
			cout << optionAndDescription.second << " ";

			++index;
			if (index % _columns == 0)
				cout << endl;
		};

		for (auto const& optionAndDescription: _extraOptions)
		{
			yulAssert(
				_optimizationSteps.count(optionAndDescription.first) == 0,
				"ERROR: Conflict between yulopti controls and Yul optimizer step abbreviations.\n"
				"Character '" + string(1, optionAndDescription.first) + "' is assigned to both " +
				optionAndDescription.second + " and " + _optimizationSteps.at(optionAndDescription.first) + " step.\n"
				"This is most likely caused by someone adding a new step abbreviation to "
				"OptimiserSuite::stepNameToAbbreviationMap() and not realizing that it's used by yulopti.\n"
				"Please update the code to use a different character and recompile yulopti."
			);
			printPair(optionAndDescription);
		}

		for (auto const& abbreviationAndName: _optimizationSteps)
			printPair(abbreviationAndName);
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
			map<char, string> const& abbreviationMap = OptimiserSuite::stepAbbreviationToNameMap();
			map<char, string> const& extraOptions = {
				{'#', "quit"},
				{',', "VarNameCleaner"},
				{';', "StackCompressor"},
			};

			printUsageBanner(abbreviationMap, extraOptions, 4);
			cout << "? ";
			cout.flush();
			int option = readStandardInputChar();
			cout << ' ' << char(option) << endl;

			OptimiserStepContext context{m_dialect, *m_nameDispenser, reservedIdentifiers};

			auto abbreviationAndName = abbreviationMap.find(option);
			if (abbreviationAndName != abbreviationMap.end())
			{
				OptimiserStep const& step = *OptimiserSuite::allSteps().at(abbreviationAndName->second);
				step.run(context, *m_ast);
			}
			else switch (option)
			{
			case '#':
				return;
			case ',':
				VarNameCleaner::run(context, *m_ast);
				// VarNameCleaner destroys the unique names guarantee of the disambiguator.
				disambiguated = false;
				break;
			case ';':
			{
				Object obj;
				obj.code = m_ast;
				StackCompressor::run(m_dialect, obj, true, 16);
				break;
			}
			default:
				cerr << "Unknown option." << endl;
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
