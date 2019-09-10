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
 * Unit tests for the code metrics.
 */

#include <test/Options.h>

#include <test/libyul/Common.h>

#include <libyul/optimiser/Metrics.h>
#include <libyul/AsmData.h>


using namespace std;
using namespace langutil;

namespace yul
{
namespace test
{

namespace
{

size_t codeSize(string const& _source)
{
	shared_ptr<Block> ast = parse(_source, false).first;
	BOOST_REQUIRE(ast);
	return CodeSize::codeSize(*ast);
}

}

BOOST_AUTO_TEST_SUITE(YulCodeSize)

BOOST_AUTO_TEST_CASE(empty_code)
{
	BOOST_CHECK_EQUAL(codeSize("{}"), 0);
}

BOOST_AUTO_TEST_CASE(nested_blocks)
{
	BOOST_CHECK_EQUAL(codeSize("{ {} {} {{ }} }"), 0);
}

BOOST_AUTO_TEST_CASE(instruction)
{
	BOOST_CHECK_EQUAL(codeSize("{ pop(calldatasize()) }"), 2);
}

BOOST_AUTO_TEST_CASE(variables_are_free)
{
	BOOST_CHECK_EQUAL(codeSize("{ let x let y let a, b, c }"), 0);
}

BOOST_AUTO_TEST_CASE(constants_cost_one)
{
	BOOST_CHECK_EQUAL(codeSize("{ let x := 3 }"), 1);
}

BOOST_AUTO_TEST_CASE(functions_are_skipped)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) -> r { r := mload(x) } }"), 0);
}

BOOST_AUTO_TEST_CASE(function_with_arguments)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) { sstore(x, 2) } f(2) }"), 2);
}

BOOST_AUTO_TEST_CASE(function_with_variables_as_arguments)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) { sstore(x, 2) } let y f(y) }"), 1);
}

BOOST_AUTO_TEST_CASE(function_with_variables_and_constants_as_arguments)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ function f(x, r) -> z { sstore(x, r) z := r } let y let t := f(y, 2) }"
	), 2);
}

BOOST_AUTO_TEST_CASE(assignment)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a a := 3 }"), 1);
}

BOOST_AUTO_TEST_CASE(assignments_between_vars_are_free)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a let b := a a := b }"), 0);
}

BOOST_AUTO_TEST_CASE(assignment_complex)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a let x := mload(a) a := sload(x) }"), 2);
}

BOOST_AUTO_TEST_CASE(empty_for_loop)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} {} }"
	), 4);
}

BOOST_AUTO_TEST_CASE(break_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} { break } }"
	), 6);
}

BOOST_AUTO_TEST_CASE(continue_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} { continue } }"
	), 6);
}

BOOST_AUTO_TEST_CASE(regular_for_loop)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for { let x := 0 } lt(x, 10) { x := add(x, 1) } { mstore(x, 1) } }"
	), 10);
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ if 1 {} }"
	), 3);
}

BOOST_AUTO_TEST_CASE(switch_statement_tiny)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() default {} }"
	), 4);
}

BOOST_AUTO_TEST_CASE(switch_statement_small)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() case 0 {} default {} }"
	), 6);
}

BOOST_AUTO_TEST_CASE(switch_statement_medium)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() case 0 {} case 1 {} case 2 {} }"
	), 8);
}

BOOST_AUTO_TEST_CASE(switch_statement_large)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() case 0 {} case 1 {} case 2 {} default {} }"
	), 10);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
