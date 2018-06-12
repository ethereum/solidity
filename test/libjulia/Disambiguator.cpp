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
 * @date 2017
 * Unit tests for the Yul name disambiguator.
 */

#include <test/libjulia/Common.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev::julia::test;
using namespace dev::solidity;

#define CHECK(_original, _expectation)\
do\
{\
	assembly::AsmPrinter p(true);\
	string result = p(disambiguate(_original));\
	BOOST_CHECK_EQUAL(result, format(_expectation));\
	BOOST_CHECK_EQUAL(result, p(disambiguate(result)));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(IuliaDisambiguator)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(variables)
{
	CHECK(
		"{ { let a:u256 } { let a:u256 } }",
		"{ { let a:u256 } { let a_1:u256 } }"
	);
}

BOOST_AUTO_TEST_CASE(variables_clash)
{
	CHECK(
		"{ { let a:u256 let a_1:u256 } { let a:u256 } }",
		"{ { let a:u256 let a_1:u256 } { let a_2:u256 } }"
	);
}

BOOST_AUTO_TEST_CASE(variables_inside_functions)
{
	CHECK(
		"{ { let c:u256 let b:u256 } function f(a:u256, c:u256) -> b:u256 { let x:u256 } { let a:u256 let x:u256 } }",
		"{ { let c:u256 let b:u256 } function f(a:u256, c_1:u256) -> b_1:u256 { let x:u256 } { let a_1:u256 let x_1:u256 } }"
	);
}

BOOST_AUTO_TEST_CASE(function_call)
{
	CHECK(
		"{ { let a:u256, b:u256, c:u256, d:u256, f:u256 } { function f(a:u256) -> c:u256, d:u256 { let b:u256, c_1:u256 := f(a) } } }",
		"{ { let a:u256, b:u256, c:u256, d:u256, f:u256 } { function f_1(a_1:u256) -> c_1:u256, d_1:u256 { let b_1:u256, c_1_1:u256 := f_1(a_1) } } }"
	);
}

BOOST_AUTO_TEST_CASE(for_statement)
{
	CHECK(
		"{ { let a:u256, b:u256 } { for { let a:u256 } a { a := a } { let b:u256 := a } } }",
		"{ { let a:u256, b:u256 } { for { let a_1:u256 } a_1 { a_1 := a_1 } { let b_1:u256 := a_1 } } }"
	);
}

BOOST_AUTO_TEST_CASE(switch_statement)
{
	CHECK(
		"{ { let a:u256, b:u256, c:u256 } { let a:u256 switch a case 0:u256 { let b:u256 := a } default { let c:u256 := a } } }",
		"{ { let a:u256, b:u256, c:u256 } { let a_1:u256 switch a_1 case 0:u256 { let b_1:u256 := a_1 } default { let c_1:u256 := a_1 } } }"
	);
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	CHECK(
		"{ { let a:u256, b:u256, c:u256 } { let a:bool if a { let b:bool := a } } }",
		"{ { let a:u256, b:u256, c:u256 } { let a_1:bool if a_1 { let b_1:bool := a_1 } } }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
