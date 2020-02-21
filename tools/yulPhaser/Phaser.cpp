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

#include <tools/yulPhaser/Phaser.h>

#include <tools/yulPhaser/Exceptions.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::phaser;

namespace po = boost::program_options;

istream& phaser::operator>>(istream& _inputStream, Algorithm& _algorithm)
{
	string value;
	_inputStream >> value;

	if (value == "random")
		_algorithm = Algorithm::Random;
	else if (value == "GEWEP")
		_algorithm = Algorithm::GEWEP;
	else
		_inputStream.setstate(ios_base::failbit);

	return _inputStream;
}

ostream& phaser::operator<<(ostream& _outputStream, Algorithm _algorithm)
{
	if (_algorithm == Algorithm::Random)
		_outputStream << "random";
	else if (_algorithm == Algorithm::GEWEP)
		_outputStream << "GEWEP";
	else
		_outputStream.setstate(ios_base::failbit);

	return _outputStream;
}

namespace
{

CharStream loadSource(string const& _sourcePath)
{
	assertThrow(boost::filesystem::exists(_sourcePath), InvalidProgram, "Source file does not exist");

	string sourceCode = readFileAsString(_sourcePath);
	return CharStream(sourceCode, _sourcePath);
}

}

int Phaser::main(int _argc, char** _argv)
{
	CommandLineParsingResult parsingResult = parseCommandLine(_argc, _argv);
	if (parsingResult.exitCode != 0)
		return parsingResult.exitCode;

	initialiseRNG(parsingResult.arguments);

	runAlgorithm(
		parsingResult.arguments["input-file"].as<string>(),
		parsingResult.arguments["algorithm"].as<Algorithm>()
	);
	return 0;
}

Phaser::CommandLineParsingResult Phaser::parseCommandLine(int _argc, char** _argv)
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
		(
			"algorithm",
			po::value<Algorithm>()->default_value(Algorithm::GEWEP),
			"Algorithm"
		)
	;

	po::positional_options_description positionalDescription;
	po::variables_map arguments;
	positionalDescription.add("input-file", 1);
	po::notify(arguments);

	try
	{
		po::command_line_parser parser(_argc, _argv);
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

void Phaser::initialiseRNG(po::variables_map const& _arguments)
{
	uint32_t seed;
	if (_arguments.count("seed") > 0)
		seed = _arguments["seed"].as<uint32_t>();
	else
		seed = SimulationRNG::generateSeed();

	SimulationRNG::reset(seed);
	cout << "Random seed: " << seed << endl;
}

void Phaser::runAlgorithm(string const& _sourcePath, Algorithm _algorithm)
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

	switch (_algorithm)
	{
		case Algorithm::Random:
		{
			RandomAlgorithm(
				population,
				cout,
				{
					/* elitePoolSize = */ 1.0 / populationSize,
					/* minChromosomeLength = */ minChromosomeLength,
					/* maxChromosomeLength = */ maxChromosomeLength,
				}
			).run();

			break;
		}
		case Algorithm::GEWEP:
		{
			GenerationalElitistWithExclusivePools(
				population,
				cout,
				{
					/* mutationPoolSize = */ 0.25,
					/* crossoverPoolSize = */ 0.25,
					/* randomisationChance = */ 0.9,
					/* deletionVsAdditionChance = */ 0.5,
					/* percentGenesToRandomise = */ 1.0 / maxChromosomeLength,
					/* percentGenesToAddOrDelete = */ 1.0 / maxChromosomeLength,
				}
			).run();

			break;
		}
	}
}
