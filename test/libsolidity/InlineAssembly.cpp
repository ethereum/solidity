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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Unit tests for inline assembly.
 */

#include <test/Common.h>

#include <test/libsolidity/ErrorCheck.h>

#include <libsolidity/ast/AST.h>

#include <libyul/AssemblyStack.h>

#include <liblangutil/Scanner.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libevmasm/Assembly.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>
#include <optional>
#include <string>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace solidity::frontend::test
{

namespace
{

std::optional<Error> parseAndReturnFirstError(
	string const& _source,
	bool _assemble = false,
	bool _allowWarnings = true,
	AssemblyStack::Language _language = AssemblyStack::Language::Assembly,
	AssemblyStack::Machine _machine = AssemblyStack::Machine::EVM
)
{
	AssemblyStack stack(solidity::test::CommonOptions::get().evmVersion(), _language, solidity::frontend::OptimiserSettings::none());
	bool success = false;
	try
	{
		success = stack.parseAndAnalyze("", _source);
		if (success && _assemble)
			stack.assemble(_machine);
	}
	catch (FatalError const&)
	{
		BOOST_FAIL("Fatal error leaked.");
		success = false;
	}
	shared_ptr<Error const> error;
	for (auto const& e: stack.errors())
	{
		if (_allowWarnings && e->type() == Error::Type::Warning)
			continue;
		if (error)
		{
			string errors;
			for (auto const& err: stack.errors())
				errors += SourceReferenceFormatter::formatErrorInformation(*err);
			BOOST_FAIL("Found more than one error:\n" + errors);
		}
		error = e;
	}
	if (!success)
		BOOST_REQUIRE(error);
	if (error)
		return *error;
	return {};
}

bool successParse(
	string const& _source,
	bool _assemble = false,
	bool _allowWarnings = true,
	AssemblyStack::Language _language = AssemblyStack::Language::Assembly,
	AssemblyStack::Machine _machine = AssemblyStack::Machine::EVM
)
{
	return !parseAndReturnFirstError(_source, _assemble, _allowWarnings, _language, _machine);
}

bool successAssemble(string const& _source, bool _allowWarnings = true, AssemblyStack::Language _language = AssemblyStack::Language::Assembly)
{
	return
		successParse(_source, true, _allowWarnings, _language, AssemblyStack::Machine::EVM) &&
		successParse(_source, true, _allowWarnings, _language, AssemblyStack::Machine::EVM15);
}

Error expectError(
	std::string const& _source,
	bool _assemble,
	bool _allowWarnings = false,
	AssemblyStack::Language _language = AssemblyStack::Language::Assembly
)
{

	auto error = parseAndReturnFirstError(_source, _assemble, _allowWarnings, _language);
	BOOST_REQUIRE(error);
	return *error;
}

void parsePrintCompare(string const& _source, bool _canWarn = false)
{
	AssemblyStack stack(solidity::test::CommonOptions::get().evmVersion(), AssemblyStack::Language::Assembly, OptimiserSettings::none());
	BOOST_REQUIRE(stack.parseAndAnalyze("", _source));
	if (_canWarn)
		BOOST_REQUIRE(Error::containsOnlyWarnings(stack.errors()));
	else
		BOOST_REQUIRE(stack.errors().empty());
	string expectation = "object \"object\" {\n    code " + boost::replace_all_copy(_source, "\n", "\n    ") + "\n}\n";
	BOOST_CHECK_EQUAL(stack.print(), expectation);
}

}

#define CHECK_ERROR_LANG(text, assemble, typ, substring, warnings, language) \
do \
{ \
	Error err = expectError((text), (assemble), warnings, (language)); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(searchErrorMessage(err, (substring))); \
} while(0)

#define CHECK_ERROR(text, assemble, typ, substring, warnings) \
CHECK_ERROR_LANG(text, assemble, typ, substring, warnings, AssemblyStack::Language::Assembly)

#define CHECK_PARSE_ERROR(text, type, substring) \
CHECK_ERROR(text, false, type, substring, false)

#define CHECK_PARSE_WARNING(text, type, substring) \
CHECK_ERROR(text, false, type, substring, false)

#define CHECK_ASSEMBLE_ERROR(text, type, substring) \
CHECK_ERROR(text, true, type, substring, false)

#define CHECK_STRICT_ERROR(text, type, substring) \
CHECK_ERROR_LANG(text, false, type, substring, false, AssemblyStack::Language::StrictAssembly)

#define CHECK_STRICT_WARNING(text, type, substring) \
CHECK_ERROR(text, false, type, substring, false, AssemblyStack::Language::StrictAssembly)

#define SUCCESS_STRICT(text) \
do { successParse((text), false, false, AssemblyStack::Language::StrictAssembly); } while (false)


BOOST_AUTO_TEST_SUITE(SolidityInlineAssembly)


BOOST_AUTO_TEST_SUITE(Parsing)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	BOOST_CHECK(successParse("{ }"));
}

BOOST_AUTO_TEST_CASE(surplus_input)
{
	CHECK_PARSE_ERROR("{ } { }", ParserError, "Expected end of source but got '{'");
}

BOOST_AUTO_TEST_CASE(simple_instructions)
{
	BOOST_CHECK(successParse("{ let y := mul(0x10, mul(0x20, mload(0x40)))}"));
}

BOOST_AUTO_TEST_CASE(selfdestruct)
{
	BOOST_CHECK(successParse("{ selfdestruct(0x02) }"));
}

BOOST_AUTO_TEST_CASE(keywords)
{
	BOOST_CHECK(successParse("{ return (byte(1, 2), 2) pop(address()) }"));
}

BOOST_AUTO_TEST_CASE(constants)
{
	BOOST_CHECK(successParse("{ pop(mul(7, 8)) }"));
}

BOOST_AUTO_TEST_CASE(vardecl)
{
	BOOST_CHECK(successParse("{ let x := 7 }"));
}

BOOST_AUTO_TEST_CASE(vardecl_name_clashes)
{
	CHECK_PARSE_ERROR("{ let x := 1 let x := 2 }", DeclarationError, "Variable name x already taken in this scope.");
}

BOOST_AUTO_TEST_CASE(vardecl_multi)
{
	BOOST_CHECK(successParse("{ function f() -> x, y {} let x, y := f() }"));
}

BOOST_AUTO_TEST_CASE(vardecl_multi_conflict)
{
	CHECK_PARSE_ERROR("{ function f() -> x, y {} let x, x := f() }", DeclarationError, "Variable name x already taken in this scope.");
}

BOOST_AUTO_TEST_CASE(vardecl_bool)
{
	successParse("{ let x := true }");
	successParse("{ let x := false }");
}

BOOST_AUTO_TEST_CASE(vardecl_empty)
{
	BOOST_CHECK(successParse("{ let x }"));
}

BOOST_AUTO_TEST_CASE(functional)
{
	BOOST_CHECK(successParse("{ let x := 2 x := add(add(7, mul(6, x)), mul(7, 8)) }"));
}

BOOST_AUTO_TEST_CASE(functional_partial)
{
	CHECK_PARSE_ERROR("{ let x := byte }", ParserError, "Expected '(' but got '}'");
}

BOOST_AUTO_TEST_CASE(functional_partial_success)
{
	BOOST_CHECK(successParse("{ let x := byte(1, 2) }"));
}

BOOST_AUTO_TEST_CASE(functional_assignment)
{
	BOOST_CHECK(successParse("{ let x := 2 x := 7 }"));
}

BOOST_AUTO_TEST_CASE(functional_assignment_complex)
{
	BOOST_CHECK(successParse("{ let x := 2 x := add(add(7, mul(6, x)), mul(7, 8)) }"));
}

BOOST_AUTO_TEST_CASE(vardecl_complex)
{
	BOOST_CHECK(successParse("{ let y := 2 let x := add(add(7, mul(6, y)), mul(7, 8)) }"));
}

BOOST_AUTO_TEST_CASE(variable_use_before_decl)
{
	CHECK_PARSE_ERROR("{ x := 2 let x := 3 }", DeclarationError, "Variable x used before it was declared.");
	CHECK_PARSE_ERROR("{ let x := mul(2, x) }", DeclarationError, "Variable x used before it was declared.");
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	BOOST_CHECK(successParse("{ if 42 {} }"));
	BOOST_CHECK(successParse("{ if 42 { let x := 3 } }"));
	BOOST_CHECK(successParse("{ function f() -> x {} if f() { pop(f()) } }"));
}

BOOST_AUTO_TEST_CASE(if_statement_scope)
{
	BOOST_CHECK(successParse("{ let x := 2 if 42 { x := 3 } }"));
	CHECK_PARSE_ERROR("{ if 32 { let x := 3 } x := 2 }", DeclarationError, "Variable not found or variable not lvalue.");
}

BOOST_AUTO_TEST_CASE(if_statement_invalid)
{
	CHECK_PARSE_ERROR("{ if mload {} }", ParserError, "Expected '(' but got '{'");
	BOOST_CHECK("{ if calldatasize() {}");
	CHECK_PARSE_ERROR("{ if mstore(1, 1) {} }", TypeError, "Expected expression to evaluate to one value, but got 0 values instead.");
	CHECK_PARSE_ERROR("{ if 32 let x := 3 }", ParserError, "Expected '{' but got reserved keyword 'let'");
}

BOOST_AUTO_TEST_CASE(switch_statement)
{
	BOOST_CHECK(successParse("{ switch 42 default {} }"));
	BOOST_CHECK(successParse("{ switch 42 case 1 {} }"));
	BOOST_CHECK(successParse("{ switch 42 case 1 {} case 2 {} }"));
	BOOST_CHECK(successParse("{ switch 42 case 1 {} default {} }"));
	BOOST_CHECK(successParse("{ switch 42 case 1 {} case 2 {} default {} }"));
	BOOST_CHECK(successParse("{ switch mul(1, 2) case 1 {} case 2 {} default {} }"));
	BOOST_CHECK(successParse("{ function f() -> x {} switch f() case 1 {} case 2 {} default {} }"));
}

BOOST_AUTO_TEST_CASE(switch_no_cases)
{
	CHECK_PARSE_ERROR("{ switch 42 }", ParserError, "Switch statement without any cases.");
}

BOOST_AUTO_TEST_CASE(switch_duplicate_case)
{
	CHECK_PARSE_ERROR("{ switch 42 case 1 {} case 1 {} default {} }", DeclarationError, "Duplicate case defined.");
}

BOOST_AUTO_TEST_CASE(switch_invalid_expression)
{
	CHECK_PARSE_ERROR("{ switch {} default {} }", ParserError, "Literal or identifier expected.");
	CHECK_PARSE_ERROR("{ switch mload default {} }", ParserError, "Expected '(' but got reserved keyword 'default'");
	CHECK_PARSE_ERROR("{ switch mstore(1, 1) default {} }", TypeError, "Expected expression to evaluate to one value, but got 0 values instead.");
}

BOOST_AUTO_TEST_CASE(switch_default_before_case)
{
	CHECK_PARSE_ERROR("{ switch 42 default {} case 1 {} }", ParserError, "Case not allowed after default case.");
}

BOOST_AUTO_TEST_CASE(switch_duplicate_default_case)
{
	CHECK_PARSE_ERROR("{ switch 42 default {} default {} }", ParserError, "Only one default case allowed.");
}

BOOST_AUTO_TEST_CASE(switch_invalid_case)
{
	CHECK_PARSE_ERROR("{ switch 42 case mul(1, 2) {} case 2 {} default {} }", ParserError, "Literal expected.");
}

BOOST_AUTO_TEST_CASE(switch_invalid_body)
{
	CHECK_PARSE_ERROR("{ switch 42 case 1 mul case 2 {} default {} }", ParserError, "Expected '{' but got identifier");
}

BOOST_AUTO_TEST_CASE(for_statement)
{
	BOOST_CHECK(successParse("{ for {} 1 {} {} }"));
	BOOST_CHECK(successParse("{ for { let i := 1 } lt(i, 5) { i := add(i, 1) } {} }"));
}

BOOST_AUTO_TEST_CASE(for_invalid_expression)
{
	CHECK_PARSE_ERROR("{ for {} {} {} {} }", ParserError, "Literal or identifier expected.");
	CHECK_PARSE_ERROR("{ for 1 1 {} {} }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} 1 1 {} }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} 1 {} 1 }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} mload {} {} }", ParserError, "Expected '(' but got '{'");
	CHECK_PARSE_ERROR("{ for {} mstore(1, 1) {} {} }", TypeError, "Expected expression to evaluate to one value, but got 0 values instead.");
}

BOOST_AUTO_TEST_CASE(for_visibility)
{
	BOOST_CHECK(successParse("{ for { let i := 1 } i { pop(i) } { pop(i) } }"));
	CHECK_PARSE_ERROR("{ for {} i { let i := 1 } {} }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for {} 1 { let i := 1 } { pop(i) } }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for {} 1 { pop(i) } { let i := 1 } }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for { pop(i) } 1 { let i := 1 } {} }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for { pop(i) } 1 { } { let i := 1 } }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for {} i {} { let i := 1 } }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for {} 1 { pop(i) } { let i := 1 } }", DeclarationError, "Identifier not found");
	CHECK_PARSE_ERROR("{ for { let x := 1 } 1 { let x := 1 } {} }", DeclarationError, "Variable name x already taken in this scope");
	CHECK_PARSE_ERROR("{ for { let x := 1 } 1 {} { let x := 1 } }", DeclarationError, "Variable name x already taken in this scope");
	CHECK_PARSE_ERROR("{ let x := 1 for { let x := 1 } 1 {} {} }", DeclarationError, "Variable name x already taken in this scope");
	CHECK_PARSE_ERROR("{ let x := 1 for {} 1 { let x := 1 } {} }", DeclarationError, "Variable name x already taken in this scope");
	CHECK_PARSE_ERROR("{ let x := 1 for {} 1 {} { let x := 1 } }", DeclarationError, "Variable name x already taken in this scope");
	// Check that body and post are not sub-scopes of each other.
	BOOST_CHECK(successParse("{ for {} 1 { let x := 1 } { let x := 1 } }"));
}

BOOST_AUTO_TEST_CASE(blocks)
{
	BOOST_CHECK(successParse("{ let x := 7 { let y := 3 } { let z := 2 } }"));
}

BOOST_AUTO_TEST_CASE(number_literals)
{
	BOOST_CHECK(successParse("{ let x := 1 }"));
	CHECK_PARSE_ERROR("{ let x := .1 }", ParserError, "Invalid number literal.");
	CHECK_PARSE_ERROR("{ let x := 1e5 }", ParserError, "Invalid number literal.");
	CHECK_PARSE_ERROR("{ let x := 67.235 }", ParserError, "Invalid number literal.");
	CHECK_STRICT_ERROR("{ let x := 0x1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff }", TypeError, "Number literal too large (> 256 bits)");
}

BOOST_AUTO_TEST_CASE(function_definitions)
{
	BOOST_CHECK(successParse("{ function f() { } function g(a) -> x { } }"));
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	BOOST_CHECK(successParse("{ function f(a, d) { } function g(a, d) -> x, y { } }"));
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	BOOST_CHECK(successParse("{ function f(a) -> b {} function g(a, b, c) {} function x() { g(1, 2, f(mul(2, 3))) x() } }"));
}

BOOST_AUTO_TEST_CASE(opcode_for_functions)
{
	CHECK_PARSE_ERROR("{ function gas() { } }", ParserError, "Cannot use builtin");
}

BOOST_AUTO_TEST_CASE(opcode_for_function_args)
{
	CHECK_PARSE_ERROR("{ function f(gas) { } }", ParserError, "Cannot use builtin");
	CHECK_PARSE_ERROR("{ function f() -> gas { } }", ParserError, "Cannot use builtin");
}

BOOST_AUTO_TEST_CASE(name_clashes)
{
	CHECK_PARSE_ERROR("{ let g := 2 function g() { } }", DeclarationError, "Variable name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(name_clashes_function_subscope)
{
	CHECK_PARSE_ERROR("{ function g() { function g() {} } }", DeclarationError, "Function name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(name_clashes_function_subscope_reverse)
{
	CHECK_PARSE_ERROR("{ { function g() {} } function g() { } }", DeclarationError, "Function name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(name_clashes_function_variable_subscope)
{
	CHECK_PARSE_ERROR("{ function g() { let g := 0 } }", DeclarationError, "Variable name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(name_clashes_function_variable_subscope_reverse)
{
	CHECK_PARSE_ERROR("{ { let g := 0 } function g() { } }", DeclarationError, "Variable name g already taken in this scope");
}
BOOST_AUTO_TEST_CASE(functions_in_parallel_scopes)
{
	BOOST_CHECK(successParse("{ { function g() {} } { function g() {} } }"));
}

BOOST_AUTO_TEST_CASE(variable_access_cross_functions)
{
	CHECK_PARSE_ERROR("{ let x := 2 function g() { pop(x) } }", DeclarationError, "Identifier not found.");
}

BOOST_AUTO_TEST_CASE(invalid_tuple_assignment)
{
	CHECK_PARSE_ERROR("{ let x, y := 1 }", DeclarationError, "Variable count mismatch: 2 variables and 1 values");
}

BOOST_AUTO_TEST_CASE(instruction_too_few_arguments)
{
	CHECK_PARSE_ERROR("{ pop(mul()) }", TypeError, "Function expects 2 arguments but got 0.");
	CHECK_PARSE_ERROR("{ pop(mul(1)) }", TypeError, "Function expects 2 arguments but got 1.");
}

BOOST_AUTO_TEST_CASE(instruction_too_many_arguments)
{
	CHECK_PARSE_ERROR("{ pop(mul(1, 2, 3)) }", TypeError, "Function expects 2 arguments but got 3");
}

BOOST_AUTO_TEST_CASE(recursion_depth)
{
	string input;
	for (size_t i = 0; i < 20000; i++)
		input += "{";
	input += "let x := 0";
	for (size_t i = 0; i < 20000; i++)
		input += "}";

	CHECK_PARSE_ERROR(input, ParserError, "recursion");
}

BOOST_AUTO_TEST_CASE(multiple_assignment)
{
	CHECK_PARSE_ERROR("{ let x function f() -> a, b {} 123, x := f() }", ParserError, "Variable name must precede \",\" in multiple assignment.");
	CHECK_PARSE_ERROR("{ let x function f() -> a, b {} x, 123 := f() }", ParserError, "Variable name must precede \":=\" in assignment.");

	/// NOTE: Travis hiccups if not having a variable
	char const* text = R"(
	{
		function f(a) -> r1, r2 {
			r1 := a
			r2 := 7
		}
		let x := 9
		let y := 2
		x, y := f(x)
	}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Printing)

BOOST_AUTO_TEST_CASE(print_smoke)
{
	parsePrintCompare("{ }");
}

BOOST_AUTO_TEST_CASE(print_instructions)
{
	parsePrintCompare("{ pop(7) }");
}

BOOST_AUTO_TEST_CASE(print_subblock)
{
	parsePrintCompare("{ { pop(7) } }");
}

BOOST_AUTO_TEST_CASE(print_functional)
{
	parsePrintCompare("{ let x := mul(sload(0x12), 7) }");
}

BOOST_AUTO_TEST_CASE(print_assignments)
{
	parsePrintCompare("{\n    let x := mul(2, 3)\n    pop(7)\n    x := add(1, 2)\n}");
}

BOOST_AUTO_TEST_CASE(print_multi_assignments)
{
	parsePrintCompare("{\n    function f() -> x, y\n    { }\n    let x, y := f()\n}");
}

BOOST_AUTO_TEST_CASE(print_string_literals)
{
	parsePrintCompare("{ let x := \"\\n'\\xab\\x95\\\"\" }");
}

BOOST_AUTO_TEST_CASE(print_string_literal_unicode)
{
	string source = "{ let x := \"\\u1bac\" }";
	string parsed = "object \"object\" {\n    code { let x := \"\\xe1\\xae\\xac\" }\n}\n";
	AssemblyStack stack(solidity::test::CommonOptions::get().evmVersion(), AssemblyStack::Language::Assembly, OptimiserSettings::none());
	BOOST_REQUIRE(stack.parseAndAnalyze("", source));
	BOOST_REQUIRE(stack.errors().empty());
	BOOST_CHECK_EQUAL(stack.print(), parsed);

	string parsedInner = "{ let x := \"\\xe1\\xae\\xac\" }";
	parsePrintCompare(parsedInner);
}

BOOST_AUTO_TEST_CASE(print_if)
{
	parsePrintCompare("{ if 2 { pop(mload(0)) } }");
}

BOOST_AUTO_TEST_CASE(print_switch)
{
	parsePrintCompare("{\n    switch 42\n    case 1 { }\n    case 2 { }\n    default { }\n}");
}

BOOST_AUTO_TEST_CASE(print_for)
{
	parsePrintCompare("{\n    let ret := 5\n    for { let i := 1 } lt(i, 15) { i := add(i, 1) }\n    { ret := mul(ret, i) }\n}");
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	parsePrintCompare("{\n    function f(a, d)\n    { mstore(a, d) }\n    function g(a, d) -> x, y\n    { }\n}");
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	string source = R"({
	function y()
	{ }
	function f(a) -> b
	{ }
	function g(a, b, c)
	{ }
	g(1, mul(2, address()), f(mul(2, caller())))
	y()
})";
	boost::replace_all(source, "\t", "    ");
	parsePrintCompare(source);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Analysis)

BOOST_AUTO_TEST_CASE(string_literals)
{
	BOOST_CHECK(successAssemble("{ let x := \"12345678901234567890123456789012\" }"));
}

BOOST_AUTO_TEST_CASE(oversize_string_literals)
{
	CHECK_ASSEMBLE_ERROR("{ let x := \"123456789012345678901234567890123\" }", TypeError, "String literal too long");
}

BOOST_AUTO_TEST_CASE(magic_variables)
{
	CHECK_ASSEMBLE_ERROR("{ pop(this) }", DeclarationError, "Identifier not found");
	CHECK_ASSEMBLE_ERROR("{ pop(ecrecover) }", DeclarationError, "Identifier not found");
	BOOST_CHECK(successAssemble("{ let ecrecover := 1 pop(ecrecover) }"));
}

BOOST_AUTO_TEST_CASE(stack_variables)
{
	BOOST_CHECK(successAssemble("{ let y := 3 { let z := 2 { let x := y } } }"));
}

BOOST_AUTO_TEST_CASE(designated_invalid_instruction)
{
	BOOST_CHECK(successAssemble("{ invalid() }"));
}

BOOST_AUTO_TEST_CASE(inline_assembly_shadowed_instruction_declaration)
{
	CHECK_ASSEMBLE_ERROR("{ let gas := 1 }", ParserError, "Cannot use builtin");
}

BOOST_AUTO_TEST_CASE(revert)
{
	BOOST_CHECK(successAssemble("{ revert(0, 0) }"));
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	BOOST_CHECK(successAssemble("{ function f() {} }"));
	BOOST_CHECK(successAssemble("{ function f() { let y := 2 } }"));
	BOOST_CHECK(successAssemble("{ function f() -> z { let y := 2 } }"));
	BOOST_CHECK(successAssemble("{ function f(a) { let y := 2 } }"));
	BOOST_CHECK(successAssemble("{ function f(a) { let y := a } }"));
	BOOST_CHECK(successAssemble("{ function f() -> x, y, z {} }"));
	BOOST_CHECK(successAssemble("{ function f(x, y, z) {} }"));
	BOOST_CHECK(successAssemble("{ function f(a, b) -> x, y, z { y := a } }"));
	BOOST_CHECK(successAssemble("{ function f() {} f() }"));
	BOOST_CHECK(successAssemble("{ function f() -> x, y { x := 1 y := 2} let a, b := f() }"));
	BOOST_CHECK(successAssemble("{ function f(a, b) -> x, y { x := b y := a } let a, b := f(2, 3) }"));
	BOOST_CHECK(successAssemble("{ function rec(a) { rec(sub(a, 1)) } rec(2) }"));
	BOOST_CHECK(successAssemble("{ let r := 2 function f() -> x, y { x := 1 y := 2} let a, b := f() b := r }"));
	BOOST_CHECK(successAssemble("{ function f() { g() } function g() { f() } }"));
}

BOOST_AUTO_TEST_CASE(embedded_functions)
{
	BOOST_CHECK(successAssemble("{ function f(r, s) -> x { function g(a) -> b { } x := g(2) } let x := f(2, 3) }"));
}

BOOST_AUTO_TEST_CASE(switch_statement)
{
	BOOST_CHECK(successAssemble("{ switch 1 default {} }"));
	BOOST_CHECK(successAssemble("{ switch 1 case 1 {} default {} }"));
	BOOST_CHECK(successAssemble("{ switch 1 case 1 {} }"));
	BOOST_CHECK(successAssemble("{ let a := 3 switch a case 1 { a := 1 } case 2 { a := 5 } a := 9}"));
	BOOST_CHECK(successAssemble("{ let a := 2 switch calldataload(0) case 1 { a := 1 } case 2 { a := 5 } }"));
}

BOOST_AUTO_TEST_CASE(for_statement)
{
	BOOST_CHECK(successAssemble("{ for {} 1 {} {} }"));
	BOOST_CHECK(successAssemble("{ let x := calldatasize() for { let i := 0} lt(i, x) { i := add(i, 1) } { mstore(i, 2) } }"));
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	BOOST_CHECK(successAssemble("{ if 1 {} }"));
	BOOST_CHECK(successAssemble("{ let x := 0 if eq(calldatasize(), 0) { x := 1 } mstore(0, x) }"));
}

BOOST_AUTO_TEST_CASE(large_constant)
{
	auto source = R"({
		switch mul(1, 2)
		case 0x0000000000000000000000000000000000000000000000000000000026121ff0 {
		}
	})";
	BOOST_CHECK(successAssemble(source));
}

BOOST_AUTO_TEST_CASE(keccak256)
{
	BOOST_CHECK(successAssemble("{ pop(keccak256(0, 0)) }"));
}

BOOST_AUTO_TEST_CASE(returndatasize)
{
	if (!solidity::test::CommonOptions::get().evmVersion().supportsReturndata())
		return;
	BOOST_CHECK(successAssemble("{ let r := returndatasize() }"));
}

BOOST_AUTO_TEST_CASE(returndatacopy)
{
	if (!solidity::test::CommonOptions::get().evmVersion().supportsReturndata())
		return;
	BOOST_CHECK(successAssemble("{ returndatacopy(0, 32, 64) }"));
}

BOOST_AUTO_TEST_CASE(returndatacopy_functional)
{
	if (!solidity::test::CommonOptions::get().evmVersion().supportsReturndata())
		return;
	BOOST_CHECK(successAssemble("{ returndatacopy(0, 32, 64) }"));
}

BOOST_AUTO_TEST_CASE(staticcall)
{
	if (!solidity::test::CommonOptions::get().evmVersion().hasStaticCall())
		return;
	BOOST_CHECK(successAssemble("{ pop(staticcall(10000, 0x123, 64, 0x10, 128, 0x10)) }"));
}

BOOST_AUTO_TEST_CASE(create2)
{
	if (!solidity::test::CommonOptions::get().evmVersion().hasCreate2())
		return;
	BOOST_CHECK(successAssemble("{ pop(create2(10, 0x123, 32, 64)) }"));
}

BOOST_AUTO_TEST_CASE(shift)
{
	if (!solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
		return;
	BOOST_CHECK(successAssemble("{ pop(shl(10, 32)) }"));
	BOOST_CHECK(successAssemble("{ pop(shr(10, 32)) }"));
	BOOST_CHECK(successAssemble("{ pop(sar(10, 32)) }"));
}

BOOST_AUTO_TEST_CASE(shift_constantinople_warning)
{
	if (solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
		return;
	CHECK_PARSE_WARNING("{ pop(shl(10, 32)) }", TypeError, "The \"shl\" instruction is only available for Constantinople-compatible VMs");
	CHECK_PARSE_WARNING("{ pop(shr(10, 32)) }", TypeError, "The \"shr\" instruction is only available for Constantinople-compatible VMs");
	CHECK_PARSE_WARNING("{ pop(sar(10, 32)) }", TypeError, "The \"sar\" instruction is only available for Constantinople-compatible VMs");
}

BOOST_AUTO_TEST_CASE(jump_error)
{
	CHECK_PARSE_WARNING("{ jump(44) }", DeclarationError, "Function not found.");
	CHECK_PARSE_WARNING("{ jumpi(44, 2) }", DeclarationError, "Function not found.");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
