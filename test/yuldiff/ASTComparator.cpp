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
// SPDX-License-Identifier: GPL-3.0

#include <tools/yuldiff/ASTComparator.h>

#include <libyul/AST.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>
#include <libyul/ObjectParser.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/Scanner.h>

#include <boost/test/unit_test.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::tools::cmpast;

namespace
{

Dialect const& dialect()
{
	static auto const& d = EVMDialect::strictAssemblyForEVMObjects(EVMVersion::current(), std::nullopt);
	return d;
}

std::shared_ptr<Object> parse(std::string const& _source)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	CharStream stream(_source, "test");
	std::shared_ptr<Scanner> const scanner = std::make_shared<Scanner>(stream);
	auto object = ObjectParser(errorReporter, dialect()).parse(scanner, false);
	BOOST_REQUIRE_MESSAGE(object && !errorReporter.hasErrors(), "Failed to parse: " + _source);
	AsmAnalyzer::analyzeStrictAssertCorrect(*object);
	return object;
}

bool equivalent(std::string const& _a, std::string const& _b)
{
	std::shared_ptr<Object> const objA = parse(_a);
	std::shared_ptr<Object> const objB = parse(_b);
	ASTComparator cmp(dialect());
	return static_cast<bool>(cmp.compareObjects(*objA, *objB));
}

std::string mismatchReason(std::string const& _a, std::string const& _b)
{
	std::shared_ptr<Object> const objA = parse(_a);
	std::shared_ptr<Object> const objB = parse(_b);
	ASTComparator cmp(dialect());
	auto const result = cmp.compareObjects(*objA, *objB);
	return result.mismatch().reason;
}

}

BOOST_AUTO_TEST_SUITE(YulASTComparator, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(ASTComparatorTest)

BOOST_AUTO_TEST_SUITE(equivalence)
BOOST_AUTO_TEST_CASE(empty_objects_are_equivalent)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { } }",
		"object \"B\" { code { } }"
	));
}

BOOST_AUTO_TEST_CASE(identical_code_is_equivalent)
{
	std::string const src = "object \"X\" { code { let x := 1 let y := add(x, 2) } }";
	BOOST_TEST(equivalent(src, src));
}

BOOST_AUTO_TEST_CASE(renamed_variables_are_equivalent)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { let x := 1 let y := add(x, 2) } }",
		"object \"A\" { code { let a := 1 let b := add(a, 2) } }"
	));
}

BOOST_AUTO_TEST_CASE(renamed_function_names_are_equivalent)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { function foo() -> r { r := 1 } let x := foo() } }",
		"object \"A\" { code { function bar() -> s { s := 1 } let y := bar() } }"
	));
}

BOOST_AUTO_TEST_CASE(renamed_parameters_are_equivalent)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { function f(a, b) -> r { r := add(a, b) } } }",
		"object \"A\" { code { function g(x, y) -> z { z := add(x, y) } } }"
	));
}

BOOST_AUTO_TEST_CASE(renamed_for_loop_variables_are_equivalent)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { for { let i := 0 } lt(i, 10) { i := add(i, 1) } { } } }",
		"object \"A\" { code { for { let j := 0 } lt(j, 10) { j := add(j, 1) } { } } }"
	));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(inconsistent_renaming)
BOOST_AUTO_TEST_CASE(inconsistent_variable_renaming_not_equivalent)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { let x := 1 let y := 2 let z := add(x, y) } }",
		"object \"A\" { code { let a := 1 let b := 2 let c := add(b, a) } }"
	));
}

BOOST_AUTO_TEST_CASE(inconsistent_function_renaming_not_equivalent)
{
	// Two different functions map to the same name
	BOOST_TEST(!equivalent(
		"object \"A\" { code { function f() { } function g() { } f() g() } }",
		"object \"A\" { code { function h() { } function k() { } h() h() } }"
	));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(mismatch)
BOOST_AUTO_TEST_CASE(different_statement_types_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { let x := 1 } }",
		"object \"A\" { code { pop(1) } }"
	));
}

BOOST_AUTO_TEST_CASE(different_statement_count_in_block_mismatch)
{
	auto reason = mismatchReason(
		"object \"A\" { code { let x := 1 } }",
		"object \"A\" { code { let x := 1 let y := 2 } }"
	);
	BOOST_TEST(reason.find("block statement count differs") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(assignment_value_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { let x := 0 x := 42 } }",
		"object \"A\" { code { let x := 0 x := 99 } }"
	));
}

BOOST_AUTO_TEST_CASE(function_parameter_count_mismatch)
{
	auto reason = mismatchReason(
		"object \"A\" { code { function f(a) { } } }",
		"object \"A\" { code { function f(a, b) { } } }"
	);
	BOOST_TEST(reason.find("parameter count differs") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(function_return_variable_count_mismatch)
{
	auto reason = mismatchReason(
		"object \"A\" { code { function f() -> a { a := 1 } } }",
		"object \"A\" { code { function f() -> a, b { a := 1 b := 2 } } }"
	);
	BOOST_TEST(reason.find("return variable count differs") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(function_body_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { function f() -> r { r := 1 } } }",
		"object \"A\" { code { function f() -> r { r := 2 } } }"
	));
}

BOOST_AUTO_TEST_CASE(if_condition_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { if 1 { } } }",
		"object \"A\" { code { if 0 { } } }"
	));
}

BOOST_AUTO_TEST_CASE(if_body_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { if 1 { pop(1) } } }",
		"object \"A\" { code { if 1 { pop(2) } } }"
	));
}

BOOST_AUTO_TEST_CASE(switch_case_count_mismatch)
{
	auto reason = mismatchReason(
		"object \"A\" { code { switch 1 case 0 { } default { } } }",
		"object \"A\" { code { switch 1 case 0 { } case 1 { } default { } } }"
	);
	BOOST_TEST(reason.find("case count differs") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(switch_case_value_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { switch 1 case 0 { } default { } } }",
		"object \"A\" { code { switch 1 case 1 { } default { } } }"
	));
}

BOOST_AUTO_TEST_CASE(switch_case_body_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { switch 1 case 0 { pop(1) } default { } } }",
		"object \"A\" { code { switch 1 case 0 { pop(2) } default { } } }"
	));
}

BOOST_AUTO_TEST_CASE(switch_default_vs_nondefault_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { switch 1 case 0 { } default { } } }",
		"object \"A\" { code { switch 1 case 0 { } case 1 { } } }"
	));
}

BOOST_AUTO_TEST_CASE(switch_expression_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { switch 1 case 0 { } default { } } }",
		"object \"A\" { code { switch 2 case 0 { } default { } } }"
	));
}

BOOST_AUTO_TEST_CASE(for_loop_condition_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { for { } lt(0, 10) { } { } } }",
		"object \"A\" { code { for { } lt(0, 20) { } { } } }"
	));
}

BOOST_AUTO_TEST_CASE(for_loop_pre_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { for { let i := 0 } lt(i, 10) { i := add(i, 1) } { } } }",
		"object \"A\" { code { for { let i := 1 } lt(i, 10) { i := add(i, 1) } { } } }"
	));
}

BOOST_AUTO_TEST_CASE(for_loop_post_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { for { let i := 0 } lt(i, 10) { i := add(i, 1) } { } } }",
		"object \"A\" { code { for { let i := 0 } lt(i, 10) { i := add(i, 2) } { } } }"
	));
}

BOOST_AUTO_TEST_CASE(for_loop_body_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { for { } 1 { } { pop(1) } } }",
		"object \"A\" { code { for { } 1 { } { pop(2) } } }"
	));
}

BOOST_AUTO_TEST_CASE(literal_value_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { let x := 42 } }",
		"object \"A\" { code { let x := 43 } }"
	));
}

BOOST_AUTO_TEST_CASE(function_call_argument_count_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { function f(a) { } function g(a, b) { } f(1) } }",
		"object \"A\" { code { function f(a) { } function g(a, b) { } g(1, 2) } }"
	));
}

BOOST_AUTO_TEST_CASE(function_call_argument_value_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { pop(1) } }",
		"object \"A\" { code { pop(2) } }"
	));
}

BOOST_AUTO_TEST_CASE(builtin_function_mismatch)
{
	BOOST_TEST(!equivalent(
		"object \"A\" { code { pop(add(1, 2)) } }",
		"object \"A\" { code { pop(sub(1, 2)) } }"
	));
}

BOOST_AUTO_TEST_CASE(sub_object_count_mismatch)
{
	auto reason = mismatchReason(
		"object \"A\" { code { } object \"B\" { code { } } }",
		"object \"A\" { code { } }"
	);
	BOOST_TEST(reason.find("different number of sub-objects") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(scopes)
BOOST_AUTO_TEST_CASE(nested_block_scopes_allow_name_reuse)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { { let x := 1 } { let x := 2 x := add(x, 5) } } }",
		"object \"A\" { code { { let a := 1 } { let b := 2 b := add(b, 5) } } }"
	));
}

BOOST_AUTO_TEST_CASE(function_scoping_independent_of_outer)
{
	BOOST_TEST(equivalent(
		"object \"A\" { code { let x := 0 function f() -> r { let w := 1 r := w } } }",
		"object \"A\" { code { let a := 0 function g() -> s { let b := 1 s := b } } }"
	));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
