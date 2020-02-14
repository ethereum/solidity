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

#include <libyul/optimiser/Suite.h>

#include <boost/test/unit_test.hpp>

#include <set>

using namespace std;
using namespace solidity::yul;
using namespace boost::test_tools;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(CommonTest)

BOOST_AUTO_TEST_CASE(ChromosomeLengthMetric_evaluate_should_return_chromosome_length)
{
	BOOST_TEST(ChromosomeLengthMetric{}.evaluate(Chromosome()) == 0);
	BOOST_TEST(ChromosomeLengthMetric{}.evaluate(Chromosome("a")) == 1);
	BOOST_TEST(ChromosomeLengthMetric{}.evaluate(Chromosome("aaaaa")) == 5);
}

BOOST_AUTO_TEST_CASE(chromosomeLengths_should_return_lengths_of_all_chromosomes_in_a_population)
{
	shared_ptr<FitnessMetric> fitnessMetric = make_shared<ChromosomeLengthMetric>();

	Population population1(fitnessMetric, {Chromosome(), Chromosome("a"), Chromosome("aa"), Chromosome("aaa")});
	BOOST_TEST((chromosomeLengths(population1) == vector<size_t>{0, 1, 2, 3}));

	Population population2(fitnessMetric);
	BOOST_TEST((chromosomeLengths(population2) == vector<size_t>{}));
}

BOOST_AUTO_TEST_CASE(enumerateOptimisationSteps_should_assing_indices_to_all_available_optimisation_steps)
{
	map<string, char> stepsAndAbbreviations = OptimiserSuite::stepNameToAbbreviationMap();
	map<string, size_t> stepsAndIndices = enumerateOptmisationSteps();
	BOOST_TEST(stepsAndIndices.size() == stepsAndAbbreviations.size());

	set<string> stepsSoFar;
	for (auto& [name, index]: stepsAndIndices)
	{
		BOOST_TEST(index >= 0);
		BOOST_TEST(index <= stepsAndAbbreviations.size());
		BOOST_TEST(stepsAndAbbreviations.count(name) == 1);
		BOOST_TEST(stepsSoFar.count(name) == 0);

		stepsSoFar.insert(name);
	}
}

BOOST_AUTO_TEST_CASE(stripWhitespace_should_remove_all_whitespace_characters_from_a_string)
{
	BOOST_TEST(stripWhitespace("") == "");
	BOOST_TEST(stripWhitespace(" ") == "");
	BOOST_TEST(stripWhitespace(" \n\t\v ") == "");

	BOOST_TEST(stripWhitespace("abc") == "abc");
	BOOST_TEST(stripWhitespace(" abc") == "abc");
	BOOST_TEST(stripWhitespace("abc ") == "abc");
	BOOST_TEST(stripWhitespace(" a b c ") == "abc");
	BOOST_TEST(stripWhitespace(" a b\tc\n") == "abc");
	BOOST_TEST(stripWhitespace("   a   b \n\n  c \n\t\v") == "abc");
}

BOOST_AUTO_TEST_CASE(countSubstringOccurrences_should_count_non_overlapping_substring_occurrences_in_a_string)
{
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "a") == 6);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "aa") == 2);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "aaa") == 2);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "aaab") == 1);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "b") == 2);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "d") == 1);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "cdc") == 1);

	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "x") == 0);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "aaaa") == 0);
	BOOST_TEST(countSubstringOccurrences("aaabcdcbaaa", "dcd") == 0);

	BOOST_TEST(countSubstringOccurrences("", "a") == 0);
	BOOST_TEST(countSubstringOccurrences("", "aa") == 0);
	BOOST_TEST(countSubstringOccurrences("a", "aa") == 0);
}

BOOST_AUTO_TEST_CASE(mean_should_calculate_statistical_mean)
{
	BOOST_TEST(mean<int>({0}) == 0.0);
	BOOST_TEST(mean<int>({0, 0, 0, 0}) == 0.0);
	BOOST_TEST(mean<int>({5, 5, 5, 5}) == 5.0);
	BOOST_TEST(mean<int>({0, 1, 2, 3}) == 1.5);
	BOOST_TEST(mean<int>({-4, -3, -2, -1, 0, 1, 2, 3}) == -0.5);

	BOOST_TEST(mean<double>({1.3, 1.1, 0.0, 1.5, 1.1, 2.0, 1.5, 1.5}) == 1.25);
}

BOOST_AUTO_TEST_CASE(meanSquaredError_should_calculate_average_squared_difference_between_samples_and_expected_value)
{
	BOOST_TEST(meanSquaredError<int>({0}, 0.0) == 0.0);
	BOOST_TEST(meanSquaredError<int>({0}, 1.0) == 1.0);
	BOOST_TEST(meanSquaredError<int>({0, 0, 0, 0}, 0.0) == 0.0);
	BOOST_TEST(meanSquaredError<int>({0, 0, 0, 0}, 1.0) == 1.0);
	BOOST_TEST(meanSquaredError<int>({0, 0, 0, 0}, 2.0) == 4.0);
	BOOST_TEST(meanSquaredError<int>({5, 5, 5, 5}, 1.0) == 16.0);
	BOOST_TEST(meanSquaredError<int>({0, 1, 2, 3}, 2.0) == 1.5);
	BOOST_TEST(meanSquaredError<int>({-4, -3, -2, -1, 0, 1, 2, 3}, -4.0) == 17.5);

	BOOST_TEST(meanSquaredError<double>({1.3, 1.1, 0.0, 1.5, 1.1, 2.0, 1.5, 1.5}, 1.0) == 0.3575, tolerance(0.0001));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
