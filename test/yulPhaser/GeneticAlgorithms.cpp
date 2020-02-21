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

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <vector>

using namespace std;
using namespace boost::unit_test::framework;
using namespace boost::test_tools;

namespace solidity::phaser::test
{

class GeneticAlgorithmFixture
{
protected:
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(GeneticAlgorithmsTest)
BOOST_AUTO_TEST_SUITE(RandomAlgorithmTest)

BOOST_FIXTURE_TEST_CASE(runNextRound_should_preserve_elite_and_randomise_rest_of_population, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
	RandomAlgorithm algorithm({0.5, 1, 1});

	Population newPopulation = algorithm.runNextRound(population);
	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{1, 1, 1, 1, 3, 3, 3, 3}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_elite_with_worse_individuals, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
	RandomAlgorithm algorithm({0.5, 7, 7});

	Population newPopulation = algorithm.runNextRound(population);
	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{3, 3, 3, 3, 7, 7, 7, 7}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_replace_all_chromosomes_if_zero_size_elite, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
	RandomAlgorithm algorithm({0.0, 1, 1});

	Population newPopulation = algorithm.runNextRound(population);
	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{1, 1, 1, 1, 1, 1, 1, 1}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_any_chromosomes_if_whole_population_is_the_elite, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
	RandomAlgorithm algorithm({1.0, 1, 1});

	Population newPopulation = algorithm.runNextRound(population);
	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(GenerationalElitistWithExclusivePoolsTest)

BOOST_FIXTURE_TEST_CASE(runNextRound_should_preserve_elite_and_regenerate_rest_of_population, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 6, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 3, 3, 5, 5, 5, 5}));

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.2,
		/* crossoverPoolSize = */ 0.2,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 1.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 1.0,
	};
	GenerationalElitistWithExclusivePools algorithm(options);

	Population newPopulation = algorithm.runNextRound(population);

	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{0, 0, 3, 3, 3, 3, 3, 3, 3, 3}));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_not_replace_elite_with_worse_individuals, GeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 6, 3, 3) + Population::makeRandom(m_fitnessMetric, 4, 5, 5);
	assert(chromosomeLengths(population) == (vector<size_t>{3, 3, 3, 3, 3, 3, 5, 5, 5, 5}));

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.2,
		/* crossoverPoolSize = */ 0.2,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 0.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 1.0,
	};
	GenerationalElitistWithExclusivePools algorithm(options);

	Population newPopulation = algorithm.runNextRound(population);

	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{3, 3, 3, 3, 3, 3, 3, 3, 7, 7}));
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
	GenerationalElitistWithExclusivePools algorithm(options);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	BOOST_TEST((
		chromosomeLengths(newPopulation) ==
		vector<size_t>{0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 11, 11, 11}
	));
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_generate_individuals_in_the_crossover_pool_by_crossing_over_the_elite, GeneticAlgorithmFixture)
{
	auto population = (
		Population(m_fitnessMetric, {Chromosome("aa"), Chromosome("ff")}) +
		Population::makeRandom(m_fitnessMetric, 8, 6, 6)
	);
	assert((chromosomeLengths(population) == vector<size_t>{2, 2, 6, 6, 6, 6, 6, 6, 6, 6}));

	GenerationalElitistWithExclusivePools::Options options = {
		/* mutationPoolSize = */ 0.0,
		/* crossoverPoolSize = */ 0.8,
		/* randomisationChance = */ 0.0,
		/* deletionVsAdditionChance = */ 0.0,
		/* percentGenesToRandomise = */ 0.0,
		/* percentGenesToAddOrDelete = */ 0.0,
	};
	GenerationalElitistWithExclusivePools algorithm(options);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	vector<Individual> const& newIndividuals = newPopulation.individuals();
	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{2, 2, 2, 2, 2, 2, 2, 2, 2, 2}));
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
