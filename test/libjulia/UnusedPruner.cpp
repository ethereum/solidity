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
 * Unit tests for the pruning of unused variables and functions.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/UnusedPruner.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;


#define CHECK(_original, _expectation)\
do\
{\
	assembly::AsmPrinter p;\
	Block b = disambiguate(_original, false);\
	UnusedPruner::runUntilStabilised(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulUnusedPruner)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(trivial)
{
	CHECK(
		"{ let a := 1 let b := 1 mstore(0, 1) }",
		"{ mstore(0, 1) }"
	);
}

BOOST_AUTO_TEST_CASE(multi_declarations)
{
	CHECK(
		"{ let x, y }",
		"{ }"
	);
}

BOOST_AUTO_TEST_CASE(multi_assignments)
{
	CHECK(
		"{ let x, y x := 1 y := 2 }",
		"{ let x, y x := 1 y := 2 }"
	);
}

BOOST_AUTO_TEST_CASE(multi_partial_assignments)
{
	CHECK(
		"{ let x, y x := 1 }",
		"{ let x, y x := 1 }"
	);
}

BOOST_AUTO_TEST_CASE(functions)
{
	CHECK(
		"{ function f() { let a := 1 } function g() { f() } }",
		"{ }"
	);
}

BOOST_AUTO_TEST_CASE(intermediate_assignment)
{
	CHECK(
		"{ let a := 1 a := 4 let b := 1 }",
		"{ let a := 1 a := 4 }"
	);
}

BOOST_AUTO_TEST_CASE(intermediate_multi_assignment){
	CHECK(
		"{ let a, b function f() -> x { } a := f() b := 1 }",
		"{ let a, b function f() -> x { } a := f() b := 1 }"
	);
}

BOOST_AUTO_TEST_CASE(multi_declare)
{
	CHECK(
		"{ function f() -> x, y { } let a, b := f() }",
		"{ function f() -> x, y { } let a, b := f() }"
	);
}

BOOST_AUTO_TEST_CASE(multi_assign)
{
	CHECK(
		"{ let a let b function f() -> x, y { } a, b := f() }",
		"{ let a let b function f() -> x, y { } a, b := f() }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
