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

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/optimiser/UnusedPruner.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::yul;
using namespace solidity::util;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(ChromosomeTest)

BOOST_AUTO_TEST_CASE(makeRandom_should_create_chromosome_with_random_optimisation_steps)
{
	constexpr uint32_t numSteps = 1000;

	auto chromosome1 = Chromosome::makeRandom(numSteps);
	auto chromosome2 = Chromosome::makeRandom(numSteps);
	BOOST_CHECK_EQUAL(chromosome1.length(), numSteps);
	BOOST_CHECK_EQUAL(chromosome2.length(), numSteps);

	multiset<string> steps1;
	multiset<string> steps2;
	for (auto const& step: chromosome1.optimisationSteps())
		steps1.insert(step);
	for (auto const& step: chromosome2.optimisationSteps())
		steps2.insert(step);

	// Check if steps are different and also if they're not just a permutation of the same set.
	// Technically they could be the same and still random but the probability is infinitesimally low.
	BOOST_TEST(steps1 != steps2);
}

BOOST_AUTO_TEST_CASE(constructor_should_store_optimisation_steps)
{
	vector<string> steps = {
		StructuralSimplifier::name,
		BlockFlattener::name,
		UnusedPruner::name,
	};
	Chromosome chromosome(steps);

	BOOST_TEST(steps == chromosome.optimisationSteps());
}

BOOST_AUTO_TEST_CASE(constructor_should_allow_duplicate_steps)
{
	vector<string> steps = {
		StructuralSimplifier::name,
		StructuralSimplifier::name,
		BlockFlattener::name,
		UnusedPruner::name,
		BlockFlattener::name,
	};
	Chromosome chromosome(steps);

	BOOST_TEST(steps == chromosome.optimisationSteps());
}

BOOST_AUTO_TEST_CASE(output_operator_should_create_concise_and_unambiguous_string_representation)
{
	vector<string> allSteps;
	for (auto const& step: OptimiserSuite::allSteps())
		allSteps.push_back(step.first);
	Chromosome chromosome(allSteps);

	BOOST_TEST(chromosome.length() == allSteps.size());
	BOOST_TEST(chromosome.optimisationSteps() == allSteps);
	BOOST_TEST(toString(chromosome) == "fcCUnDvejsxIOoighTLMrmVatud");
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
