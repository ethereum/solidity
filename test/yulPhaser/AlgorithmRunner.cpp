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

#include <test/yulPhaser/TestHelpers.h>

#include <tools/yulPhaser/AlgorithmRunner.h>
#include <tools/yulPhaser/Common.h>
#include <tools/yulPhaser/FitnessMetrics.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/TemporaryDirectory.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <fstream>
#include <regex>
#include <sstream>

using namespace boost::unit_test::framework;
using namespace boost::test_tools;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

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
	std::regex RoundSummaryRegex{R"(-+ROUND\d+\[round:[0-9.]+s,total:[0-9.]+s\]-+)"};
	std::regex InitialPopulationHeaderRegex{"-+INITIALPOPULATION-+"};

	std::string individualPattern(Individual const& individual) const
	{
		std::ostringstream output;
		output << individual.fitness << individual.chromosome;
		return output.str();
	}

	std::string topChromosomePattern(size_t roundNumber, Individual const& individual) const
	{
		std::ostringstream output;
		output << roundNumber << R"(\|[0-9.]+\|)" << individualPattern(individual);
		return output.str();
	}

	bool nextLineMatches(std::stringstream& stream, std::regex const& pattern) const
	{
		std::string line;
		if (std::getline(stream, line).fail())
			return false;

		return std::regex_match(stripWhitespace(line), pattern);
	}

	std::shared_ptr<FitnessMetric> m_fitnessMetric = std::make_shared<ChromosomeLengthMetric>();
	Population const m_population = Population::makeRandom(m_fitnessMetric, 5, 0, 20);
	std::stringstream m_output;
	AlgorithmRunner::Options m_options;
};

class AlgorithmRunnerAutosaveFixture: public AlgorithmRunnerFixture
{
public:
	static std::vector<std::string> chromosomeStrings(Population const& _population)
	{
		std::vector<std::string> lines;
		for (auto const& individual: _population.individuals())
			lines.push_back(toString(individual.chromosome));

		return lines;
	}

protected:
	TemporaryDirectory m_tempDir;
	std::string const m_autosavePath = (m_tempDir.path() / "population-autosave.txt").string();
	RandomisingAlgorithm m_algorithm;
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
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
	m_options.showOnlyTopChromosome = false;
	m_options.showRoundInfo = true;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, RoundSummaryRegex));
	for (auto const& individual: runner.population().individuals())
		BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(individual))));

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, RoundSummaryRegex));
	for (auto const& individual: runner.population().individuals())
		BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_round_summary_if_not_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showOnlyTopChromosome = false;
	m_options.showRoundInfo = false;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, std::regex("")));
	for (auto const& individual: runner.population().individuals())
		BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_population_if_its_empty, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showOnlyTopChromosome = false;
	m_options.showRoundInfo = true;
	AlgorithmRunner runner(Population(m_fitnessMetric), {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, RoundSummaryRegex));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_only_top_chromosome_if_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showOnlyTopChromosome = true;
	m_options.showRoundInfo = true;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, std::regex(topChromosomePattern(1, runner.population().individuals()[0]))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_round_number_for_top_chromosome_if_round_info_not_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showOnlyTopChromosome = true;
	m_options.showRoundInfo = false;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(runner.population().individuals()[0]))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_population_if_its_empty_and_only_top_chromosome_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 3;
	m_options.showRoundInfo = true;
	m_options.showInitialPopulation = false;
	m_options.showOnlyTopChromosome = true;
	AlgorithmRunner runner(Population(m_fitnessMetric), {}, m_options, m_output);
	RandomisingAlgorithm algorithm;

	runner.run(algorithm);
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_initial_population_if_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 0;
	m_options.showInitialPopulation = true;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = false;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(nextLineMatches(m_output, InitialPopulationHeaderRegex));
	for (auto const& individual: m_population.individuals())
		BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_not_print_initial_population_if_not_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 0;
	m_options.showInitialPopulation = false;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = false;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_whole_initial_population_even_if_only_top_chromosome_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 0;
	m_options.showInitialPopulation = true;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = true;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(nextLineMatches(m_output, InitialPopulationHeaderRegex));
	for (auto const& individual: m_population.individuals())
		BOOST_TEST(nextLineMatches(m_output, std::regex(individualPattern(individual))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_cache_stats_if_requested, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 4;
	m_options.showInitialPopulation = false;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = true;
	m_options.showCacheStats = true;
	RandomisingAlgorithm algorithm;

	std::vector<CharStream> sourceStreams = {
		CharStream("{mstore(10, 20)}", ""),
		CharStream("{mstore(10, 20)\nsstore(10, 20)}", ""),
	};
	std::vector<Program> programs = {
		get<Program>(Program::load(sourceStreams[0])),
		get<Program>(Program::load(sourceStreams[1])),
	};
	std::vector<std::shared_ptr<ProgramCache>> caches = {
			std::make_shared<ProgramCache>(programs[0]),
			std::make_shared<ProgramCache>(programs[1]),
	};
	std::shared_ptr<FitnessMetric> fitnessMetric = std::make_shared<FitnessMetricAverage>(std::vector<std::shared_ptr<FitnessMetric>>{
		std::make_shared<ProgramSize>(std::nullopt, caches[0], CodeWeights{}),
		std::make_shared<ProgramSize>(std::nullopt, caches[1], CodeWeights{}),
	});
	Population population = Population::makeRandom(fitnessMetric, 2, 0, 5);

	AlgorithmRunner runner(population, caches, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(caches[0]->currentRound() == m_options.maxRounds.value());
	BOOST_TEST(caches[1]->currentRound() == m_options.maxRounds.value());

	CacheStats stats = caches[0]->gatherStats() + caches[1]->gatherStats();

	for (size_t i = 0; i < m_options.maxRounds.value() - 1; ++i)
	{
		BOOST_TEST(nextLineMatches(m_output, std::regex(".*")));
		BOOST_TEST(nextLineMatches(m_output, std::regex("-+CACHESTATS-+")));
		if (i > 0)
			BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Round\d+:\d+entries)")));
		BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Round\d+:\d+entries)")));
		BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Totalhits:\d+)")));
		BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Totalmisses:\d+)")));
		BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Sizeofcachedcode:\d+)")));
	}

	BOOST_REQUIRE(stats.roundEntryCounts.size() == 2);
	BOOST_REQUIRE(stats.roundEntryCounts.count(m_options.maxRounds.value() - 1) == 1);
	BOOST_REQUIRE(stats.roundEntryCounts.count(m_options.maxRounds.value()) == 1);

	size_t round = m_options.maxRounds.value();
	BOOST_TEST(nextLineMatches(m_output, std::regex(".*")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("-+CACHESTATS-+")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("Round" + toString(round - 1) + ":" + toString(stats.roundEntryCounts[round - 1]) + "entries")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("Round" + toString(round) + ":" + toString(stats.roundEntryCounts[round]) + "entries")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("Totalhits:" + toString(stats.hits))));
	BOOST_TEST(nextLineMatches(m_output, std::regex("Totalmisses:" + toString(stats.misses))));
	BOOST_TEST(nextLineMatches(m_output, std::regex("Sizeofcachedcode:" + toString(stats.totalCodeSize))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_message_if_cache_stats_requested_but_cache_disabled, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = true;
	m_options.showCacheStats = true;
	RandomisingAlgorithm algorithm;

	AlgorithmRunner runner(m_population, {nullptr}, m_options, m_output);
	runner.run(algorithm);

	BOOST_TEST(nextLineMatches(m_output, std::regex(".*")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("-+CACHESTATS-+")));
	BOOST_TEST(nextLineMatches(m_output, std::regex(stripWhitespace("Program cache disabled"))));
	BOOST_TEST(m_output.peek() == EOF);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_partial_stats_and_message_if_some_caches_disabled, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 1;
	m_options.showInitialPopulation = false;
	m_options.showRoundInfo = false;
	m_options.showOnlyTopChromosome = true;
	m_options.showCacheStats = true;
	RandomisingAlgorithm algorithm;

	CharStream sourceStream = CharStream("{}", "");
	std::shared_ptr<ProgramCache> cache = std::make_shared<ProgramCache>(std::get<Program>(Program::load(sourceStream)));

	AlgorithmRunner runner(m_population, {cache, nullptr}, m_options, m_output);
	BOOST_REQUIRE(cache->gatherStats().roundEntryCounts.size() == 0);

	runner.run(algorithm);
	BOOST_TEST(nextLineMatches(m_output, std::regex(".*")));
	BOOST_TEST(nextLineMatches(m_output, std::regex("-+CACHESTATS-+")));
	BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Totalhits:\d+)")));
	BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Totalmisses:\d+)")));
	BOOST_TEST(nextLineMatches(m_output, std::regex(R"(Sizeofcachedcode:\d+)")));
	BOOST_TEST(nextLineMatches(m_output, std::regex(stripWhitespace("Program cache disabled for 1 out of 2 programs"))));
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

	std::vector<std::string> originalContent = {"Original content"};
	{
		std::ofstream tmpFile(m_autosavePath);
		tmpFile << originalContent[0] << std::endl;
	}
	assert(fs::exists(m_autosavePath));
	assert(readLinesFromFile(m_autosavePath) == originalContent);

	runner.run(m_algorithm);

	BOOST_TEST(fs::is_regular_file(m_autosavePath));
	BOOST_TEST(readLinesFromFile(m_autosavePath) != originalContent);
}

// Disabled due to a fairly obscure bug in GCC 13+
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109561
// The issue is on line 444, and more specifically the Options struct std::optional<std::string>
/*
BOOST_FIXTURE_TEST_CASE(run_should_not_save_population_to_file_if_autosave_file_not_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 5;
	m_options.populationAutosaveFile = std::nullopt;
	AlgorithmRunner runner(m_population, {}, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	runner.run(m_algorithm);

	BOOST_TEST(!fs::exists(m_autosavePath));
}
*/

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
	std::vector<std::shared_ptr<ProgramCache>> caches = {
		std::make_shared<ProgramCache>(std::get<Program>(Program::load(sourceStream))),
		std::make_shared<ProgramCache>(std::get<Program>(Program::load(sourceStream))),
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
