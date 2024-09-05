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
/**
 * Unit tests for DisjointSet.
 */

#include <libsolutil/DisjointSet.h>

#include <boost/test/unit_test.hpp>

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(DisjointSetTest)

BOOST_AUTO_TEST_CASE(full_union)
{
	ContiguousDisjointSet ds(10);
	for (size_t i = 1; i < 10; ++i)
	{
		BOOST_CHECK(!ds.sameSubset(0, i));
		ds.merge(0, i);
		BOOST_CHECK(ds.sameSubset(0, i));
	}
	BOOST_CHECK_EQUAL(ds.numSets(), 1);
	auto const& subsets = ds.subsets();
	BOOST_CHECK_EQUAL(subsets.size(), 1);
	std::set<size_t> fullSet{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	BOOST_CHECK_EQUAL_COLLECTIONS(subsets[0].begin(), subsets[0].end(), fullSet.begin(), fullSet.end());

	for (size_t i = 1; i < 10; ++i) BOOST_CHECK_EQUAL(ds.find(0), ds.find(i));
}

BOOST_AUTO_TEST_CASE(pairs)
{
	ContiguousDisjointSet ds(10);
	BOOST_CHECK_EQUAL(ds.numSets(), 10);
	BOOST_CHECK_EQUAL(ds.subsets().size(), 10);

	auto const checkPair = [&](size_t expectedNumSubsets, size_t x, size_t y)
	{
		BOOST_CHECK_EQUAL(ds.numSets(), expectedNumSubsets);
		BOOST_CHECK_EQUAL(ds.subsets().size(), expectedNumSubsets);
		BOOST_CHECK(ds.sameSubset(x, y));
		auto const subset = ds.subset(x);
		std::set<size_t> subsetRef{x, y};
		BOOST_CHECK_EQUAL_COLLECTIONS(subset.begin(), subset.end(), subsetRef.begin(), subsetRef.end());
	};

	ds.merge(5, 3);
	checkPair(9, 5, 3);

	ds.merge(1, 6);
	checkPair(8, 1, 6);

	ds.merge(0, 2);
	checkPair(7, 0, 2);

	ds.merge(1, 5);
	BOOST_CHECK_EQUAL(ds.sizeOfSubset(3), 4);

	ds.merge(1, 0);

	// now we should have a subset with {0, 1, 2, 3, 5, 6}
	std::set<size_t> subsetRef{0, 1, 2, 3, 5, 6};
	BOOST_CHECK_EQUAL(ds.sizeOfSubset(6), subsetRef.size());
	auto const subset = ds.subset(2);
	BOOST_CHECK_EQUAL_COLLECTIONS(subset.begin(), subset.end(), subsetRef.begin(), subsetRef.end());
}

BOOST_AUTO_TEST_CASE(merge_with_fixed_representative)
{
	ContiguousDisjointSet ds(10);
	ds.merge(5, 3, false);
	BOOST_CHECK_EQUAL(ds.find(5), 5);
	ds.merge(1, 2);
	ds.merge(7, 8);
	ds.merge(0, 9);
	ds.merge(5, 1, false);
	BOOST_CHECK_EQUAL(ds.find(5), 5);
	ds.merge(5, 0, false);
	BOOST_CHECK_EQUAL(ds.find(5), 5);
	ds.merge(5, 7, false);
	BOOST_CHECK_EQUAL(ds.find(5), 5);
}

BOOST_AUTO_TEST_SUITE_END()

}
