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

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/Population.h>
#include <tools/yulPhaser/Program.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/UnusedPruner.h>

#include <liblangutil/CharStream.h>

#include <boost/test/unit_test.hpp>

#include <optional>
#include <string>
#include <sstream>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace boost::unit_test::framework;

namespace solidity::phaser::test
{

namespace
{
	bool fitnessNotSet(Individual const& individual)
	{
		return !individual.fitness.has_value();
	}

	bool fitnessSet(Individual const& individual)
	{
		return individual.fitness.has_value();
	}
}

class PopulationFixture
{
protected:
	PopulationFixture():
		m_sourceStream(SampleSourceCode, ""),
		m_program(Program::load(m_sourceStream)) {}

	static constexpr char SampleSourceCode[] =
		"{\n"
		"    let factor := 13\n"
		"    {\n"
		"        if factor\n"
		"        {\n"
		"            let variable := add(1, 2)\n"
		"        }\n"
		"        let result := factor\n"
		"    }\n"
		"    let something := 6\n"
		"    {\n"
		"        {\n"
		"            {\n"
		"                let value := 15\n"
		"            }\n"
		"        }\n"
		"    }\n"
		"    let something_else := mul(mul(something, 1), add(factor, 0))\n"
		"    if 1 { let x := 1 }\n"
		"    if 0 { let y := 2 }\n"
		"}\n";

	CharStream m_sourceStream;
	Program m_program;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(PopulationTest)

BOOST_FIXTURE_TEST_CASE(constructor_should_copy_chromosomes_and_not_compute_fitness, PopulationFixture)
{
	vector<Chromosome> chromosomes = {
		Chromosome::makeRandom(5),
		Chromosome::makeRandom(10),
	};
	Population population(m_program, chromosomes);

	BOOST_TEST(population.individuals().size() == 2);
	BOOST_TEST(population.individuals()[0].chromosome == chromosomes[0]);
	BOOST_TEST(population.individuals()[1].chromosome == chromosomes[1]);

	auto fitnessNotSet = [](auto const& individual){ return !individual.fitness.has_value(); };
	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_get_chromosome_lengths_from_specified_generator, PopulationFixture)
{
	size_t chromosomeCount = 30;
	size_t maxLength = 5;
	assert(chromosomeCount % maxLength == 0);

	auto nextLength = [counter = 0, maxLength]() mutable { return counter++ % maxLength; };
	auto population = Population::makeRandom(m_program, chromosomeCount, nextLength);

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
	auto population = Population::makeRandom(m_program, 100, 5, 10);
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

	auto population = Population::makeRandom(m_program, populationSize, minLength, maxLength);
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
	auto population = Population::makeRandom(m_program, populationSize, chromosomeLength, chromosomeLength);

	vector<size_t> samples;
	for (auto& individual: population.individuals())
		for (auto& step: individual.chromosome.optimisationSteps())
			samples.push_back(stepIndices.at(step));

	const double expectedValue = (stepIndices.size() - 1) / 2.0;
	const double variance = (stepIndices.size() * stepIndices.size() - 1) / 12.0;

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_FIXTURE_TEST_CASE(makeRandom_should_not_compute_fitness, PopulationFixture)
{
	auto population = Population::makeRandom(m_program, 3, 5, 10);

	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));
}

BOOST_FIXTURE_TEST_CASE(run_should_evaluate_fitness, PopulationFixture)
{
	stringstream output;
	auto population = Population::makeRandom(m_program, 5, 5, 10);
	assert(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));

	population.run(1, output);

	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessSet));
}

BOOST_FIXTURE_TEST_CASE(run_should_not_make_fitness_of_top_chromosomes_worse, PopulationFixture)
{
	stringstream output;
	vector<Chromosome> chromosomes = {
		Chromosome(vector<string>{StructuralSimplifier::name}),
		Chromosome(vector<string>{BlockFlattener::name}),
		Chromosome(vector<string>{SSAReverser::name}),
		Chromosome(vector<string>{UnusedPruner::name}),
		Chromosome(vector<string>{StructuralSimplifier::name, BlockFlattener::name}),
	};
	Population population(m_program, chromosomes);

	size_t initialTopFitness[2] = {
		Population::measureFitness(chromosomes[0], m_program),
		Population::measureFitness(chromosomes[1], m_program),
	};

	for (int i = 0; i < 6; ++i)
	{
		population.run(1, output);
		BOOST_TEST(population.individuals().size() == 5);
		BOOST_TEST(fitnessSet(population.individuals()[0]));
		BOOST_TEST(fitnessSet(population.individuals()[1]));

		size_t currentTopFitness[2] = {
			population.individuals()[0].fitness.value(),
			population.individuals()[1].fitness.value(),
		};
		BOOST_TEST(currentTopFitness[0] <= initialTopFitness[0]);
		BOOST_TEST(currentTopFitness[1] <= initialTopFitness[1]);
		BOOST_TEST(currentTopFitness[0] <= currentTopFitness[1]);
	}
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
