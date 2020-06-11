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

#include <test/Common.h>

#include <test/libyul/Common.h>

#include <libyul/optimiser/Metrics.h>
#include <libyul/AsmData.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;

namespace solidity::yul::test
{

namespace
{

size_t codeSize(string const& _source, CodeWeights const _weights = {})
{
	shared_ptr<Block> ast = parse(_source, false).first;
	BOOST_REQUIRE(ast);
	return CodeSize::codeSize(*ast, _weights);
}

}

class CustomWeightFixture
{
protected:
	CodeWeights m_weights{
		/* expressionStatementCost = */ 1,
		/* assignmentCost = */ 2,
		/* variableDeclarationCost = */ 3,
		/* functionDefinitionCost = */ 4,
		/* ifCost = */ 5,
		/* switchCost = */ 6,
		/* caseCost = */ 7,
		/* forLoopCost = */ 8,
		/* breakCost = */ 9,
		/* continueCost = */ 10,
		/* leaveCost = */ 11,
		/* blockCost = */ 12,

		/* functionCallCost = */ 13,
		/* identifierCost = */ 14,
		/* literalCost = */ 15,
	};
};

BOOST_AUTO_TEST_SUITE(YulCodeSize)

BOOST_AUTO_TEST_CASE(empty_code)
{
	BOOST_CHECK_EQUAL(codeSize("{}"), 0);
}

BOOST_FIXTURE_TEST_CASE(empty_code_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(codeSize("{}", m_weights), 0);
}

BOOST_AUTO_TEST_CASE(nested_blocks)
{
	BOOST_CHECK_EQUAL(codeSize("{ {} {} {{ }} }"), 0);
}

BOOST_FIXTURE_TEST_CASE(nested_blocks_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(codeSize("{ {} {} {{ }} }", m_weights), 4 * m_weights.blockCost);
}

BOOST_AUTO_TEST_CASE(instruction)
{
	BOOST_CHECK_EQUAL(codeSize("{ pop(calldatasize()) }"), 2);
}

BOOST_FIXTURE_TEST_CASE(instruction_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ pop(calldatasize()) }", m_weights),
		2 * m_weights.functionCallCost +
		1 * m_weights.expressionStatementCost
	);
}

BOOST_AUTO_TEST_CASE(variables_are_free)
{
	BOOST_CHECK_EQUAL(codeSize("{ let x let y let a, b, c }"), 0);
}

BOOST_FIXTURE_TEST_CASE(variables_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ let x let y let a, b, c }", m_weights),
		3 * m_weights.variableDeclarationCost
	);
}

BOOST_AUTO_TEST_CASE(constants_cost_one)
{
	BOOST_CHECK_EQUAL(codeSize("{ let x := 3 }"), 1);
}

BOOST_FIXTURE_TEST_CASE(constants_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ let x := 3 }", m_weights),
		1 * m_weights.variableDeclarationCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(functions_are_skipped)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) -> r { r := mload(x) } }"), 0);
}

BOOST_FIXTURE_TEST_CASE(functions_are_skipped_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) -> r { r := mload(x) } }", m_weights), 0);
}

BOOST_AUTO_TEST_CASE(function_with_arguments)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) { sstore(x, 2) } f(2) }"), 2);
}

BOOST_FIXTURE_TEST_CASE(function_with_arguments_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ function f(x) { sstore(x, 2) } f(2) }", m_weights),
		1 * m_weights.expressionStatementCost +
		1 * m_weights.functionCallCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(function_with_variables_as_arguments)
{
	BOOST_CHECK_EQUAL(codeSize("{ function f(x) { sstore(x, 2) } let y f(y) }"), 1);
}

BOOST_FIXTURE_TEST_CASE(function_with_variables_as_arguments_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ function f(x) { sstore(x, 2) } let y f(y) }", m_weights),
		1 * m_weights.variableDeclarationCost +
		1 * m_weights.expressionStatementCost +
		1 * m_weights.functionCallCost +
		1 * m_weights.identifierCost
	);
}

BOOST_AUTO_TEST_CASE(function_with_variables_and_constants_as_arguments)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ function f(x, r) -> z { sstore(x, r) z := r } let y let t := f(y, 2) }"
	), 2);
}

BOOST_FIXTURE_TEST_CASE(
	function_with_variables_and_constants_as_arguments_custom_weights,
	CustomWeightFixture
)
{
	BOOST_CHECK_EQUAL(
		codeSize(
			"{ function f(x, r) -> z { sstore(x, r) z := r } let y let t := f(y, 2) }",
			m_weights
		),
		2 * m_weights.variableDeclarationCost +
		1 * m_weights.functionCallCost +
		1 * m_weights.identifierCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(assignment)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a a := 3 }"), 1);
}

BOOST_FIXTURE_TEST_CASE(assignment_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ let a a := 3 }", m_weights),
		1 * m_weights.variableDeclarationCost +
		1 * m_weights.assignmentCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(assignments_between_vars_are_free)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a let b := a a := b }"), 0);
}

BOOST_FIXTURE_TEST_CASE(assignments_between_vars_are_free_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ let a let b := a a := b }", m_weights),
		2 * m_weights.variableDeclarationCost +
		1 * m_weights.assignmentCost +
		2 * m_weights.identifierCost
	);
}

BOOST_AUTO_TEST_CASE(assignment_complex)
{
	BOOST_CHECK_EQUAL(codeSize("{ let a let x := mload(a) a := sload(x) }"), 2);
}

BOOST_FIXTURE_TEST_CASE(assignment_complex_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ let a let x := mload(a) a := sload(x) }", m_weights),
		2 * m_weights.variableDeclarationCost +
		1 * m_weights.assignmentCost +
		2 * m_weights.identifierCost +
		2 * m_weights.functionCallCost
	);
}

BOOST_AUTO_TEST_CASE(empty_for_loop)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} {} }"
	), 4);
}

BOOST_FIXTURE_TEST_CASE(empty_for_loop_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ for {} 1 {} {} }", m_weights),
		1 * m_weights.forLoopCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(break_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} { break } }"
	), 6);
}

BOOST_FIXTURE_TEST_CASE(break_statement_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ for {} 1 {} { break } }", m_weights),
		1 * m_weights.forLoopCost +
		1 * m_weights.literalCost +
		1 * m_weights.breakCost
	);
}

BOOST_AUTO_TEST_CASE(continue_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for {} 1 {} { continue } }"
	), 6);
}

BOOST_FIXTURE_TEST_CASE(continue_statement_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ for {} 1 {} { continue } }", m_weights),
		1 * m_weights.forLoopCost +
		1 * m_weights.literalCost +
		1 * m_weights.continueCost
	);
}

BOOST_AUTO_TEST_CASE(regular_for_loop)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ for { let x := 0 } lt(x, 10) { x := add(x, 1) } { mstore(x, 1) } }"
	), 10);
}

BOOST_FIXTURE_TEST_CASE(regular_for_loop_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ for { let x := 0 } lt(x, 10) { x := add(x, 1) } { mstore(x, 1) } }", m_weights),
		1 * m_weights.forLoopCost +
		1 * m_weights.variableDeclarationCost +
		1 * m_weights.assignmentCost +
		3 * m_weights.functionCallCost +
		4 * m_weights.literalCost +
		3 * m_weights.identifierCost +
		1 * m_weights.expressionStatementCost
	);
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ if 1 {} }"
	), 3);
}

BOOST_FIXTURE_TEST_CASE(if_statement_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ if 1 {} }", m_weights),
		1 * m_weights.ifCost +
		1 * m_weights.literalCost
	);
}

BOOST_AUTO_TEST_CASE(switch_statement_tiny)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() case 0 {} }"
	), 4);
}

BOOST_AUTO_TEST_CASE(switch_statement_small)
{
	BOOST_CHECK_EQUAL(codeSize(
		"{ switch calldatasize() case 0 {} default {} }"
	), 6);
}

BOOST_FIXTURE_TEST_CASE(switch_statement_small_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ switch calldatasize() case 0 {} default {} }", m_weights),
		1 * m_weights.functionCallCost +
		1 * m_weights.switchCost +
		2 * m_weights.caseCost
	);
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

BOOST_FIXTURE_TEST_CASE(switch_statement_large_custom_weights, CustomWeightFixture)
{
	BOOST_CHECK_EQUAL(
		codeSize("{ switch calldatasize() case 0 {} case 1 {} case 2 {} default {} }", m_weights),
		1 * m_weights.functionCallCost +
		1 * m_weights.switchCost +
		4 * m_weights.caseCost
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
