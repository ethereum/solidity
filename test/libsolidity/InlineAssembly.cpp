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

#include <test/Options.h>

#include <libsolidity/interface/AssemblyStack.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/ast/AST.h>
#include <test/libsolidity/ErrorCheck.h>
#include <libevmasm/Assembly.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <string>
#include <memory>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

boost::optional<Error> parseAndReturnFirstError(
	string const& _source,
	bool _assemble = false,
	bool _allowWarnings = true,
	AssemblyStack::Language _language = AssemblyStack::Language::Assembly,
	AssemblyStack::Machine _machine = AssemblyStack::Machine::EVM
)
{
	AssemblyStack stack(dev::test::Options::get().evmVersion(), _language);
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
			BOOST_FAIL("Found more than one error.");
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
	AssemblyStack stack(dev::test::Options::get().evmVersion());
	BOOST_REQUIRE(stack.parseAndAnalyze("", _source));
	if (_canWarn)
		BOOST_REQUIRE(Error::containsOnlyWarnings(stack.errors()));
	else
		BOOST_REQUIRE(stack.errors().empty());
	BOOST_CHECK_EQUAL(stack.print(), _source);
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
	BOOST_CHECK(successParse("{ dup1 dup1 mul dup1 sub pop }"));
}

BOOST_AUTO_TEST_CASE(selfdestruct)
{
	BOOST_CHECK(successParse("{ 0x02 selfdestruct }"));
}

BOOST_AUTO_TEST_CASE(keywords)
{
	BOOST_CHECK(successParse("{ 1 2 byte 2 return address pop }"));
}

BOOST_AUTO_TEST_CASE(constants)
{
	BOOST_CHECK(successParse("{ 7 8 mul pop }"));
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
	CHECK_PARSE_ERROR("{ let x := true }", ParserError, "True and false are not valid literals.");
	CHECK_PARSE_ERROR("{ let x := false }", ParserError, "True and false are not valid literals.");
}

BOOST_AUTO_TEST_CASE(vardecl_empty)
{
	BOOST_CHECK(successParse("{ let x }"));
}

BOOST_AUTO_TEST_CASE(assignment)
{
	BOOST_CHECK(successParse("{ let x := 2 7 8 add =: x }"));
}

BOOST_AUTO_TEST_CASE(label)
{
	BOOST_CHECK(successParse("{ 7 abc: 8 eq abc jump pop }"));
}

BOOST_AUTO_TEST_CASE(label_complex)
{
	BOOST_CHECK(successParse("{ 7 abc: 8 eq jump(abc) jumpi(eq(7, 8), abc) pop }"));
}

BOOST_AUTO_TEST_CASE(functional)
{
	BOOST_CHECK(successParse("{ let x := 2 add(7, mul(6, x)) mul(7, 8) add =: x }"));
}

BOOST_AUTO_TEST_CASE(functional_partial)
{
	CHECK_PARSE_ERROR("{ let x := byte }", ParserError, "Expected '(' (instruction \"byte\" expects 2 arguments)");
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
	BOOST_CHECK(successParse("{ let x := 2 x := add(7, mul(6, x)) mul(7, 8) add }"));
}

BOOST_AUTO_TEST_CASE(vardecl_complex)
{
	BOOST_CHECK(successParse("{ let y := 2 let x := add(7, mul(6, y)) add mul(7, 8) }"));
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
	CHECK_PARSE_ERROR("{ if mload {} }", ParserError, "Expected '(' (instruction \"mload\" expects 1 arguments)");
	BOOST_CHECK("{ if calldatasize() {}");
	CHECK_PARSE_ERROR("{ if mstore(1, 1) {} }", ParserError, "Instruction \"mstore\" not allowed in this context");
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
	CHECK_PARSE_ERROR("{ switch 42 case 1 {} case 1 {} default {} }", DeclarationError, "Duplicate case defined");
}

BOOST_AUTO_TEST_CASE(switch_invalid_expression)
{
	CHECK_PARSE_ERROR("{ switch {} default {} }", ParserError, "Literal, identifier or instruction expected.");
	CHECK_PARSE_ERROR("{ switch mload default {} }", ParserError, "Expected '(' (instruction \"mload\" expects 1 arguments)");
	CHECK_PARSE_ERROR("{ switch mstore(1, 1) default {} }", ParserError, "Instruction \"mstore\" not allowed in this context");
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
	CHECK_PARSE_ERROR("{ for {} {} {} {} }", ParserError, "Literal, identifier or instruction expected.");
	CHECK_PARSE_ERROR("{ for 1 1 {} {} }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} 1 1 {} }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} 1 {} 1 }", ParserError, "Expected '{' but got 'Number'");
	CHECK_PARSE_ERROR("{ for {} mload {} {} }", ParserError, "Expected '(' (instruction \"mload\" expects 1 arguments)");
	CHECK_PARSE_ERROR("{ for {} mstore(1, 1) {} {} }", ParserError, "Instruction \"mstore\" not allowed in this context");
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
	CHECK_PARSE_ERROR("{ function gas() { } }", ParserError, "Cannot use instruction names for identifier names.");
}

BOOST_AUTO_TEST_CASE(opcode_for_function_args)
{
	CHECK_PARSE_ERROR("{ function f(gas) { } }", ParserError, "Cannot use instruction names for identifier names.");
	CHECK_PARSE_ERROR("{ function f() -> gas { } }", ParserError, "Cannot use instruction names for identifier names.");
}

BOOST_AUTO_TEST_CASE(name_clashes)
{
	CHECK_PARSE_ERROR("{ let g := 2 function g() { } }", DeclarationError, "Function name g already taken in this scope");
}

BOOST_AUTO_TEST_CASE(variable_access_cross_functions)
{
	CHECK_PARSE_ERROR("{ let x := 2 function g() { x pop } }", DeclarationError, "Identifier not found.");
}

BOOST_AUTO_TEST_CASE(invalid_tuple_assignment)
{
	/// The push(42) is added here to silence the unbalanced stack error, so that there's only one error reported.
	CHECK_PARSE_ERROR("{ 42 let x, y := 1 }", DeclarationError, "Variable count mismatch.");
}

BOOST_AUTO_TEST_CASE(instruction_too_few_arguments)
{
	CHECK_PARSE_ERROR("{ mul() }", ParserError, "Expected expression (instruction \"mul\" expects 2 arguments)");
	CHECK_PARSE_ERROR("{ mul(1) }", ParserError, "Expected ',' (instruction \"mul\" expects 2 arguments)");
}

BOOST_AUTO_TEST_CASE(instruction_too_many_arguments)
{
	CHECK_PARSE_ERROR("{ mul(1, 2, 3) }", ParserError, "Expected ')' (instruction \"mul\" expects 2 arguments)");
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
	CHECK_PARSE_ERROR("{ let x function f() -> a, b {} 123, x := f() }", ParserError, "Label name / variable name must precede \",\" (multiple assignment).");
	CHECK_PARSE_ERROR("{ let x function f() -> a, b {} x, 123 := f() }", ParserError, "Variable name expected in multiple assignment.");

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

BOOST_AUTO_TEST_SUITE(LooseStrictMode)

BOOST_AUTO_TEST_CASE(no_opcodes_in_strict)
{
	BOOST_CHECK(successParse("{ pop(callvalue) }"));
	BOOST_CHECK(successParse("{ callvalue pop }"));
	CHECK_STRICT_ERROR("{ pop(callvalue) }", ParserError, "Non-functional instructions are not allowed in this context.");
	CHECK_STRICT_ERROR("{ callvalue pop }", ParserError, "Call or assignment expected");
	SUCCESS_STRICT("{ pop(callvalue()) }");
	BOOST_CHECK(successParse("{ switch callvalue case 0 {} }"));
	CHECK_STRICT_ERROR("{ switch callvalue case 0 {} }", ParserError, "Non-functional instructions are not allowed in this context.");
}

BOOST_AUTO_TEST_CASE(no_labels_in_strict)
{
	BOOST_CHECK(successParse("{ a: }"));
	CHECK_STRICT_ERROR("{ a: }", ParserError, "Labels are not supported");
}

BOOST_AUTO_TEST_CASE(no_stack_assign_in_strict)
{
	BOOST_CHECK(successParse("{ let x 4 =: x }"));
	CHECK_STRICT_ERROR("{ let x 4 =: x }", ParserError, "Call or assignment expected.");
}

BOOST_AUTO_TEST_CASE(no_dup_swap_in_strict)
{
	BOOST_CHECK(successParse("{ swap1 }"));
	CHECK_STRICT_ERROR("{ swap1 }", ParserError, "Call or assignment expected.");
	BOOST_CHECK(successParse("{ dup1 pop }"));
	CHECK_STRICT_ERROR("{ dup1 pop }", ParserError, "Call or assignment expected.");
	BOOST_CHECK(successParse("{ swap2 }"));
	CHECK_STRICT_ERROR("{ swap2 }", ParserError, "Call or assignment expected.");
	BOOST_CHECK(successParse("{ dup2 pop }"));
	CHECK_STRICT_ERROR("{ dup2 pop }", ParserError, "Call or assignment expected.");
	CHECK_PARSE_ERROR("{ switch dup1 case 0 {} }", ParserError, "Instruction \"dup1\" not allowed in this context");
	CHECK_STRICT_ERROR("{ switch dup1 case 0 {} }", ParserError, "Instruction \"dup1\" not allowed in this context");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Printing)

BOOST_AUTO_TEST_CASE(print_smoke)
{
	parsePrintCompare("{\n}");
}

BOOST_AUTO_TEST_CASE(print_instructions)
{
	parsePrintCompare("{\n    7\n    8\n    mul\n    dup10\n    add\n    pop\n}");
}

BOOST_AUTO_TEST_CASE(print_subblock)
{
	parsePrintCompare("{\n    {\n        dup4\n        add\n    }\n}");
}

BOOST_AUTO_TEST_CASE(print_functional)
{
	parsePrintCompare("{\n    let x := mul(sload(0x12), 7)\n}");
}

BOOST_AUTO_TEST_CASE(print_label)
{
	parsePrintCompare("{\n    loop:\n    jump(loop)\n}", true);
}

BOOST_AUTO_TEST_CASE(print_assignments)
{
	parsePrintCompare("{\n    let x := mul(2, 3)\n    7\n    =: x\n    x := add(1, 2)\n}");
}

BOOST_AUTO_TEST_CASE(print_multi_assignments)
{
	parsePrintCompare("{\n    function f() -> x, y\n    {\n    }\n    let x, y := f()\n}");
}

BOOST_AUTO_TEST_CASE(print_string_literals)
{
	parsePrintCompare("{\n    \"\\n'\\xab\\x95\\\"\"\n    pop\n}");
}

BOOST_AUTO_TEST_CASE(print_string_literal_unicode)
{
	string source = "{ let x := \"\\u1bac\" }";
	string parsed = "{\n    let x := \"\\xe1\\xae\\xac\"\n}";
	AssemblyStack stack(dev::test::Options::get().evmVersion());
	BOOST_REQUIRE(stack.parseAndAnalyze("", source));
	BOOST_REQUIRE(stack.errors().empty());
	BOOST_CHECK_EQUAL(stack.print(), parsed);
	parsePrintCompare(parsed);
}

BOOST_AUTO_TEST_CASE(print_if)
{
	parsePrintCompare("{\n    if 2\n    {\n        pop(mload(0))\n    }\n}");
}

BOOST_AUTO_TEST_CASE(print_switch)
{
	parsePrintCompare("{\n    switch 42\n    case 1 {\n    }\n    case 2 {\n    }\n    default {\n    }\n}");
}

BOOST_AUTO_TEST_CASE(print_for)
{
	parsePrintCompare("{\n    let ret := 5\n    for {\n        let i := 1\n    }\n    lt(i, 15)\n    {\n        i := add(i, 1)\n    }\n    {\n        ret := mul(ret, i)\n    }\n}");
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	parsePrintCompare("{\n    function f(a, d)\n    {\n        mstore(a, d)\n    }\n    function g(a, d) -> x, y\n    {\n    }\n}");
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	string source = R"({
	function y()
	{
	}
	function f(a) -> b
	{
	}
	function g(a, b, c)
	{
	}
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

BOOST_AUTO_TEST_CASE(assignment_after_tag)
{
	BOOST_CHECK(successParse("{ let x := 1 { 7 tag: =: x } }"));
}

BOOST_AUTO_TEST_CASE(magic_variables)
{
	CHECK_ASSEMBLE_ERROR("{ this pop }", DeclarationError, "Identifier not found");
	CHECK_ASSEMBLE_ERROR("{ ecrecover pop }", DeclarationError, "Identifier not found");
	BOOST_CHECK(successAssemble("{ let ecrecover := 1 ecrecover pop }"));
}

BOOST_AUTO_TEST_CASE(stack_variables)
{
	BOOST_CHECK(successAssemble("{ let y := 3 { 2 { let x := y } pop} }"));
}

BOOST_AUTO_TEST_CASE(imbalanced_stack)
{
	BOOST_CHECK(successAssemble("{ 1 2 mul pop }", false));
	CHECK_ASSEMBLE_ERROR("{ 1 }", DeclarationError, "Unbalanced stack at the end of a block: 1 surplus item(s).");
	CHECK_ASSEMBLE_ERROR("{ pop }", DeclarationError, "Unbalanced stack at the end of a block: 1 missing item(s).");
	BOOST_CHECK(successAssemble("{ let x := 4 7 add }", false));
}

BOOST_AUTO_TEST_CASE(error_tag)
{
	CHECK_ERROR("{ jump(invalidJumpLabel) }", true, DeclarationError, "Identifier not found", true);
}

BOOST_AUTO_TEST_CASE(designated_invalid_instruction)
{
	BOOST_CHECK(successAssemble("{ invalid }"));
}

BOOST_AUTO_TEST_CASE(inline_assembly_shadowed_instruction_declaration)
{
	CHECK_ASSEMBLE_ERROR("{ let gas := 1 }", ParserError, "Cannot use instruction names for identifier names.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_shadowed_instruction_assignment)
{
	CHECK_ASSEMBLE_ERROR("{ 2 =: gas }", ParserError, "Identifier expected, got instruction name.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_shadowed_instruction_functional_assignment)
{
	CHECK_ASSEMBLE_ERROR("{ gas := 2 }", ParserError, "Label name / variable name must precede \":\"");
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
	BOOST_CHECK(successAssemble("{ 0 0 keccak256 pop }"));
	BOOST_CHECK(successAssemble("{ pop(keccak256(0, 0)) }"));
}

BOOST_AUTO_TEST_CASE(returndatasize)
{
	BOOST_CHECK(successAssemble("{ let r := returndatasize }"));
}

BOOST_AUTO_TEST_CASE(returndatasize_functional)
{
	BOOST_CHECK(successAssemble("{ let r := returndatasize() }"));
}

BOOST_AUTO_TEST_CASE(returndatacopy)
{
	BOOST_CHECK(successAssemble("{ 64 32 0 returndatacopy }"));
}

BOOST_AUTO_TEST_CASE(returndatacopy_functional)
{
	BOOST_CHECK(successAssemble("{ returndatacopy(0, 32, 64) }"));
}

BOOST_AUTO_TEST_CASE(staticcall)
{
	BOOST_CHECK(successAssemble("{ pop(staticcall(10000, 0x123, 64, 0x10, 128, 0x10)) }"));
}

BOOST_AUTO_TEST_CASE(create2)
{
	BOOST_CHECK(successAssemble("{ pop(create2(10, 0x123, 32, 64)) }"));
}

BOOST_AUTO_TEST_CASE(shift)
{
	BOOST_CHECK(successAssemble("{ pop(shl(10, 32)) }"));
	BOOST_CHECK(successAssemble("{ pop(shr(10, 32)) }"));
	BOOST_CHECK(successAssemble("{ pop(sar(10, 32)) }"));
}

BOOST_AUTO_TEST_CASE(shift_constantinople_warning)
{
	if (dev::test::Options::get().evmVersion().hasBitwiseShifting())
		return;
	CHECK_PARSE_WARNING("{ pop(shl(10, 32)) }", Warning, "The \"shl\" instruction is only available for Constantinople-compatible VMs.");
	CHECK_PARSE_WARNING("{ pop(shr(10, 32)) }", Warning, "The \"shr\" instruction is only available for Constantinople-compatible VMs.");
	CHECK_PARSE_WARNING("{ pop(sar(10, 32)) }", Warning, "The \"sar\" instruction is only available for Constantinople-compatible VMs.");
}

BOOST_AUTO_TEST_CASE(jump_warning)
{
	CHECK_PARSE_WARNING("{ 1 jump }", Warning, "Jump instructions");
	CHECK_PARSE_WARNING("{ 1 2 jumpi }", Warning, "Jump instructions");
	CHECK_PARSE_WARNING("{ jump(44) }", Warning, "Jump instructions");
	CHECK_PARSE_WARNING("{ jumpi(44, 2) }", Warning, "Jump instructions");
	CHECK_PARSE_WARNING("{ a: }", Warning, "Jump instructions");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
