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
/**
 * Unit tests for LEB128.
 */

#include <libsolutil/LEB128.h>

#include <boost/test/unit_test.hpp>


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(LEB128Test)

BOOST_AUTO_TEST_CASE(encode_unsigned)
{
	bytes zero = solidity::util::lebEncode(0);
	BOOST_REQUIRE(zero.size() == 1);
	BOOST_REQUIRE(zero[0] == 0x00);

	bytes one = solidity::util::lebEncode(1);
	BOOST_REQUIRE(one.size() == 1);
	BOOST_REQUIRE(one[0] == 0x01);

	bytes large = solidity::util::lebEncode(624485);
	BOOST_REQUIRE(large.size() == 3);
	BOOST_REQUIRE(large[0] == 0xE5);
	BOOST_REQUIRE(large[1] == 0x8E);
	BOOST_REQUIRE(large[2] == 0x26);

	bytes larger = solidity::util::lebEncodeSigned(123456123456);
	BOOST_REQUIRE(larger.size() == 6);
	BOOST_REQUIRE(larger[0] == 0xC0);
	BOOST_REQUIRE(larger[1] == 0xE4);
	BOOST_REQUIRE(larger[2] == 0xBB);
	BOOST_REQUIRE(larger[3] == 0xF4);
	BOOST_REQUIRE(larger[4] == 0xCB);
	BOOST_REQUIRE(larger[5] == 0x03);
}

BOOST_AUTO_TEST_CASE(encode_signed)
{
	bytes zero = solidity::util::lebEncodeSigned(0);
	BOOST_REQUIRE(zero.size() == 1);
	BOOST_REQUIRE(zero[0] == 0x00);

	bytes one = solidity::util::lebEncodeSigned(1);
	BOOST_REQUIRE(one.size() == 1);
	BOOST_REQUIRE(one[0] == 0x01);

	bytes negative_one = solidity::util::lebEncodeSigned(-1);
	BOOST_REQUIRE(negative_one.size() == 1);
	BOOST_REQUIRE(negative_one[0] == 0x7f);

	bytes negative_two = solidity::util::lebEncodeSigned(-2);
	BOOST_REQUIRE(negative_two.size() == 1);
	BOOST_REQUIRE(negative_two[0] == 0x7e);

	bytes large = solidity::util::lebEncodeSigned(624485);
	BOOST_REQUIRE(large.size() == 3);
	BOOST_REQUIRE(large[0] == 0xE5);
	BOOST_REQUIRE(large[1] == 0x8E);
	BOOST_REQUIRE(large[2] == 0x26);

	bytes negative_large = solidity::util::lebEncodeSigned(-123456);
	BOOST_REQUIRE(negative_large.size() == 3);
	BOOST_REQUIRE(negative_large[0] == 0xC0);
	BOOST_REQUIRE(negative_large[1] == 0xBB);
	BOOST_REQUIRE(negative_large[2] == 0x78);

	bytes negative_larger = solidity::util::lebEncodeSigned(-123456123456);
	BOOST_REQUIRE(negative_larger.size() == 6);
	BOOST_REQUIRE(negative_larger[0] == 0xC0);
	BOOST_REQUIRE(negative_larger[1] == 0x9B);
	BOOST_REQUIRE(negative_larger[2] == 0xC4);
	BOOST_REQUIRE(negative_larger[3] == 0x8B);
	BOOST_REQUIRE(negative_larger[4] == 0xB4);
	BOOST_REQUIRE(negative_larger[5] == 0x7C);
}

BOOST_AUTO_TEST_SUITE_END()

}
