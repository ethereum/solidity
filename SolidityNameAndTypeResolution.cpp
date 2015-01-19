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
 * Unit tests for the name and type resolution of the solidity parser.
 */

#include <string>

#include <libdevcore/Log.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <boost/test/unit_test.hpp>

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{
ASTPointer<SourceUnit> parseTextAndResolveNames(std::string const& _source)
{
	Parser parser;
	ASTPointer<SourceUnit> sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(_source)));
	NameAndTypeResolver resolver({});
	resolver.registerDeclarations(*sourceUnit);
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			resolver.resolveNamesAndTypes(*contract);
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			resolver.checkTypeRequirements(*contract);

	return sourceUnit;
}
}

BOOST_AUTO_TEST_SUITE(SolidityNameAndTypeResolution)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVariable1;\n"
					   "  function fun(uint256 arg1) { var x; uint256 y; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(double_stateVariable_declaration)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  uint128 variable;\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(double_function_declaration)
{
	char const* text = "contract test {\n"
					   "  function fun() { var x; }\n"
					   "  function fun() { var x; }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(double_variable_declaration)
{
	char const* text = "contract test {\n"
					   "  function f() { uint256 x; if (true)  { uint256 x; } }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(name_shadowing)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f() { uint32 variable ; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(name_references)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f(uint256 arg) returns (uint out) { f(variable); test; out; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(undeclared_name)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f(uint256 arg) { f(notfound); }"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(reference_to_later_declaration)
{
	char const* text = "contract test {\n"
					   "  function g() { f(); }"
					   "  function f() {  }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(struct_definition_directly_recursive)
{
	char const* text = "contract test {\n"
					   "  struct MyStructName {\n"
					   "    address addr;\n"
					   "    MyStructName x;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), ParserError);
}

BOOST_AUTO_TEST_CASE(struct_definition_indirectly_recursive)
{
	char const* text = "contract test {\n"
					   "  struct MyStructName1 {\n"
					   "    address addr;\n"
					   "    uint256 count;\n"
					   "    MyStructName2 x;\n"
					   "  }\n"
					   "  struct MyStructName2 {\n"
					   "    MyStructName1 x;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), ParserError);
}

BOOST_AUTO_TEST_CASE(struct_definition_recursion_via_mapping)
{
	char const* text = "contract test {\n"
					   "  struct MyStructName1 {\n"
					   "    address addr;\n"
					   "    uint256 count;\n"
					   "    mapping(uint => MyStructName1) x;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(type_inference_smoke_test)
{
	char const* text = "contract test {\n"
					   "  function f(uint256 arg1, uint32 arg2) returns (bool ret) { var x = arg1 + arg2 == 8; ret = x; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(type_checking_return)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_number)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r1, bool r2) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_type)
{
	char const* text = "contract test {\n"
					   "  function f() returns (uint256 r) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(type_checking_function_call)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r) { return g(12, true) == 3; }\n"
					   "  function g(uint256 a, bool b) returns (uint256 r) { }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison)
{
	char const* text = "contract test {\n"
					   "  function f() { uint32(2) == int64(2); }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison_invalid)
{
	char const* text = "contract test {\n"
					   "  function f() { int32(2) == uint64(2); }"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(type_inference_explicit_conversion)
{
	char const* text = "contract test {\n"
					   "  function f() returns (int256 r) { var x = int256(uint32(2)); return x; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(large_string_literal)
{
	char const* text = "contract test {\n"
					   "  function f() { var x = \"123456789012345678901234567890123\"; }"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(balance)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint256 x = address(0).balance;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(balance_invalid)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    address(0).balance = 7;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_to_mapping)
{
	char const* text = "contract test {\n"
					   "  struct str {\n"
					   "    mapping(uint=>uint) map;\n"
					   "  }\n"
					   "  str data;"
					   "  function fun() {\n"
					   "    var a = data.map;\n"
					   "    data.map = a;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_to_struct)
{
	char const* text = "contract test {\n"
					   "  struct str {\n"
					   "    mapping(uint=>uint) map;\n"
					   "  }\n"
					   "  str data;"
					   "  function fun() {\n"
					   "    var a = data;\n"
					   "    data = a;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(returns_in_constructor)
{
	char const* text = "contract test {\n"
					   "  function test() returns (uint a) {\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(forward_function_reference)
{
	char const* text = "contract First {\n"
					   "  function fun() returns (bool ret) {\n"
					   "    return Second(1).fun(1, true, 3) > 0;\n"
					   "  }\n"
					   "}\n"
					   "contract Second {\n"
					   "  function fun(uint a, bool b, uint c) returns (uint ret) {\n"
					   "    if (First(2).fun() == true) return 1;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(comparison_bitop_precedence)
{
	char const* text = "contract First {\n"
					   "  function fun() returns (bool ret) {\n"
					   "    return 1 & 2 == 8 & 9 && 1 ^ 2 < 4 | 6;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(function_canonical_signature)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract Test {\n"
					   "  function foo(uint256 arg1, uint64 arg2, bool arg3) returns (uint256 ret) {\n"
					   "    ret = arg1 + arg2;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(sourceUnit = parseTextAndResolveNames(text));
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			BOOST_CHECK_EQUAL("foo(uint256,uint64,bool)", functions[0]->getCanonicalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_canonical_signature_type_aliases)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract Test {\n"
					   "  function boo(uint arg1, hash arg2, address arg3) returns (uint ret) {\n"
					   "    ret = 5;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_NO_THROW(sourceUnit = parseTextAndResolveNames(text));
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			BOOST_CHECK_EQUAL("boo(uint256,hash256,address)", functions[0]->getCanonicalSignature());
		}
}

BOOST_AUTO_TEST_CASE(hash_collision_in_interface)
{
	char const* text = "contract test {\n"
					   "  function gsf() {\n"
					   "  }\n"
					   "  function tgeo() {\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(inheritance_basic)
{
	char const* text = R"(
		contract base { uint baseMember; struct BaseType { uint element; } }
		contract derived is base {
			BaseType data;
			function f() { baseMember = 7; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(inheritance_diamond_basic)
{
	char const* text = R"(
		contract root { function rootFunction() {} }
		contract inter1 is root { function f() {} }
		contract inter2 is root { function f() {} }
		contract derived is inter1, inter2, root {
			function g() { f(); rootFunction(); }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(cyclic_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B is A { }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_direct)
{
	char const* text = R"(
		contract B { function f() {} }
		contract C is B { function f(uint i) {} }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_indirect)
{
	char const* text = R"(
		contract A { function f(uint a) {} }
		contract B { function f() {} }
		contract C is A, B { }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(complex_inheritance)
{
	char const* text = R"(
		contract A { function f() { uint8 x = C(0).g(); } }
		contract B { function f() {} function g() returns (uint8 r) {} }
		contract C is A, B { }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(constructor_visibility)
{
	// The constructor of a base class should not be visible in the derived class
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function f() { A x = A(0); } }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(overriding_constructor)
{
	// It is fine to "override" constructor of a base class since it is invisible
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function A() returns (uint8 r) {} }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(missing_base_constructor_arguments)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(base_constructor_arguments_override)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

