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
 * Unit tests for FunctionSelector.
 */

#include <libsolutil/FunctionSelector.h>

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <sstream>


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(FunctionSelectorTest)

BOOST_AUTO_TEST_CASE(conversions)
{
	BOOST_CHECK_EQUAL(
		util::selectorFromSignatureH32("test()"),
		util::FixedHash<4>(0xf8a8fd6d)
	);
	BOOST_CHECK_EQUAL(
		util::selectorFromSignatureU32("test()"),
		0xf8a8fd6d
	);
	BOOST_CHECK_EQUAL(
		util::selectorFromSignatureU256("test()"),
		u256("0xf8a8fd6d00000000000000000000000000000000000000000000000000000000")
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
