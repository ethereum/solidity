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
 * @author Raphael <raphael.s.norwitz@gmail.com>
 * Unit tests for the solidity expression compiler, testing the behaviour of the code.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/ftime.h>

/*using namespace std;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{*/

BOOST_AUTO_TEST_SUITE(FTime)

BOOST_AUTO_TEST_CASE(hello_world)
{
	bool f = false;
	BOOST_CHECK(f);
	BOOST_REQUIRE(f);
};
	
BOOST_AUTO_TEST_SUITE_END()

//}
//}
//} // end namespaces

