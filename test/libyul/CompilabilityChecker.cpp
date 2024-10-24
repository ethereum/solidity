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
 * Unit tests for the compilability checker.
 */

#include <test/Common.h>

#include <test/libyul/Common.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/CompilabilityChecker.h>

#include <boost/test/unit_test.hpp>

namespace solidity::yul::test
{

namespace
{
std::string check(std::string const& _input)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVM(
			solidity::test::CommonOptions::get().evmVersion(),
			solidity::test::CommonOptions::get().eofVersion()
	);
	Object obj{dialect};
	auto parsingResult = yul::test::parse(_input);
	obj.setCode(parsingResult.first, parsingResult.second);
	BOOST_REQUIRE(obj.hasCode());
	auto functions = CompilabilityChecker(dialect, obj, true).stackDeficit;
	std::string out;
	for (auto const& function: functions)
		out += function.first.str() + ": " + std::to_string(function.second) + " ";
	return out;
}
}

BOOST_AUTO_TEST_SUITE(CompilabilityChecker)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	std::string out = check("{}");
	BOOST_CHECK_EQUAL(out, "");
}

BOOST_AUTO_TEST_CASE(simple_function)
{
	std::string out = check("{ function f(a, b) -> x, y { x := a y := b } }");
	BOOST_CHECK_EQUAL(out, "");
}

BOOST_AUTO_TEST_CASE(many_variables_few_uses)
{
	std::string out = check(R"({
		function f(a, b) -> x, y {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(x, r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 4 ");
}

BOOST_AUTO_TEST_CASE(many_variables_many_uses)
{
	std::string out = check(R"({
		function f(a, b) -> x, y {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 10 ");
}

BOOST_AUTO_TEST_CASE(many_return_variables_unused_arguments)
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 3 ");
}

BOOST_AUTO_TEST_CASE(many_return_variables_used_arguments)
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
			r1 := 0
			sstore(a, b)
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 5 ");
}

BOOST_AUTO_TEST_CASE(multiple_functions_used_arguments)
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
			r1 := 0
			sstore(a, b)
		}
		function g(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19) -> x, y {
			x := 0
			sstore(r1, r2)
		}
		function h(x) {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "h: 9 g: 5 f: 5 ");
}

BOOST_AUTO_TEST_CASE(multiple_functions_unused_arguments)
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
		}
		function g(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19) -> x, y {
		}
		function h(x) {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "h: 9 f: 3 ");
}

BOOST_AUTO_TEST_CASE(nested_used_arguments)
{
	std::string out = check(R"({
		function h(x) {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			function f(a, b) -> t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19 {
				function g(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19) -> w, v {
					w := v
					sstore(s1, s2)
				}
				t1 := t2
				sstore(a, b)
			}
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "h: 9 g: 5 f: 5 ");
}


BOOST_AUTO_TEST_CASE(nested_unused_arguments)
{
	std::string out = check(R"({
		function h(x) {
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			function f(a, b) -> t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19 {
				function g(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19) -> w, v {
				}
			}
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
		}
	})");
	BOOST_CHECK_EQUAL(out, "h: 9 f: 3 ");
}


BOOST_AUTO_TEST_CASE(also_in_outer_block_used_arguments)
{
	std::string out = check(R"({
			let x := 0
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
			function g(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19) -> w, v {
				w := v
				sstore(s1, s2)
			}
	})");
	BOOST_CHECK_EQUAL(out, "g: 5 : 9 ");
}

BOOST_AUTO_TEST_CASE(also_in_outer_block_unused_arguments)
{
	std::string out = check(R"({
			let x := 0
			let r1 := 0
			let r2 := 0
			let r3 := 0
			let r4 := 0
			let r5 := 0
			let r6 := 0
			let r7 := 0
			let r8 := 0
			let r9 := 0
			let r10 := 0
			let r11 := 0
			let r12 := 0
			let r13 := 0
			let r14 := 0
			let r15 := 0
			let r16 := 0
			let r17 := 0
			let r18 := 0
			x := add(add(add(add(add(add(add(add(add(add(add(add(x, r12), r11), r10), r9), r8), r7), r6), r5), r4), r3), r2), r1)
			function g(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19) -> w, v {
			}
	})");
	BOOST_CHECK_EQUAL(out, ": 9 ");
}

BOOST_AUTO_TEST_SUITE_END()

}
