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
#include <libsolutil/Exceptions.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/ErrorReporter.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libsolidity/parsing/Parser.h>
#include <libyul/AST.h>
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

#include <libsolidity/interface/OptimiserSettings.h>
#include <liblangutil/CharStreamProvider.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/program_options.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/set_algorithm.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/transform.hpp>

#include <string>
#include <iostream>
#include <variant>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::yul;

namespace po = boost::program_options;

class YulOpti
{
public:
	static void printErrors(CharStream const& _charStream, ErrorList const& _errors)
	{
		SourceReferenceFormatter{
			std::cerr,
			SingletonCharStreamProvider(_charStream),
			true,
			false
		}.printErrorInformation(_errors);
	}

	void parse(std::string const& _input)
	{
		ErrorList errors;
		ErrorReporter errorReporter(errors);
		CharStream _charStream(_input, "");
		try
		{
			auto ast = yul::Parser(errorReporter, m_dialect).parse(_charStream);
			if (!m_astRoot || !errorReporter.errors().empty())
			{
				std::cerr << "Error parsing source." << std::endl;
				printErrors(_charStream, errors);
				throw std::runtime_error("Could not parse source.");
			}
			m_astRoot = std::make_shared<yul::Block>(std::get<yul::Block>(ASTCopier{}(ast->root())));
			m_analysisInfo = std::make_unique<yul::AsmAnalysisInfo>();
			AsmAnalyzer analyzer(
				*m_analysisInfo,
				errorReporter,
				m_dialect
			);
			if (!analyzer.analyze(*m_astRoot) || !errorReporter.errors().empty())
			{
				std::cerr << "Error analyzing source." << std::endl;
				printErrors(_charStream, errors);
				throw std::runtime_error("Could not analyze source.");
			}
		}
		catch(...)
		{
			std::cerr << "Fatal error during parsing: " << std::endl;
			printErrors(_charStream, errors);
			throw;
		}
	}

	void printUsageBanner(
		std::map<char, std::string> const& _extraOptions,
		size_t _columns
	)
	{
		yulAssert(_columns > 0);
		auto const& optimiserSteps = OptimiserSuite::stepAbbreviationToNameMap();
		auto hasShorterString = [](auto const& a, auto const& b) { return a.second.size() < b.second.size(); };
		size_t longestDescriptionLength = std::max(
			max_element(optimiserSteps.begin(), optimiserSteps.end(), hasShorterString)->second.size(),
			max_element(_extraOptions.begin(), _extraOptions.end(), hasShorterString)->second.size()
		);

		std::vector<std::string> overlappingAbbreviations =
			ranges::views::set_intersection(_extraOptions | ranges::views::keys, optimiserSteps | ranges::views::keys) |
			ranges::views::transform([](char _abbreviation){ return std::string(1, _abbreviation); }) |
			ranges::to<std::vector>();

		yulAssert(
			overlappingAbbreviations.empty(),
			"ERROR: Conflict between yulopti controls and the following Yul optimizer step abbreviations: " +
			boost::join(overlappingAbbreviations, ", ") + ".\n"
			"This is most likely caused by someone adding a new step abbreviation to "
			"OptimiserSuite::stepNameToAbbreviationMap() and not realizing that it's used by yulopti.\n"
			"Please update the code to use a different character and recompile yulopti."
		);

		std::vector<std::tuple<char, std::string>> sortedOptions =
			ranges::views::concat(optimiserSteps, _extraOptions) |
			ranges::to<std::vector<std::tuple<char, std::string>>>() |
			ranges::actions::sort([](std::tuple<char, std::string> const& _a, std::tuple<char, std::string> const& _b) {
				return (
					!boost::algorithm::iequals(get<1>(_a), get<1>(_b)) ?
					boost::algorithm::lexicographical_compare(get<1>(_a), get<1>(_b), boost::algorithm::is_iless()) :
					toLower(get<0>(_a)) < toLower(get<0>(_b))
				);
			});

		yulAssert(sortedOptions.size() > 0);
		size_t rows = (sortedOptions.size() - 1) / _columns + 1;
		for (size_t row = 0; row < rows; ++row)
		{
			for (auto const& [key, name]: sortedOptions | ranges::views::drop(row) | ranges::views::stride(rows))
				std::cout << key << ": " << std::setw(static_cast<int>(longestDescriptionLength)) << std::setiosflags(std::ios::left) << name << " ";

			std::cout << std::endl;
		}
	}

	void disambiguate()
	{
		*m_astRoot = std::get<yul::Block>(Disambiguator(m_dialect, *m_analysisInfo)(*m_astRoot));
		m_analysisInfo.reset();
		m_nameDispenser.reset(*m_astRoot);
	}

	void runSteps(std::string _source, std::string _steps)
	{
		parse(_source);
		disambiguate();
		OptimiserSuite{m_context}.runSequence(_steps, *m_astRoot);
		std::cout << AsmPrinter{}(*m_astRoot) << std::endl;
	}

	void runInteractive(std::string _source, bool _disambiguated = false)
	{
		bool disambiguated = _disambiguated;
		while (true)
		{
			parse(_source);
			disambiguated = disambiguated || (disambiguate(), true);
			std::map<char, std::string> const& extraOptions = {
				// QUIT starts with a non-letter character on purpose to get it to show up on top of the list
				{'#', ">>> QUIT <<<"},
				{',', "VarNameCleaner"},
				{';', "StackCompressor"}
			};

			printUsageBanner(extraOptions, 4);
			std::cout << "? ";
			std::cout.flush();
			char option = static_cast<char>(readStandardInputChar());
			std::cout << ' ' << option << std::endl;

			try
			{
				switch (option)
				{
					case 4:
					case '#':
						return;
					case ',':
						VarNameCleaner::run(m_context, *m_astRoot);
						// VarNameCleaner destroys the unique names guarantee of the disambiguator.
						disambiguated = false;
						break;
					case ';':
					{
						Object obj;
						obj.setCode(std::make_shared<AST>(std::get<yul::Block>(ASTCopier{}(*m_astRoot))));
						*m_astRoot = std::get<1>(StackCompressor::run(m_dialect, obj, true, 16));
						break;
					}
					default:
						OptimiserSuite{m_context}.runSequence(
							std::string_view(&option, 1),
							*m_astRoot
						);
				}
				_source = AsmPrinter{}(*m_astRoot);
			}
			catch (...)
			{
				std::cerr << std::endl << "Exception during optimiser step:" << std::endl;
				std::cerr << boost::current_exception_diagnostic_information() << std::endl;
			}
			std::cout << "----------------------" << std::endl;
			std::cout << _source << std::endl;
		}
	}

private:
	std::shared_ptr<yul::Block> m_astRoot;
	Dialect const& m_dialect{EVMDialect::strictAssemblyForEVMObjects(EVMVersion{})};
	std::unique_ptr<AsmAnalysisInfo> m_analysisInfo;
	std::set<YulName> const m_reservedIdentifiers = {};
	NameDispenser m_nameDispenser{m_dialect, m_reservedIdentifiers};
	OptimiserStepContext m_context{
		m_dialect,
		m_nameDispenser,
		m_reservedIdentifiers,
		solidity::frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	};
};

int main(int argc, char** argv)
{
	try
	{
		bool nonInteractive = false;
		po::options_description options(
			R"(yulopti, yul optimizer exploration tool.
	Usage: yulopti [Options] <file>
	Reads <file> as yul code and applies optimizer steps to it,
	interactively read from stdin.
	In non-interactive mode a list of steps has to be provided.
	If <file> is -, yul code is read from stdin and run non-interactively.

	Allowed options)",
			po::options_description::m_default_line_length,
			po::options_description::m_default_line_length - 23);
		options.add_options()
			(
				"input-file",
				po::value<std::string>(),
				"input file"
			)
			(
				"steps",
				po::value<std::string>(),
				"steps to execute non-interactively"
			)
			(
				"non-interactive,n",
				po::bool_switch(&nonInteractive)->default_value(false),
				"stop after executing the provided steps"
			)
			("help,h", "Show this help screen.");

		// All positional options should be interpreted as input files
		po::positional_options_description filesPositions;
		filesPositions.add("input-file", 1);

		po::variables_map arguments;
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options).positional(filesPositions);
		po::store(cmdLineParser.run(), arguments);
		po::notify(arguments);

		if (arguments.count("help"))
		{
			std::cout << options;
			return 0;
		}

		std::string input;
		if (arguments.count("input-file"))
		{
			std::string filename = arguments["input-file"].as<std::string>();
			if (filename == "-")
			{
				nonInteractive = true;
				input = readUntilEnd(std::cin);
			}
			else
				input = readFileAsString(arguments["input-file"].as<std::string>());
		}
		else
		{
			std::cout << options;
			return 1;
		}

		if (nonInteractive && !arguments.count("steps"))
		{
			std::cout << options;
			return 1;
		}

		YulOpti yulOpti;
		bool disambiguated = false;
		if (!nonInteractive)
			std::cout << input << std::endl;
		if (arguments.count("steps"))
		{
			std::string sequence = arguments["steps"].as<std::string>();
			if (!nonInteractive)
				std::cout << "----------------------" << std::endl;
			yulOpti.runSteps(input, sequence);
			disambiguated = true;
		}
		if (!nonInteractive)
			yulOpti.runInteractive(input, disambiguated);

		return 0;
	}
	catch (po::error const& _exception)
	{
		std::cerr << _exception.what() << std::endl;
		return 1;
	}
	catch (FileNotFound const& _exception)
	{
		std::cerr << "File not found:" << _exception.comment() << std::endl;
		return 1;
	}
	catch (NotAFile const& _exception)
	{
		std::cerr << "Not a regular file:" << _exception.comment() << std::endl;
		return 1;
	}
	catch(...)
	{
		std::cerr << std::endl << "Exception:" << std::endl;
		std::cerr << boost::current_exception_diagnostic_information() << std::endl;
		return 1;
	}
}
