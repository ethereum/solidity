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

#include <boost/test/unit_test.hpp>

using namespace boost::test_tools;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(CommonTest)

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
