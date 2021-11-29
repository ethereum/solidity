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
using namespace solidity::util;

namespace solidity::phaser::test
{

class GeneticAlgorithmFixture
{
protected:
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
};

class ClassicGeneticAlgorithmFixture: public GeneticAlgorithmFixture
{
protected:
	ClassicGeneticAlgorithm::Options m_options = {
		/* elitePoolSize = */ 0.0,
		/* crossoverChance = */ 0.0,
		/* mutationChance = */ 0.0,
		/* deletionChance = */ 0.0,
		/* additionChance = */ 0.0,
		/* CrossoverChoice = */ CrossoverChoice::SinglePoint,
		/* uniformCrossoverSwapChance= */ 0.5,
	};
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
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
		/* CrossoverChoice = */ CrossoverChoice::SinglePoint,
		/* uniformCrossoverSwapChance= */ 0.5,
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
		/* CrossoverChoice = */ CrossoverChoice::SinglePoint,
		/* uniformCrossoverSwapChance= */ 0.5,
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
		/* CrossoverChoice = */ CrossoverChoice::SinglePoint,
		/* uniformCrossoverSwapChance= */ 0.5,
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
		/* CrossoverChoice = */ CrossoverChoice::SinglePoint,
		/* uniformCrossoverSwapChance= */ 0.5,
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
BOOST_AUTO_TEST_SUITE(ClassicGeneticAlgorithmTest)

BOOST_FIXTURE_TEST_CASE(runNextRound_should_select_individuals_with_probability_proportional_to_fitness, ClassicGeneticAlgorithmFixture)
{
	constexpr double relativeTolerance = 0.1;
	constexpr size_t populationSize = 1000;
	assert(populationSize % 4 == 0 && "Choose a number divisible by 4 for this test");

	auto population =
		Population::makeRandom(m_fitnessMetric, populationSize / 4, 0, 0) +
		Population::makeRandom(m_fitnessMetric, populationSize / 4, 1, 1) +
		Population::makeRandom(m_fitnessMetric, populationSize / 4, 2, 2) +
		Population::makeRandom(m_fitnessMetric, populationSize / 4, 3, 3);

	map<size_t, double> expectedProbabilities = {
		{0, 4.0 / (4 + 3 + 2 + 1)},
		{1, 3.0 / (4 + 3 + 2 + 1)},
		{2, 2.0 / (4 + 3 + 2 + 1)},
		{3, 1.0 / (4 + 3 + 2 + 1)},
	};
	double const expectedValue = (
		0.0 * expectedProbabilities[0] +
		1.0 * expectedProbabilities[1] +
		2.0 * expectedProbabilities[2] +
		3.0 * expectedProbabilities[3]
	);
	double const variance = (
		(0.0 - expectedValue) * (0.0 - expectedValue) * expectedProbabilities[0] +
		(1.0 - expectedValue) * (1.0 - expectedValue) * expectedProbabilities[1] +
		(2.0 - expectedValue) * (2.0 - expectedValue) * expectedProbabilities[2] +
		(3.0 - expectedValue) * (3.0 - expectedValue) * expectedProbabilities[3]
	);

	ClassicGeneticAlgorithm algorithm(m_options);
	Population newPopulation = algorithm.runNextRound(population);

	BOOST_TEST(newPopulation.individuals().size() == population.individuals().size());

	vector<size_t> newFitness = chromosomeLengths(newPopulation);
	BOOST_TEST(abs(mean(newFitness) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(newFitness, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_select_only_individuals_existing_in_the_original_population, ClassicGeneticAlgorithmFixture)
{
	constexpr size_t populationSize = 1000;
	auto population = Population::makeRandom(m_fitnessMetric, populationSize, 1, 10);

	set<string> originalSteps;
	for (auto const& individual: population.individuals())
		originalSteps.insert(toString(individual.chromosome));

	ClassicGeneticAlgorithm algorithm(m_options);
	Population newPopulation = algorithm.runNextRound(population);

	for (auto const& individual: newPopulation.individuals())
		BOOST_TEST(originalSteps.count(toString(individual.chromosome)) == 1);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_do_crossover, ClassicGeneticAlgorithmFixture)
{
	auto population = Population(m_fitnessMetric, {
		Chromosome("aa"), Chromosome("aa"), Chromosome("aa"),
		Chromosome("ff"), Chromosome("ff"), Chromosome("ff"),
		Chromosome("gg"), Chromosome("gg"), Chromosome("gg"),
	});

	set<string> originalSteps{"aa", "ff", "gg"};
	set<string> crossedSteps{"af", "fa", "fg", "gf", "ga", "ag"};

	m_options.crossoverChance = 0.8;
	ClassicGeneticAlgorithm algorithm(m_options);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	size_t totalCrossed = 0;
	size_t totalUnchanged = 0;
	for (auto const& individual: newPopulation.individuals())
	{
		totalCrossed += crossedSteps.count(toString(individual.chromosome));
		totalUnchanged += originalSteps.count(toString(individual.chromosome));
	}
	BOOST_TEST(totalCrossed + totalUnchanged == newPopulation.individuals().size());
	BOOST_TEST(totalCrossed >= 2);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_do_mutation, ClassicGeneticAlgorithmFixture)
{
	m_options.mutationChance = 0.6;
	ClassicGeneticAlgorithm algorithm(m_options);

	constexpr size_t populationSize = 1000;
	constexpr double relativeTolerance = 0.05;
	double const expectedValue = m_options.mutationChance;
	double const variance = m_options.mutationChance * (1 - m_options.mutationChance);

	Chromosome chromosome("aaaaaaaaaa");
	vector<Chromosome> chromosomes(populationSize, chromosome);
	Population population(m_fitnessMetric, chromosomes);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	vector<size_t> bernoulliTrials;
	for (auto const& individual: newPopulation.individuals())
	{
		string steps = toString(individual.chromosome);
		for (char step: steps)
			bernoulliTrials.push_back(static_cast<size_t>(step != 'a'));
	}

	BOOST_TEST(abs(mean(bernoulliTrials) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(bernoulliTrials, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_do_deletion, ClassicGeneticAlgorithmFixture)
{
	m_options.deletionChance = 0.6;
	ClassicGeneticAlgorithm algorithm(m_options);

	constexpr size_t populationSize = 1000;
	constexpr double relativeTolerance = 0.05;
	double const expectedValue = m_options.deletionChance;
	double const variance = m_options.deletionChance * (1 - m_options.deletionChance);

	Chromosome chromosome("aaaaaaaaaa");
	vector<Chromosome> chromosomes(populationSize, chromosome);
	Population population(m_fitnessMetric, chromosomes);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	vector<size_t> bernoulliTrials;
	for (auto const& individual: newPopulation.individuals())
	{
		string steps = toString(individual.chromosome);
		for (size_t i = 0; i < chromosome.length(); ++i)
			bernoulliTrials.push_back(static_cast<size_t>(i >= steps.size()));
	}

	BOOST_TEST(abs(mean(bernoulliTrials) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(bernoulliTrials, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_do_addition, ClassicGeneticAlgorithmFixture)
{
	m_options.additionChance = 0.6;
	ClassicGeneticAlgorithm algorithm(m_options);

	constexpr size_t populationSize = 1000;
	constexpr double relativeTolerance = 0.05;
	double const expectedValue = m_options.additionChance;
	double const variance = m_options.additionChance * (1 - m_options.additionChance);

	Chromosome chromosome("aaaaaaaaaa");
	vector<Chromosome> chromosomes(populationSize, chromosome);
	Population population(m_fitnessMetric, chromosomes);

	SimulationRNG::reset(1);
	Population newPopulation = algorithm.runNextRound(population);

	vector<size_t> bernoulliTrials;
	for (auto const& individual: newPopulation.individuals())
	{
		string steps = toString(individual.chromosome);
		for (size_t i = 0; i < chromosome.length() + 1; ++i)
		{
			BOOST_REQUIRE(chromosome.length() <= steps.size() && steps.size() <= 2 * chromosome.length() + 1);
			bernoulliTrials.push_back(static_cast<size_t>(i < steps.size() - chromosome.length()));
		}
	}

	BOOST_TEST(abs(mean(bernoulliTrials) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(bernoulliTrials, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(runNextRound_should_preserve_elite, ClassicGeneticAlgorithmFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 4, 3, 3) + Population::makeRandom(m_fitnessMetric, 6, 5, 5);
	assert((chromosomeLengths(population) == vector<size_t>{3, 3, 3, 3, 5, 5, 5, 5, 5, 5}));

	m_options.elitePoolSize = 0.5;
	m_options.deletionChance = 1.0;
	ClassicGeneticAlgorithm algorithm(m_options);
	Population newPopulation = algorithm.runNextRound(population);

	BOOST_TEST((chromosomeLengths(newPopulation) == vector<size_t>{0, 0, 0, 0, 0, 3, 3, 3, 3, 5}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
