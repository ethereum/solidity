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
 * Unit tests for the StringUtils routines.
 */

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/FixedHash.h>
#include <libsolidity/ast/Types.h> // for IntegerType

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::frontend;

// TODO: Fix Boost...
BOOST_TEST_DONT_PRINT_LOG_VALUE(solidity::bytes)

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(CommonData)

BOOST_AUTO_TEST_CASE(fromhex_char)
{
	BOOST_CHECK_EQUAL(fromHex('0', WhenError::DontThrow), 0x0);
	BOOST_CHECK_EQUAL(fromHex('a', WhenError::DontThrow), 0xa);
	BOOST_CHECK_EQUAL(fromHex('x', WhenError::DontThrow), -1);
	BOOST_CHECK_EQUAL(fromHex('x', static_cast<WhenError>(42)), -1);

	BOOST_CHECK_EQUAL(fromHex('0', WhenError::Throw), 0x0);
	BOOST_CHECK_EQUAL(fromHex('a', WhenError::Throw), 0xa);
	BOOST_CHECK_THROW(fromHex('x', WhenError::Throw), BadHexCharacter);
}

BOOST_AUTO_TEST_CASE(fromhex_string)
{
	bytes expectation_even = {{0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
	bytes expectation_odd = {{0x00, 0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0}};

	// Defaults to WhenError::DontThrow
	BOOST_CHECK_EQUAL(fromHex(""), bytes());
	BOOST_CHECK_EQUAL(fromHex("00112233445566778899aabbccddeeff"), expectation_even);
	BOOST_CHECK_EQUAL(fromHex("0x00112233445566778899aabbccddeeff"), expectation_even);
	BOOST_CHECK_EQUAL(fromHex("0x00112233445566778899aabbccddeeff0"), expectation_odd);
	BOOST_CHECK_EQUAL(fromHex("gg"), bytes());
	BOOST_CHECK_EQUAL(fromHex("0xgg"), bytes());

	BOOST_CHECK_EQUAL(fromHex("", WhenError::Throw), bytes());
	BOOST_CHECK_EQUAL(fromHex("00112233445566778899aabbccddeeff", WhenError::Throw), expectation_even);
	BOOST_CHECK_EQUAL(fromHex("0x00112233445566778899aabbccddeeff", WhenError::Throw), expectation_even);
	BOOST_CHECK_EQUAL(fromHex("0x00112233445566778899aabbccddeeff0", WhenError::Throw), expectation_odd);
	BOOST_CHECK_THROW(fromHex("gg", WhenError::Throw), BadHexCharacter);
	BOOST_CHECK_THROW(fromHex("0xgg", WhenError::Throw), BadHexCharacter);
}

BOOST_AUTO_TEST_CASE(tohex_uint8)
{
	BOOST_CHECK_EQUAL(toHex(0xaa), "aa");
	BOOST_CHECK_EQUAL(toHex(0xaa, HexCase::Lower), "aa");
	BOOST_CHECK_EQUAL(toHex(0xaa, HexCase::Upper), "AA");
	BOOST_CHECK_THROW(toHex(0xaa, HexCase::Mixed), BadHexCase);
	// Defaults to lower case on invalid setting.
	BOOST_CHECK_EQUAL(toHex(0xaa, static_cast<HexCase>(42)), "aa");
}

BOOST_AUTO_TEST_CASE(tohex_bytes)
{
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::DontAdd, HexCase::Lower), "00112233445566778899aabbccddeeff");
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::DontAdd, HexCase::Upper), "00112233445566778899AABBCCDDEEFF");
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::DontAdd, HexCase::Mixed), "00112233445566778899aabbCCDDeeff");
	// Defaults to lower case on invalid setting.
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::DontAdd, static_cast<HexCase>(42)), "00112233445566778899aabbccddeeff");

	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::Add, HexCase::Lower), "0x00112233445566778899aabbccddeeff");
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::Add, HexCase::Upper), "0x00112233445566778899AABBCCDDEEFF");
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899AaBbCcDdEeFf"), HexPrefix::Add, HexCase::Mixed), "0x00112233445566778899aabbCCDDeeff");
	// Defaults to lower case on invalid setting.
	BOOST_CHECK_EQUAL(toHex(fromHex("00112233445566778899aAbBcCdDeEfF"), HexPrefix::Add, static_cast<HexCase>(42)), "0x00112233445566778899aabbccddeeff");
}

BOOST_AUTO_TEST_CASE(test_format_number)
{
	BOOST_CHECK_EQUAL(formatNumber(u256(0x8000000)), "0x08000000");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x80000000)), "0x80000000");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x800000000)), "0x0800000000");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x8000000000)), "0x8000000000");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x80000000000)), "0x080000000000");

	BOOST_CHECK_EQUAL(formatNumber(u256(0x7ffffff)), "0x07ffffff");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x7fffffff)), "0x7fffffff");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x7ffffffff)), "0x07ffffffff");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x7fffffffff)), "0x7fffffffff");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x7ffffffffff)), "0x07ffffffffff");

	BOOST_CHECK_EQUAL(formatNumber(u256(0x88000000)), "0x88000000");
	BOOST_CHECK_EQUAL(formatNumber(u256(0x8888888888000000)), "0x8888888888000000");

	u256 b = 0;
	for (int i = 0; i < 32; i++)
	{
		b <<= 8;
		b |= 0x55;
	}
	u256 c = u256(FixedHash<32>(
		fromHex("0xabcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789")
	));
	u256 d = u256(0xAAAAaaaaAAAAaaaa) << 192 |
		u256(0xFFFFffffFFFFffff) << 128 |
		u256(0xFFFFffffFFFFffff) << 64 |
		u256(0xFFFFffffFFFFffff);
	BOOST_CHECK_EQUAL(formatNumber(b), "0x5555555555555555555555555555555555555555555555555555555555555555");
	BOOST_CHECK_EQUAL(formatNumber(c), "0xabcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789");
	BOOST_CHECK_EQUAL(formatNumber(d), "0xaaaaaaaaaaaaaaaaffffffffffffffffffffffffffffffffffffffffffffffff");

	BOOST_CHECK_EQUAL(formatNumber(IntegerType(256).minValue()), "0");
	BOOST_CHECK_EQUAL(
		formatNumber(IntegerType(256).maxValue()),
		"0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
