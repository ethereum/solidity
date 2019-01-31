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

#include <test/Options.h>

#include <test/libyul/Common.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/CompilabilityChecker.h>


using namespace std;

namespace yul
{
namespace test
{

namespace
{
string check(string const& _input)
{
	shared_ptr<Block> ast = yul::test::parse(_input, false).first;
	BOOST_REQUIRE(ast);
	map<YulString, int> functions = CompilabilityChecker::run(EVMDialect::strictAssemblyForEVM(), *ast);
	string out;
	for (auto const& function: functions)
		out += function.first.str() + ": " + to_string(function.second) + " ";
	return out;
}
}

BOOST_AUTO_TEST_SUITE(CompilabilityChecker)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	string out = check("{}");
	BOOST_CHECK_EQUAL(out, "");
}

BOOST_AUTO_TEST_CASE(simple_function)
{
	string out = check("{ function f(a, b) -> x, y { x := a y := b } }");
	BOOST_CHECK_EQUAL(out, "");
}

BOOST_AUTO_TEST_CASE(many_variables_few_uses)
{
	string out = check(R"({
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
	string out = check(R"({
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

BOOST_AUTO_TEST_CASE(many_return_variables)
{
	string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 5 ");
}

BOOST_AUTO_TEST_CASE(multiple_functions)
{
	string out = check(R"({
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
	BOOST_CHECK_EQUAL(out, "h: 9 g: 5 f: 5 ");
}

BOOST_AUTO_TEST_CASE(nested)
{
	string out = check(R"({
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
	BOOST_CHECK_EQUAL(out, "h: 9 ");
}

BOOST_AUTO_TEST_CASE(also_in_outer_block)
{
	string out = check(R"({
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
}
