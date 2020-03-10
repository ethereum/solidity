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

#include <tools/yulPhaser/Exceptions.h>
#include <tools/yulPhaser/Population.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/CommonIO.h>
#include <liblangutil/CharStream.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::phaser;
using namespace solidity::util;

namespace po = boost::program_options;

namespace
{

struct CommandLineParsingResult
{
	int exitCode;
	po::variables_map arguments;
};


void initializeRNG(po::variables_map const& arguments)
{
	uint32_t seed;
	if (arguments.count("seed") > 0)
		seed = arguments["seed"].as<uint32_t>();
	else
		seed = SimulationRNG::generateSeed();

	SimulationRNG::reset(seed);
	cout << "Random seed: " << seed << endl;
}

CharStream loadSource(string const& _sourcePath)
{
	assertThrow(boost::filesystem::exists(_sourcePath), InvalidProgram, "Source file does not exist");

	string sourceCode = readFileAsString(_sourcePath);
	return CharStream(sourceCode, _sourcePath);
}

void runAlgorithm(string const& _sourcePath)
{
	constexpr size_t populationSize = 20;
	constexpr size_t minChromosomeLength = 12;
	constexpr size_t maxChromosomeLength = 30;

	CharStream sourceCode = loadSource(_sourcePath);
	shared_ptr<FitnessMetric> fitnessMetric = make_shared<ProgramSize>(Program::load(sourceCode), 5);
	auto population = Population::makeRandom(
		fitnessMetric,
		populationSize,
		minChromosomeLength,
		maxChromosomeLength
	);
	RandomAlgorithm(
		population,
		cout,
		{
			/* elitePoolSize = */ 1.0 / populationSize,
			/* minChromosomeLength = */ minChromosomeLength,
			/* maxChromosomeLength = */ maxChromosomeLength,
		}
	).run();
}

CommandLineParsingResult parseCommandLine(int argc, char** argv)
{
	po::options_description description(
		"yul-phaser, a tool for finding the best sequence of Yul optimisation phases.\n"
		"\n"
		"Usage: yul-phaser [options] <file>\n"
		"Reads <file> as Yul code and tries to find the best order in which to run optimisation"
		" phases using a genetic algorithm.\n"
		"Example:\n"
		"yul-phaser program.yul\n"
		"\n"
		"Allowed options",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23
	);

	description.add_options()
		("help", "Show help message and exit.")
		("input-file", po::value<string>()->required(), "Input file")
		("seed", po::value<uint32_t>(), "Seed for the random number generator")
	;

	po::positional_options_description positionalDescription;
	po::variables_map arguments;
	positionalDescription.add("input-file", 1);
	po::notify(arguments);

	try
	{
		po::command_line_parser parser(argc, argv);
		parser.options(description).positional(positionalDescription);
		po::store(parser.run(), arguments);
	}
	catch (po::error const & _exception)
	{
		cerr << _exception.what() << endl;
		return {1, move(arguments)};
	}

	if (arguments.count("help") > 0)
	{
		cout << description << endl;
		return {2, move(arguments)};
	}

	if (arguments.count("input-file") == 0)
	{
		cerr << "Missing argument: input-file." << endl;
		return {1, move(arguments)};
	}

	return {0, arguments};
}

}

int main(int argc, char** argv)
{
	CommandLineParsingResult parsingResult = parseCommandLine(argc, argv);
	if (parsingResult.exitCode != 0)
		return parsingResult.exitCode;

	initializeRNG(parsingResult.arguments);

	try
	{
		runAlgorithm(parsingResult.arguments["input-file"].as<string>());
	}
	catch (InvalidProgram const& _exception)
	{
		cerr << "ERROR: " << _exception.what() << endl;
		return 1;
	}

	return 0;
}
