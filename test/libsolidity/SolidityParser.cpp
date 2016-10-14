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
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/interface/Exceptions.h>
#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{
ASTPointer<ContractDefinition> parseText(std::string const& _source, ErrorList& _errors)
{
	ASTPointer<SourceUnit> sourceUnit = Parser(_errors).parse(std::make_shared<Scanner>(CharStream(_source)));
	if (!sourceUnit)
		return ASTPointer<ContractDefinition>();
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ASTPointer<ContractDefinition> contract = dynamic_pointer_cast<ContractDefinition>(node))
			return contract;
	BOOST_FAIL("No contract found in source.");
	return ASTPointer<ContractDefinition>();
}

bool successParse(std::string const& _source)
{
	ErrorList errors;
	try
	{
		auto sourceUnit = parseText(_source, errors);
		if (!sourceUnit)
			return false;
	}
	catch (FatalError const& /*_exception*/)
	{
		if (Error::containsErrorOfType(errors, Error::Type::ParserError))
			return false;
	}
	if (Error::containsErrorOfType(errors, Error::Type::ParserError))
		return false;

	BOOST_CHECK(Error::containsOnlyWarnings(errors));
	return true;
}

void checkFunctionNatspec(
	FunctionDefinition const* _function,
	std::string const& _expectedDoc
)
{
	auto doc = _function->documentation();
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
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(missing_variable_name_in_declaration)
{
	char const* text = "contract test {\n"
					   "  uint256 ;\n"
					   "}\n";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(empty_function)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName(bytes20 arg1, address addr) constant\n"
					   "    returns (int id)\n"
					   "  { }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(no_function_params)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName() {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(single_function_param)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function functionName(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_no_body)
{
	char const* text = "contract test {\n"
					   "  function functionName(bytes32 input) returns (bytes32 out);\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(missing_parameter_name_in_named_args)
{
	char const* text = "contract test {\n"
					   "  function a(uint a, uint b, uint c) returns (uint r) { r = a * 100 + b * 10 + c * 1; }\n"
					   "  function b() returns (uint r) { r = a({: 1, : 2, : 3}); }\n"
					   "}\n";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(missing_argument_in_named_args)
{
	char const* text = "contract test {\n"
					   "  function a(uint a, uint b, uint c) returns (uint r) { r = a * 100 + b * 10 + c * 1; }\n"
					   "  function b() returns (uint r) { r = a({a: , b: , c: }); }\n"
					   "}\n";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(two_exact_functions)
{
	char const* text = R"(
		contract test {
			function fun(uint a) returns(uint r) { return a; }
			function fun(uint a) returns(uint r) { return a; }
		}
	)";
	// with support of overloaded functions, during parsing,
	// we can't determine whether they match exactly, however
	// it will throw DeclarationError in following stage.
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(overloaded_functions)
{
	char const* text = R"(
		contract test {
			function fun(uint a) returns(uint r) { return a; }
			function fun(uint a, uint b) returns(uint r) { return a + b; }
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_natspec_documentation)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is a test function\n"
					   "  function functionName(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	FunctionDefinition const* function = nullptr;
	auto functions = contract->definedFunctions();

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is a test function");
}

BOOST_AUTO_TEST_CASE(function_normal_comments)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  // We won't see this comment\n"
					   "  function functionName(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();
	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	BOOST_CHECK_MESSAGE(function->documentation() == nullptr,
						"Should not have gotten a Natspecc comment for this function");
}

BOOST_AUTO_TEST_CASE(multiple_functions_natspec_documentation)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is test function 1\n"
					   "  function functionName1(bytes32 input) returns (bytes32 out) {}\n"
					   "  /// This is test function 2\n"
					   "  function functionName2(bytes32 input) returns (bytes32 out) {}\n"
					   "  // nothing to see here\n"
					   "  function functionName3(bytes32 input) returns (bytes32 out) {}\n"
					   "  /// This is test function 4\n"
					   "  function functionName4(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is test function 1");

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(1), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is test function 2");

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(2), "Failed to retrieve function");
	BOOST_CHECK_MESSAGE(function->documentation() == nullptr,
						"Should not have gotten natspec comment for functionName3()");

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(3), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is test function 4");
}

BOOST_AUTO_TEST_CASE(multiline_function_documentation)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  /// This is a test function\n"
					   "  /// and it has 2 lines\n"
					   "  function functionName1(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();
	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is a test function\n"
						 " and it has 2 lines");
}

BOOST_AUTO_TEST_CASE(natspec_comment_in_function_body)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  /// fun1 description\n"
					   "  function fun1(uint256 a) {\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>bytes32) d;\n"
					   "    bytes7 name = \"Solidity\";"
					   "  }\n"
					   "  /// This is a test function\n"
					   "  /// and it has 2 lines\n"
					   "  function fun(bytes32 input) returns (bytes32 out) {}\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	checkFunctionNatspec(function, "fun1 description");

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(1), "Failed to retrieve function");
	checkFunctionNatspec(function, "This is a test function\n"
						 " and it has 2 lines");
}

BOOST_AUTO_TEST_CASE(natspec_docstring_between_keyword_and_signature)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function ///I am in the wrong place \n"
					   "  fun1(uint256 a) {\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>bytes32) d;\n"
					   "    bytes7 name = \"Solidity\";"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	BOOST_CHECK_MESSAGE(!function->documentation(),
						"Shouldn't get natspec docstring for this function");
}

BOOST_AUTO_TEST_CASE(natspec_docstring_after_signature)
{
	FunctionDefinition const* function = nullptr;
	char const* text = "contract test {\n"
					   "  uint256 stateVar;\n"
					   "  function fun1(uint256 a) {\n"
					   "  /// I should have been above the function signature\n"
					   "    var b;\n"
					   "    /// I should not interfere with actual natspec comments\n"
					   "    uint256 c;\n"
					   "    mapping(address=>bytes32) d;\n"
					   "    bytes7 name = \"Solidity\";"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
	ErrorList errors;
	ASTPointer<ContractDefinition> contract = parseText(text, errors);
	auto functions = contract->definedFunctions();

	ETH_TEST_REQUIRE_NO_THROW(function = functions.at(0), "Failed to retrieve function");
	BOOST_CHECK_MESSAGE(!function->documentation(),
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
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(mapping)
{
	char const* text = "contract test {\n"
					   "  mapping(address => bytes32) names;\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(mapping_in_struct)
{
	char const* text = "contract test {\n"
					   "  struct test_struct {\n"
					   "    address addr;\n"
					   "    uint256 count;\n"
					   "    mapping(bytes32 => test_struct) self_reference;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(mapping_to_mapping_in_struct)
{
	char const* text = "contract test {\n"
					   "  struct test_struct {\n"
					   "    address addr;\n"
					   "    mapping (uint64 => mapping (bytes32 => uint)) complex_mapping;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(variable_definition)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    var b;\n"
					   "    uint256 c;\n"
					   "    mapping(address=>bytes32) d;\n"
					   "    customtype varname;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(variable_definition_with_initialization)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    var b = 2;\n"
					   "    uint256 c = 0x87;\n"
					   "    mapping(address=>bytes32) d;\n"
					   "    bytes7 name = \"Solidity\";"
					   "    customtype varname;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(variable_definition_in_function_parameter)
{
	char const* text = R"(
		contract test {
			function fun(var a) {}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(variable_definition_in_mapping)
{
	char const* text = R"(
		contract test {
			function fun() {
				mapping(var=>bytes32) d;
			}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(variable_definition_in_function_return)
{
	char const* text = R"(
		contract test {
			function fun() returns(var d) {
				return 1;
			}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(operator_expression)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 x = (1 + 4) || false && (1 - 12) + -9;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(complex_expression)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    uint256 x = (1 + 4).member(++67)[a/=9] || true;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(exp_expression)
{
	char const* text = R"(
		contract test {
			function fun(uint256 a) {
				uint256 x = 3 ** a;
			}
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(while_loop)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    while (true) { uint256 x = 1; break; continue; } x = 9;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(for_loop_vardef_initexpr)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    for (uint256 i = 0; i < 10; i++)\n"
					   "    { uint256 x = i; break; continue; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(if_statement)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) {\n"
					   "    if (a >= 8) { return 2; } else { var b = 7; }\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(else_if_statement)
{
	char const* text = "contract test {\n"
					   "  function fun(uint256 a) returns (address b) {\n"
					   "    if (a < 0) b = 0x67; else if (a == 0) b = 0x12; else b = 0x78;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(statement_starting_with_type_conversion)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "    uint64[7](3);\n"
					   "    uint64[](3);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(type_conversion_to_dynamic_array)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    var x = uint64[](3);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(import_directive)
{
	char const* text = "import \"abc\";\n"
					   "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(placeholder_in_function_context)
{
	char const* text = "contract c {\n"
					   "  function fun() returns (uint r) {\n"
					   "    var _ = 8;\n"
					   "    return _ + 1;"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(modifier)
{
	char const* text = "contract c {\n"
					   "  modifier mod { if (msg.sender == 0) _; }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(modifier_without_semicolon)
{
	char const* text = "contract c {\n"
					   "  modifier mod { if (msg.sender == 0) _ }\n"
					   "}\n";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(modifier_arguments)
{
	char const* text = "contract c {\n"
					   "  modifier mod(uint a) { if (msg.sender == a) _; }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(modifier_invocation)
{
	char const* text = "contract c {\n"
					   "  modifier mod1(uint a) { if (msg.sender == a) _; }\n"
					   "  modifier mod2 { if (msg.sender == 2) _; }\n"
					   "  function f() mod1(7) mod2 { }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = "contract c {\n"
					   "  function() { }\n"
					   "}\n";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e();
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(event_arguments)
{
	char const* text = R"(
		contract c {
			event e(uint a, bytes32 s);
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(event_arguments_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint a, bytes32 indexed s, bool indexed b);
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(visibility_specifiers)
{
	char const* text = R"(
		contract c {
			uint private a;
			uint internal b;
			uint public c;
			uint d;
			function f() {}
			function f_priv() private {}
			function f_public() public {}
			function f_internal() internal {}
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(multiple_visibility_specifiers)
{
	char const* text = R"(
		contract c {
			uint private internal a;
		})";
	BOOST_CHECK(!successParse(text));
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
	BOOST_CHECK(successParse(text));
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
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(enum_valid_declaration)
{
	char const* text = R"(
		contract c {
			enum validEnum { Value1, Value2, Value3, Value4 }
			function c ()
			{
				a = foo.Value3;
			}
			uint256 a;
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(empty_enum_declaration)
{
	char const* text = R"(
		contract c {
			enum foo { }
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(malformed_enum_declaration)
{
	char const* text = R"(
		contract c {
			enum foo { WARNING,}
		})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(external_function)
{
	char const* text = R"(
		contract c {
			function x() external {}
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(external_variable)
{
	char const* text = R"(
		contract c {
			uint external x;
		})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(arrays_in_storage)
{
	char const* text = R"(
		contract c {
			uint[10] a;
			uint[] a2;
			struct x { uint[2**20] b; y[0] c; }
			struct y { uint d; mapping(uint=>x)[] e; }
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(arrays_in_events)
{
	char const* text = R"(
		contract c {
			event e(uint[10] a, bytes7[8] indexed b, c[3] x);
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(arrays_in_expressions)
{
	char const* text = R"(
		contract c {
			function f() { c[10] a = 7; uint8[10 * 2] x; }
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(multi_arrays)
{
	char const* text = R"(
		contract c {
			mapping(uint => mapping(uint => int8)[8][][9])[] x;
		})";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(constant_is_keyword)
{
	char const* text = R"(
		contract Foo {
			uint constant = 4;
	})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(var_array)
{
	char const* text = R"(
		contract Foo {
			function f() { var[] a; }
	})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(location_specifiers_for_params)
{
	char const* text = R"(
		contract Foo {
			function f(uint[] storage constant x, uint[] memory y) { }
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(location_specifiers_for_locals)
{
	char const* text = R"(
		contract Foo {
			function f() {
				uint[] storage x;
				uint[] memory y;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(location_specifiers_for_state)
{
	char const* text = R"(
		contract Foo {
			uint[] memory x;
	})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(location_specifiers_with_var)
{
	char const* text = R"(
		contract Foo {
			function f() { var memory x; }
	})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(empty_comment)
{
	char const* text = R"(
		//
		contract test
		{}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(library_simple)
{
	char const* text = R"(
		library Lib {
			function f() { }
		}
	)";
	BOOST_CHECK(successParse(text));
}


BOOST_AUTO_TEST_CASE(local_const_variable)
{
	char const* text = R"(
		contract Foo {
			function localConst() returns (uint ret)
			{
				uint constant local = 4;
				return local;
			}
	})";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration)
{
	char const* text = R"(
		contract C {
			function f() {
				var (a,b,c) = g();
				var (d) = 2;
				var (,e) = 3;
				var (f,) = 4;
				var (x,,) = g();
				var (,y,) = g();
				var () = g();
				var (,,) = g();
			}
			function g() returns (uint, uint, uint) {}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(tuples)
{
	char const* text = R"(
		contract C {
			function f() {
				uint a = (1);
				var (b,) = (1,);
				var (c,d) = (1, 2 + a);
				var (e,) = (1, 2, b);
				(a) = 3;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(member_access_parser_ambiguity)
{
	char const* text = R"(
		contract C {
			struct S { uint a; uint b; uint[][][] c; }
			function f() {
				C.S x;
				C.S memory y;
				C.S[10] memory z;
				C.S[10](x);
				x.a = 2;
				x.c[1][2][3] = 9;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(using_for)
{
	char const* text = R"(
		contract C {
			struct s { uint a; }
			using LibraryName for uint;
			using Library2 for *;
			using Lib for s;
			function f() {
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(complex_import)
{
	char const* text = R"(
		import "abc" as x;
		import * as x from "abc";
		import {a as b, c as d, f} from "def";
		contract x {}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(from_is_not_keyword)
{
	// "from" is not a keyword although it is used as a keyword in import directives.
	char const* text = R"(
		contract from {
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration)
{
	char const* text = R"(
		contract c {
			uint[] a;
			function f() returns (uint, uint) {
				a = [1,2,3];
				return (a[3], [2,3,4][0]);
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}


BOOST_AUTO_TEST_CASE(inline_array_empty_cells_check_lvalue)
{
	char const* text = R"(
		contract c {
			uint[] a;
			function f() returns (uint) {
				a = [,2,3];
				return (a[0]);
			}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(inline_array_empty_cells_check_without_lvalue)
{
	char const* text = R"(
		contract c {
			uint[] a;
			function f() returns (uint, uint) {
				return ([3, ,4][0]);
			}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(conditional_true_false_literal)
{
	char const* text = R"(
		contract A {
			function f() {
				uint x = true ? 1 : 0;
				uint y = false ? 0 : 1;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(conditional_with_constants)
{
	char const* text = R"(
		contract A {
			function f() {
				uint x = 3 > 0 ? 3 : 0;
				uint y = (3 > 0) ? 3 : 0;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(conditional_with_variables)
{
	char const* text = R"(
		contract A {
			function f() {
				uint x = 3;
				uint y = 1;
				uint z = (x > y) ? x : y;
				uint w = x > y ? x : y;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(conditional_multiple)
{
	char const* text = R"(
		contract A {
			function f() {
				uint x = 3 < 0 ? 2 > 1 ? 2 : 1 : 7 > 2 ? 7 : 6;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(conditional_with_assignment)
{
	char const* text = R"(
		contract A {
			function f() {
				uint y = 1;
				uint x = 3 < 0 ? x = 3 : 6;
				true ? x = 3 : 4;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(declaring_fixed_and_ufixed_variables)
{
	char const* text = R"(
		contract A {
			fixed40x40 storeMe;
			function f(ufixed x, fixed32x32 y) {
				ufixed8x8 a;
				fixed b;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(declaring_fixed_literal_variables)
{
	char const* text = R"(
		contract A {
			fixed40x40 pi = 3.14;
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(no_double_radix_in_fixed_literal)
{
	char const* text = R"(
		contract A {
			fixed40x40 pi = 3.14.15;
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(invalid_fixed_conversion_leading_zeroes_check)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 1.0x2;
			}
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(payable_accessor)
{
	char const* text = R"(
		contract test {
			uint payable x;
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_in_expression)
{
	char const* text = R"(
		contract test {
			function f(uint x, uint y) returns (uint a) {}
			function g() {
				function (uint, uint) internal returns (uint) f1 = f;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_as_storage_variable)
{
	char const* text = R"(
		contract test {
			function (uint, uint) internal returns (uint) f1;
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_as_storage_variable_with_modifiers)
{
	char const* text = R"(
		contract test {
			function (uint, uint) modifier1() returns (uint) f1;
		}
	)";
	BOOST_CHECK(!successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_as_storage_variable_with_assignment)
{
	char const* text = R"(
		contract test {
			function f(uint x, uint y) returns (uint a) {}
			function (uint, uint) internal returns (uint) f1 = f;
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_in_struct)
{
	char const* text = R"(
		contract test {
			struct S {
				function (uint x, uint y) internal returns (uint a) f;
				function (uint, uint) external returns (uint) g;
				uint d;
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(function_type_as_parameter)
{
	char const* text = R"(
		contract test {
			function f(function(uint) external returns (uint) g) internal returns (uint a) {
				return g(1);
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(calling_function)
{
	char const* text = R"(
		contract test {
			function f() {
				function() returns(function() returns(function() returns(function() returns(uint)))) x;
				uint y;
				y = x()()()();
			}
		}
	)";
	BOOST_CHECK(successParse(text));
}


BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
