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
#include <tools/yulPhaser/Common.h>
#include <tools/yulPhaser/Exceptions.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::phaser;

namespace po = boost::program_options;

namespace
{

map<Algorithm, string> const AlgorithmToStringMap =
{
	{Algorithm::Random, "random"},
	{Algorithm::GEWEP, "GEWEP"},
};
map<string, Algorithm> const StringToAlgorithmMap = invertMap(AlgorithmToStringMap);

}

istream& phaser::operator>>(istream& _inputStream, Algorithm& _algorithm) { return deserializeChoice(_inputStream, _algorithm, StringToAlgorithmMap); }
ostream& phaser::operator<<(ostream& _outputStream, Algorithm _algorithm) { return serializeChoice(_outputStream, _algorithm, AlgorithmToStringMap); }

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

FitnessMetricFactory::Options FitnessMetricFactory::Options::fromCommandLine(po::variables_map const& _arguments)
{
	return {
		_arguments["chromosome-repetitions"].as<size_t>(),
	};
}

unique_ptr<FitnessMetric> FitnessMetricFactory::build(
	Options const& _options,
	Program _program
)
{
	return make_unique<ProgramSize>(move(_program), _options.chromosomeRepetitions);
}

PopulationFactory::Options PopulationFactory::Options::fromCommandLine(po::variables_map const& _arguments)
{
	return {
		_arguments.count("population") > 0 ?
			_arguments["population"].as<vector<string>>() :
			vector<string>{},
		_arguments.count("random-population") > 0 ?
			_arguments["random-population"].as<vector<size_t>>() :
			vector<size_t>{},
		_arguments.count("population-from-file") > 0 ?
			_arguments["population-from-file"].as<vector<string>>() :
			vector<string>{},
	};
}

Population PopulationFactory::build(
	Options const& _options,
	shared_ptr<FitnessMetric> _fitnessMetric
)
{
	Population population = buildFromStrings(_options.population, _fitnessMetric);

	size_t combinedSize = 0;
	for (size_t populationSize: _options.randomPopulation)
		combinedSize += populationSize;

	population = move(population) + buildRandom(
		combinedSize,
		_fitnessMetric
	);

	for (string const& populationFilePath: _options.populationFromFile)
		population = move(population) + buildFromFile(populationFilePath, _fitnessMetric);

	return population;
}

Population PopulationFactory::buildFromStrings(
	vector<string> const& _geneSequences,
	shared_ptr<FitnessMetric> _fitnessMetric
)
{
	vector<Chromosome> chromosomes;
	for (string const& geneSequence: _geneSequences)
		chromosomes.emplace_back(geneSequence);

	return Population(move(_fitnessMetric), move(chromosomes));
}

Population PopulationFactory::buildRandom(
	size_t _populationSize,
	shared_ptr<FitnessMetric> _fitnessMetric
)
{
	return Population::makeRandom(
		move(_fitnessMetric),
		_populationSize,
		MinChromosomeLength,
		MaxChromosomeLength
	);
}

Population PopulationFactory::buildFromFile(
	string const& _filePath,
	shared_ptr<FitnessMetric> _fitnessMetric
)
{
	return buildFromStrings(readLinesFromFile(_filePath), move(_fitnessMetric));
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
	variant<Program, ErrorList> programOrErrors = Program::load(sourceCode);
	if (holds_alternative<ErrorList>(programOrErrors))
	{
		cerr << get<ErrorList>(programOrErrors) << endl;
		assertThrow(false, InvalidProgram, "Failed to load program " + _options.inputFile);
	}
	return move(get<Program>(programOrErrors));
}

CharStream ProgramFactory::loadSource(string const& _sourcePath)
{
	assertThrow(boost::filesystem::exists(_sourcePath), MissingFile, "Source file does not exist: " + _sourcePath);

	string sourceCode = readFileAsString(_sourcePath);
	return CharStream(sourceCode, _sourcePath);
}

void Phaser::main(int _argc, char** _argv)
{
	optional<po::variables_map> arguments = parseCommandLine(_argc, _argv);
	if (!arguments.has_value())
		return;

	initialiseRNG(arguments.value());

	runAlgorithm(arguments.value());
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

	po::options_description generalDescription("GENERAL", lineLength, minDescriptionLength);
	generalDescription.add_options()
		("help", "Show help message and exit.")
		("input-file", po::value<string>()->required()->value_name("<PATH>"), "Input file.")
		("seed", po::value<uint32_t>()->value_name("<NUM>"), "Seed for the random number generator.")
		(
			"rounds",
			po::value<size_t>()->value_name("<NUM>"),
			"The number of rounds after which the algorithm should stop. (default=no limit)."
		)
	;
	keywordDescription.add(generalDescription);

	po::options_description algorithmDescription("ALGORITHM", lineLength, minDescriptionLength);
	algorithmDescription.add_options()
		(
			"algorithm",
			po::value<Algorithm>()->value_name("<NAME>")->default_value(Algorithm::GEWEP),
			"Algorithm"
		)
	;
	keywordDescription.add(algorithmDescription);

	po::options_description populationDescription("POPULATION", lineLength, minDescriptionLength);
	populationDescription.add_options()
		(
			"population",
			po::value<vector<string>>()->multitoken()->value_name("<CHROMOSOMES>"),
			"List of chromosomes to be included in the initial population. "
			"You can specify multiple values separated with spaces or invoke the option multiple times "
			"and all the values will be included."
		)
		(
			"random-population",
			po::value<vector<size_t>>()->value_name("<SIZE>"),
			"The number of randomly generated chromosomes to be included in the initial population."
		)
		(
			"population-from-file",
			po::value<vector<string>>()->value_name("<FILE>"),
			"A text file with a list of chromosomes (one per line) to be included in the initial population."
		)
		(
			"population-autosave",
			po::value<string>()->value_name("<FILE>"),
			"If specified, the population is saved in the specified file after each round. (default=autosave disabled)"
		)
	;
	keywordDescription.add(populationDescription);

	po::options_description metricsDescription("METRICS", lineLength, minDescriptionLength);
	metricsDescription.add_options()
		(
			"chromosome-repetitions",
			po::value<size_t>()->value_name("<COUNT>")->default_value(1),
			"Number of times to repeat the sequence optimisation steps represented by a chromosome."
		)
	;
	keywordDescription.add(metricsDescription);

	po::positional_options_description positionalDescription;
	positionalDescription.add("input-file", 1);

	return {keywordDescription, positionalDescription};
}

optional<po::variables_map> Phaser::parseCommandLine(int _argc, char** _argv)
{
	auto [keywordDescription, positionalDescription] = buildCommandLineDescription();

	po::variables_map arguments;
	po::notify(arguments);

	po::command_line_parser parser(_argc, _argv);
	parser.options(keywordDescription).positional(positionalDescription);
	po::store(parser.run(), arguments);

	if (arguments.count("help") > 0)
	{
		cout << keywordDescription << endl;
		return nullopt;
	}

	if (arguments.count("input-file") == 0)
		assertThrow(false, NoInputFiles, "Missing argument: input-file.");

	return arguments;
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

AlgorithmRunner::Options Phaser::buildAlgorithmRunnerOptions(po::variables_map const& _arguments)
{
	return {
		_arguments.count("rounds") > 0 ? static_cast<optional<size_t>>(_arguments["rounds"].as<size_t>()) : nullopt,
		_arguments.count("population-autosave") > 0 ? static_cast<optional<string>>(_arguments["population-autosave"].as<string>()) : nullopt,
	};
}

void Phaser::runAlgorithm(po::variables_map const& _arguments)
{
	auto programOptions = ProgramFactory::Options::fromCommandLine(_arguments);
	auto metricOptions = FitnessMetricFactory::Options::fromCommandLine(_arguments);
	auto populationOptions = PopulationFactory::Options::fromCommandLine(_arguments);
	auto algorithmOptions = GeneticAlgorithmFactory::Options::fromCommandLine(_arguments);

	Program program = ProgramFactory::build(programOptions);
	unique_ptr<FitnessMetric> fitnessMetric = FitnessMetricFactory::build(metricOptions, move(program));
	Population population = PopulationFactory::build(populationOptions, move(fitnessMetric));

	unique_ptr<GeneticAlgorithm> geneticAlgorithm = GeneticAlgorithmFactory::build(
		algorithmOptions,
		population.individuals().size(),
		PopulationFactory::MinChromosomeLength,
		PopulationFactory::MaxChromosomeLength
	);

	AlgorithmRunner algorithmRunner(population, buildAlgorithmRunnerOptions(_arguments), cout);
	algorithmRunner.run(*geneticAlgorithm);
}
