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

#include <libdevcore/CommonData.h>

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(CommonData)

BOOST_AUTO_TEST_CASE(test_format_number_readable)
{
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x8000000), "0x08 * 2^24");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x80000000), "0x80 * 2^24");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x800000000), "0x08 * 2^32");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x8000000000), "0x80 * 2^32");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x80000000000), "0x08 * 2^40");
	
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x7ffffff), "0x08 * 2^24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x7fffffff), "0x80 * 2^24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x7ffffffff), "0x08 * 2^32 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x7fffffffff), "0x80 * 2^32 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x7ffffffffff), "0x08 * 2^40 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x88000000), "0x88 * 2^24");

	// BOOST_CHECK_EQUAL(formatNumberReadableChris("0x880000"), "0x88 * 2^16");
	// BOOST_CHECK_EQUAL(formatNumberReadableChris("0x8880000"), "0x888 * 2^16");
	// BOOST_CHECK_EQUAL(formatNumberReadableChris("0x88880000"), "0x8888 * 2^16");

	BOOST_CHECK_EQUAL(formatNumberReadableChris("0x80000"), "0x8 * 2^16");
	BOOST_CHECK_EQUAL(formatNumberReadableChris("0x800000"), "0x80 * 2^16");
	BOOST_CHECK_EQUAL(formatNumberReadableChris("0x8000000"), "0x8 * 2^24");
	BOOST_CHECK_EQUAL(formatNumberReadableChris("0x80000000"), "0x80 * 2^24");
	BOOST_CHECK_EQUAL(formatNumberReadableChris("0x800000000"), "0x8 * 2^32");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
