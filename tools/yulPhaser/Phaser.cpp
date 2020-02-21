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

#include <tools/yulPhaser/AlgorithmRunner.h>
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

GeneticAlgorithmFactory::Options GeneticAlgorithmFactory::Options::fromCommandLine(po::variables_map const& _arguments)
{
	return {
		_arguments["algorithm"].as<Algorithm>(),
	};
}

unique_ptr<GeneticAlgorithm> GeneticAlgorithmFactory::build(
	Options const& _options,
	size_t _populationSize,
	size_t _minChromosomeLength,
	size_t _maxChromosomeLength
)
{
	assert(_populationSize > 0);

	switch (_options.algorithm)
	{
		case Algorithm::Random:
			return make_unique<RandomAlgorithm>(RandomAlgorithm::Options{
				/* elitePoolSize = */ 1.0 / _populationSize,
				/* minChromosomeLength = */ _minChromosomeLength,
				/* maxChromosomeLength = */ _maxChromosomeLength,
			});
		case Algorithm::GEWEP:
			return make_unique<GenerationalElitistWithExclusivePools>(GenerationalElitistWithExclusivePools::Options{
				/* mutationPoolSize = */ 0.25,
				/* crossoverPoolSize = */ 0.25,
				/* randomisationChance = */ 0.9,
				/* deletionVsAdditionChance = */ 0.5,
				/* percentGenesToRandomise = */ 1.0 / _maxChromosomeLength,
				/* percentGenesToAddOrDelete = */ 1.0 / _maxChromosomeLength,
			});
		default:
			assertThrow(false, solidity::util::Exception, "Invalid Algorithm value.");
	}
}

unique_ptr<FitnessMetric> FitnessMetricFactory::build(
	Program _program
)
{
	return make_unique<ProgramSize>(move(_program), RepetitionCount);
}

Population PopulationFactory::build(
	shared_ptr<FitnessMetric> _fitnessMetric
)
{
	return Population::makeRandom(
		move(_fitnessMetric),
		PopulationSize,
		MinChromosomeLength,
		MaxChromosomeLength
	);
}

ProgramFactory::Options ProgramFactory::Options::fromCommandLine(po::variables_map const& _arguments)
{
	return {
		_arguments["input-file"].as<string>(),
	};
}

Program ProgramFactory::build(Options const& _options)
{
	CharStream sourceCode = loadSource(_options.inputFile);
	return Program::load(sourceCode);
}

CharStream ProgramFactory::loadSource(string const& _sourcePath)
{
	assertThrow(boost::filesystem::exists(_sourcePath), InvalidProgram, "Source file does not exist");

	string sourceCode = readFileAsString(_sourcePath);
	return CharStream(sourceCode, _sourcePath);
}

int Phaser::main(int _argc, char** _argv)
{
	CommandLineParsingResult parsingResult = parseCommandLine(_argc, _argv);
	if (parsingResult.exitCode != 0)
		return parsingResult.exitCode;

	initialiseRNG(parsingResult.arguments);

	runAlgorithm(parsingResult.arguments);
	return 0;
}

Phaser::CommandLineDescription Phaser::buildCommandLineDescription()
{
	size_t const lineLength = po::options_description::m_default_line_length;
	size_t const minDescriptionLength = lineLength - 23;

	po::options_description keywordDescription(
		"yul-phaser, a tool for finding the best sequence of Yul optimisation phases.\n"
		"\n"
		"Usage: yul-phaser [options] <file>\n"
		"Reads <file> as Yul code and tries to find the best order in which to run optimisation"
		" phases using a genetic algorithm.\n"
		"Example:\n"
		"yul-phaser program.yul\n"
		"\n"
		"Allowed options",
		lineLength,
		minDescriptionLength
	);
	keywordDescription.add_options()
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
	positionalDescription.add("input-file", 1);

	return {keywordDescription, positionalDescription};
}

Phaser::CommandLineParsingResult Phaser::parseCommandLine(int _argc, char** _argv)
{
	auto [keywordDescription, positionalDescription] = buildCommandLineDescription();

	po::variables_map arguments;
	po::notify(arguments);

	try
	{
		po::command_line_parser parser(_argc, _argv);
		parser.options(keywordDescription).positional(positionalDescription);
		po::store(parser.run(), arguments);
	}
	catch (po::error const & _exception)
	{
		cerr << _exception.what() << endl;
		return {1, move(arguments)};
	}

	if (arguments.count("help") > 0)
	{
		cout << keywordDescription << endl;
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

void Phaser::runAlgorithm(po::variables_map const& _arguments)
{
	auto programOptions = ProgramFactory::Options::fromCommandLine(_arguments);
	auto algorithmOptions = GeneticAlgorithmFactory::Options::fromCommandLine(_arguments);

	Program program = ProgramFactory::build(programOptions);
	unique_ptr<FitnessMetric> fitnessMetric = FitnessMetricFactory::build(move(program));
	Population population = PopulationFactory::build(move(fitnessMetric));

	unique_ptr<GeneticAlgorithm> geneticAlgorithm = GeneticAlgorithmFactory::build(
		algorithmOptions,
		population.individuals().size(),
		PopulationFactory::MinChromosomeLength,
		PopulationFactory::MaxChromosomeLength
	);

	AlgorithmRunner algorithmRunner(population, AlgorithmRunner::Options{}, cout);
	algorithmRunner.run(*geneticAlgorithm);
}
