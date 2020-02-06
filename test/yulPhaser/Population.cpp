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

BOOST_AUTO_TEST_CASE(constructor_should_copy_chromosomes_and_compute_fitness)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	vector<Chromosome> chromosomes = {
		Chromosome::makeRandom(5),
		Chromosome::makeRandom(10),
	};
	shared_ptr<FitnessMetric> fitnessMetric = make_shared<ProgramSize>(Program::load(sourceStream));
	Population population(fitnessMetric, chromosomes);

	vector<Individual> const& individuals = population.individuals();

	BOOST_TEST(individuals.size() == 2);
	BOOST_TEST(individuals[0].fitness == fitnessMetric->evaluate(individuals[0].chromosome));
	BOOST_TEST(individuals[1].fitness == fitnessMetric->evaluate(individuals[1].chromosome));
	BOOST_TEST(individuals[0].fitness <= individuals[0].fitness);

	size_t fitness[2] = {
		fitnessMetric->evaluate(chromosomes[0]),
		fitnessMetric->evaluate(chromosomes[1])
	};
	BOOST_TEST((
		// The order depends on fitness. We're getting random chromosomes so we have to be prepared
		// for either one being first.
		(
			individuals[0].chromosome == chromosomes[0] &&
			individuals[1].chromosome == chromosomes[1] &&
			fitness[0] <= fitness[1]
		) ||
		(
			individuals[0].chromosome == chromosomes[1] &&
			individuals[1].chromosome == chromosomes[0] &&
			fitness[0] >= fitness[1]
		)
	));
}

BOOST_AUTO_TEST_CASE(makeRandom_should_return_population_with_random_chromosomes)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	shared_ptr<FitnessMetric> fitnessMetric = make_shared<ProgramSize>(Program::load(sourceStream));
	auto population1 = Population::makeRandom(fitnessMetric, 100, []{ return 30; });
	auto population2 = Population::makeRandom(fitnessMetric, 100, []{ return 30; });

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

BOOST_AUTO_TEST_CASE(makeRandom_should_compute_fitness)
{
	CharStream sourceStream(sampleSourceCode, current_test_case().p_name);
	shared_ptr<FitnessMetric> fitnessMetric = make_shared<ProgramSize>(Program::load(sourceStream));
	auto population = Population::makeRandom(fitnessMetric, 3, []{ return 10; });

	BOOST_TEST(population.individuals()[0].fitness == fitnessMetric->evaluate(population.individuals()[0].chromosome));
	BOOST_TEST(population.individuals()[1].fitness == fitnessMetric->evaluate(population.individuals()[1].chromosome));
	BOOST_TEST(population.individuals()[2].fitness == fitnessMetric->evaluate(population.individuals()[2].chromosome));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
