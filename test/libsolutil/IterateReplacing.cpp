// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for the iterateReplacing function
 */

#include <libsolutil/CommonData.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace solidity::util::test
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
