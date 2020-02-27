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

#include <test/yulPhaser/TestHelpers.h>

#include <tools/yulPhaser/AlgorithmRunner.h>
#include <tools/yulPhaser/Common.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <regex>
#include <sstream>

using namespace std;
using namespace boost::unit_test::framework;
using namespace boost::test_tools;
using namespace solidity::langutil;
using namespace solidity::util;

namespace fs = boost::filesystem;

namespace solidity::phaser::test
{

class CountingAlgorithm: public GeneticAlgorithm
{
public:
	using GeneticAlgorithm::GeneticAlgorithm;
	Population runNextRound(Population _population) override
	{
		++m_currentRound;
		return _population;
	}

	size_t m_currentRound = 0;
};

class RandomisingAlgorithm: public GeneticAlgorithm
{
public:
	using GeneticAlgorithm::GeneticAlgorithm;
	Population runNextRound(Population _population) override
	{
		return Population::makeRandom(_population.fitnessMetric(), _population.individuals().size(), 10, 20);
	}
};

class AlgorithmRunnerFixture
{
protected:
	// NOTE: Regexes here should not contain spaces because we strip them before matching
	regex RoundSummaryRegex{R"(-+ROUND\d+-+)"};
	regex InitialPopulationHeaderRegex{"-+INITIALPOPULATION-+"};

	string individualPattern(Individual const& individual) const
	{
		ostringstream output;
		output << "Fitness:" << individual.fitness << ",";
		output << "optimisations:" << individual.chromosome;
		return output.str();
		return output.str();
	}

	bool nextLineMatches(stringstream& stream, regex const& pattern) const
	{
		string line;
		if (getline(stream, line).fail())
			return false;

		return regex_match(stripWhitespace(line), pattern);
	}

	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
	Population const m_population = Population::makeRandom(m_fitnessMetric, 5, 0, 20);
	stringstream m_output;
	AlgorithmRunner::Options m_options;
};

class AlgorithmRunnerAutosaveFixture: public AlgorithmRunnerFixture
{
public:
	static vector<string> chromosomeStrings(Population const& _population)
	{
		vector<string> lines;
		for (auto const& individual: _population.individuals())
			lines.push_back(toString(individual.chromosome));

		return lines;
	}

protected:
	TemporaryDirectory m_tempDir;
	string const m_autosavePath = m_tempDir.memberPath("population-autosave.txt");
	RandomisingAlgorithm m_algorithm;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(AlgorithmRunnerTest)

BOOST_FIXTURE_TEST_CASE(run_should_call_runNextRound_once_per_round, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 5;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);

	CountingAlgorithm algorithm;

	BOOST_TEST(algorithm.m_currentRound == 0);
	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 5);
	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 10);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_round_summary_after_each_round, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, RoundSummaryRegex));
	for (auto const& individual: runner.population().individuals())
		BOOST_TEST(nextLineMatches(m_output, regex(individualPattern(individual))));

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, RoundSummaryRegex));
	for (auto const& individual: runner.population().individuals())
		BOOST_TEST(nextLineMatches(m_output, regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_initial_population_if_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 0;
	m_options.showInitialPopulation = true;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(nextLineMatches(m_output, InitialPopulationHeaderRegex));
	for (auto const& individual: m_population.individuals())
		BOOST_TEST(nextLineMatches(m_output, regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_initial_population_if_not_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 0;
	m_options.showInitialPopulation = false;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_save_initial_population_to_file_if_autosave_file_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 0;
	m_options.populationAutosaveFile = m_autosavePath;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	runner.run(m_algorithm);
	assert(runner.population() == m_population);

	BOOST_TEST(fs::is_regular_file(m_autosavePath));
	BOOST_TEST(readLinesFromFile(m_autosavePath) == chromosomeStrings(runner.population()));
}

BOOST_FIXTURE_TEST_CASE(run_should_save_population_to_file_if_autosave_file_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 1;
	m_options.populationAutosaveFile = m_autosavePath;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	runner.run(m_algorithm);
	assert(runner.population() != m_population);

	BOOST_TEST(fs::is_regular_file(m_autosavePath));
	BOOST_TEST(readLinesFromFile(m_autosavePath) == chromosomeStrings(runner.population()));
}

BOOST_FIXTURE_TEST_CASE(run_should_overwrite_existing_file_if_autosave_file_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 5;
	m_options.populationAutosaveFile = m_autosavePath;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	vector<string> originalContent = {"Original content"};
	{
		ofstream tmpFile(m_autosavePath);
		tmpFile << originalContent[0] << endl;
	}
	assert(fs::exists(m_autosavePath));
	assert(readLinesFromFile(m_autosavePath) == originalContent);

	runner.run(m_algorithm);

	BOOST_TEST(fs::is_regular_file(m_autosavePath));
	BOOST_TEST(readLinesFromFile(m_autosavePath) != originalContent);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_save_population_to_file_if_autosave_file_not_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 5;
	m_options.populationAutosaveFile = nullopt;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	runner.run(m_algorithm);

	BOOST_TEST(!fs::exists(m_autosavePath));
}

BOOST_FIXTURE_TEST_CASE(run_should_randomise_duplicate_chromosomes_if_requested, AlgorithmRunnerFixture)
{
	Chromosome duplicate("afc");
	Population population(m_fitnessMetric, {duplicate, duplicate, duplicate});
	CountingAlgorithm algorithm;

	m_options.maxRounds = 1;
	m_options.randomiseDuplicates = true;
	m_options.minChromosomeLength = 50;
	m_options.maxChromosomeLength = 50;
	AlgorithmRunner runner(population, {}, m_options, m_output);

	runner.run(algorithm);

	auto const& newIndividuals = runner.population().individuals();

	BOOST_TEST(newIndividuals.size() == 3);
	BOOST_TEST((
		newIndividuals[0].chromosome == duplicate ||
		newIndividuals[1].chromosome == duplicate ||
		newIndividuals[2].chromosome == duplicate
	));
	BOOST_TEST(newIndividuals[0] != newIndividuals[1]);
	BOOST_TEST(newIndividuals[0] != newIndividuals[2]);
	BOOST_TEST(newIndividuals[1] != newIndividuals[2]);

	BOOST_TEST((newIndividuals[0].chromosome.length() == 50 || newIndividuals[0].chromosome == duplicate));
	BOOST_TEST((newIndividuals[1].chromosome.length() == 50 || newIndividuals[1].chromosome == duplicate));
	BOOST_TEST((newIndividuals[2].chromosome.length() == 50 || newIndividuals[2].chromosome == duplicate));
}

BOOST_FIXTURE_TEST_CASE(run_should_not_randomise_duplicate_chromosomes_if_not_requested, AlgorithmRunnerFixture)
{
	Chromosome duplicate("afc");
	Population population(m_fitnessMetric, {duplicate, duplicate, duplicate});
	CountingAlgorithm algorithm;

	m_options.maxRounds = 1;
	m_options.randomiseDuplicates = false;
	AlgorithmRunner runner(population, {}, m_options, m_output);

	runner.run(algorithm);

	BOOST_TEST(runner.population().individuals().size() == 3);
	BOOST_TEST(runner.population().individuals()[0].chromosome == duplicate);
	BOOST_TEST(runner.population().individuals()[1].chromosome == duplicate);
	BOOST_TEST(runner.population().individuals()[2].chromosome == duplicate);
}

BOOST_FIXTURE_TEST_CASE(run_should_clear_cache_at_the_beginning_and_update_it_before_each_round, AlgorithmRunnerFixture)
{
	CharStream sourceStream = CharStream("{}", current_test_case().p_name);
	vector<shared_ptr<ProgramCache>> caches = {
		make_shared<ProgramCache>(get<Program>(Program::load(sourceStream))),
		make_shared<ProgramCache>(get<Program>(Program::load(sourceStream))),
	};

	m_options.maxRounds = 10;
	AlgorithmRunner runner(m_population, caches, m_options, m_output);
	CountingAlgorithm algorithm;

	BOOST_TEST(algorithm.m_currentRound == 0);
	BOOST_TEST(caches[0]->currentRound() == 0);
	BOOST_TEST(caches[1]->currentRound() == 0);

	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 10);
	BOOST_TEST(caches[0]->currentRound() == 10);
	BOOST_TEST(caches[1]->currentRound() == 10);

	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 20);
	BOOST_TEST(caches[0]->currentRound() == 10);
	BOOST_TEST(caches[1]->currentRound() == 10);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
