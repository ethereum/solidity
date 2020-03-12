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

#include <test/yulPhaser/Common.h>

#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Population.h>
#include <tools/yulPhaser/Program.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <algorithm>
#include <vector>

using namespace std;
using namespace boost::unit_test::framework;
using namespace boost::test_tools;
using namespace solidity::langutil;
using namespace solidity::util;

namespace solidity::phaser::test
{

class DummyAlgorithm: public GeneticAlgorithm
{
public:
	using GeneticAlgorithm::GeneticAlgorithm;
	void runNextRound() override { ++m_currentRound; }

	size_t m_currentRound = 0;
};

class GeneticAlgorithmFixture
{
protected:
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
	output_test_stream m_output;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(GeneticAlgorithmsTest)
BOOST_AUTO_TEST_SUITE(GeneticAlgorithmTest)

BOOST_FIXTURE_TEST_CASE(run_should_call_runNextRound_once_per_round, GeneticAlgorithmFixture)
{
	DummyAlgorithm algorithm(Population(m_fitnessMetric), m_output);

	BOOST_TEST(algorithm.m_currentRound == 0);
	algorithm.run(10);
	BOOST_TEST(algorithm.m_currentRound == 10);
	algorithm.run(3);
	BOOST_TEST(algorithm.m_currentRound == 13);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_the_top_chromosome, GeneticAlgorithmFixture)
{
	// run() is allowed to print more but should at least print the first one

	DummyAlgorithm algorithm(
		// NOTE: Chromosomes chosen so that they're not substrings of each other and are not
		// words likely to appear in the output in normal circumstances.
		Population(m_fitnessMetric, {Chromosome("fcCUnDve"), Chromosome("jsxIOo"), Chromosome("ighTLM")}),
		m_output
	);

	BOOST_TEST(m_output.is_empty());
	algorithm.run(1);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(algorithm.population().individuals()[0].chromosome)) == 1);
	algorithm.run(3);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(algorithm.population().individuals()[0].chromosome)) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(RandomAlgorithmTest)

BOOST_FIXTURE_TEST_CASE(runNextRound_should_preserve_elite_and_randomise_rest_of_population, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	RandomAlgorithm algorithm(population, m_output, {0.5, 1, 1});
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();
	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{1, 1, 1, 1, 3, 3, 3, 3}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_elite_with_worse_individuals, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	RandomAlgorithm algorithm(population, m_output, {0.5, 7, 7});
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();
	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 7, 7, 7, 7}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_replace_all_chromosomes_if_zero_size_elite, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	RandomAlgorithm algorithm(population, m_output, {0.0, 1, 1});
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();
	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{1, 1, 1, 1, 1, 1, 1, 1}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_any_chromosomes_if_whole_population_is_the_elite, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	RandomAlgorithm algorithm(population, m_output, {1.0, 1, 1});
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();
	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(GenerationalElitistWithExclusivePoolsTest)

BOOST_FIXTURE_TEST_CASE(runNextRound_should_preserve_elite_and_regenerate_rest_of_population, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 6, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.2,
		/* crossoverPoolSize = */ 0.2,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 1.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 1.0,
	};
	GenerationalElitistWithExclusivePools algorithm(population, m_output, options);
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();

	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{0, 0, 3, 3, 3, 3, 3, 3, 3, 3}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_elite_with_worse_individuals, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 6, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.2,
		/* crossoverPoolSize = */ 0.2,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 0.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 1.0,
	};
	GenerationalElitistWithExclusivePools algorithm(population, m_output, options);
	assert(chromosomeLengths(algorithm.population()) == (vector<size_t>{3, 3, 3, 3, 3, 3, 5, 5, 5, 5}));

	algorithm.runNextRound();

	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{3, 3, 3, 3, 3, 3, 3, 3, 7, 7}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_generate_individuals_in_the_crossover_pool_by_mutating_the_elite, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 20, 5, 5);

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.8,
		/* crossoverPoolSize = */ 0.0,
		/* randomisationChance = */ 0.5,
		/* deletionVsAdditionChance = */ 0.5,
		/* percentGenesToRandomise = */ 1.0,
		/* percentGenesToAddOrDelete = */ 1.0,
	};
	GenerationalElitistWithExclusivePools algorithm(population, m_output, options);

	SimulationRNG::reset(1);
	algorithm.runNextRound();

	BOOST_TEST((
		chromosomeLengths(algorithm.population()) ==
		vector<size_t>{0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 11, 11, 11}
	));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_generate_individuals_in_the_crossover_pool_by_crossing_over_the_elite, GeneticAlgorithmFixture)
{
	auto population = (
		Population(m_fitnessMetric, {Chromosome("aa"), Chromosome("ff")}) +
		Population::makeRandom(m_fitnessMetric, 8, 6, 6)
	);

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.0,
		/* crossoverPoolSize = */ 0.8,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 0.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 0.0,
	};
	GenerationalElitistWithExclusivePools algorithm(population, m_output, options);
	assert((chromosomeLengths(algorithm.population()) == vector<size_t>{2, 2, 6, 6, 6, 6, 6, 6, 6, 6}));

	SimulationRNG::reset(1);
	algorithm.runNextRound();

	vector<Individual> const& newIndividuals = algorithm.population().individuals();
	BOOST_TEST((chromosomeLengths(algorithm.population()) == vector<size_t>{2, 2, 2, 2, 2, 2, 2, 2, 2, 2}));
	for (auto& individual: newIndividuals)
		BOOST_TEST((
			individual.chromosome == Chromosome("aa") ||
			individual.chromosome == Chromosome("af") ||
			individual.chromosome == Chromosome("fa") ||
			individual.chromosome == Chromosome("ff")
		));
	BOOST_TEST(any_of(newIndividuals.begin() + 2, newIndividuals.end(), [](auto& individual){
		return individual.chromosome != Chromosome("aa") && individual.chromosome != Chromosome("ff");
	}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
