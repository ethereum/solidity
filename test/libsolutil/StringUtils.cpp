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

#include <libsolutil/CommonData.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/StringUtils.h>

#include <libsolidity/ast/Types.h>  // for IntegerType

#include <test/Common.h>

#include <boost/test/unit_test.hpp>


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(StringUtils, *boost::unit_test::label("nooptions"))

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
	std::vector<std::string> strings;
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "");
	strings.emplace_back("a");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\"");
	strings.emplace_back("b");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\" or \"b\"");
	strings.emplace_back("c");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\", \"b\" or \"c\"");
	strings.emplace_back("d");
	BOOST_CHECK_EQUAL(quotedAlternativesList(strings), "\"a\", \"b\", \"c\" or \"d\"");
}

BOOST_AUTO_TEST_CASE(test_human_readable_join)
{
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({})), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a"})), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b"})), "a, b");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b", "c"})), "a, b, c");

	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({}), "; "), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a"}), "; "), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b"}), "; "), "a; b");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b", "c"}), "; "), "a; b; c");

	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({}), "; ", " or "), "");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a"}), "; ", " or "), "a");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b"}), "; ", " or "), "a or b");
	BOOST_CHECK_EQUAL(joinHumanReadable(std::vector<std::string>({"a", "b", "c"}), "; ", " or "), "a; b or c");
}

BOOST_AUTO_TEST_CASE(test_format_number_readable)
{
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x10000000)), "2**28");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x100000000)), "2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x8000000)), "2**27");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x80000000)), "2**31");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x800000000)), "2**35");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x8000000000)), "2**39");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x80000000000)), "2**43");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x7ffffff)), "2**27 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x7fffffff)), "2**31 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x7ffffffff)), "2**35 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x7fffffffff)), "2**39 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x7ffffffffff)), "2**43 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xfffffff)), "2**28 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xffffffff)), "2**32 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x88000000)), "0x88 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x8888888888000000)), "0x8888888888 * 2**24");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x100000000)), "2**32");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xFFFFffff)), "2**32 - 1");

	u256 b = 0;
	for (int i = 0; i < 32; i++)
	{
		b <<= 8;
		b |= 0x55;
	}
	u256 c = (u256)FixedHash<32>(
		fromHex("0xabcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789")
	);
	u256 d = u256(0xAAAAaaaaAAAAaaaa) << 192;
	u256 e = u256(0xAAAAaaaaAAAAaaaa) << 192 |
		u256(0xFFFFffffFFFFffff) << 128 |
		u256(0xFFFFffffFFFFffff) << 64 |
		u256(0xFFFFffffFFFFffff);
	BOOST_CHECK_EQUAL(formatNumberReadable(b, true), "0x5555...{+56 more}...5555");
	BOOST_CHECK_EQUAL(formatNumberReadable(c, true), "0xABCD...{+56 more}...6789");
	BOOST_CHECK_EQUAL(formatNumberReadable(d, true), "0xAAAAaaaaAAAAaaaa * 2**192");
	BOOST_CHECK_EQUAL(formatNumberReadable(e, true), "0xAAAAaaaaAAAAaaab * 2**192 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x20000000)), "2**29");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x200000000)), "2**33");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x2000000000)), "2**37");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x20000000000)), "2**41");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x200000000000)), "2**45");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1FFFFFFF)), "2**29 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1FFFFFFFF)), "2**33 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1FFFFFFFFF)), "2**37 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1FFFFFFFFFF)), "2**41 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1FFFFFFFFFFF)), "2**45 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3000000)), "0x03 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x30000000)), "0x30 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x300000000)), "0x03 * 2**32");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3000000000)), "0x30 * 2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x2FFFFFF)), "0x03 * 2**24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x2FFFFFFF)), "0x30 * 2**24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x2FFFFFFFF)), "0x03 * 2**32 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x2FFFFFFFFF)), "0x30 * 2**32 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA3000000)), "0xa3 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA30000000)), "0x0a30 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA300000000)), "0xa3 * 2**32");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA3000000000)), "0x0a30 * 2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA2FFFFFF)), "0xa3 * 2**24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA2FFFFFFF)), "0x0a30 * 2**24 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA2FFFFFFFF)), "0xa3 * 2**32 - 1");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xA2FFFFFFFFF)), "0x0a30 * 2**32 - 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 128, true), "0x03 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 129, true), "0x06 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 130, true), "0x0c * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 131, true), "0x18 * 2**128");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 248, true), "0x03 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 249, true), "0x06 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 250, true), "0x0c * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x3) << 251, true), "0x18 * 2**248");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 128, true), "0x09 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 129, true), "0x12 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 130, true), "0x24 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 131, true), "0x48 * 2**128");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 248, true), "0x09 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 249, true), "0x12 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 250, true), "0x24 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x9) << 251, true), "0x48 * 2**248");

	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0)), "0");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x10000)), "65536");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xFFFF)), "65535");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0x1000000)), "16777216");
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(0xFFFFFF)), "16777215");

	//for codegen/ExpressionCompiler
	BOOST_CHECK_EQUAL(formatNumberReadable(u256(-1)), "2**256 - 1");

	// for formal/SMTChecker
	BOOST_CHECK_EQUAL(
			formatNumberReadable(frontend::IntegerType(256).minValue()), "0");
	BOOST_CHECK_EQUAL(
			formatNumberReadable(frontend::IntegerType(256).maxValue()), "2**256 - 1");
}

BOOST_AUTO_TEST_CASE(test_format_number_readable_signed)
{
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x10000000)), "-2**28");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x100000000)), "-2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x8000000)), "-2**27");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x80000000)), "-2**31");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x800000000)), "-2**35");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x8000000000)), "-2**39");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x80000000000)), "-2**43");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x7ffffff)), "-2**27 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x7fffffff)), "-2**31 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x7ffffffff)), "-2**35 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x7fffffffff)), "-2**39 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x7ffffffffff)), "-2**43 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xfffffff)), "-2**28 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xffffffff)), "-2**32 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x88000000)), "-0x88 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x8888888888000000)), "-0x8888888888 * 2**24");

	s256 b = 0;
	for (int i = 0; i < 32; i++)
	{
		b <<= 8;
		b |= 0x55;
	}
	b = b * (-1);

	s256 c = (-1) * u2s((u256)FixedHash<32>(
		fromHex("0x0bcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789")
	));

	s256 d = (-1) * u2s(
		u256(0x5555555555555555) << 192 |
		u256(0xFFFFffffFFFFffff) << 128 |
		u256(0xFFFFffffFFFFffff) << 64 |
		u256(0xFFFFffffFFFFffff)
	);

	BOOST_CHECK_EQUAL(formatNumberReadable(b, true), "-0x5555...{+56 more}...5555");
	BOOST_CHECK_EQUAL(formatNumberReadable(c, true), "-0x0BCD...{+56 more}...6789");
	BOOST_CHECK_EQUAL(formatNumberReadable(d, true), "-0x5555555555555556 * 2**192 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x20000000)), "-2**29");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x200000000)), "-2**33");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x2000000000)), "-2**37");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x20000000000)), "-2**41");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x200000000000)), "-2**45");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1FFFFFFF)), "-2**29 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1FFFFFFFF)), "-2**33 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1FFFFFFFFF)), "-2**37 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1FFFFFFFFFF)), "-2**41 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1FFFFFFFFFFF)), "-2**45 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x3000000)), "-0x03 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x30000000)), "-0x30 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x300000000)), "-0x03 * 2**32");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x3000000000)), "-0x30 * 2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x2FFFFFF)), "-0x03 * 2**24 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x2FFFFFFF)), "-0x30 * 2**24 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x2FFFFFFFF)), "-0x03 * 2**32 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x2FFFFFFFFF)), "-0x30 * 2**32 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA3000000)), "-0xa3 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA30000000)), "-0x0a30 * 2**24");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA300000000)), "-0xa3 * 2**32");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA3000000000)), "-0x0a30 * 2**32");

	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA2FFFFFF)), "-0xa3 * 2**24 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA2FFFFFFF)), "-0x0a30 * 2**24 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA2FFFFFFFF)), "-0xa3 * 2**32 + 1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xA2FFFFFFFFF)), "-0x0a30 * 2**32 + 1");

	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 128, true), "-0x03 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 129, true), "-0x06 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 130, true), "-0x0c * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 131, true), "-0x18 * 2**128");

	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 248, true), "-0x03 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 249, true), "-0x06 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 250, true), "-0x0c * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x3)) << 251, true), "-0x18 * 2**248");

	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 128, true), "-0x09 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 129, true), "-0x12 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 130, true), "-0x24 * 2**128");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 131, true), "-0x48 * 2**128");

	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 248, true), "-0x09 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 249, true), "-0x12 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 250, true), "-0x24 * 2**248");
	BOOST_CHECK_EQUAL(formatNumberReadable(((-1) * s256(0x9)) << 251, true), "-0x48 * 2**248");

	BOOST_CHECK_EQUAL(formatNumberReadable(s256(-1)), "-1");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x10000)), "-65536");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xFFFF)), "-65535");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0x1000000)), "-16777216");
	BOOST_CHECK_EQUAL(formatNumberReadable((-1) * s256(0xFFFFFF)), "-16777215");

	BOOST_CHECK_EQUAL(
		formatNumberReadable(
			frontend::IntegerType(256, frontend::IntegerType::Modifier::Signed).minValue()
		),
		"-2**255"
	);
	BOOST_CHECK_EQUAL(
		formatNumberReadable(
			frontend::IntegerType(256, frontend::IntegerType::Modifier::Signed).maxValue()
		),
		"2**255 - 1"
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
