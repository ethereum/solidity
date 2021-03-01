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
 * @author Rocky Bernstein <rocky.bernstein@consensys.net>
 * @date 2019
 * Unit tests for the CharStream class.
 */

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

namespace solidity::langutil::test
{

BOOST_AUTO_TEST_SUITE(CharStreamTest)

BOOST_AUTO_TEST_CASE(test_fail)
{
	auto const source = std::make_shared<CharStream>("now is the time for testing", "source");

	BOOST_CHECK('n' == source->get());
	BOOST_CHECK('n' == source->get());
	BOOST_CHECK('o' == source->advanceAndGet());
	BOOST_CHECK('n' == source->rollback(1));
	BOOST_CHECK('w' == source->setPosition(2));
	BOOST_REQUIRE_THROW(
		source->setPosition(200),
		::solidity::langutil::InternalCompilerError
	);
}

#define testLineColumnToPositionTranslationFail(_line, _column, _text) \
	do                                                                       \
	{                                                                        \
		auto const source = std::make_shared<CharStream>((_text), "source"); \
		auto const actualPosition =                                          \
			source->translateLineColumnToPosition((_line), (_column));       \
		BOOST_CHECK_EQUAL(actualPosition.value_or(-1), -1);                  \
	}                                                                        \
	while (0)

#define testLineColumnToPositionTranslation(_line, _column, _expectedPosition, _text) \
	do                                                                       \
	{                                                                        \
		auto const source = std::make_shared<CharStream>((_text), "source"); \
		auto const actualPosition =                                          \
			source->translateLineColumnToPosition((_line), (_column));       \
		BOOST_CHECK(actualPosition.has_value());                             \
		BOOST_CHECK_EQUAL(*actualPosition, (_expectedPosition));             \
	}                                                                        \
	while (0)

BOOST_AUTO_TEST_CASE(translateLineColumnToPosition)
{
	testLineColumnToPositionTranslationFail(0, 0, "");

	// With last line containing no LF
	testLineColumnToPositionTranslation(0, 0, 0, "ABC");
	testLineColumnToPositionTranslation(0, 1, 1, "ABC");
	testLineColumnToPositionTranslation(0, 2, 2, "ABC");

	testLineColumnToPositionTranslation(1, 0, 4, "ABC\nDEF");
	testLineColumnToPositionTranslation(1, 1, 5, "ABC\nDEF");
	testLineColumnToPositionTranslation(1, 2, 6, "ABC\nDEF");

	// With last line containing LF
	testLineColumnToPositionTranslation(0, 0, 0, "ABC\nDEF\n");
	testLineColumnToPositionTranslation(0, 1, 1, "ABC\nDEF\n");
	testLineColumnToPositionTranslation(0, 2, 2, "ABC\nDEF\n");

	testLineColumnToPositionTranslation(1, 0, 4, "ABC\nDEF\n");
	testLineColumnToPositionTranslation(1, 1, 5, "ABC\nDEF\n");
	testLineColumnToPositionTranslation(1, 2, 6, "ABC\nDEF\n");

	testLineColumnToPositionTranslation(2, 0, 8, "ABC\nDEF\nGHI\n");
	testLineColumnToPositionTranslation(2, 1, 9, "ABC\nDEF\nGHI\n");
	testLineColumnToPositionTranslation(2, 2, 10, "ABC\nDEF\nGHI\n");

	// Column overflows.
	testLineColumnToPositionTranslationFail(0, 3, "ABC\nDEF\n");
	testLineColumnToPositionTranslationFail(1, 3, "ABC\nDEF\n");
	testLineColumnToPositionTranslationFail(2, 3, "ABC\nDEF\nGHI\n");

	// Line overflow.
	testLineColumnToPositionTranslationFail(3, 0, "ABC\nDEF\nGHI\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
