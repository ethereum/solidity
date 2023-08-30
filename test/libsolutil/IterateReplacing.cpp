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
 * Unit tests for the iterateReplacing function
 */

#include <libsolutil/CommonData.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(IterateReplacing, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(no_replacement)
{
	std::vector<std::string> v{"abc", "def", "ghi"};
	std::function<std::optional<std::vector<std::string>>(std::string&)> f = [](std::string&) -> std::optional<std::vector<std::string>> { return {}; };
	iterateReplacing(v, f);
	std::vector<std::string> expectation{"abc", "def", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(empty_input)
{
	std::vector<std::string> v;
	std::function<std::optional<std::vector<std::string>>(std::string&)> f = [](std::string&) -> std::optional<std::vector<std::string>> { return {}; };
	iterateReplacing(v, f);
	std::vector<std::string> expectation;
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(delete_some)
{
	std::vector<std::string> v{"abc", "def", "ghi"};
	std::function<std::optional<std::vector<std::string>>(std::string&)> f = [](std::string& _s) -> std::optional<std::vector<std::string>> {
		if (_s == "def")
			return std::vector<std::string>();
		else
			return {};
	};
	iterateReplacing(v, f);
	std::vector<std::string> expectation{"abc", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(inject_some_start)
{
	std::vector<std::string> v{"abc", "def", "ghi"};
	std::function<std::optional<std::vector<std::string>>(std::string&)> f = [](std::string& _s) -> std::optional<std::vector<std::string>> {
		if (_s == "abc")
			return std::vector<std::string>{"x", "y"};
		else
			return {};
	};
	iterateReplacing(v, f);
	std::vector<std::string> expectation{"x", "y", "def", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(inject_some_end)
{
	std::vector<std::string> v{"abc", "def", "ghi"};
	std::function<std::optional<std::vector<std::string>>(std::string&)> f = [](std::string& _s) -> std::optional<std::vector<std::string>> {
		if (_s == "ghi")
			return std::vector<std::string>{"x", "y"};
		else
			return {};
	};
	iterateReplacing(v, f);
	std::vector<std::string> expectation{"abc", "def", "x", "y"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_SUITE_END()

}
