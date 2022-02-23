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

#include <tools/yulPhaser/PairSelections.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

using namespace std;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(PairSelectionsTest)
BOOST_AUTO_TEST_SUITE(RandomPairSelectionTest)

BOOST_AUTO_TEST_CASE(materialise_should_return_random_values_with_equal_probabilities)
{
	constexpr int collectionSize = 10;
	constexpr int selectionSize = 100;
	constexpr double relativeTolerance = 0.1;
	constexpr double expectedValue = (collectionSize - 1) / 2.0;
	constexpr double variance = (collectionSize * collectionSize - 1) / 12.0;

	SimulationRNG::reset(1);
	vector<tuple<size_t, size_t>> pairs = RandomPairSelection(selectionSize).materialise(collectionSize);
	vector<size_t> samples;
	for (auto& [first, second]: pairs)
	{
		samples.push_back(first);
		samples.push_back(second);
	}

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_only_values_that_can_be_used_as_collection_indices)
{
	const size_t collectionSize = 200;

	vector<tuple<size_t, size_t>> pairs = RandomPairSelection(0.5).materialise(collectionSize);

	BOOST_TEST(pairs.size() == 100);
	BOOST_TEST(all_of(pairs.begin(), pairs.end(), [&](auto const& pair){ return get<0>(pair) <= collectionSize; }));
	BOOST_TEST(all_of(pairs.begin(), pairs.end(), [&](auto const& pair){ return get<1>(pair) <= collectionSize; }));
}

BOOST_AUTO_TEST_CASE(materialise_should_never_return_a_pair_of_identical_indices)
{
	vector<tuple<size_t, size_t>> pairs = RandomPairSelection(0.5).materialise(100);

	BOOST_TEST(pairs.size() == 50);
	BOOST_TEST(all_of(pairs.begin(), pairs.end(), [](auto const& pair){ return get<0>(pair) != get<1>(pair); }));
}

BOOST_AUTO_TEST_CASE(materialise_should_return_number_of_pairs_thats_a_fraction_of_collection_size)
{
	BOOST_TEST(RandomPairSelection(0.0).materialise(10).size() == 0);
	BOOST_TEST(RandomPairSelection(0.3).materialise(10).size() == 3);
	BOOST_TEST(RandomPairSelection(0.5).materialise(10).size() == 5);
	BOOST_TEST(RandomPairSelection(0.7).materialise(10).size() == 7);
	BOOST_TEST(RandomPairSelection(1.0).materialise(10).size() == 10);
}

BOOST_AUTO_TEST_CASE(materialise_should_support_number_of_pairs_bigger_than_collection_size)
{
	BOOST_TEST(RandomPairSelection(2.0).materialise(5).size() == 10);
	BOOST_TEST(RandomPairSelection(1.5).materialise(10).size() == 15);
	BOOST_TEST(RandomPairSelection(10.0).materialise(10).size() == 100);
}

BOOST_AUTO_TEST_CASE(materialise_should_round_the_number_of_pairs_to_the_nearest_integer)
{
	BOOST_TEST(RandomPairSelection(0.49).materialise(3).size() == 1);
	BOOST_TEST(RandomPairSelection(0.50).materialise(3).size() == 2);
	BOOST_TEST(RandomPairSelection(0.51).materialise(3).size() == 2);

	BOOST_TEST(RandomPairSelection(1.51).materialise(3).size() == 5);

	BOOST_TEST(RandomPairSelection(0.01).materialise(2).size() == 0);
	BOOST_TEST(RandomPairSelection(0.01).materialise(3).size() == 0);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_pairs_if_collection_is_empty)
{
	BOOST_TEST(RandomPairSelection(0).materialise(0).empty());
	BOOST_TEST(RandomPairSelection(0.5).materialise(0).empty());
	BOOST_TEST(RandomPairSelection(1.0).materialise(0).empty());
	BOOST_TEST(RandomPairSelection(2.0).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_pairs_if_collection_has_one_element)
{
	BOOST_TEST(RandomPairSelection(0).materialise(1).empty());
	BOOST_TEST(RandomPairSelection(0.5).materialise(1).empty());
	BOOST_TEST(RandomPairSelection(1.0).materialise(1).empty());
	BOOST_TEST(RandomPairSelection(2.0).materialise(1).empty());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(PairsFromRandomSubsetTest)

BOOST_AUTO_TEST_CASE(materialise_should_return_random_values_with_equal_probabilities)
{
	constexpr int collectionSize = 1000;
	constexpr double selectionChance = 0.7;
	constexpr double relativeTolerance = 0.001;
	constexpr double expectedValue = selectionChance;
	constexpr double variance = selectionChance * (1 - selectionChance);

	SimulationRNG::reset(1);
	vector<tuple<size_t, size_t>> pairs = PairsFromRandomSubset(selectionChance).materialise(collectionSize);
	vector<double> bernoulliTrials(collectionSize, 0);
	for (auto& pair: pairs)
	{
		BOOST_REQUIRE(get<1>(pair) < collectionSize);
		BOOST_REQUIRE(get<1>(pair) < collectionSize);
		bernoulliTrials[get<0>(pair)] = 1.0;
		bernoulliTrials[get<1>(pair)] = 1.0;
	}

	BOOST_TEST(abs(mean(bernoulliTrials) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(bernoulliTrials, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_only_values_that_can_be_used_as_collection_indices)
{
	const size_t collectionSize = 200;
	constexpr double selectionChance = 0.5;

	vector<tuple<size_t, size_t>> pairs = PairsFromRandomSubset(selectionChance).materialise(collectionSize);

	BOOST_TEST(all_of(pairs.begin(), pairs.end(), [&](auto const& pair){ return get<0>(pair) <= collectionSize; }));
	BOOST_TEST(all_of(pairs.begin(), pairs.end(), [&](auto const& pair){ return get<1>(pair) <= collectionSize; }));
}

BOOST_AUTO_TEST_CASE(materialise_should_use_unique_indices)
{
	constexpr size_t collectionSize = 200;
	constexpr double selectionChance = 0.5;

	vector<tuple<size_t, size_t>> pairs = PairsFromRandomSubset(selectionChance).materialise(collectionSize);
	set<size_t> indices;
	for (auto& pair: pairs)
	{
		indices.insert(get<0>(pair));
		indices.insert(get<1>(pair));
	}

	BOOST_TEST(indices.size() == 2 * pairs.size());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_indices_if_collection_is_empty)
{
	BOOST_TEST(PairsFromRandomSubset(0.0).materialise(0).empty());
	BOOST_TEST(PairsFromRandomSubset(0.5).materialise(0).empty());
	BOOST_TEST(PairsFromRandomSubset(1.0).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_pairs_if_selection_chance_is_zero)
{
	BOOST_TEST(PairsFromRandomSubset(0.0).materialise(0).empty());
	BOOST_TEST(PairsFromRandomSubset(0.0).materialise(100).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_all_pairs_if_selection_chance_is_one)
{
	BOOST_TEST(PairsFromRandomSubset(1.0).materialise(0).empty());
	BOOST_TEST(PairsFromRandomSubset(1.0).materialise(100).size() == 50);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(PairMosaicSelectionTest)

using IndexPairs = vector<tuple<size_t, size_t>>;

BOOST_AUTO_TEST_CASE(materialise)
{
	BOOST_TEST(PairMosaicSelection({{1, 1}}, 0.5).materialise(4) == IndexPairs({{1, 1}, {1, 1}}));
	BOOST_TEST(PairMosaicSelection({{1, 1}}, 1.0).materialise(4) == IndexPairs({{1, 1}, {1, 1}, {1, 1}, {1, 1}}));
	BOOST_TEST(PairMosaicSelection({{1, 1}}, 2.0).materialise(4) == IndexPairs({{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}}));
	BOOST_TEST(PairMosaicSelection({{1, 1}}, 1.0).materialise(2) == IndexPairs({{1, 1}, {1, 1}}));

	IndexPairs pairs1{{0, 1}, {1, 0}};
	BOOST_TEST(PairMosaicSelection(pairs1, 0.5).materialise(4) == IndexPairs({{0, 1}, {1, 0}}));
	BOOST_TEST(PairMosaicSelection(pairs1, 1.0).materialise(4) == IndexPairs({{0, 1}, {1, 0}, {0, 1}, {1, 0}}));
	BOOST_TEST(PairMosaicSelection(pairs1, 2.0).materialise(4) == IndexPairs({{0, 1}, {1, 0}, {0, 1}, {1, 0}, {0, 1}, {1, 0}, {0, 1}, {1, 0}}));
	BOOST_TEST(PairMosaicSelection(pairs1, 1.0).materialise(2) == IndexPairs({{0, 1}, {1, 0}}));

	IndexPairs pairs2{{3, 2}, {2, 3}, {1, 0}, {1, 1}};
	BOOST_TEST(PairMosaicSelection(pairs2, 0.5).materialise(4) == IndexPairs({{3, 2}, {2, 3}}));
	BOOST_TEST(PairMosaicSelection(pairs2, 1.0).materialise(4) == IndexPairs({{3, 2}, {2, 3}, {1, 0}, {1, 1}}));
	BOOST_TEST(PairMosaicSelection(pairs2, 2.0).materialise(4) == IndexPairs({{3, 2}, {2, 3}, {1, 0}, {1, 1}, {3, 2}, {2, 3}, {1, 0}, {1, 1}}));

	IndexPairs pairs3{{1, 0}, {1, 1}, {1, 0}, {1, 1}};
	BOOST_TEST(PairMosaicSelection(pairs3, 1.0).materialise(2) == IndexPairs({{1, 0}, {1, 1}}));
}

BOOST_AUTO_TEST_CASE(materialise_should_round_indices)
{
	IndexPairs pairs{{4, 4}, {3, 3}, {2, 2}, {1, 1}, {0, 0}};
	BOOST_TEST(PairMosaicSelection(pairs, 0.49).materialise(5) == IndexPairs({{4, 4}, {3, 3}}));
	BOOST_TEST(PairMosaicSelection(pairs, 0.50).materialise(5) == IndexPairs({{4, 4}, {3, 3}, {2, 2}}));
	BOOST_TEST(PairMosaicSelection(pairs, 0.51).materialise(5) == IndexPairs({{4, 4}, {3, 3}, {2, 2}}));
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_pairs_if_collection_is_empty)
{
	BOOST_TEST(PairMosaicSelection({{1, 1}}, 1.0).materialise(0).empty());
	BOOST_TEST(PairMosaicSelection({{1, 1}, {3, 3}}, 2.0).materialise(0).empty());
	BOOST_TEST(PairMosaicSelection({{5, 5}, {4, 4}, {3, 3}, {2, 2}}, 0.5).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_pairs_if_collection_has_one_element)
{
	IndexPairs pairs{{4, 4}, {3, 3}, {2, 2}, {1, 1}, {0, 0}};
	BOOST_TEST(PairMosaicSelection(pairs, 0.0).materialise(1).empty());
	BOOST_TEST(PairMosaicSelection(pairs, 0.5).materialise(1).empty());
	BOOST_TEST(PairMosaicSelection(pairs, 1.0).materialise(1).empty());
	BOOST_TEST(PairMosaicSelection(pairs, 7.0).materialise(1).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_clamp_indices_at_collection_size)
{
	IndexPairs pairs{{4, 4}, {3, 3}, {2, 2}, {1, 1}, {0, 0}};
	BOOST_TEST(PairMosaicSelection(pairs, 1.0).materialise(4) == IndexPairs({{3, 3}, {3, 3}, {2, 2}, {1, 1}}));
	BOOST_TEST(PairMosaicSelection(pairs, 2.0).materialise(3) == IndexPairs({{2, 2}, {2, 2}, {2, 2}, {1, 1}, {0, 0}, {2, 2}}));

}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
