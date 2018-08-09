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
 * Unit tests for the expression simplifier optimizer stage.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/ExpressionSimplifier.h>

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
	Block b = *(parse(_original, false).first);\
	(ExpressionSimplifier{})(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulSimplifier)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(constants)
{
	CHECK(
		"{ let a := add(1, mul(3, 4)) }",
		"{ let a := 13 }"
	);
}

BOOST_AUTO_TEST_CASE(invariant)
{
	CHECK(
		"{ let a := mload(sub(7, 7)) let b := sub(a, 0) }",
		"{ let a := mload(0) let b := a }"
	);
}

BOOST_AUTO_TEST_CASE(reversed)
{
	CHECK(
		"{ let a := add(0, mload(0)) }",
		"{ let a := mload(0) }"
	);
}

BOOST_AUTO_TEST_CASE(constant_propagation)
{
	CHECK(
		"{ let a := add(7, sub(mload(0), 7)) }",
		"{ let a := mload(0) }"
	);
}

BOOST_AUTO_TEST_CASE(identity_rules_simple)
{
	CHECK(
		"{ let a := mload(0) let b := sub(a, a) }",
		"{ let a := mload(0) let b := 0 }"
	);
}

BOOST_AUTO_TEST_CASE(identity_rules_complex)
{
	CHECK(
		"{ let a := sub(calldataload(0), calldataload(0)) }",
		"{ let a := 0 }"
	);
}

BOOST_AUTO_TEST_CASE(identity_rules_negative)
{
	CHECK(
		"{ let a := sub(calldataload(1), calldataload(0)) }",
		"{ let a := sub(calldataload(1), calldataload(0)) }"
	);
}

BOOST_AUTO_TEST_CASE(including_function_calls)
{
	CHECK(
		"{ function f() -> a {} let b := add(7, sub(f(), 7)) }",
		"{ function f() -> a {} let b := f() }"
	);
}

BOOST_AUTO_TEST_CASE(inside_for)
{
	CHECK(
		"{ for { let a := 10 } iszero(eq(a, 0)) { a := add(a, 1) } {} }",
		"{ for { let a := 10 } iszero(iszero(a)) { a := add(a, 1) } {} }"
	);
}

BOOST_AUTO_TEST_CASE(mod_and)
{
	CHECK(
		"{ mstore(0, mod(calldataload(0), exp(2, 8))) }",
		"{ mstore(0, and(calldataload(0), 255)) }"
	);
	CHECK(
		"{ mstore(0, mod(calldataload(0), exp(2, 255))) }",
		"{ mstore(0, and(calldataload(0), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)) }"
	);
}

BOOST_AUTO_TEST_CASE(not_applied_removes_non_constant_and_not_movable)
{
	CHECK(
		// The first argument of div is not constant.
		// keccak256 is not movable.
		"{ let a := div(keccak256(0, 0), 0) }",
		"{ let a := div(keccak256(0, 0), 0) }"
	);
}

BOOST_AUTO_TEST_CASE(not_applied_function_call_different_names)
{
	CHECK(
		"{ function f1() -> a { } function f2() -> b {} let c := sub(f1(), f2()) }",
		"{ function f1() -> a { } function f2() -> b {} let c := sub(f1(), f2()) }"
	);
}

BOOST_AUTO_TEST_CASE(not_applied_function_call_different_arguments)
{
	CHECK(
		"{ function f(a) -> b { } let c := sub(f(0), f(1)) }",
		"{ function f(a) -> b { } let c := sub(f(0), f(1)) }"
	);
}

BOOST_AUTO_TEST_CASE(not_applied_function_call_equality_not_movable)
{
	CHECK(
		// Even if the functions pass the equality check, they are not movable.
		"{ function f() -> a { } let b := sub(f(), f()) }",
		"{ function f() -> a { } let b := sub(f(), f()) }"
	);
}


BOOST_AUTO_TEST_SUITE_END()
