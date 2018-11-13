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

#include <test/Options.h>

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
	BOOST_CHECK_EQUAL(stringWithinDistance("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY", "YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYZ", 2, 6400), false);
}

BOOST_AUTO_TEST_CASE(test_dldistance)
{
	BOOST_CHECK_EQUAL(stringDistance("hello", "hellw"), 1);
	BOOST_CHECK_EQUAL(stringDistance("hello", "helol"), 1);
	BOOST_CHECK_EQUAL(stringDistance("hello", "helo"), 1);
	BOOST_CHECK_EQUAL(stringDistance("hello", "helllo"), 1);
	BOOST_CHECK_EQUAL(stringDistance("hello", "hlllo"), 1);
	BOOST_CHECK_EQUAL(stringDistance("hello", "hllllo"), 2);
	BOOST_CHECK_EQUAL(stringDistance("a", ""), 1);
	BOOST_CHECK_EQUAL(stringDistance("abc", "ba"), 2);
	BOOST_CHECK_EQUAL(stringDistance("abc", "abcdef"), 3);
	BOOST_CHECK_EQUAL(stringDistance("abcd", "wxyz"), 4);
	BOOST_CHECK_EQUAL(stringDistance("", ""), 0);
	BOOST_CHECK_EQUAL(stringDistance("abcdefghijklmnopqrstuvwxyz", "abcabcabcabcabcabcabcabca"), 23);

}

BOOST_AUTO_TEST_CASE(test_alternatives_list)
{
	vector<string> strings;
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "");
	strings.push_back("a");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\"");
	strings.push_back("b");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\" or \"b\"");
	strings.push_back("c");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\", \"b\" or \"c\"");
	strings.push_back("d");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\", \"b\", \"c\" or \"d\"");
}

BOOST_AUTO_TEST_CASE(test_human_readable_join)
{
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({})), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a"})), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b"})), "a, b");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b", "c"})), "a, b, c");

	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({}), "; "), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a"}), "; "), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b"}), "; "), "a; b");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b", "c"}), "; "), "a; b; c");

	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({}), "; ", " or "), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a"}), "; ", " or "), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b"}), "; ", " or "), "a or b");
	BOOST_CHECK_EQUAL(joinHumanReadable(vector<string>({"a", "b", "c"}), "; ", " or "), "a; b or c");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
