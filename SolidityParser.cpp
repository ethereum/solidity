/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity parser.
 */

#include <string>
#include <memory>
#include <libdevcore/Log.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/Exceptions.h>
#include <boost/test/unit_test.hpp>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{
ASTPointer<ContractDefinition> parseText(std::string const& _source)
{
	Parser parser;
	ASTPointer<SourceUnit> sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(_source)));
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ASTPointer<ContractDefinition> contract = dynamic_pointer_cast<ContractDefinition>(node))
			return contract;
	BOOST_FAIL("No contract found in source.");
	return ASTPointer<ContractDefinition>();
}

ASTPointer<ContractDefinition> parseTextExplainError(std::string const& _source)
{
	try
	{
		return parseText(_source);
	}
	catch (Exception const& exception)
	{
		// LTODO: Print the error in a kind of a better way?
		// In absence of CompilerStack we can't use SourceReferenceFormatter
		cout << "Exception while parsing: " << diagnostic_information(exception);
		// rethrow to signal test failure
		throw exception;
	}
}

static void checkFunctionNatspec(ASTPointer<FunctionDefinition> _function,
								 std::string const& _expectedDoc)
{
	auto doc = _function->getDocumentation();
	BOOST_CHECK_MESSAGE(doc != nullptr, "Function does not have Natspec Doc as expected");
	BOOST_CHECK_EQUAL(*doc, _expectedDoc);
}

}


BOOST_AUTO_TEST_SUITE(SolidityParser)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVariable1;\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(missing_variable_name_in_declaration)
{
	char const* text = "contract test {\n"
					   "  uint256 ;\n"
					   "}\n";
	BOOST_CHECK_THROW(parseText(text), ParserError);
}

BOOST_AUTO_TEST_CASE(empty_function)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName(hash160 arg1, address addr) constant\n"
					   "    returns (int id)\n"
					   "  { }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(no_function_params)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName() {}\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(single_function_param)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(missing_parameter_name_in_named_args)
{
	char const* text = "contract test {\n"
					   "  function a(uint a, uint b, uint c) returns (uint r) { r = a * 100 + b * 10 + c * 1; }\n"
					   "  function b() returns (uint r) { r = a({: 1, : 2, : 3}); }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseText(text), ParserError);
}

BOOST_AUTO_TEST_CASE(missing_argument_in_named_args)
{
	char const* text = "contract test {\n"
					   "  function a(uint a, uint b, uint c) returns (uint r) { r = a * 100 + b * 10 + c * 1; }\n"
					   "  function b() returns (uint r) { r = a({a: , b: , c: }); }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseText(text), ParserError);
}

BOOST_AUTO_TEST_CASE(function_natspec_documentation)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is a test function\n"
					   "  function functionName(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();
	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	checkFunctionNatspec(function, "This is a test function");
}

BOOST_AUTO_TEST_CASE(function_normal_comments)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  // We won't see this comment\n"
					   "  function functionName(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();
	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	BOOST_CHECK_MESSAGE(function->getDocumentation() == nullptr,
						"Should not have gotten a Natspecc comment for this function");
}

BOOST_AUTO_TEST_CASE(multiple_functions_natspec_documentation)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is test function 1\n"
					   "  function functionName1(hash hashin) returns (hash hashout) {}\n"
					   "  /// This is test function 2\n"
					   "  function functionName2(hash hashin) returns (hash hashout) {}\n"
					   "  // nothing to see here\n"
					   "  function functionName3(hash hashin) returns (hash hashout) {}\n"
					   "  /// This is test function 4\n"
					   "  function functionName4(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();

	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	checkFunctionNatspec(function, "This is test function 1");

	BOOST_REQUIRE_NO_THROW(function = functions.at(1));
	checkFunctionNatspec(function, "This is test function 2");

	BOOST_REQUIRE_NO_THROW(function = functions.at(2));
	BOOST_CHECK_MESSAGE(function->getDocumentation() == nullptr,
						"Should not have gotten natspec comment for functionName3()");

	BOOST_REQUIRE_NO_THROW(function = functions.at(3));
	checkFunctionNatspec(function, "This is test function 4");
}

BOOST_AUTO_TEST_CASE(multiline_function_documentation)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is a test function\n"
					   "  /// and it has 2 lines\n"
					   "  function functionName1(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();

	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	checkFunctionNatspec(function, "This is a test function\n"
						 " and it has 2 lines");
}

BOOST_AUTO_TEST_CASE(natspec_comment_in_function_body)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  /// fun1 description\n"
					   "  function fun1(uint256 a) {\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>hash) d;\n"
					   "    string name = \"Solidity\";"
					   "  }\n"
					   "  /// This is a test function\n"
					   "  /// and it has 2 lines\n"
					   "  function fun(hash hashin) returns (hash hashout) {}\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();

	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	checkFunctionNatspec(function, "fun1 description");

	BOOST_REQUIRE_NO_THROW(function = functions.at(1));
	checkFunctionNatspec(function, "This is a test function\n"
						 " and it has 2 lines");
}

BOOST_AUTO_TEST_CASE(natspec_docstring_between_keyword_and_signature)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function ///I am in the wrong place \n"
					   "  fun1(uint256 a) {\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>hash) d;\n"
					   "    string name = \"Solidity\";"
					   "  }\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();

	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	BOOST_CHECK_MESSAGE(!function->getDocumentation(),
						"Shouldn't get natspec docstring for this function");
}

BOOST_AUTO_TEST_CASE(natspec_docstring_after_signature)
{
	ASTPointer<ContractDefinition> contract;
	ASTPointer<FunctionDefinition> function;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function fun1(uint256 a) {\n"
					   "  /// I should have been above the function signature\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>hash) d;\n"
					   "    string name = \"Solidity\";"
					   "  }\n"
					   "}\n";
	BOOST_REQUIRE_NO_THROW(contract = parseText(text));
	auto functions = contract->getDefinedFunctions();

	BOOST_REQUIRE_NO_THROW(function = functions.at(0));
	BOOST_CHECK_MESSAGE(!function->getDocumentation(),
						"Shouldn't get natspec docstring for this function");
}

BOOST_AUTO_TEST_CASE(struct_definition)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  struct MyStructName {\n"
					   "    address addr;\n"
					   "    uint256 count;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(mapping)
{
	char const* text = "contract test {\n"
					   "  mapping(address => string) names;\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(mapping_in_struct)
{
	char const* text = "contract test {\n"
					   "  struct test_struct {\n"
					   "    address addr;\n"
					   "    uint256 count;\n"
					   "    mapping(hash => test_struct) self_reference;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(mapping_to_mapping_in_struct)
{
	char const* text = "contract test {\n"
					   "  struct test_struct {\n"
					   "    address addr;\n"
					   "    mapping (uint64 => mapping (hash => uint)) complex_mapping;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(variable_definition)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    var b;\n"
					   "    uint256 c;\n"
					   "    mapping(address=>hash) d;\n"
					   "    customtype varname;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(variable_definition_with_initialization)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    var b = 2;\n"
					   "    uint256 c = 0x87;\n"
					   "    mapping(address=>hash) d;\n"
					   "    string name = \"Solidity\";"
					   "    customtype varname;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(operator_expression)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 x = (1 + 4) || false && (1 - 12) + -9;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(complex_expression)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 x = (1 + 4).member(++67)[a/=9] || true;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(exp_expression)
{
	char const* text = R"(
		contract test {
			function fun(uint256 a) {
				uint256 x = 3 ** a;
			}
		})";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(while_loop)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    while (true) { uint256 x = 1; break; continue; } x = 9;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(for_loop_vardef_initexpr)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    for (uint256 i = 0; i < 10; i++)\n"
					   "    { uint256 x = i; break; continue; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_CASE(for_loop_simple_initexpr)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 i =0;\n"
					   "    for (i = 0; i < 10; i++)\n"
					   "    { uint256 x = i; break; continue; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_CASE(for_loop_simple_noexpr)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 i =0;\n"
					   "    for (;;)\n"
					   "    { uint256 x = i; break; continue; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_CASE(for_loop_single_stmt_body)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 i =0;\n"
					   "    for (i = 0; i < 10; i++)\n"
					   "        continue;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    if (a >= 8) return 2; else { var b = 7; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(else_if_statement)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) returns (address b) {\n"
					   "    if (a < 0) b = 0x67; else if (a == 0) b = 0x12; else b = 0x78;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(statement_starting_with_type_conversion)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(import_directive)
{
	char const* text = "import \"abc\";\n"
					   "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(multiple_contracts)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "contract test2 {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(multiple_contracts_and_imports)
{
	char const* text = "import \"abc\";\n"
					   "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "import \"def\";\n"
					   "contract test2 {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "import \"ghi\";\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(contract_inheritance)
{
	char const* text = "contract base {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "contract derived is base {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(contract_multiple_inheritance)
{
	char const* text = "contract base {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "contract derived is base, nonExisting {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(contract_multiple_inheritance_with_arguments)
{
	char const* text = "contract base {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n"
					   "contract derived is base(2), nonExisting(\"abc\", \"def\", base.fun()) {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(placeholder_in_function_context)
{
	char const* text = "contract c {\n"
					   "  function fun() returns (uint r) {\n"
					   "    var _ = 8;\n"
					   "    return _ + 1;"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(modifier)
{
	char const* text = "contract c {\n"
					   "  modifier mod { if (msg.sender == 0) _ }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(modifier_arguments)
{
	char const* text = "contract c {\n"
					   "  modifier mod(uint a) { if (msg.sender == a) _ }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(modifier_invocation)
{
	char const* text = "contract c {\n"
					   "  modifier mod1(uint a) { if (msg.sender == a) _ }\n"
					   "  modifier mod2 { if (msg.sender == 2) _ }\n"
					   "  function f() mod1(7) mod2 { }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = "contract c {\n"
					   "  function() { }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e();
		})";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(event_arguments)
{
	char const* text = R"(
		contract c {
			event e(uint a, string32 s);
		})";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(event_arguments_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint a, string32 indexed s, bool indexed b);
		})";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(visibility_specifiers)
{
	char const* text = R"(
		contract c {
			uint private a;
			uint protected b;
			uint public c;
			uint d;
			function f() {}
			function f_priv() private {}
			function f_public() public {}
			function f_protected() protected {}
		})";
	BOOST_CHECK_NO_THROW(parseText(text));
}

BOOST_AUTO_TEST_CASE(multiple_visibility_specifiers)
{
	char const* text = R"(
		contract c {
			uint private protected a;
		})";
	BOOST_CHECK_THROW(parseText(text), ParserError);
}

BOOST_AUTO_TEST_CASE(literal_constants_with_ether_subdenominations)
{
	char const* text = R"(
		contract c {
			function c ()
			{
				 a = 1 wei;
				 b = 2 szabo;
				 c = 3 finney;
				 b = 4 ether;
			}
			uint256 a;
			uint256 b;
			uint256 c;
			uint256 d;
		})";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_CASE(literal_constants_with_ether_subdenominations_in_expressions)
{
	char const* text = R"(
		contract c {
			function c ()
			{
				 a = 1 wei * 100 wei + 7 szabo - 3;
			}
			uint256 a;
		})";
	BOOST_CHECK_NO_THROW(parseTextExplainError(text));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

