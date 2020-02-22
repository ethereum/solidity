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

#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

using namespace std;
using namespace boost::unit_test::framework;
using namespace boost::test_tools;
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
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
	output_test_stream m_output;
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
	Population const m_population = Population::makeRandom(m_fitnessMetric, 5, 0, 20);
	RandomisingAlgorithm m_algorithm;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(AlgorithmRunnerTest)

BOOST_FIXTURE_TEST_CASE(run_should_call_runNextRound_once_per_round, AlgorithmRunnerFixture)
{
	m_options.maxRounds = 5;
	AlgorithmRunner runner(Population(m_fitnessMetric), m_options, m_output);

	CountingAlgorithm algorithm;

	BOOST_TEST(algorithm.m_currentRound == 0);
	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 5);
	runner.run(algorithm);
	BOOST_TEST(algorithm.m_currentRound == 10);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_the_top_chromosome, AlgorithmRunnerFixture)
{
	// run() is allowed to print more but should at least print the first one

	m_options.maxRounds = 1;
	AlgorithmRunner runner(
		// NOTE: Chromosomes chosen so that they're not substrings of each other and are not
		// words likely to appear in the output in normal circumstances.
		Population(m_fitnessMetric, {Chromosome("fcCUnDve"), Chromosome("jsxIOo"), Chromosome("ighTLM")}),
		m_options,
		m_output
	);

	CountingAlgorithm algorithm;

	BOOST_TEST(m_output.is_empty());
	runner.run(algorithm);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(runner.population().individuals()[0].chromosome)) == 1);
	runner.run(algorithm);
	runner.run(algorithm);
	runner.run(algorithm);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(runner.population().individuals()[0].chromosome)) == 4);
}

BOOST_FIXTURE_TEST_CASE(run_should_save_initial_population_to_file_if_autosave_file_specified, AlgorithmRunnerAutosaveFixture)
{
	m_options.maxRounds = 0;
	m_options.populationAutosaveFile = m_autosavePath;
	AlgorithmRunner runner(m_population, m_options, m_output);
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
	AlgorithmRunner runner(m_population, m_options, m_output);
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
	AlgorithmRunner runner(m_population, m_options, m_output);
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
	AlgorithmRunner runner(m_population, m_options, m_output);
	assert(!fs::exists(m_autosavePath));

	runner.run(m_algorithm);

	BOOST_TEST(!fs::exists(m_autosavePath));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
