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

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(PopulationTest)

string const& sampleSourceCode =
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

BOOST_AUTO_TEST_CASE(constructor_should_copy_chromosomes_and_not_compute_fitness)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	vector<Chromosome> chromosomes = {
		Chromosome::makeRandom(5),
		Chromosome::makeRandom(10),
	};
	Population population(Program::load(sourceStream), chromosomes);

	BOOST_TEST(population.individuals().size() == 2);
	BOOST_TEST(population.individuals()[0].chromosome == chromosomes[0]);
	BOOST_TEST(population.individuals()[1].chromosome == chromosomes[1]);

	auto fitnessNotSet = [](auto const& individual){ return !individual.fitness.has_value(); };
	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));
}

BOOST_AUTO_TEST_CASE(makeRandom_should_return_population_with_random_chromosomes)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	auto program = Program::load(sourceStream);
	auto population1 = Population::makeRandom(program, 100);
	auto population2 = Population::makeRandom(program, 100);

	BOOST_TEST(population1.individuals().size() == 100);
	BOOST_TEST(population2.individuals().size() == 100);

	int numMatchingPositions = 0;
	for (size_t i = 0; i < 100; ++i)
		if (population1.individuals()[i].chromosome == population2.individuals()[i].chromosome)
			++numMatchingPositions;

	// Assume that the results are random if there are no more than 10 identical chromosomes on the
	// same positions. One duplicate is very unlikely but still possible after billions of runs
	// (especially for short chromosomes). For ten the probability is so small that we can ignore it.
	BOOST_TEST(numMatchingPositions < 10);
}

BOOST_AUTO_TEST_CASE(makeRandom_should_not_compute_fitness)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	auto population = Population::makeRandom(Program::load(sourceStream), 5);

	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));
}

BOOST_AUTO_TEST_CASE(run_should_evaluate_fitness)
{
	stringstream output;
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	auto population = Population::makeRandom(Program::load(sourceStream), 5);
	assert(all_of(population.individuals().begin(), population.individuals().end(), fitnessNotSet));

	population.run(1, output);

	BOOST_TEST(all_of(population.individuals().begin(), population.individuals().end(), fitnessSet));
}

BOOST_AUTO_TEST_CASE(run_should_not_make_fitness_of_top_chromosomes_worse)
{
	stringstream output;
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	vector<Chromosome> chromosomes = {
		Chromosome({StructuralSimplifier::name}),
		Chromosome({BlockFlattener::name}),
		Chromosome({SSAReverser::name}),
		Chromosome({UnusedPruner::name}),
		Chromosome({StructuralSimplifier::name, BlockFlattener::name}),
	};
	auto program = Program::load(sourceStream);
	Population population(program, chromosomes);

	size_t initialTopFitness[2] = {
		Population::measureFitness(chromosomes[0], program),
		Population::measureFitness(chromosomes[1], program),
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
