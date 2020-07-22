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

#include <tools/yulPhaser/Selections.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <libsolutil/CommonData.h>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <set>
#include <vector>

using namespace std;
using namespace solidity::util;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(SelectionsTest)
BOOST_AUTO_TEST_SUITE(RangeSelectionTest)

BOOST_AUTO_TEST_CASE(materialise)
{
	BOOST_TEST(RangeSelection(0.0, 1.0).materialise(10) == vector<size_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
	BOOST_TEST(RangeSelection(0.0, 0.1).materialise(10) == vector<size_t>({0}));
	BOOST_TEST(RangeSelection(0.0, 0.2).materialise(10) == vector<size_t>({0, 1}));
	BOOST_TEST(RangeSelection(0.0, 0.7).materialise(10) == vector<size_t>({0, 1, 2, 3, 4, 5, 6}));

	BOOST_TEST(RangeSelection(0.9, 1.0).materialise(10) == vector<size_t>({                           9}));
	BOOST_TEST(RangeSelection(0.8, 1.0).materialise(10) == vector<size_t>({                        8, 9}));
	BOOST_TEST(RangeSelection(0.5, 1.0).materialise(10) == vector<size_t>({               5, 6, 7, 8, 9}));

	BOOST_TEST(RangeSelection(0.3, 0.6).materialise(10) == vector<size_t>({         3, 4, 5            }));
	BOOST_TEST(RangeSelection(0.2, 0.7).materialise(10) == vector<size_t>({      2, 3, 4, 5, 6         }));
	BOOST_TEST(RangeSelection(0.4, 0.7).materialise(10) == vector<size_t>({            4, 5, 6         }));

	BOOST_TEST(RangeSelection(0.4, 0.7).materialise(5) == vector<size_t>({2, 3}));
}

BOOST_AUTO_TEST_CASE(materialise_should_round_indices)
{
	BOOST_TEST(RangeSelection(0.01, 0.99).materialise(10) == vector<size_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
	BOOST_TEST(RangeSelection(0.04, 0.96).materialise(10) == vector<size_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
	BOOST_TEST(RangeSelection(0.05, 0.95).materialise(10) == vector<size_t>({   1, 2, 3, 4, 5, 6, 7, 8, 9}));
	BOOST_TEST(RangeSelection(0.06, 0.94).materialise(10) == vector<size_t>({   1, 2, 3, 4, 5, 6, 7, 8   }));
}

BOOST_AUTO_TEST_CASE(materialise_should_handle_empty_collections)
{
	BOOST_TEST(RangeSelection(0.0, 0.0).materialise(0).empty());
	BOOST_TEST(RangeSelection(0.0, 1.0).materialise(0).empty());
	BOOST_TEST(RangeSelection(0.5, 1.0).materialise(0).empty());
	BOOST_TEST(RangeSelection(0.0, 0.5).materialise(0).empty());
	BOOST_TEST(RangeSelection(0.2, 0.7).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_handle_empty_selection_ranges)
{
	BOOST_TEST(RangeSelection(0.0, 0.0).materialise(1).empty());
	BOOST_TEST(RangeSelection(1.0, 1.0).materialise(1).empty());

	BOOST_TEST(RangeSelection(0.0, 0.0).materialise(100).empty());
	BOOST_TEST(RangeSelection(1.0, 1.0).materialise(100).empty());
	BOOST_TEST(RangeSelection(0.5, 0.5).materialise(100).empty());

	BOOST_TEST(RangeSelection(0.45, 0.54).materialise(10).empty());
	BOOST_TEST(!RangeSelection(0.45, 0.54).materialise(100).empty());
	BOOST_TEST(RangeSelection(0.045, 0.054).materialise(100).empty());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(MosaicSelectionTest)

BOOST_AUTO_TEST_CASE(materialise)
{
	BOOST_TEST(MosaicSelection({1}, 0.5).materialise(4) == vector<size_t>({1, 1}));
	BOOST_TEST(MosaicSelection({1}, 1.0).materialise(4) == vector<size_t>({1, 1, 1, 1}));
	BOOST_TEST(MosaicSelection({1}, 2.0).materialise(4) == vector<size_t>({1, 1, 1, 1, 1, 1, 1, 1}));
	BOOST_TEST(MosaicSelection({1}, 1.0).materialise(2) == vector<size_t>({1, 1}));

	BOOST_TEST(MosaicSelection({0, 1}, 0.5).materialise(4) == vector<size_t>({0, 1}));
	BOOST_TEST(MosaicSelection({0, 1}, 1.0).materialise(4) == vector<size_t>({0, 1, 0, 1}));
	BOOST_TEST(MosaicSelection({0, 1}, 2.0).materialise(4) == vector<size_t>({0, 1, 0, 1, 0, 1, 0, 1}));
	BOOST_TEST(MosaicSelection({0, 1}, 1.0).materialise(2) == vector<size_t>({0, 1}));

	BOOST_TEST(MosaicSelection({3, 2, 1, 0}, 0.5).materialise(4) == vector<size_t>({3, 2}));
	BOOST_TEST(MosaicSelection({3, 2, 1, 0}, 1.0).materialise(4) == vector<size_t>({3, 2, 1, 0}));
	BOOST_TEST(MosaicSelection({3, 2, 1, 0}, 2.0).materialise(4) == vector<size_t>({3, 2, 1, 0, 3, 2, 1, 0}));
	BOOST_TEST(MosaicSelection({1, 0, 1, 0}, 1.0).materialise(2) == vector<size_t>({1, 0}));
}

BOOST_AUTO_TEST_CASE(materialise_should_round_indices)
{
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 0.49).materialise(5) == vector<size_t>({4, 3}));
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 0.50).materialise(5) == vector<size_t>({4, 3, 2}));
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 0.51).materialise(5) == vector<size_t>({4, 3, 2}));
}

BOOST_AUTO_TEST_CASE(materialise_should_handle_empty_collections)
{
	BOOST_TEST(MosaicSelection({1}, 1.0).materialise(0).empty());
	BOOST_TEST(MosaicSelection({1, 3}, 2.0).materialise(0).empty());
	BOOST_TEST(MosaicSelection({5, 4, 3, 2}, 0.5).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_handle_empty_selections)
{
	BOOST_TEST(MosaicSelection({1}, 0.0).materialise(8).empty());
	BOOST_TEST(MosaicSelection({1, 3}, 0.0).materialise(8).empty());
	BOOST_TEST(MosaicSelection({5, 4, 3, 2}, 0.0).materialise(8).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_clamp_indices_at_collection_size)
{
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 1.0).materialise(4) == vector<size_t>({3, 3, 2, 1}));
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 2.0).materialise(3) == vector<size_t>({2, 2, 2, 1, 0, 2}));
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 1.0).materialise(1) == vector<size_t>({0}));
	BOOST_TEST(MosaicSelection({4, 3, 2, 1, 0}, 7.0).materialise(1) == vector<size_t>({0, 0, 0, 0, 0, 0, 0}));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(RandomSelectionTest)

BOOST_AUTO_TEST_CASE(materialise_should_return_random_values_with_equal_probabilities)
{
	constexpr int collectionSize = 10;
	constexpr int selectionSize = 100;
	constexpr double relativeTolerance = 0.1;
	constexpr double expectedValue = (collectionSize - 1) / 2.0;
	constexpr double variance = (collectionSize * collectionSize - 1) / 12.0;

	SimulationRNG::reset(1);
	vector<size_t> samples = RandomSelection(selectionSize).materialise(collectionSize);

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_only_values_that_can_be_used_as_collection_indices)
{
	const size_t collectionSize = 200;

	vector<size_t> indices = RandomSelection(0.5).materialise(collectionSize);

	BOOST_TEST(indices.size() == 100);
	BOOST_TEST(all_of(indices.begin(), indices.end(), [&](auto const& index){ return index <= collectionSize; }));
}

BOOST_AUTO_TEST_CASE(materialise_should_return_number_of_indices_thats_a_fraction_of_collection_size)
{
	BOOST_TEST(RandomSelection(0.0).materialise(10).size() == 0);
	BOOST_TEST(RandomSelection(0.3).materialise(10).size() == 3);
	BOOST_TEST(RandomSelection(0.5).materialise(10).size() == 5);
	BOOST_TEST(RandomSelection(0.7).materialise(10).size() == 7);
	BOOST_TEST(RandomSelection(1.0).materialise(10).size() == 10);
}

BOOST_AUTO_TEST_CASE(materialise_should_support_number_of_indices_bigger_than_collection_size)
{
	BOOST_TEST(RandomSelection(2.0).materialise(5).size() == 10);
	BOOST_TEST(RandomSelection(1.5).materialise(10).size() == 15);
	BOOST_TEST(RandomSelection(10.0).materialise(10).size() == 100);
}

BOOST_AUTO_TEST_CASE(materialise_should_round_the_number_of_indices_to_the_nearest_integer)
{
	BOOST_TEST(RandomSelection(0.49).materialise(3).size() == 1);
	BOOST_TEST(RandomSelection(0.50).materialise(3).size() == 2);
	BOOST_TEST(RandomSelection(0.51).materialise(3).size() == 2);

	BOOST_TEST(RandomSelection(1.51).materialise(3).size() == 5);

	BOOST_TEST(RandomSelection(0.01).materialise(2).size() == 0);
	BOOST_TEST(RandomSelection(0.01).materialise(3).size() == 0);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_indices_if_collection_is_empty)
{
	BOOST_TEST(RandomSelection(0.0).materialise(0).empty());
	BOOST_TEST(RandomSelection(0.5).materialise(0).empty());
	BOOST_TEST(RandomSelection(1.0).materialise(0).empty());
	BOOST_TEST(RandomSelection(2.0).materialise(0).empty());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(RandomSubsetTest)

BOOST_AUTO_TEST_CASE(materialise_should_return_random_values_with_equal_probabilities)
{
	constexpr int collectionSize = 1000;
	constexpr double selectionChance = 0.7;
	constexpr double relativeTolerance = 0.001;
	constexpr double expectedValue = selectionChance;
	constexpr double variance = selectionChance * (1 - selectionChance);

	SimulationRNG::reset(1);
	auto indices = convertContainer<set<size_t>>(RandomSubset(selectionChance).materialise(collectionSize));

	vector<double> bernoulliTrials(collectionSize);
	for (size_t i = 0; i < collectionSize; ++i)
		bernoulliTrials[i] = indices.count(i);

	BOOST_TEST(abs(mean(bernoulliTrials) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(bernoulliTrials, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_only_values_that_can_be_used_as_collection_indices)
{
	const size_t collectionSize = 200;
	vector<size_t> indices = RandomSubset(0.5).materialise(collectionSize);

	BOOST_TEST(all_of(indices.begin(), indices.end(), [&](auto const& index){ return index <= collectionSize; }));
}

BOOST_AUTO_TEST_CASE(materialise_should_return_indices_in_the_same_order_they_are_in_the_container)
{
	const size_t collectionSize = 200;
	vector<size_t> indices = RandomSubset(0.5).materialise(collectionSize);

	for (size_t i = 1; i < indices.size(); ++i)
		BOOST_TEST(indices[i - 1] < indices[i]);
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_indices_if_collection_is_empty)
{
	BOOST_TEST(RandomSubset(0.5).materialise(0).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_no_indices_if_selection_chance_is_zero)
{
	BOOST_TEST(RandomSubset(0.0).materialise(10).empty());
}

BOOST_AUTO_TEST_CASE(materialise_should_return_all_indices_if_selection_chance_is_one)
{
	BOOST_TEST(RandomSubset(1.0).materialise(10).size() == 10);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
