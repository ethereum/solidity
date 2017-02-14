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

#include "../TestHelper.h"

#include <libsolidity/inlineasm/AsmStack.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/ast/AST.h>
#include <test/libsolidity/ErrorCheck.h>
#include <libevmasm/Assembly.h>

#include <boost/optional.hpp>

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

boost::optional<Error> parseAndReturnFirstError(string const& _source, bool _assemble = false, bool _allowWarnings = true)
{
	assembly::InlineAssemblyStack stack;
	bool success = false;
	try
	{
		success = stack.parse(std::make_shared<Scanner>(CharStream(_source)));
		if (success && _assemble)
			stack.assemble();
	}
	catch (FatalError const&)
	{
		BOOST_FAIL("Fatal error leaked.");
		success = false;
	}
	if (!success)
	{
		BOOST_CHECK_EQUAL(stack.errors().size(), 1);
		return *stack.errors().front();
	}
	else
	{
		// If success is true, there might still be an error in the assembly stage.
		if (_allowWarnings && Error::containsOnlyWarnings(stack.errors()))
			return {};
		else if (!stack.errors().empty())
		{
			if (!_allowWarnings)
				BOOST_CHECK_EQUAL(stack.errors().size(), 1);
			return *stack.errors().front();
		}
	}
	return {};
}

bool successParse(std::string const& _source, bool _assemble = false, bool _allowWarnings = true)
{
	return !parseAndReturnFirstError(_source, _assemble, _allowWarnings);
}

bool successAssemble(string const& _source, bool _allowWarnings = true)
{
	return successParse(_source, true, _allowWarnings);
}

Error expectError(std::string const& _source, bool _assemble, bool _allowWarnings = false)
{

	auto error = parseAndReturnFirstError(_source, _assemble, _allowWarnings);
	BOOST_REQUIRE(error);
	return *error;
}

void parsePrintCompare(string const& _source)
{
	assembly::InlineAssemblyStack stack;
	BOOST_REQUIRE(stack.parse(std::make_shared<Scanner>(CharStream(_source))));
	BOOST_REQUIRE(stack.errors().empty());
	BOOST_CHECK_EQUAL(stack.toString(), _source);
}

}

#define CHECK_ERROR(text, assemble, typ, substring) \
do \
{ \
	Error err = expectError((text), (assemble), false); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(searchErrorMessage(err, (substring))); \
} while(0)

#define CHECK_PARSE_ERROR(text, type, substring) \
CHECK_ERROR(text, false, type, substring)

#define CHECK_ASSEMBLE_ERROR(text, type, substring) \
CHECK_ERROR(text, true, type, substring)



BOOST_AUTO_TEST_SUITE(SolidityInlineAssembly)


BOOST_AUTO_TEST_SUITE(Parsing)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	BOOST_CHECK(successParse("{ }"));
}

BOOST_AUTO_TEST_CASE(simple_instructions)
{
	BOOST_CHECK(successParse("{ dup1 dup1 mul dup1 sub }"));
}

BOOST_AUTO_TEST_CASE(suicide_selfdestruct)
{
	BOOST_CHECK(successParse("{ suicide selfdestruct }"));
}

BOOST_AUTO_TEST_CASE(keywords)
{
	BOOST_CHECK(successParse("{ byte return address }"));
}

BOOST_AUTO_TEST_CASE(constants)
{
	BOOST_CHECK(successParse("{ 7 8 mul }"));
}

BOOST_AUTO_TEST_CASE(vardecl)
{
	BOOST_CHECK(successParse("{ let x := 7 }"));
}

BOOST_AUTO_TEST_CASE(assignment)
{
	BOOST_CHECK(successParse("{ 7 8 add =: x }"));
}

BOOST_AUTO_TEST_CASE(label)
{
	BOOST_CHECK(successParse("{ 7 abc: 8 eq abc jump }"));
}

BOOST_AUTO_TEST_CASE(label_complex)
{
	BOOST_CHECK(successParse("{ 7 abc: 8 eq jump(abc) jumpi(eq(7, 8), abc) }"));
}

BOOST_AUTO_TEST_CASE(functional)
{
	BOOST_CHECK(successParse("{ add(7, mul(6, x)) add mul(7, 8) }"));
}

BOOST_AUTO_TEST_CASE(functional_assignment)
{
	BOOST_CHECK(successParse("{ x := 7 }"));
}

BOOST_AUTO_TEST_CASE(functional_assignment_complex)
{
	BOOST_CHECK(successParse("{ x := add(7, mul(6, x)) add mul(7, 8) }"));
}

BOOST_AUTO_TEST_CASE(vardecl_complex)
{
	BOOST_CHECK(successParse("{ let x := add(7, mul(6, x)) add mul(7, 8) }"));
}

BOOST_AUTO_TEST_CASE(blocks)
{
	BOOST_CHECK(successParse("{ let x := 7 { let y := 3 } { let z := 2 } }"));
}

BOOST_AUTO_TEST_CASE(labels_with_stack_info)
{
	BOOST_CHECK(successParse("{ x[-1]: y[a]: z[d, e]: h[100]: g[]: }"));
}

BOOST_AUTO_TEST_CASE(function_definitions)
{
	BOOST_CHECK(successParse("{ function f() { } function g(a) -> (x) { } }"));
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	BOOST_CHECK(successParse("{ function f(a, d) { } function g(a, d) -> (x, y) { } }"));
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	BOOST_CHECK(successParse("{ g(1, 2, f(mul(2, 3))) x() }"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Printing)

BOOST_AUTO_TEST_CASE(print_smoke)
{
	parsePrintCompare("{\n}");
}

BOOST_AUTO_TEST_CASE(print_instructions)
{
	parsePrintCompare("{\n    7\n    8\n    mul\n    dup10\n    add\n}");
}

BOOST_AUTO_TEST_CASE(print_subblock)
{
	parsePrintCompare("{\n    {\n        dup4\n        add\n    }\n}");
}

BOOST_AUTO_TEST_CASE(print_functional)
{
	parsePrintCompare("{\n    mul(sload(0x12), 7)\n}");
}

BOOST_AUTO_TEST_CASE(print_label)
{
	parsePrintCompare("{\n    loop:\n    jump(loop)\n}");
}

BOOST_AUTO_TEST_CASE(print_label_with_stack)
{
	parsePrintCompare("{\n    loop[x, y]:\n    other[-2]:\n    third[10]:\n}");
}

BOOST_AUTO_TEST_CASE(print_assignments)
{
	parsePrintCompare("{\n    let x := mul(2, 3)\n    7\n    =: x\n    x := add(1, 2)\n}");
}

BOOST_AUTO_TEST_CASE(print_string_literals)
{
	parsePrintCompare("{\n    \"\\n'\\xab\\x95\\\"\"\n}");
}

BOOST_AUTO_TEST_CASE(print_string_literal_unicode)
{
	string source = "{ \"\\u1bac\" }";
	string parsed = "{\n    \"\\xe1\\xae\\xac\"\n}";
	assembly::InlineAssemblyStack stack;
	BOOST_REQUIRE(stack.parse(std::make_shared<Scanner>(CharStream(source))));
	BOOST_REQUIRE(stack.errors().empty());
	BOOST_CHECK_EQUAL(stack.toString(), parsed);
	parsePrintCompare(parsed);
}

BOOST_AUTO_TEST_CASE(function_definitions_multiple_args)
{
	parsePrintCompare("{\n    function f(a, d)\n    {\n        mstore(a, d)\n    }\n    function g(a, d) -> (x, y)\n    {\n    }\n}");
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	parsePrintCompare("{\n    g(1, mul(2, x), f(mul(2, 3)))\n    x()\n}");
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
	BOOST_CHECK(successParse("{ let x := 1 { tag: =: x } }"));
}

BOOST_AUTO_TEST_CASE(magic_variables)
{
	CHECK_ASSEMBLE_ERROR("{ this pop }", DeclarationError, "Identifier not found or not unique");
	CHECK_ASSEMBLE_ERROR("{ ecrecover pop }", DeclarationError, "Identifier not found or not unique");
	BOOST_CHECK(successAssemble("{ let ecrecover := 1 ecrecover }"));
}

BOOST_AUTO_TEST_CASE(imbalanced_stack)
{
	BOOST_CHECK(successAssemble("{ 1 2 mul pop }", false));
	CHECK_ASSEMBLE_ERROR("{ 1 }", Warning, "Inline assembly block is not balanced. It leaves");
	CHECK_ASSEMBLE_ERROR("{ pop }", Warning, "Inline assembly block is not balanced. It takes");
	BOOST_CHECK(successAssemble("{ let x := 4 7 add }", false));
}

BOOST_AUTO_TEST_CASE(error_tag)
{
	BOOST_CHECK(successAssemble("{ invalidJumpLabel }"));
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
