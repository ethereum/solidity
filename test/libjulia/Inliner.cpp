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
 * Unit tests for the Yul function inliner.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/ExpressionInliner.h>
#include <libjulia/optimiser/InlinableExpressionFunctionFinder.h>
#include <libjulia/optimiser/FullInliner.h>
#include <libjulia/optimiser/FunctionHoister.h>
#include <libjulia/optimiser/FunctionGrouper.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;

namespace
{
string inlinableFunctions(string const& _source)
{
	auto ast = disambiguate(_source);

	InlinableExpressionFunctionFinder funFinder;
	funFinder(ast);

	return boost::algorithm::join(
		funFinder.inlinableFunctions() | boost::adaptors::map_keys,
		","
	);
}

string inlineFunctions(string const& _source, bool _yul = true)
{
	auto ast = disambiguate(_source, _yul);
	ExpressionInliner(ast).run();
	return assembly::AsmPrinter(_yul)(ast);
}
string fullInline(string const& _source, bool _yul = true)
{
	Block ast = disambiguate(_source, _yul);
	(FunctionHoister{})(ast);
	(FunctionGrouper{})(ast);\
	FullInliner(ast).run();
	return assembly::AsmPrinter(_yul)(ast);
}
}


BOOST_AUTO_TEST_SUITE(YulInlinableFunctionFilter)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ }"), "");
}

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := 2:u256 } }"), "f");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"function g(a:u256) -> b:u256 { b := a }"
		"function f() -> x:u256 { x := g(2:u256) }"
	"}"), "f,g");
}

BOOST_AUTO_TEST_CASE(simple_inside_structures)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"switch 2:u256 "
		"case 2:u256 {"
			"function g(a:u256) -> b:u256 { b := a }"
			"function f() -> x:u256 { x := g(2:u256) }"
		"}"
	"}"), "f,g");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"for {"
			"function g(a:u256) -> b:u256 { b := a }"
		"} 1:u256 {"
			"function f() -> x:u256 { x := g(2:u256) }"
		"}"
		"{"
			"function h() -> y:u256 { y := 2:u256 }"
		"}"
	"}"), "f,g,h");
}

BOOST_AUTO_TEST_CASE(negative)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := 2:u256 {} } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := f() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := x } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256, y:u256 { x := 2:u256 } }"), "");
  BOOST_CHECK_EQUAL(inlinableFunctions(
    "{ function g() -> x:u256, y:u256 {} function f(y:u256) -> x:u256 { x,y := g() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f(y:u256) -> x:u256 { y := 2:u256 } }"), "");
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(YulFunctionInliner)

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_EQUAL(
		inlineFunctions("{ function f() -> x:u256 { x := 2:u256 } let y:u256 := f() }"),
		format("{ function f() -> x:u256 { x := 2:u256 } let y:u256 := 2:u256 }")
	);
}

BOOST_AUTO_TEST_CASE(with_args)
{
	BOOST_CHECK_EQUAL(
		inlineFunctions("{ function f(a:u256) -> x:u256 { x := a } let y:u256 := f(7:u256) }"),
		format("{ function f(a:u256) -> x:u256 { x := a } let y:u256 := 7:u256 }")
	);
}

BOOST_AUTO_TEST_CASE(no_inline_with_mload)
{
	// Does not inline because mload could be moved out of sequence
	BOOST_CHECK_EQUAL(
		inlineFunctions("{ function f(a) -> x { x := a } let y := f(mload(2)) }", false),
		format("{ function f(a) -> x { x := a } let y := f(mload(2)) }", false)
	);
}

BOOST_AUTO_TEST_CASE(no_move_with_side_effects)
{
	// The calls to g and h cannot be moved because g and h are not movable. Therefore, the call
	// to f is not inlined.
	BOOST_CHECK_EQUAL(
		inlineFunctions("{"
			"function f(a, b) -> x { x := add(b, a) }"
			"function g() -> y { y := mload(0) mstore(0, 4) }"
			"function h() -> z { mstore(0, 4) z := mload(0) }"
			"let r := f(g(), h())"
		"}", false),
		format("{"
			"function f(a, b) -> x { x := add(b, a) }"
			"function g() -> y { y := mload(0) mstore(0, 4) }"
			"function h() -> z { mstore(0, 4) z := mload(0) }"
			"let r := f(g(), h())"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(complex_with_evm)
{
	BOOST_CHECK_EQUAL(
		inlineFunctions("{ function f(a) -> x { x := add(a, a) } let y := f(calldatasize()) }", false),
		format("{ function f(a) -> x { x := add(a, a) } let y := add(calldatasize(), calldatasize()) }", false)
	);
}

BOOST_AUTO_TEST_CASE(double_calls)
{
	BOOST_CHECK_EQUAL(
		inlineFunctions("{"
			"function f(a) -> x { x := add(a, a) }"
			"function g(b, c) -> y { y := mul(mload(c), f(b)) }"
			"let y := g(calldatasize(), 7)"
		"}", false),
		format("{"
			"function f(a) -> x { x := add(a, a) }"
			"function g(b, c) -> y { y := mul(mload(c), add(b, b)) }"
			"let y_1 := mul(mload(7), add(calldatasize(), calldatasize()))"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(double_recursive_calls)
{
	BOOST_CHECK_EQUAL(
		inlineFunctions("{"
			"function f(a, r) -> x { x := g(a, g(r, r)) }"
			"function g(b, s) -> y { y := f(b, f(s, s)) }"
			"let y := g(calldatasize(), 7)"
		"}", false),
		format("{"
			"function f(a, r) -> x { x := g(a, f(r, f(r, r))) }"
			"function g(b, s) -> y { y := f(b, g(s, f(s, f(s, s))))}"
			"let y_1 := f(calldatasize(), g(7, f(7, f(7, 7))))"
		"}", false)
	);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(YulFullInliner)

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_EQUAL(
		fullInline("{"
			"function f(a) -> x { let r := mul(a, a) x := add(r, r) }"
			"let y := add(f(sload(mload(2))), mload(7))"
		"}", false),
		format("{"
			"{"
				"let _1 := mload(7)"
				"let f_a := sload(mload(2))"
				"let f_x"
				"{"
					"let f_r := mul(f_a, f_a)"
					"f_x := add(f_r, f_r)"
				"}"
				"let y := add(f_x, _1)"
			"}"
			"function f(a) -> x"
			"{"
				"let r := mul(a, a)"
				"x := add(r, r)"
			"}"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(multi_fun)
{
	BOOST_CHECK_EQUAL(
		fullInline("{"
			"function f(a) -> x { x := add(a, a) }"
			"function g(b, c) -> y { y := mul(mload(c), f(b)) }"
			"let y := g(f(3), 7)"
		"}", false),
		format("{"
			"{"
				"let g_c := 7 "
				"let f_a_1 := 3 "
				"let f_x_1 "
				"{ f_x_1 := add(f_a_1, f_a_1) } "
				"let g_y "
				"{"
					"let g_f_a := f_x_1 "
					"let g_f_x "
					"{"
						"g_f_x := add(g_f_a, g_f_a)"
					"}"
					"g_y := mul(mload(g_c), g_f_x)"
				"}"
				"let y_1 := g_y"
			"}"
			"function f(a) -> x"
			"{"
				"x := add(a, a)"
			"}"
			"function g(b, c) -> y"
			"{"
				"let f_a := b "
				"let f_x "
				"{"
					"f_x := add(f_a, f_a)"
				"}"
				"y := mul(mload(c), f_x)"
			"}"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(move_up_rightwards_arguments)
{
	BOOST_CHECK_EQUAL(
		fullInline("{"
			"function f(a, b, c) -> x { x := add(a, b) x := mul(x, c) }"
			"let y := add(mload(1), add(f(mload(2), mload(3), mload(4)), mload(5)))"
		"}", false),
		format("{"
			"{"
				"let _1 := mload(5)"
				"let f_c := mload(4)"
				"let f_b := mload(3)"
				"let f_a := mload(2)"
				"let f_x"
				"{"
					"f_x := add(f_a, f_b)"
					"f_x := mul(f_x, f_c)"
				"}"
				"let y := add(mload(1), add(f_x, _1))"
			"}"
			"function f(a, b, c) -> x"
			"{"
				"x := add(a, b)"
				"x := mul(x, c)"
			"}"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(pop_result)
{
	// This tests that `pop(r)` is removed.
	BOOST_CHECK_EQUAL(
		fullInline("{"
			"function f(a) -> x { let r := mul(a, a) x := add(r, r) }"
			"pop(add(f(7), 2))"
		"}", false),
		format("{"
			"{"
				"let _1 := 2 "
				"let f_a := 7 "
				"let f_x "
				"{"
					"let f_r := mul(f_a, f_a) "
					"f_x := add(f_r, f_r)"
				"}"
				"{"
				"}"
			"}"
			"function f(a) -> x"
			"{"
				"let r := mul(a, a) "
				"x := add(r, r)"
			"}"
		"}", false)
	);
}

BOOST_AUTO_TEST_CASE(inside_condition)
{
	// This tests that breaking the expression inside the condition works properly.
	BOOST_CHECK_EQUAL(
		fullInline("{"
			"if gt(f(mload(1)), mload(0)) {"
				"sstore(0, 2)"
			"}"
			"function f(a) -> r {"
				"a := mload(a)"
				"r := add(a, calldatasize())"
			"}"
		"}", false),
		format("{"
			"{"
				"let _1 := mload(0)"
				"let f_a := mload(1)"
				"let f_r"
				"{"
					"f_a := mload(f_a)"
					"f_r := add(f_a, calldatasize())"
				"}"
				"if gt(f_r, _1)"
				"{"
					"sstore(0, 2)"
				"}"
			"}"
			"function f(a) -> r"
			"{"
				"a := mload(a)"
				"r := add(a, calldatasize())"
			"}"
		"}", false)
	);
}

BOOST_AUTO_TEST_SUITE_END()
