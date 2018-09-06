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
 * Unit tests for the rematerialiser optimizer stage.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/Rematerialiser.h>

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
	(Rematerialiser{})(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulRematerialiser)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(trivial)
{
	CHECK(
		"{ let a := 1 let b := a mstore(0, b) }",
		"{ let a := 1 let b := 1 mstore(0, 1) }"
	);
}

BOOST_AUTO_TEST_CASE(expression)
{
	CHECK(
		"{ let a := add(mul(calldatasize(), 2), number()) let b := add(a, a) }",
		"{ let a := add(mul(calldatasize(), 2), number()) let b := add("
			"add(mul(calldatasize(), 2), number()),"
			"add(mul(calldatasize(), 2), number())"
		") }"
	);
}

BOOST_AUTO_TEST_CASE(reassign)
{
	CHECK(
		"{ let a := extcodesize(0) let b := a let c := b a := 2 let d := add(b, c) pop(a) pop(b) pop(c) pop(d) }",
		"{ let a := extcodesize(0) let b := a let c := a a := 2 let d := add(b, c) pop(2) pop(b) pop(c) pop(add(b, c)) }"
	);
}

BOOST_AUTO_TEST_CASE(non_movable_instr)
{
	CHECK(
		"{ let a := 1 let b := mload(a) let c := a mstore(add(a, b), c) }",
		"{ let a := 1 let b := mload(1) let c := 1 mstore(add(1, b), 1) }"
	);
}

BOOST_AUTO_TEST_CASE(non_movable_fun)
{
	CHECK(
		"{ function f(x) -> y {} let a := 1 let b := f(a) let c := a mstore(add(a, b), c) }",
		"{ function f(x) -> y {} let a := 1 let b := f(1) let c := 1 mstore(add(1, b), 1) }"
	);
}

BOOST_AUTO_TEST_CASE(branches_if)
{
	CHECK(
		"{ let a := 1 let b := 2 if b { pop(b) b := a } let c := b }",
		"{ let a := 1 let b := 2 if 2 { pop(2) b := 1 } let c := b }"
	);
}

BOOST_AUTO_TEST_CASE(branches_switch)
{
	CHECK(
		"{ let a := 1 let b := 2 switch number() case 1 { b := a } default { let x := a let y := b b := a } pop(add(a, b)) }",
		"{ let a := 1 let b := 2 switch number() case 1 { b := 1 } default { let x := 1 let y := b b := 1 } pop(add(1, b)) }"
	);
}

BOOST_AUTO_TEST_CASE(branches_for)
{
	CHECK(
		"{ let a := 1 for { pop(a) } a { pop(a) } { pop(a) } }",
		"{ let a := 1 for { pop(1) } 1 { pop(1) } { pop(1) } }"
	);
	CHECK(
		"{ let a := 1 for { pop(a) } a { pop(a) } { a := 7 let c := a } let x := a }",
		"{ let a := 1 for { pop(1) } a { pop(7) } { a := 7 let c := 7 } let x := a }"
	);
}

BOOST_AUTO_TEST_CASE(branches_for_declared_in_init)
{
	CHECK(
		"{ let b := 0 for { let a := 1 pop(a) } a { pop(a) } { b := 1 pop(a) } }",
		"{ let b := 0 for { let a := 1 pop(1) } 1 { pop(1) } { b := 1 pop(1) } }"
	);
	CHECK(
		"{ let b := 0 for { let a := 1 pop(a) } lt(a, 0) { pop(a) a := add(a, 3) } { b := 1 pop(a) } }",
		"{ let b := 0 for { let a := 1 pop(1) } lt(a, 0) { pop(a) a := add(a, 3) } { b := 1 pop(a) } }"
	);
}

BOOST_AUTO_TEST_CASE(reassignment)
{
	CHECK(
		"{ let a := 1 pop(a) if a { a := 2 } let b := mload(a) pop(b) }",
		"{ let a := 1 pop(1) if 1 { a := 2 } let b := mload(a) pop(b) }"
	);
}

BOOST_AUTO_TEST_CASE(update_assignment_remat)
{
	// We cannot substitute `a` in `let b := a`
	CHECK(
		"{ let a := extcodesize(0) a := mul(a, 2) let b := a }",
		"{ let a := extcodesize(0) a := mul(a, 2) let b := a }"
	);
}

BOOST_AUTO_TEST_CASE(do_not_move_out_of_scope)
{
	// Cannot replace by `let b := x` by `let b := a` since a is out of scope.
	CHECK(
		"{ let x { let a := sload(0) x := a } let b := x }",
		"{ let x { let a := sload(0) x := a } let b := x }"
	);
}

BOOST_AUTO_TEST_CASE(do_not_remat_large_amounts_of_code)
{
	CHECK(
		"{ let x := add(mul(calldataload(2), calldataload(4)), mul(2, calldatasize())) let b := x }",
		"{ let x := add(mul(calldataload(2), calldataload(4)), mul(2, calldatasize())) let b := x }"
	);
	CHECK(
		"{ let x := add(mul(calldataload(2), calldataload(4)), calldatasize()) let b := x }",
		"{ let x := add(mul(calldataload(2), calldataload(4)), calldatasize()) let b := add(mul(calldataload(2), calldataload(4)), calldatasize()) }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
