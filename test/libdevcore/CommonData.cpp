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
#include <libdevcore/FixedHash.h>

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
	BOOST_CHECK_EQUAL(formatNumberReadable((u256)0x8888888888000000), "0x8888888888 * 2^24");

	u160 a = 0;	for (int i=0; i<20; i++) { a <<= 8; a |= 0x55; }
	u256 b = 0;	for (int i=0; i<32; i++) { b <<= 8; b |= 0x55; }
	u256 c = (u256)FixedHash<32>(fromHex("0xabcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789"));
	u256 d = (u256)0xAAAAaaaaAAAAaaaa << 192
		   | (u256)0xFFFFffffFFFFffff << 128
		   | (u256)0xFFFFffffFFFFffff << 64
		   | (u256)0xFFFFffffFFFFffff;
	BOOST_CHECK_EQUAL(formatNumber(b),         "0x5555555555555555555555555555555555555555555555555555555555555555");
	BOOST_CHECK_EQUAL(formatNumber(c),         "0xabcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789");
	BOOST_CHECK_EQUAL(formatNumber(d),         "0xaaaaaaaaaaaaaaaaffffffffffffffffffffffffffffffffffffffffffffffff");
	BOOST_CHECK_EQUAL(formatNumberReadable(a), "0x55555555...");
	BOOST_CHECK_EQUAL(formatNumberReadable(b), "0x55555555...");
	BOOST_CHECK_EQUAL(formatNumberReadable(c), "0xABCDef01...");
	BOOST_CHECK_EQUAL(formatNumberReadable(d), "0xaaaaaaaaaaaaaaab * 2^192 - 1");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
