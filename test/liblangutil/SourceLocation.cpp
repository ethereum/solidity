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

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SourceLocationTest)

BOOST_AUTO_TEST_CASE(test_fail)
{
	BOOST_CHECK(SourceLocation() == SourceLocation());
	BOOST_CHECK(SourceLocation(0, 3, std::make_shared<std::string>("sourceA")) != SourceLocation(0, 3, std::make_shared<std::string>("sourceB")));
	BOOST_CHECK(SourceLocation(0, 3, std::make_shared<std::string>("source")) == SourceLocation(0, 3, std::make_shared<std::string>("source")));
	BOOST_CHECK(SourceLocation(3, 7, std::make_shared<std::string>("source")).contains(SourceLocation(4, 6, std::make_shared<std::string>("source"))));
	BOOST_CHECK(!SourceLocation(3, 7, std::make_shared<std::string>("sourceA")).contains(SourceLocation(4, 6, std::make_shared<std::string>("sourceB"))));
	BOOST_CHECK(SourceLocation(3, 7, std::make_shared<std::string>("sourceA")) < SourceLocation(4, 6, std::make_shared<std::string>("sourceB")));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
