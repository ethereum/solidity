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
 * Unit tests for the StringUtils routines.
 */

#include <libdevcore/StringUtils.h>

#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(StringUtils)

BOOST_AUTO_TEST_CASE(test_similarity)
{
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hello", 0), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hello", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hellw", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "helol", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "helo", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "helllo", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hlllo", 1), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hllllo", 1), false);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hllllo", 2), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("hello", "hlllo", 2), true);
	BOOST_CHECK_EQUAL(stringWithinDistance("a", "", 2), false);
	BOOST_CHECK_EQUAL(stringWithinDistance("abc", "ba", 2), false);
	BOOST_CHECK_EQUAL(stringWithinDistance("abc", "abcdef", 2), false);
	BOOST_CHECK_EQUAL(stringWithinDistance("abcd", "wxyz", 2), false);
	BOOST_CHECK_EQUAL(stringWithinDistance("", "", 2), true);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
