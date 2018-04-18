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
 * @author Killian <krr2125@columbia.edu>
 * @author Jiayang <prokingsley@gmail.com>
 * @author Raphael <raphael.s.norwitz@gmail.com>
 * Unit tests for the solidity expression compiler, testing the behaviour of the code.
 */

#include <string>
#include <regex>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/FTime.h>

BOOST_AUTO_TEST_SUITE(FTimeCompiler)

BOOST_AUTO_TEST_CASE(basic_push)
{
	BOOST_REQUIRE_NO_THROW(t_stack.push("Basic test"));
};

BOOST_AUTO_TEST_CASE(basic_push_pop)
{
	BOOST_REQUIRE_NO_THROW(t_stack.push("Basic test"));
	BOOST_REQUIRE_NO_THROW(t_stack.pop());
};


BOOST_AUTO_TEST_CASE(defalut_print_false)
{
	BOOST_REQUIRE(!t_stack.print_flag);
};

	

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(FTimeFunc)

BOOST_AUTO_TEST_CASE(new_push_pop)
{
	TimeNodeStack new_stack;

	BOOST_REQUIRE_NO_THROW(new_stack.push("hello"));
	
	BOOST_REQUIRE_NO_THROW(new_stack.pop());
};

BOOST_AUTO_TEST_CASE(push_pop_error)
{
	TimeNodeStack new_stack;

	BOOST_REQUIRE_NO_THROW(new_stack.push("hello"));
	
	BOOST_REQUIRE_NO_THROW(new_stack.pop());

	BOOST_CHECK_THROW(new_stack.pop(), std::runtime_error);

};

BOOST_AUTO_TEST_CASE(pop_empty_error)
{
	TimeNodeStack new_stack;

	BOOST_CHECK_THROW(new_stack.pop(), std::runtime_error);

};

BOOST_AUTO_TEST_CASE(tree_two_level)
{
	TimeNodeStack new_stack;

	new_stack.push("hello");
	new_stack.push("world");
	new_stack.pop();
	new_stack.pop();


	std::string result = new_stack.printString(true);
	std::cout << result;

	std::regex reg("(namespace/function name)[\\s]+(unix begin time)"
			"(.*)[\\s]+(.*)\n(-*)\n"
			"(hello)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"( \\\\_world)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n");
	BOOST_REQUIRE(std::regex_match(result, reg));

};

BOOST_AUTO_TEST_CASE(tree_three_level)
{
	TimeNodeStack new_stack;

	new_stack.push("hello");
	new_stack.push("world");
	new_stack.push("!!");
	new_stack.pop();
	new_stack.pop();
	new_stack.pop();


	std::string result = new_stack.printString(true);
	std::cout << result;

	std::regex reg("(namespace/function name)[\\s]+(unix begin time)"
			"(.*)[\\s]+(.*)\n(-*)\n"
			"(hello)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"( \\\\_world)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"(     \\\\_!!)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n");
	BOOST_REQUIRE(std::regex_match(result, reg));

};

BOOST_AUTO_TEST_CASE(notree_two_level)
{
	TimeNodeStack new_stack;

	new_stack.push("hello");
	new_stack.push("world");
	new_stack.pop();
	new_stack.pop();


	std::string result = new_stack.printString(false);
	std::cout << result;

	std::regex reg("(namespace/function name)[\\s]+(unix begin time)"
			"(.*)[\\s]+(.*)\n(-*)\n"
			"(hello)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"( \\\\_world)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n");
	BOOST_REQUIRE(std::regex_match(result, reg));

};

BOOST_AUTO_TEST_CASE(notree_three_level)
{
	TimeNodeStack new_stack;

	new_stack.push("hello");
	new_stack.push("world");
	new_stack.push("!!");
	new_stack.pop();
	new_stack.pop();
	new_stack.pop();


	std::string result = new_stack.printString(false);
	std::cout << result;

	std::regex reg("(namespace/function name)[\\s]+(unix begin time)"
			"(.*)[\\s]+(.*)\n(-*)\n"
			"(hello)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"( \\\\_world)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n"
			"( \\\\_!!)(\\s+)(\\d+)(\\s+)(\\d+)(.*)\n");
	BOOST_REQUIRE(std::regex_match(result, reg));

};



BOOST_AUTO_TEST_SUITE_END()
