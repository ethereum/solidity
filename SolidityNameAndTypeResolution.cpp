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
#include <libdevcrypto/SHA3.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
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

ASTPointer<SourceUnit> parseTextAndResolveNamesWithChecks(std::string const& _source)
{
	Parser parser;
	ASTPointer<SourceUnit> sourceUnit;
	try
	{
		sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(_source)));
		NameAndTypeResolver resolver({});
		resolver.registerDeclarations(*sourceUnit);
		for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
				resolver.resolveNamesAndTypes(*contract);
		for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
				resolver.checkTypeRequirements(*contract);
	}
	catch(boost::exception const& _e)
	{
		auto msg = std::string("Parsing text and resolving names failed with: \n") + boost::diagnostic_information(_e);
		BOOST_FAIL(msg);
	}
	return sourceUnit;
}

static ContractDefinition const* retrieveContract(ASTPointer<SourceUnit> _source, unsigned index)
{
	ContractDefinition* contract;
	unsigned counter = 0;
	for (ASTPointer<ASTNode> const& node: _source->getNodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && counter == index)
			return contract;

	return NULL;
}

static FunctionTypePointer const& retrieveFunctionBySignature(ContractDefinition const* _contract,
															  std::string const& _signature)
{
	FixedHash<4> hash(dev::sha3(_signature));
	return _contract->getInterfaceFunctions()[hash];
}
}

BOOST_AUTO_TEST_SUITE(SolidityNameAndTypeResolution)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVariable1;\n"
					   "  function fun(uint256 arg1) { var x; uint256 y; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNamesWithChecks(text));
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
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
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
		contract derived is root, inter2, inter1 {
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

BOOST_AUTO_TEST_CASE(illegal_override_visibility)
{
	char const* text = R"(
		contract B { function f() protected {} }
		contract C is B { function f() public {} }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_constness)
{
	char const* text = R"(
		contract B { function f() constant {} }
		contract C is B { function f() {} }
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

BOOST_AUTO_TEST_CASE(implicit_derived_to_base_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() { A a = B(1); }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(implicit_base_to_derived_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() { B b = A(1); }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation)
{
	char const* text = R"(
		contract B {
			function f() mod1(2, true) mod2("0123456") { }
			modifier mod1(uint a, bool b) { if (b) _ }
			modifier mod2(string7 a) { while (a == "1234567") _ }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(invalid_function_modifier_type)
{
	char const* text = R"(
		contract B {
			function f() mod1(true) { }
			modifier mod1(uint a) { if (a > 0) _ }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation_parameters)
{
	char const* text = R"(
		contract B {
			function f(uint8 a) mod1(a, true) mod2(r) returns (string7 r) { }
			modifier mod1(uint a, bool b) { if (b) _ }
			modifier mod2(string7 a) { while (a == "1234567") _ }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation_local_variables)
{
	char const* text = R"(
		contract B {
			function f() mod(x) { uint x = 7; }
			modifier mod(uint a) { if (a > 0) _ }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(legal_modifier_override)
{
	char const* text = R"(
		contract A { modifier mod(uint a) {} }
		contract B is A { modifier mod(uint a) {} }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(illegal_modifier_override)
{
	char const* text = R"(
		contract A { modifier mod(uint a) {} }
		contract B is A { modifier mod(uint8 a) {} }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(modifier_overrides_function)
{
	char const* text = R"(
		contract A { modifier mod(uint a) {} }
		contract B is A { function mod(uint a) {} }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(function_overrides_modifier)
{
	char const* text = R"(
		contract A { function mod(uint a) {} }
		contract B is A { modifier mod(uint a) {} }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(modifier_returns_value)
{
	char const* text = R"(
		contract A {
			function f(uint a) mod(2) returns (uint r) {}
			modifier mod(uint a) { return 7; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(state_variable_accessors)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "uint256 public foo;\n"
					   "mapping(uint=>string4) public map;\n"
					   "mapping(uint=>mapping(uint=>string4)) public multiple_map;\n"
					   "}\n";

	ASTPointer<SourceUnit> source;
	ContractDefinition const* contract;
	BOOST_CHECK_NO_THROW(source = parseTextAndResolveNamesWithChecks(text));
	BOOST_REQUIRE((contract = retrieveContract(source, 0)) != nullptr);
	FunctionTypePointer function = retrieveFunctionBySignature(contract, "foo()");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto returnParams = function->getReturnParameterTypeNames();
	BOOST_CHECK_EQUAL(returnParams.at(0), "uint256");
	BOOST_CHECK(function->isConstant());

	function = retrieveFunctionBySignature(contract, "map(uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto params = function->getParameterTypeNames();
	BOOST_CHECK_EQUAL(params.at(0), "uint256");
	returnParams = function->getReturnParameterTypeNames();
	BOOST_CHECK_EQUAL(returnParams.at(0), "string4");
	BOOST_CHECK(function->isConstant());

	function = retrieveFunctionBySignature(contract, "multiple_map(uint256,uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	params = function->getParameterTypeNames();
	BOOST_CHECK_EQUAL(params.at(0), "uint256");
	BOOST_CHECK_EQUAL(params.at(1), "uint256");
	returnParams = function->getReturnParameterTypeNames();
	BOOST_CHECK_EQUAL(returnParams.at(0), "string4");
	BOOST_CHECK(function->isConstant());
}

BOOST_AUTO_TEST_CASE(function_clash_with_state_variable_accessor)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "uint256 foo;\n"
					   "   function foo() {}\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(private_state_variable)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint64(2);\n"
					   "  }\n"
					   "uint256 private foo;\n"
					   "uint256 protected bar;\n"
					   "}\n";

	ASTPointer<SourceUnit> source;
	ContractDefinition const* contract;
	BOOST_CHECK_NO_THROW(source = parseTextAndResolveNamesWithChecks(text));
	BOOST_CHECK((contract = retrieveContract(source, 0)) != nullptr);
	FunctionTypePointer function;
	function = retrieveFunctionBySignature(contract, "foo()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a private variable should not exist");
	function = retrieveFunctionBySignature(contract, "bar()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a protected variable should not exist");
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = R"(
		contract C {
			uint x;
			function() { x = 2; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(fallback_function_with_arguments)
{
	char const* text = R"(
		contract C {
			uint x;
			function(uint a) { x = 2; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function_twice)
{
	char const* text = R"(
		contract C {
			uint x;
			function() { x = 2; }
			function() { x = 3; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(fallback_function_inheritance)
{
	char const* text = R"(
		contract A {
			uint x;
			function() { x = 1; }
		}
		contract C is A {
			function() { x = 2; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, string3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, string3 indexed b, bool indexed c, uint indexed d);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(event_call)
{
	char const* text = R"(
		contract c {
			event e(uint a, string3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(event_inheritance)
{
	char const* text = R"(
		contract base {
			event e(uint a, string3 indexed s, bool indexed b);
		}
		contract c is base {
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(multiple_events_argument_clash)
{
	char const* text = R"(
		contract c {
			event e1(uint a, uint e1, uint e2);
			event e2(uint a, uint e1, uint e2);
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(access_to_default_function_visibility)
{
	char const* text = R"(
		contract c {
			function f() {}
		}
		contract d {
			function g() { c(0).f(); }
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(access_to_protected_function)
{
	char const* text = R"(
		contract c {
			function f() protected {}
		}
		contract d {
			function g() { c(0).f(); }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(access_to_default_state_variable_visibility)
{
	char const* text = R"(
		contract c {
			uint a;
		}
		contract d {
			function g() { c(0).a(); }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(access_to_protected_state_variable)
{
	char const* text = R"(
		contract c {
			uint public a;
		}
		contract d {
			function g() { c(0).a(); }
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(error_count_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1}); }\n"
							 "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(empty_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({}); }\n"
							 "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(duplicate_parameter_names_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1, a: 2}); }\n"
							 "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_parameter_names_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1, c: 2}); }\n"
							 "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter)
{
	char const* text = R"(
		contract test {
			function f(uint){
		}
	})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* text = R"(
		contract test {
			function f() returns(bool){
		}
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f(uint, uint k) returns(uint ret_k){
				return k;
		}
	})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f() returns(uint ret_k, uint){
				return 5;
		}
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(disallow_declaration_of_void_type)
{
	char const* sourceCode = "contract c { function f() { var x = f(); } }";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(overflow_caused_by_ether_units)
{
	char const* sourceCodeFine = R"(
		contract c {
			function c ()
			{
				 a = 115792089237316195423570985008687907853269984665640564039458;
			}
			uint256 a;
		})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCodeFine));
	char const* sourceCode = R"(
		contract c {
			function c ()
			{
				 a = 115792089237316195423570985008687907853269984665640564039458 ether;
			}
			uint256 a;
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(exp_operator_negative_exponent)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns(uint d) { return 2 ** -3; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(exp_operator_exponent_too_big)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns(uint d) { return 2 ** 10000000000; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

