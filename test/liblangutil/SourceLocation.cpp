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
 * @author Yoichi Hirai <yoichi@ethereum.org>
 * @date 2016
 * Unit tests for the SourceLocation class.
 */

#include <liblangutil/SourceLocation.h>

#include <test/Options.h>

namespace langutil
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SourceLocationTest)

BOOST_AUTO_TEST_CASE(test_fail)
{
	auto const source = std::make_shared<CharStream>("", "source");
	auto const sourceA = std::make_shared<CharStream>("", "sourceA");
	auto const sourceB = std::make_shared<CharStream>("", "sourceB");

	BOOST_CHECK(SourceLocation{} == SourceLocation{});
	BOOST_CHECK((SourceLocation{0, 3, sourceA} != SourceLocation{0, 3, sourceB}));
	BOOST_CHECK((SourceLocation{0, 3, source} == SourceLocation{0, 3, source}));
	BOOST_CHECK((SourceLocation{3, 7, source}.contains(SourceLocation{4, 6, source})));
	BOOST_CHECK((!SourceLocation{3, 7, sourceA}.contains(SourceLocation{4, 6, sourceB})));
	BOOST_CHECK((SourceLocation{3, 7, sourceA} < SourceLocation{4, 6, sourceB}));
}

BOOST_AUTO_TEST_SUITE_END()

}
} // end namespaces
