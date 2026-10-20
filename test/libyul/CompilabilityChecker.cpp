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

#include <test/libsolidity/util/SoltestErrors.h>

#include <test/libyul/Common.h>

#include <libyul/CompilabilityChecker.h>
#include <libyul/YulStack.h>

#include <libsolutil/CommonData.h>

#include <boost/test/unit_test.hpp>

using namespace solidity::langutil;
using namespace solidity::test;


namespace solidity::yul::test
{

namespace
{
std::string check(std::string const& _input)
{
	YulStack yulStack = parseYul(_input);
	solUnimplementedAssert(yulStack.parserResult()->subObjects.empty(), "Tests with subobjects not supported.");
	soltestAssert(!yulStack.hasErrorsWarningsOrInfos());

	auto functions = CompilabilityChecker(*yulStack.parserResult(), true).stackDeficit;
	std::string out;
	for (auto const& function: functions)
		out += function.first.str() + ": " + std::to_string(function.second) + " ";
	return out;
}

/// @returns the source of a function with @a _numVariables local variables of which the
/// first @a _numUses are used in a computation afterwards.
std::string manyVariablesSource(size_t _numVariables, size_t _numUses)
{
	soltestAssert(_numUses <= _numVariables);

	std::vector<std::string> variables;
	for (size_t i = 1; i <= _numVariables; ++i)
		variables.emplace_back("r" + std::to_string(i));

	std::string expression = "x";
	for (size_t i = _numUses; i >= 1; --i)
		expression = "add(" + expression + ", r" + std::to_string(i) + ")";

	return
		"{ function f(a, b) -> x, y {\n"
		"let " + util::joinHumanReadable(variables, ", ") + "\n"
		"x := " + expression + "\n"
		"} }\n";
}

/// @returns the source of a function with @a _numReturnVariables return variables whose
/// arguments are used in the body if @a _useArguments is set.
std::string manyReturnVariablesSource(size_t _numReturnVariables, bool _useArguments)
{
	std::vector<std::string> returnVariables;
	for (size_t i = 1; i <= _numReturnVariables; ++i)
		returnVariables.emplace_back("r" + std::to_string(i));

	return
		"{ function f(a, b) -> " + util::joinHumanReadable(returnVariables, ", ") + " {\n" +
		(_useArguments ? "r1 := 0\nsstore(a, b)\n" : "") +
		"} }\n";
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

BOOST_AUTO_TEST_CASE(many_variables_few_uses, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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

BOOST_AUTO_TEST_CASE(many_variables_many_uses, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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

BOOST_AUTO_TEST_CASE(many_return_variables_unused_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 3 ");
}

BOOST_AUTO_TEST_CASE(many_return_variables_used_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
{
	std::string out = check(R"({
		function f(a, b) -> r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19 {
			r1 := 0
			sstore(a, b)
		}
	})");
	BOOST_CHECK_EQUAL(out, "f: 5 ");
}

// Starting from "amsterdam", DUPN and SWAPN (EIP-8024) extend the reachable stack depth to 235,
// so the deficits only appear with correspondingly more values on the stack.

BOOST_AUTO_TEST_CASE(many_variables_few_uses_dupn_swapn, *boost::unit_test::precondition(minEVMVersionCheck(EVMVersion::amsterdam())))
{
	BOOST_CHECK_EQUAL(check(manyVariablesSource(18, 9)), "");
	BOOST_CHECK_EQUAL(check(manyVariablesSource(237, 228)), "f: 223 ");
}

BOOST_AUTO_TEST_CASE(many_variables_many_uses_dupn_swapn, *boost::unit_test::precondition(minEVMVersionCheck(EVMVersion::amsterdam())))
{
	BOOST_CHECK_EQUAL(check(manyVariablesSource(18, 12)), "");
	BOOST_CHECK_EQUAL(check(manyVariablesSource(237, 231)), "f: 229 ");
}

BOOST_AUTO_TEST_CASE(many_return_variables_unused_arguments_dupn_swapn, *boost::unit_test::precondition(minEVMVersionCheck(EVMVersion::amsterdam())))
{
	BOOST_CHECK_EQUAL(check(manyReturnVariablesSource(19, false)), "");
	BOOST_CHECK_EQUAL(check(manyReturnVariablesSource(238, false)), "f: 3 ");
}

BOOST_AUTO_TEST_CASE(many_return_variables_used_arguments_dupn_swapn, *boost::unit_test::precondition(minEVMVersionCheck(EVMVersion::amsterdam())))
{
	BOOST_CHECK_EQUAL(check(manyReturnVariablesSource(19, true)), "");
	BOOST_CHECK_EQUAL(check(manyReturnVariablesSource(238, true)), "f: 5 ");
}

BOOST_AUTO_TEST_CASE(multiple_functions_used_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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

BOOST_AUTO_TEST_CASE(multiple_functions_unused_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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

BOOST_AUTO_TEST_CASE(nested_used_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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


BOOST_AUTO_TEST_CASE(nested_unused_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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


BOOST_AUTO_TEST_CASE(also_in_outer_block_used_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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

BOOST_AUTO_TEST_CASE(also_in_outer_block_unused_arguments, *boost::unit_test::precondition(maxEVMVersionCheck(EVMVersion::osaka())))
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
