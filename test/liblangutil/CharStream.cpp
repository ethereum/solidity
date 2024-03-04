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

using namespace solidity::test;

namespace boost::test_tools::tt_detail
{

template<>
struct print_log_value<std::optional<int>>
{
	void operator()(std::ostream& _out, std::optional<int> const& _value) const
	{
		_out << (_value ? to_string(*_value) : "[std::nullopt]");
	}
};

template<>
struct print_log_value<std::nullopt_t>
{
	void operator()(std::ostream& _out, std::nullopt_t const&) const
	{
		_out << "[std::nullopt]";
	}
};

} // namespace boost::test_tools::tt_detail


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

namespace
{
std::optional<int> toPosition(int _line, int _column, std::string const& _text)
{
	return CharStream{_text, "source"}.translateLineColumnToPosition(
		LineColumn{_line, _column}
	);
}

}

BOOST_AUTO_TEST_CASE(translateLineColumnToPosition)
{
	BOOST_CHECK_EQUAL(toPosition(-1, 0, "ABC"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(0, -1, "ABC"), std::nullopt);

	BOOST_CHECK_EQUAL(toPosition(0, 0, ""), 0);
	BOOST_CHECK_EQUAL(toPosition(1, 0, ""), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(0, 1, ""), std::nullopt);

	// With last line containing no LF
	BOOST_CHECK_EQUAL(toPosition(0, 0, "ABC"), 0);
	BOOST_CHECK_EQUAL(toPosition(0, 1, "ABC"), 1);
	BOOST_CHECK_EQUAL(toPosition(0, 2, "ABC"), 2);
	BOOST_CHECK_EQUAL(toPosition(0, 3, "ABC"), 3);
	BOOST_CHECK_EQUAL(toPosition(0, 4, "ABC"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(1, 0, "ABC"), std::nullopt);

	BOOST_CHECK_EQUAL(toPosition(0, 3, "ABC\nDEF"), 3);
	BOOST_CHECK_EQUAL(toPosition(0, 4, "ABC\nDEF"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(1, 0, "ABC\nDEF"), 4);
	BOOST_CHECK_EQUAL(toPosition(1, 1, "ABC\nDEF"), 5);
	BOOST_CHECK_EQUAL(toPosition(1, 2, "ABC\nDEF"), 6);
	BOOST_CHECK_EQUAL(toPosition(1, 3, "ABC\nDEF"), 7);
	BOOST_CHECK_EQUAL(toPosition(1, 4, "ABC\nDEF"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(2, 0, "ABC\nDEF"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(2, 1, "ABC\nDEF"), std::nullopt);

	// With last line containing LF
	BOOST_CHECK_EQUAL(toPosition(0, 0, "ABC\nDEF\n"), 0);
	BOOST_CHECK_EQUAL(toPosition(0, 1, "ABC\nDEF\n"), 1);
	BOOST_CHECK_EQUAL(toPosition(0, 2, "ABC\nDEF\n"), 2);

	BOOST_CHECK_EQUAL(toPosition(1, 0, "ABC\nDEF\n"), 4);
	BOOST_CHECK_EQUAL(toPosition(1, 1, "ABC\nDEF\n"), 5);
	BOOST_CHECK_EQUAL(toPosition(1, 2, "ABC\nDEF\n"), 6);
	BOOST_CHECK_EQUAL(toPosition(1, 3, "ABC\nDEF\n"), 7);
	BOOST_CHECK_EQUAL(toPosition(1, 4, "ABC\nDEF\n"), std::nullopt);
	BOOST_CHECK_EQUAL(toPosition(2, 0, "ABC\nDEF\n"), 8);
	BOOST_CHECK_EQUAL(toPosition(2, 1, "ABC\nDEF\n"), std::nullopt);

	BOOST_CHECK_EQUAL(toPosition(2, 0, "ABC\nDEF\nGHI\n"), 8);
	BOOST_CHECK_EQUAL(toPosition(2, 1, "ABC\nDEF\nGHI\n"), 9);
	BOOST_CHECK_EQUAL(toPosition(2, 2, "ABC\nDEF\nGHI\n"), 10);
}

BOOST_AUTO_TEST_SUITE_END()

}
