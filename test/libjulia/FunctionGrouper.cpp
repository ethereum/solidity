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
 * Unit tests for the Yul function grouper.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/FunctionGrouper.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;

#define CHECK(_original, _expectation)\
do\
{\
	assembly::AsmPrinter p(true);\
	Block b = disambiguate(_original);\
	(FunctionGrouper{})(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulFunctionGrouper)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ { } }");
}

BOOST_AUTO_TEST_CASE(single_fun)
{
	CHECK(
		"{ let a:u256 function f() {} }",
		"{ { let a:u256 } function f() {} }"
	);
}

BOOST_AUTO_TEST_CASE(multi_fun_mixed)
{
	CHECK(
		"{ let a:u256 function f() { let b:u256 } let c:u256 function g() { let d:u256 } let e:u256 }",
		"{ { let a:u256 let c:u256 let e:u256 } function f() { let b:u256 } function g() { let d:u256 } }"
	);
}

BOOST_AUTO_TEST_CASE(nested_fun)
{
	CHECK(
		"{ let a:u256 function f() { let b:u256 function g() { let c:u256} let d:u256 } }",
		"{ { let a:u256 } function f() { let b:u256 function g() { let c:u256} let d:u256 } }"
	);
}

BOOST_AUTO_TEST_CASE(empty_block)
{
	CHECK(
		"{ let a:u256 { } function f() -> x:bool { let b:u256 := 4:u256 {} for {} f() {} {} } }",
		"{ { let a:u256 { } } function f() -> x:bool { let b:u256 := 4:u256 {} for {} f() {} {} } }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
