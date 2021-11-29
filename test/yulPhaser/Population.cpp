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

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/PairSelections.h>
#include <tools/yulPhaser/Population.h>
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/Selections.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/UnusedPruner.h>

#include <liblangutil/CharStream.h>

#include <boost/test/unit_test.hpp>

#include <cmath>
#include <optional>
#include <string>
#include <sstream>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace boost::unit_test::framework;

namespace solidity::phaser::test
{

class PopulationFixture
{
protected:
	static ChromosomePair twoStepSwap(Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		return ChromosomePair{
			Chromosome(vector<string>{_chromosome1.optimisationSteps()[0], _chromosome2.optimisationSteps()[1]}),
			Chromosome(vector<string>{_chromosome2.optimisationSteps()[0], _chromosome1.optimisationSteps()[1]}),
		};
	}

	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(PopulationTest)

BOOST_AUTO_TEST_CASE(isFitter_should_use_fitness_as_the_main_criterion)
{
	BOOST_TEST(isFitter(Individual(Chromosome("a"), 5), Individual(Chromosome("a"), 10)));
	BOOST_TEST(!isFitter(Individual(Chromosome("a"), 10), Individual(Chromosome("a"), 5)));

	BOOST_TEST(isFitter(Individual(Chromosome("aaa"), 5), Individual(Chromosome("aaaaa"), 10)));
	BOOST_TEST(!isFitter(Individual(Chromosome("aaaaa"), 10), Individual(Chromosome("aaa"), 5)));

	BOOST_TEST(isFitter(Individual(Chromosome("aaaaa"), 5), Individual(Chromosome("aaa"), 10)));
	BOOST_TEST(!isFitter(Individual(Chromosome("aaa"), 10), Individual(Chromosome("aaaaa"), 5)));
}

BOOST_AUTO_TEST_CASE(isFitter_should_use_alphabetical_order_when_fitness_is_the_same)
{
	BOOST_TEST(isFitter(Individual(Chromosome("a"), 3), Individual(Chromosome("c"), 3)));
	BOOST_TEST(!isFitter(Individual(Chromosome("c"), 3), Individual(Chromosome("a"), 3)));

	BOOST_TEST(isFitter(Individual(Chromosome("a"), 3), Individual(Chromosome("aa"), 3)));
	BOOST_TEST(!isFitter(Individual(Chromosome("aa"), 3), Individual(Chromosome("a"), 3)));

	BOOST_TEST(isFitter(Individual(Chromosome("T"), 3), Individual(Chromosome("a"), 3)));
	BOOST_TEST(!isFitter(Individual(Chromosome("a"), 3), Individual(Chromosome("T"), 3)));
}

BOOST_AUTO_TEST_CASE(isFitter_should_return_false_for_identical_individuals)
{
	BOOST_TEST(!isFitter(Individual(Chromosome("a"), 3), Individual(Chromosome("a"), 3)));
	BOOST_TEST(!isFitter(Individual(Chromosome("acT"), 0), Individual(Chromosome("acT"), 0)));
}

BOOST_FIXTURE_TEST_CASE(constructor_should_copy_chromosomes_compute_fitness_and_sort_chromosomes, PopulationFixture)
{
	vector<Chromosome> chromosomes = {
		Chromosome::makeRandom(5),
		Chromosome::makeRandom(15),
		Chromosome::makeRandom(10),
	};
	Population population(m_fitnessMetric, chromosomes);

	vector<Individual> const& individuals = population.individuals();

	BOOST_TEST(individuals.size() == 3);
	BOOST_TEST(individuals[0].fitness == 5);
	BOOST_TEST(individuals[1].fitness == 10);
	BOOST_TEST(individuals[2].fitness == 15);
	BOOST_TEST(individuals[0].chromosome == chromosomes[0]);
	BOOST_TEST(individuals[1].chromosome == chromosomes[2]);
	BOOST_TEST(individuals[2].chromosome == chromosomes[1]);
}

BOOST_FIXTURE_TEST_CASE(constructor_should_accept_individuals_without_recalculating_fitness, PopulationFixture)
{
	vector<Individual> customIndividuals = {
		Individual(Chromosome("aaaccc"), 20),
		Individual(Chromosome("aaa"), 10),
		Individual(Chromosome("aaaf"), 30),
	};
	assert(customIndividuals[0].fitness != m_fitnessMetric->evaluate(customIndividuals[0].chromosome));
	assert(customIndividuals[1].fitness != m_fitnessMetric->evaluate(customIndividuals[1].chromosome));
	assert(customIndividuals[2].fitness != m_fitnessMetric->evaluate(customIndividuals[2].chromosome));

	Population population(m_fitnessMetric, customIndividuals);

	vector<Individual> expectedIndividuals{customIndividuals[1], customIndividuals[0], customIndividuals[2]};
	BOOST_TEST(population.individuals() == expectedIndividuals);
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_get_chromosome_lengths_from_specified_generator, PopulationFixture)
{
	size_t chromosomeCount = 30;
	size_t maxLength = 5;
	assert(chromosomeCount % maxLength == 0);

	auto nextLength = [counter = 0ul, maxLength]() mutable { return counter++ % maxLength; };
	auto population = Population::makeRandom(m_fitnessMetric, chromosomeCount, nextLength);

	// We can't rely on the order since the population sorts its chromosomes immediately but
	// we can check the number of occurrences of each length.
	for (size_t length = 0; length < maxLength; ++length)
		BOOST_TEST(
			count_if(
				population.individuals().begin(),
				population.individuals().end(),
				[&length](auto const& individual) { return individual.chromosome.length() == length; }
			) == chromosomeCount / maxLength
		);
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_get_chromosome_lengths_from_specified_range, PopulationFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 100, 5, 10);
	BOOST_TEST(all_of(
		population.individuals().begin(),
		population.individuals().end(),
		[](auto const& individual){ return 5 <= individual.chromosome.length() && individual.chromosome.length() <= 10; }
	));
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_use_random_chromosome_length, PopulationFixture)
{
	SimulationRNG::reset(1);
	constexpr int populationSize = 200;
	constexpr int minLength = 5;
	constexpr int maxLength = 10;
	constexpr double relativeTolerance = 0.05;

	auto population = Population::makeRandom(m_fitnessMetric, populationSize, minLength, maxLength);
	vector<size_t> samples = chromosomeLengths(population);

	const double expectedValue = (maxLength + minLength) / 2.0;
	const double variance = ((maxLength - minLength + 1) * (maxLength - minLength + 1) - 1) / 12.0;

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_return_population_with_random_chromosomes, PopulationFixture)
{
	SimulationRNG::reset(1);
	constexpr int populationSize = 100;
	constexpr int chromosomeLength = 30;
	constexpr double relativeTolerance = 0.01;

	map<string, size_t> stepIndices = enumerateOptmisationSteps();
	auto population = Population::makeRandom(m_fitnessMetric, populationSize, chromosomeLength, chromosomeLength);

	vector<size_t> samples;
	for (auto& individual: population.individuals())
		for (auto& step: individual.chromosome.optimisationSteps())
			samples.push_back(stepIndices.at(step));

	const double expectedValue = double(stepIndices.size() - 1) / 2.0;
	const double variance = double(stepIndices.size() * stepIndices.size() - 1) / 12.0;

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_compute_fitness, PopulationFixture)
{
	auto population = Population::makeRandom(m_fitnessMetric, 3, 5, 10);

	BOOST_TEST(population.individuals()[0].fitness == m_fitnessMetric->evaluate(population.individuals()[0].chromosome));
	BOOST_TEST(population.individuals()[1].fitness == m_fitnessMetric->evaluate(population.individuals()[1].chromosome));
	BOOST_TEST(population.individuals()[2].fitness == m_fitnessMetric->evaluate(population.individuals()[2].chromosome));
}

BOOST_FIXTURE_TEST_CASE(plus_operator_should_add_two_populations, PopulationFixture)
{
	BOOST_CHECK_EQUAL(
		Population(m_fitnessMetric, {Chromosome("ac"), Chromosome("cx")}) +
		Population(m_fitnessMetric, {Chromosome("g"), Chromosome("h"), Chromosome("iI")}),
		Population(m_fitnessMetric, {Chromosome("ac"), Chromosome("cx"), Chromosome("g"), Chromosome("h"), Chromosome("iI")})
	);
}

BOOST_FIXTURE_TEST_CASE(select_should_return_population_containing_individuals_indicated_by_selection, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("a"), Chromosome("c"), Chromosome("g"), Chromosome("h")});
	RangeSelection selection(0.25, 0.75);
	assert(selection.materialise(population.individuals().size()) == (vector<size_t>{1, 2}));

	BOOST_TEST(
		population.select(selection) ==
		Population(m_fitnessMetric, {population.individuals()[1].chromosome, population.individuals()[2].chromosome})
	);
}

BOOST_FIXTURE_TEST_CASE(select_should_include_duplicates_if_selection_contains_duplicates, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("a"), Chromosome("c")});
	MosaicSelection selection({0, 1}, 2.0);
	assert(selection.materialise(population.individuals().size()) == (vector<size_t>{0, 1, 0, 1}));

	BOOST_TEST(population.select(selection) == Population(m_fitnessMetric, {
		population.individuals()[0].chromosome,
		population.individuals()[1].chromosome,
		population.individuals()[0].chromosome,
		population.individuals()[1].chromosome,
	}));
}

BOOST_FIXTURE_TEST_CASE(select_should_return_empty_population_if_selection_is_empty, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("a"), Chromosome("c")});
	RangeSelection selection(0.0, 0.0);
	assert(selection.materialise(population.individuals().size()).empty());

	BOOST_TEST(population.select(selection).individuals().empty());
}

BOOST_FIXTURE_TEST_CASE(mutate_should_return_population_containing_individuals_indicated_by_selection_with_mutation_applied, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc"), Chromosome("gg"), Chromosome("hh")});
	RangeSelection selection(0.25, 0.75);
	assert(selection.materialise(population.individuals().size()) == (vector<size_t>{1, 2}));

	Population expectedPopulation(m_fitnessMetric, {Chromosome("fc"), Chromosome("fg")});

	BOOST_TEST(population.mutate(selection, geneSubstitution(0, BlockFlattener::name)) == expectedPopulation);
}

BOOST_FIXTURE_TEST_CASE(mutate_should_include_duplicates_if_selection_contains_duplicates, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("aa")});
	RangeSelection selection(0.0, 1.0);
	assert(selection.materialise(population.individuals().size()) == (vector<size_t>{0, 1}));

	BOOST_TEST(
		population.mutate(selection, geneSubstitution(0, BlockFlattener::name)) ==
		Population(m_fitnessMetric, {Chromosome("fa"), Chromosome("fa")})
	);
}

BOOST_FIXTURE_TEST_CASE(mutate_should_return_empty_population_if_selection_is_empty, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc")});
	RangeSelection selection(0.0, 0.0);
	assert(selection.materialise(population.individuals().size()).empty());

	BOOST_TEST(population.mutate(selection, geneSubstitution(0, BlockFlattener::name)).individuals().empty());
}

BOOST_FIXTURE_TEST_CASE(crossover_should_return_population_containing_individuals_indicated_by_selection_with_crossover_applied, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc"), Chromosome("gg"), Chromosome("hh")});
	PairMosaicSelection selection({{0, 1}, {2, 1}}, 1.0);
	assert(selection.materialise(population.individuals().size()) == (vector<tuple<size_t, size_t>>{{0, 1}, {2, 1}, {0, 1}, {2, 1}}));

	Population expectedPopulation(m_fitnessMetric, {Chromosome("ac"), Chromosome("ac"), Chromosome("gc"), Chromosome("gc")});

	BOOST_TEST(population.crossover(selection, fixedPointCrossover(0.5)) == expectedPopulation);
}

BOOST_FIXTURE_TEST_CASE(crossover_should_include_duplicates_if_selection_contains_duplicates, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("aa")});
	PairMosaicSelection selection({{0, 0}, {1, 1}}, 2.0);
	assert(selection.materialise(population.individuals().size()) == (vector<tuple<size_t, size_t>>{{0, 0}, {1, 1}, {0, 0}, {1, 1}}));

	BOOST_TEST(
		population.crossover(selection, fixedPointCrossover(0.5)) ==
		Population(m_fitnessMetric, {Chromosome("aa"), Chromosome("aa"), Chromosome("aa"), Chromosome("aa")})
	);
}

BOOST_FIXTURE_TEST_CASE(crossover_should_return_empty_population_if_selection_is_empty, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc")});
	PairMosaicSelection selection({}, 0.0);
	assert(selection.materialise(population.individuals().size()).empty());

	BOOST_TEST(population.crossover(selection, fixedPointCrossover(0.5)).individuals().empty());
}

BOOST_FIXTURE_TEST_CASE(symmetricCrossoverWithRemainder_should_return_crossed_population_and_remainder, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc"), Chromosome("gg"), Chromosome("hh")});
	PairMosaicSelection selection({{2, 1}}, 0.25);
	assert(selection.materialise(population.individuals().size()) == (vector<tuple<size_t, size_t>>{{2, 1}}));

	Population expectedCrossedPopulation(m_fitnessMetric, {Chromosome("gc"), Chromosome("cg")});
	Population expectedRemainder(m_fitnessMetric, {Chromosome("aa"), Chromosome("hh")});

	BOOST_TEST(
		population.symmetricCrossoverWithRemainder(selection, twoStepSwap) ==
		(tuple<Population, Population>{expectedCrossedPopulation, expectedRemainder})
	);
}

BOOST_FIXTURE_TEST_CASE(symmetricCrossoverWithRemainder_should_allow_crossing_the_same_individual_multiple_times, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc"), Chromosome("gg"), Chromosome("hh")});
	PairMosaicSelection selection({{0, 0}, {2, 1}}, 1.0);
	assert(selection.materialise(population.individuals().size()) == (vector<tuple<size_t, size_t>>{{0, 0}, {2, 1}, {0, 0}, {2, 1}}));

	Population expectedCrossedPopulation(m_fitnessMetric, {
		Chromosome("aa"), Chromosome("aa"),
		Chromosome("aa"), Chromosome("aa"),
		Chromosome("gc"), Chromosome("cg"),
		Chromosome("gc"), Chromosome("cg"),
	});
	Population expectedRemainder(m_fitnessMetric, {Chromosome("hh")});

	BOOST_TEST(
		population.symmetricCrossoverWithRemainder(selection, twoStepSwap) ==
		(tuple<Population, Population>{expectedCrossedPopulation, expectedRemainder})
	);
}

BOOST_FIXTURE_TEST_CASE(symmetricCrossoverWithRemainder_should_return_empty_population_if_selection_is_empty, PopulationFixture)
{
	Population population(m_fitnessMetric, {Chromosome("aa"), Chromosome("cc")});
	PairMosaicSelection selection({}, 0.0);
	assert(selection.materialise(population.individuals().size()).empty());

	BOOST_TEST(
		population.symmetricCrossoverWithRemainder(selection, twoStepSwap) ==
		(tuple<Population, Population>{Population(m_fitnessMetric), population})
	);
}

BOOST_FIXTURE_TEST_CASE(combine_should_add_two_populations_from_a_pair, PopulationFixture)
{
	Population population1(m_fitnessMetric, {Chromosome("aa"), Chromosome("hh")});
	Population population2(m_fitnessMetric, {Chromosome("gg"), Chromosome("cc")});

	BOOST_TEST(Population::combine({population1, population2}) == population1 + population2);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
