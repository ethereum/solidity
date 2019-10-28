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
 * Unit tests for the iterateReplacing function
 */

#include <libdevcore/CommonData.h>

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(IterateReplacing)

BOOST_AUTO_TEST_CASE(no_replacement)
{
	vector<string> v{"abc", "def", "ghi"};
	function<std::optional<vector<string>>(string&)> f = [](string&) -> std::optional<vector<string>> { return {}; };
	iterateReplacing(v, f);
	vector<string> expectation{"abc", "def", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(empty_input)
{
	vector<string> v;
	function<std::optional<vector<string>>(string&)> f = [](string&) -> std::optional<vector<string>> { return {}; };
	iterateReplacing(v, f);
	vector<string> expectation;
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(delete_some)
{
	vector<string> v{"abc", "def", "ghi"};
	function<std::optional<vector<string>>(string&)> f = [](string& _s) -> std::optional<vector<string>> {
		if (_s == "def")
			return vector<string>();
		else
			return {};
	};
	iterateReplacing(v, f);
	vector<string> expectation{"abc", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(inject_some_start)
{
	vector<string> v{"abc", "def", "ghi"};
	function<std::optional<vector<string>>(string&)> f = [](string& _s) -> std::optional<vector<string>> {
		if (_s == "abc")
			return vector<string>{"x", "y"};
		else
			return {};
	};
	iterateReplacing(v, f);
	vector<string> expectation{"x", "y", "def", "ghi"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_CASE(inject_some_end)
{
	vector<string> v{"abc", "def", "ghi"};
	function<std::optional<vector<string>>(string&)> f = [](string& _s) -> std::optional<vector<string>> {
		if (_s == "ghi")
			return vector<string>{"x", "y"};
		else
			return {};
	};
	iterateReplacing(v, f);
	vector<string> expectation{"abc", "def", "x", "y"};
	BOOST_CHECK(v == expectation);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
