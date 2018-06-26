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
 * Unit tests for the common subexpression eliminator optimizer stage.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/CommonSubexpressionEliminator.h>

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
	(CommonSubexpressionEliminator{})(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulCSE)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(trivial)
{
	CHECK(
		"{ let a := mul(1, codesize()) let b := mul(1, codesize()) }",
		"{ let a := mul(1, codesize()) let b := a }"
	);
}

BOOST_AUTO_TEST_CASE(non_movable_instr)
{
	CHECK(
		"{ let a := mload(1) let b := mload(1) }",
		"{ let a := mload(1) let b := mload(1) }"
	);
}

BOOST_AUTO_TEST_CASE(non_movable_instr2)
{
	CHECK(
		"{ let a := gas() let b := gas() }",
		"{ let a := gas() let b := gas() }"
	);
}

BOOST_AUTO_TEST_CASE(branches_if)
{
	CHECK(
		"{ let b := 1 if b { b := 1 } let c := 1 }",
		"{ let b := 1 if b { b := b } let c := 1 }"
	);
}

BOOST_AUTO_TEST_CASE(branches_for)
{
	CHECK(
		"{ let a := 1 let b := codesize()"
		"for { } lt(1, codesize()) { mstore(1, codesize()) a := add(a, codesize()) }"
		"{ mstore(1, codesize()) } mstore(1, codesize()) }",

		"{ let a := 1 let b := codesize()"
		"for { } lt(1, b) { mstore(1, b) a := add(a, b) }"
		"{ mstore(1, b) } mstore(1, b) }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
