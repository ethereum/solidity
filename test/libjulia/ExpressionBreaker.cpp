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
 * Unit tests for the expression breaker.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/ExpressionBreaker.h>
#include <libjulia/optimiser/NameCollector.h>

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
	auto result = parse(_original, false);\
	NameDispenser nameDispenser;\
	nameDispenser.m_usedNames = NameCollector(*result.first).names();\
	ExpressionBreaker{nameDispenser}(*result.first);\
	BOOST_CHECK_EQUAL(assembly::AsmPrinter{}(*result.first), format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(YulExpressionBreaker)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(trivial)
{
	CHECK(
		"{ mstore(add(calldataload(2), mload(3)), 8) }",
		"{ let _1 := mload(3) let _2 := calldataload(2) let _3 := add(_2, _1) mstore(_3, 8) }"
	);
}

BOOST_AUTO_TEST_CASE(control_flow)
{
	string input = R"({
		let x := calldataload(0)
		if mul(add(x, 2), 3) {
			for { let a := 2 } lt(a, mload(a)) { a := add(a, mul(a, 2)) } {
				let b := mul(add(a, 2), 4)
				sstore(b, mul(b, 2))
			}
		}
	})";
	string expectation = R"({
		let x := calldataload(0)
		let _1 := add(x, 2)
		let _2 := mul(_1, 3)
		if _2
		{
			for { let a := 2 } lt(a, mload(a))
				{
					let _3 := mul(a, 2)
					a := add(a, _3)
				}
			{
				let _4 := add(a, 2)
				let b := mul(_4, 4)
				let _5 := mul(b, 2)
				sstore(b, _5)
			}
		}
	})";
	CHECK(input, expectation);
}

BOOST_AUTO_TEST_CASE(switch_)
{
	string input = R"({
		let x := 8
		switch add(2, calldataload(0))
		case 0 { sstore(0, mload(2)) }
		default { mstore(0, mload(3)) }
		x := add(mload(3), 4)
	})";
	string expectation = R"({
		let x := 8
		let _1 := calldataload(0)
		let _2 := add(2, _1)
		switch _2
		case 0 {
			let _3 := mload(2)
			sstore(0, _3)
		}
		default {
			let _4 := mload(3)
			mstore(0, _4)
		}
		let _5 := mload(3)
		x := add(_5, 4)
	})";

	CHECK(input, expectation);
}

BOOST_AUTO_TEST_CASE(inside_function)
{
	string input = R"({
		let x := mul(f(0, mload(7)), 3)
		function f(a, b) -> c {
			c := mul(a, mload(add(b, c)))
		}
		sstore(x, f(mload(2), mload(2)))
	})";
	string expectation = R"({
		let _1 := mload(7)
		let _2 := f(0, _1)
		let x := mul(_2, 3)
		function f(a, b) -> c
		{
			let _3 := add(b, c)
			let _4 := mload(_3)
			c := mul(a, _4)
		}
		let _5 := mload(2)
		let _6 := mload(2)
		let _7 := f(_6, _5)
		sstore(x, _7)
	})";

	CHECK(input, expectation);
}

BOOST_AUTO_TEST_SUITE_END()
