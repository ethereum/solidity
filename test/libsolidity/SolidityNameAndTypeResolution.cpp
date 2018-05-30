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
 * @date 2014
 * Unit tests for the name and type resolution of the solidity parser.
 */

#include <test/libsolidity/AnalysisFramework.h>

#include <test/Options.h>

#include <libsolidity/ast/AST.h>

#include <libdevcore/SHA3.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(SolidityNameAndTypeResolution, AnalysisFramework)

BOOST_AUTO_TEST_CASE(name_references)
{
	char const* text = R"(
		contract test {
			uint256 variable;
			function f(uint256) public returns (uint out) { f(variable); test; out; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(undeclared_name)
{
	char const* text = R"(
		contract test {
			uint256 variable;
			function f(uint256 arg) public {
				f(notfound);
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(undeclared_name_is_not_fatal)
{
	char const* text = R"(
		contract test {
			uint256 variable;
			function f(uint256 arg) public {
				f(notfound);
				f(notfound);
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, DeclarationError, (vector<string>{"Undeclared identifier", "Undeclared identifier"}));
}

BOOST_AUTO_TEST_CASE(reference_to_later_declaration)
{
	char const* text = R"(
		contract test {
			function g() public { f(); }
			function f() public {}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(type_inference_smoke_test)
{
	char const* text = R"(
		contract test {
			function f(uint256 arg1, uint32 arg2) public returns (bool ret) {
				var x = arg1 + arg2 == 8; ret = x;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(type_checking_return)
{
	char const* text = R"(
		contract test {
			function f() public returns (bool r) { return 1 >= 2; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_number)
{
	char const* text = R"(
		contract test {
			function f() public returns (bool r1, bool r2) { return 1 >= 2; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Different number of arguments in return statement than in returns declaration.");
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_type)
{
	char const* text = R"(
		contract test {
			function f() public returns (uint256 r) { return 1 >= 2; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Return argument type bool is not implicitly convertible to expected type (type of first return variable) uint256.");
}

BOOST_AUTO_TEST_CASE(type_checking_function_call)
{
	char const* text = R"(
		contract test {
			function f() public returns (bool) { return g(12, true) == 3; }
			function g(uint256, bool) public returns (uint256) { }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison)
{
	char const* text = R"(
		contract test {
			function f() public { uint32(2) == int64(2); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison_invalid)
{
	char const* text = R"(
		contract test {
			function f() public { int32(2) == uint64(2); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator == not compatible with types int32 and uint64");
}

BOOST_AUTO_TEST_CASE(type_inference_explicit_conversion)
{
	char const* text = R"(
		contract test {
			function f() public returns (int256 r) { var x = int256(uint32(2)); return x; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(large_string_literal)
{
	char const* text = R"(
		contract test {
			function f() public { var x = "123456789012345678901234567890123"; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(balance)
{
	char const* text = R"(
		contract test {
			function fun() public {
				uint256 x = address(0).balance;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(balance_invalid)
{
	char const* text = R"(
		contract test {
			function fun() public {
				address(0).balance = 7;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Expression has to be an lvalue.");
}

BOOST_AUTO_TEST_CASE(assignment_to_mapping)
{
	char const* text = R"(
		contract test {
			struct str {
				mapping(uint=>uint) map;
			}
			str data;
			function fun() public {
				var a = data.map;
				data.map = a;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Mappings cannot be assigned to.");
}

BOOST_AUTO_TEST_CASE(assignment_to_struct)
{
	char const* text = R"(
		contract test {
			struct str {
				mapping(uint=>uint) map;
			}
			str data;
			function fun() public {
				var a = data;
				data = a;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(forward_function_reference)
{
	char const* text = R"(
		contract First {
			function fun() public returns (bool) {
				return Second(1).fun(1, true, 3) > 0;
			}
		}
		contract Second {
			function fun(uint, bool, uint) public returns (uint) {
				if (First(2).fun() == true) return 1;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(comparison_bitop_precedence)
{
	char const* text = R"(
		contract First {
			function fun() public returns (bool ret) {
				return 1 & 2 == 8 & 9 && 1 ^ 2 < 4 | 6;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(comparison_of_function_types_lt_1)
{
	char const* text = R"(
		contract C {
			function f() public returns (bool ret) {
				return this.f < this.f;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator < not compatible");
}

BOOST_AUTO_TEST_CASE(comparison_of_function_types_lt_2)
{
	char const* text = R"(
		contract C {
			function f() public returns (bool ret) {
				return f < f;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator < not compatible");
}

BOOST_AUTO_TEST_CASE(comparison_of_function_types_gt_1)
{
	char const* text = R"(
		contract C {
			function f() public returns (bool ret) {
				return this.f > this.f;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator > not compatible");
}

BOOST_AUTO_TEST_CASE(comparison_of_function_types_gt_2)
{
	char const* text = R"(
		contract C {
			function f() public returns (bool ret) {
				return f > f;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator > not compatible");
}

BOOST_AUTO_TEST_CASE(comparison_of_function_types_eq)
{
	char const* text = R"(
		contract C {
			function f() public returns (bool ret) {
				return f == f;
			}
			function g() public returns (bool ret) {
				return f != f;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(comparison_of_mapping_types)
{
	char const* text = R"(
		contract C {
			mapping(uint => uint) x;
			function f() public returns (bool ret) {
				var y = x;
				return x == y;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator == not compatible");
}

BOOST_AUTO_TEST_CASE(function_no_implementation)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract test {
			function functionName(bytes32 input) public returns (bytes32 out);
		}
	)";
	sourceUnit = parseAndAnalyse(text);
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* contract = dynamic_cast<ContractDefinition*>(nodes[1].get());
	BOOST_REQUIRE(contract);
	BOOST_CHECK(!contract->annotation().unimplementedFunctions.empty());
	BOOST_CHECK(!contract->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() public {} }
	)";
	sourceUnit = parseAndAnalyse(text);
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().unimplementedFunctions.empty());
	BOOST_CHECK(!base->definedFunctions()[0]->isImplemented());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(derived->annotation().unimplementedFunctions.empty());
	BOOST_CHECK(derived->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract_with_overload)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract base { function foo(bool); }
		contract derived is base { function foo(uint) public {} }
	)";
	sourceUnit = parseAndAnalyse(text);
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().unimplementedFunctions.empty());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().unimplementedFunctions.empty());
}

BOOST_AUTO_TEST_CASE(create_abstract_contract)
{
	char const* text = R"(
		contract base { function foo(); }
		contract derived {
			base b;
			function foo() public { b = new base(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Trying to create an instance of an abstract contract.");
}

BOOST_AUTO_TEST_CASE(redeclare_implemented_abstract_function_as_abstract)
{
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() public {} }
		contract wrong is derived { function foo(); }
	)";
	CHECK_ERROR(text, TypeError, "Redeclaring an already implemented function as abstract");
}

BOOST_AUTO_TEST_CASE(implement_abstract_via_constructor)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract base { function foo(); }
		contract foo is base { function foo() public {} }
	)";
	sourceUnit = parseAndAnalyse(text);
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	BOOST_CHECK_EQUAL(nodes.size(), 3);
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().unimplementedFunctions.empty());
}

BOOST_AUTO_TEST_CASE(function_canonical_signature)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract Test {
			function foo(uint256 arg1, uint64 arg2, bool arg3) public returns (uint256 ret) {
				ret = arg1 + arg2;
			}
		}
	)";
	sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_CHECK_EQUAL("foo(uint256,uint64,bool)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_canonical_signature_type_aliases)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract Test {
			function boo(uint, bytes32, address) public returns (uint ret) {
				ret = 5;
			}
		}
	)";
	sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint256,bytes32,address)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_external_types)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			function boo(uint, bool, bytes8, bool[2], uint[], C, address[]) external returns (uint ret) {
				ret = 5;
			}
		}
	)";
	sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint256,bool,bytes8,bool[2],uint256[],address,address[])", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(enum_external_type)
{
	SourceUnit const* sourceUnit = nullptr;
	char const* text = R"(
		// test for bug #1801
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function boo(ActionChoices enumArg) external returns (uint ret) {
				ret = 5;
			}
		}
	)";
	sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint8)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(external_structs)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			struct Empty {}
			struct Nested { X[2][] a; uint y; }
			struct X { bytes32 x; Test t; Empty[] e; }
			function f(ActionChoices, uint, Empty) external {}
			function g(Test, Nested) external {}
			function h(function(Nested) external returns (uint)[]) external {}
			function i(Nested[]) external {}
		}
	)";
	SourceUnit const* sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(uint8,uint256,())", functions[0]->externalSignature());
			BOOST_CHECK_EQUAL("g(address,((bytes32,address,()[])[2][],uint256))", functions[1]->externalSignature());
			BOOST_CHECK_EQUAL("h(function[])", functions[2]->externalSignature());
			BOOST_CHECK_EQUAL("i(((bytes32,address,()[])[2][],uint256)[])", functions[3]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(external_structs_in_libraries)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;
		library Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			struct Empty {}
			struct Nested { X[2][] a; uint y; }
			struct X { bytes32 x; Test t; Empty[] e; }
			function f(ActionChoices, uint, Empty) external {}
			function g(Test, Nested) external {}
			function h(function(Nested) external returns (uint)[]) external {}
			function i(Nested[]) external {}
		}
	)";
	SourceUnit const* sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(Test.ActionChoices,uint256,Test.Empty)", functions[0]->externalSignature());
			BOOST_CHECK_EQUAL("g(Test,Test.Nested)", functions[1]->externalSignature());
			BOOST_CHECK_EQUAL("h(function[])", functions[2]->externalSignature());
			BOOST_CHECK_EQUAL("i(Test.Nested[])", functions[3]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(struct_with_mapping_in_library)
{
	char const* text = R"(
		library Test {
			struct Nested { mapping(uint => uint)[2][] a; uint y; }
			struct X { Nested n; }
			function f(X storage x) external {}
		}
	)";
	SourceUnit const* sourceUnit = parseAndAnalyse(text);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(Test.X storage)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(functions_with_identical_structs_in_interface)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct S1 { }
			struct S2 { }
			function f(S1) pure {}
			function f(S2) pure {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Function overload clash during conversion to external types for arguments");
}

BOOST_AUTO_TEST_CASE(functions_with_different_structs_in_interface)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct S1 { function() external a; }
			struct S2 { bytes24 a; }
			function f(S1) pure {}
			function f(S2) pure {}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(functions_with_stucts_of_non_external_types_in_interface)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct S { function() internal a; }
			function f(S) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal or recursive type is not allowed for public or external functions.");
}

BOOST_AUTO_TEST_CASE(functions_with_stucts_of_non_external_types_in_interface_2)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct S { mapping(uint => uint) a; }
			function f(S) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal or recursive type is not allowed for public or external functions.");
}

BOOST_AUTO_TEST_CASE(functions_with_stucts_of_non_external_types_in_interface_nested)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct T { mapping(uint => uint) a; }
			struct S { T[][2] b; }
			function f(S) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal or recursive type is not allowed for public or external functions.");
}

BOOST_AUTO_TEST_CASE(returning_multi_dimensional_arrays_new_abi)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			function f() public pure returns (string[][]) {}
		}
	)";
	CHECK_WARNING(text, "Experimental features");
}

BOOST_AUTO_TEST_CASE(returning_multi_dimensional_arrays)
{
	char const* text = R"(
		contract C {
			function f() public pure returns (string[][]) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "only supported in the new experimental ABI encoder");
}

BOOST_AUTO_TEST_CASE(returning_multi_dimensional_static_arrays)
{
	char const* text = R"(
		contract C {
			function f() public pure returns (uint[][2]) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "only supported in the new experimental ABI encoder");
}

BOOST_AUTO_TEST_CASE(returning_arrays_in_structs_new_abi)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;

		contract C {
			struct S { string[] s; }
			function f() public pure returns (S) {}
		}
	)";
	CHECK_WARNING(text, "Experimental features");
}

BOOST_AUTO_TEST_CASE(returning_arrays_in_structs_arrays)
{
	char const* text = R"(
		contract C {
			struct S { string[] s; }
			function f() public pure returns (S x) {}
		}
	)";
	CHECK_ERROR(text, TypeError, "only supported in the new experimental ABI encoder");
}

BOOST_AUTO_TEST_CASE(function_external_call_allowed_conversion)
{
	char const* text = R"(
		contract C {}
		contract Test {
			function externalCall() public {
				C arg;
				this.g(arg);
			}
			function g (C c) external {}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(function_external_call_not_allowed_conversion)
{
	char const* text = R"(
		contract C {}
		contract Test {
			function externalCall() public {
				address arg;
				this.g(arg);
			}
			function g (C c) external {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid type for argument in function call. Invalid implicit conversion from address to contract C requested.");
}

BOOST_AUTO_TEST_CASE(function_internal_allowed_conversion)
{
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			C a;
			function g (C c) public {}
			function internalCall() public {
				g(a);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(function_internal_not_allowed_conversion)
{
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			address a;
			function g (C c) public {}
			function internalCall() public {
				g(a);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid type for argument in function call. Invalid implicit conversion from address to contract C requested.");
}

BOOST_AUTO_TEST_CASE(hash_collision_in_interface)
{
	char const* text = R"(
		contract test {
			function gsf() public { }
			function tgeo() public { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Function signature hash collision for tgeo()");
}

BOOST_AUTO_TEST_CASE(inheritance_basic)
{
	char const* text = R"(
		contract base { uint baseMember; struct BaseType { uint element; } }
		contract derived is base {
			BaseType data;
			function f() public { baseMember = 7; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inheritance_diamond_basic)
{
	char const* text = R"(
		contract root { function rootFunction() public {} }
		contract inter1 is root { function f() public {} }
		contract inter2 is root { function f() public {} }
		contract derived is root, inter2, inter1 {
			function g() public { f(); rootFunction(); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(cyclic_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B is A { }
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (vector<string>{"Definition of base has to precede definition of derived contract"}));
}

BOOST_AUTO_TEST_CASE(legal_override_direct)
{
	char const* text = R"(
		contract B { function f() public {} }
		contract C is B { function f(uint i) public {} }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(legal_override_indirect)
{
	char const* text = R"(
		contract A { function f(uint a) public {} }
		contract B { function f() public {} }
		contract C is A, B { }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(illegal_override_visibility)
{
	char const* text = R"(
		contract B { function f() internal {} }
		contract C is B { function f() public {} }
	)";
	CHECK_ERROR(text, TypeError, "Overriding function visibility differs");
}

BOOST_AUTO_TEST_CASE(complex_inheritance)
{
	char const* text = R"(
		contract A { function f() public { uint8 x = C(0).g(); } }
		contract B { function f() public {} function g() public returns (uint8) {} }
		contract C is A, B { }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(missing_base_constructor_arguments)
{
	char const* text = R"(
		contract A { function A(uint a) public { } }
		contract B is A { }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(base_constructor_arguments_override)
{
	char const* text = R"(
		contract A { function A(uint a) public { } }
		contract B is A { }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(implicit_derived_to_base_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() public { A a = B(1); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(implicit_base_to_derived_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() public { B b = A(1); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Type contract A is not implicitly convertible to expected type contract B.");
}

BOOST_AUTO_TEST_CASE(super_excludes_current_contract)
{
	char const* text = R"(
		contract A {
			function b() public {}
		}

		contract B is A {
			function f() public {
				super.f();
			}
		}
	)";

	CHECK_ERROR(text, TypeError, "Member \"f\" not found or not visible after argument-dependent lookup in contract super B");
}

BOOST_AUTO_TEST_CASE(state_variable_accessors)
{
	char const* text = R"(
		contract test {
			function fun() public {
				uint64(2);
			}
			uint256 public foo;
			mapping(uint=>bytes4) public map;
			mapping(uint=>mapping(uint=>bytes4)) public multiple_map;
		}
	)";

	SourceUnit const* source;
	ContractDefinition const* contract;
	source = parseAndAnalyse(text);
	BOOST_REQUIRE((contract = retrieveContractByName(*source, "test")) != nullptr);
	FunctionTypePointer function = retrieveFunctionBySignature(*contract, "foo()");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto returnParams = function->returnParameterTypes();
	BOOST_CHECK_EQUAL(returnParams.at(0)->canonicalName(), "uint256");
	BOOST_CHECK(function->stateMutability() == StateMutability::View);

	function = retrieveFunctionBySignature(*contract, "map(uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto params = function->parameterTypes();
	BOOST_CHECK_EQUAL(params.at(0)->canonicalName(), "uint256");
	returnParams = function->returnParameterTypes();
	BOOST_CHECK_EQUAL(returnParams.at(0)->canonicalName(), "bytes4");
	BOOST_CHECK(function->stateMutability() == StateMutability::View);

	function = retrieveFunctionBySignature(*contract, "multiple_map(uint256,uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	params = function->parameterTypes();
	BOOST_CHECK_EQUAL(params.at(0)->canonicalName(), "uint256");
	BOOST_CHECK_EQUAL(params.at(1)->canonicalName(), "uint256");
	returnParams = function->returnParameterTypes();
	BOOST_CHECK_EQUAL(returnParams.at(0)->canonicalName(), "bytes4");
	BOOST_CHECK(function->stateMutability() == StateMutability::View);
}

BOOST_AUTO_TEST_CASE(function_clash_with_state_variable_accessor)
{
	char const* text = R"(
		contract test {
			function fun() public {
				uint64(2);
			}
			uint256 foo;
			function foo() public {}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(private_state_variable)
{
	char const* text = R"(
		contract test {
			function fun() public {
				uint64(2);
			}
			uint256 private foo;
			uint256 internal bar;
		}
	)";

	ContractDefinition const* contract;
	SourceUnit const* source = parseAndAnalyse(text);
	BOOST_CHECK((contract = retrieveContractByName(*source, "test")) != nullptr);
	FunctionTypePointer function;
	function = retrieveFunctionBySignature(*contract, "foo()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a private variable should not exist");
	function = retrieveFunctionBySignature(*contract, "bar()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of an internal variable should not exist");
}

BOOST_AUTO_TEST_CASE(base_class_state_variable_accessor)
{
	char const* text = R"(
		// test for issue #1126 https://github.com/ethereum/cpp-ethereum/issues/1126
		contract Parent {
			uint256 public m_aMember;
		}
		contract Child is Parent {
			function foo() public returns (uint256) { return Parent.m_aMember; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(struct_accessor_one_array_only)
{
	char const* sourceCode = R"(
		contract test {
			struct Data { uint[15] m_array; }
			Data public data;
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Internal or recursive type is not allowed for public state variables.");
}

BOOST_AUTO_TEST_CASE(base_class_state_variable_internal_member)
{
	char const* text = R"(
		contract Parent {
			uint256 internal m_aMember;
		}
		contract Child is Parent {
			function foo() public returns (uint256) { return Parent.m_aMember; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(state_variable_member_of_wrong_class1)
{
	char const* text = R"(
		contract Parent1 {
			uint256 internal m_aMember1;
		}
		contract Parent2 is Parent1 {
			uint256 internal m_aMember2;
		}
		contract Child is Parent2 {
			function foo() public returns (uint256) { return Parent2.m_aMember1; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"m_aMember1\" not found or not visible after argument-dependent lookup in type(contract Parent2)");
}

BOOST_AUTO_TEST_CASE(state_variable_member_of_wrong_class2)
{
	char const* text = R"(
		contract Parent1 {
			uint256 internal m_aMember1;
		}
		contract Parent2 is Parent1 {
			uint256 internal m_aMember2;
		}
		contract Child is Parent2 {
			function foo() public returns (uint256) { return Child.m_aMember2; }
			uint256 public m_aMember3;
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"m_aMember2\" not found or not visible after argument-dependent lookup in type(contract Child)");
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = R"(
		contract C {
			uint x;
			function() public { x = 2; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fallback_function_with_arguments)
{
	char const* text = R"(
		contract C {
			uint x;
			function(uint a) public { x = 2; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Fallback function cannot take parameters.");
}

BOOST_AUTO_TEST_CASE(fallback_function_in_library)
{
	char const* text = R"(
		library C {
			function() public {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Libraries cannot have fallback functions.");
}

BOOST_AUTO_TEST_CASE(fallback_function_with_return_parameters)
{
	char const* text = R"(
		contract C {
			function() public returns (uint) { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Fallback function cannot return values.");
}

BOOST_AUTO_TEST_CASE(fallback_function_twice)
{
	char const* text = R"(
		contract C {
			uint x;
			function() public { x = 2; }
			function() public { x = 3; }
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, DeclarationError, (vector<string>{
		"Only one fallback function is"
	}));
}

BOOST_AUTO_TEST_CASE(fallback_function_inheritance)
{
	char const* text = R"(
		contract A {
			uint x;
			function() public { x = 1; }
		}
		contract C is A {
			function() public { x = 2; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed s, bool indexed b);
			function f() public { e(2, "abc", true); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d);
		}
	)";
	CHECK_ERROR(text, TypeError, "More than 3 indexed arguments for event.");
}

BOOST_AUTO_TEST_CASE(anonymous_event_four_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d) anonymous;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(anonymous_event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d, uint indexed e) anonymous;
		}
	)";
	CHECK_ERROR(text, TypeError, "More than 4 indexed arguments for anonymous event.");
}

BOOST_AUTO_TEST_CASE(events_with_same_name)
{
	char const* text = R"(
		contract TestIt {
			event A();
			event A(uint i);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(events_with_same_name_unnamed_arguments)
{
	char const* text = R"(
		contract test {
			event A(uint);
			event A(uint, uint);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(events_with_same_name_different_types)
{
	char const* text = R"(
		contract test {
			event A(uint);
			event A(bytes);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(double_event_declaration)
{
	char const* text = R"(
		contract test {
			event A(uint i);
			event A(uint i);
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Event with same name and arguments defined twice.");
}

BOOST_AUTO_TEST_CASE(double_event_declaration_ignores_anonymous)
{
	char const* text = R"(
		contract test {
			event A(uint i);
			event A(uint i) anonymous;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Event with same name and arguments defined twice.");
}

BOOST_AUTO_TEST_CASE(double_event_declaration_ignores_indexed)
{
	char const* text = R"(
		contract test {
			event A(uint i);
			event A(uint indexed i);
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Event with same name and arguments defined twice.");
}

BOOST_AUTO_TEST_CASE(event_call)
{
	char const* text = R"(
		contract c {
			event e(uint a, bytes3 indexed s, bool indexed b);
			function f() public { e(2, "abc", true); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(event_function_inheritance_clash)
{
	char const* text = R"(
		contract A {
			function dup() public returns (uint) {
				return 1;
			}
		}
		contract B {
			event dup();
		}
		contract C is A, B {
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(function_event_inheritance_clash)
{
	char const* text = R"(
		contract B {
			event dup();
		}
		contract A {
			function dup() public returns (uint) {
				return 1;
			}
		}
		contract C is B, A {
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(function_event_in_contract_clash)
{
	char const* text = R"(
		contract A {
			event dup();
			function dup() public returns (uint) {
				return 1;
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(event_inheritance)
{
	char const* text = R"(
		contract base {
			event e(uint a, bytes3 indexed s, bool indexed b);
		}
		contract c is base {
			function f() public { e(2, "abc", true); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(multiple_events_argument_clash)
{
	char const* text = R"(
		contract c {
			event e1(uint a, uint e1, uint e2);
			event e2(uint a, uint e1, uint e2);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(access_to_default_function_visibility)
{
	char const* text = R"(
		contract c {
			function f() public {}
		}
		contract d {
			function g() public { c(0).f(); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(access_to_internal_function)
{
	char const* text = R"(
		contract c {
			function f() internal {}
		}
		contract d {
			function g() public { c(0).f(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"f\" not found or not visible after argument-dependent lookup in contract c");
}

BOOST_AUTO_TEST_CASE(access_to_default_state_variable_visibility)
{
	char const* text = R"(
		contract c {
			uint a;
		}
		contract d {
			function g() public { c(0).a(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"a\" not found or not visible after argument-dependent lookup in contract c");
}

BOOST_AUTO_TEST_CASE(access_to_internal_state_variable)
{
	char const* text = R"(
		contract c {
			uint public a;
		}
		contract d {
			function g() public { c(0).a(); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(error_count_in_named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b) public returns (uint r) {
				r = a + b;
			}
			function b() public returns (uint r) {
				r = a({a: 1});
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Wrong argument count for function call: 1 arguments given but expected 2.");
}

BOOST_AUTO_TEST_CASE(empty_in_named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b) public returns (uint r) {
				r = a + b;
			}
			function b() public returns (uint r) {
				r = a({});
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Wrong argument count for function call: 0 arguments given but expected 2.");
}

BOOST_AUTO_TEST_CASE(duplicate_parameter_names_in_named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b) public returns (uint r) {
				r = a + b;
			}
			function b() public returns (uint r) {
				r = a({a: 1, a: 2});
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Duplicate named argument.");
}

BOOST_AUTO_TEST_CASE(invalid_parameter_names_in_named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b) public returns (uint r) {
				r = a + b;
			}
			function b() public returns (uint r) {
				r = a({a: 1, c: 2});
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Named argument does not match function declaration.");
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter)
{
	char const* text = R"(
		contract test {
			function f(uint) public { }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(constant_input_parameter)
{
	char const* text = R"(
		contract test {
			function f(uint[] constant a) public { }
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (vector<string>{
		"Illegal use of \"constant\" specifier",
		"Constants of non-value type not yet implemented",
		"Uninitialized \"constant\" variable"
	}));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* text = R"(
		contract test {
			function f() public returns (bool) { }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f(uint, uint k) public returns (uint ret_k) {
				return k;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f() public returns (uint ret_k, uint) {
				return 5;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Different number of arguments in return statement than in returns declaration.");
}

BOOST_AUTO_TEST_CASE(disallow_declaration_of_void_type)
{
	char const* sourceCode = R"(
		contract c {
			function f() public { var (x) = f(); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Not enough components (0) in value to assign all variables (1).");
}

BOOST_AUTO_TEST_CASE(no_overflow_with_large_literal)
{
	char const* text = R"(
		contract c {
			function c () public {
				a = 115792089237316195423570985008687907853269984665640564039458;
			}
			uint256 a;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(overflow_caused_by_ether_units)
{
	char const* text = R"(
		contract c {
			function c () public {
				a = 115792089237316195423570985008687907853269984665640564039458 ether;
			}
			uint256 a;
		}
	)";
	CHECK_ERROR(text, TypeError, "Type int_const 1157...(70 digits omitted)...0000 is not implicitly convertible to expected type uint256.");
}

BOOST_AUTO_TEST_CASE(exp_operator_exponent_too_big)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (uint d) { return 2 ** 10000000000; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator ** not compatible with types int_const 2 and int_const 10000000000");
}

BOOST_AUTO_TEST_CASE(exp_warn_literal_base_1)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				uint8 x = 100;
				return 10**x;
			}
		}
	)";
	CHECK_WARNING(sourceCode, "might overflow");
}

BOOST_AUTO_TEST_CASE(exp_warn_literal_base_2)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				uint8 x = 100;
				return uint8(10)**x;
			}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(exp_warn_literal_base_3)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				return 2**80;
			}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(shift_warn_literal_base_1)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				uint8 x = 100;
				return 10 << x;
			}
		}
	)";
	CHECK_WARNING(sourceCode, "might overflow");
}

BOOST_AUTO_TEST_CASE(shift_warn_literal_base_2)
{
	const char* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				uint8 x = 100;
				return uint8(10) << x;
			}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(shift_warn_literal_base_3)
{
	const char* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				return 2 << 80;
			}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(shift_warn_literal_base_4)
{
	const char* sourceCode = R"(
		contract test {
			function f() pure public returns(uint) {
				 uint8 x = 100;
				 return 10 >> x;
			}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(warn_var_from_uint8)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns (uint) {
				var i = 1;
				return i;
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(sourceCode, (std::vector<std::string>{
		"uint8, which can hold values between 0 and 255",
		"Use of the \"var\" keyword is deprecated."
	}));
}

BOOST_AUTO_TEST_CASE(warn_var_from_uint256)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public {
				var i = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
				i;
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(sourceCode, (std::vector<std::string>{
		"uint256, which can hold values between 0 and 115792089237316195423570985008687907853269984665640564039457584007913129639935",
		"Use of the \"var\" keyword is deprecated."
	}));
}

BOOST_AUTO_TEST_CASE(warn_var_from_int8)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public {
				var i = -2;
				i;
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(sourceCode, (std::vector<std::string>{
		"int8, which can hold values between -128 and 127",
		"Use of the \"var\" keyword is deprecated."
	}));
}

BOOST_AUTO_TEST_CASE(warn_var_from_zero)
{
	char const* sourceCode = R"(
		 contract test {
			 function f() pure public {
				 for (var i = 0; i < msg.data.length; i++) { }
			 }
		 }
	)";
	CHECK_WARNING_ALLOW_MULTI(sourceCode, (std::vector<std::string>{
		"uint8, which can hold",
		"Use of the \"var\" keyword is deprecated."
	}));
}

BOOST_AUTO_TEST_CASE(enum_member_access)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test()
			{
				choices = ActionChoices.GoStraight;
			}
			ActionChoices choices;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(enum_member_access_accross_contracts)
{
	char const* text = R"(
		contract Interface {
			enum MyEnum { One, Two }
		}
		contract Impl {
			function test() public returns (Interface.MyEnum) {
				return Interface.MyEnum.One;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(enum_invalid_member_access)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				choices = ActionChoices.RunAroundWavingYourHands;
			}
			ActionChoices choices;
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"RunAroundWavingYourHands\" not found or not visible after argument-dependent lookup in type(enum test.ActionChoices)");
}

BOOST_AUTO_TEST_CASE(enum_invalid_direct_member_access)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				choices = Sit;
			}
			ActionChoices choices;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(enum_explicit_conversion_is_okay)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				a = uint256(ActionChoices.GoStraight);
				b = uint64(ActionChoices.Sit);
			}
			uint256 a;
			uint64 b;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(int_to_enum_explicit_conversion_is_okay)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				a = 2;
				b = ActionChoices(a);
			}
			uint256 a;
			ActionChoices b;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(enum_implicit_conversion_is_not_okay_256)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				a = ActionChoices.GoStraight;
			}
			uint256 a;
		}
	)";
	CHECK_ERROR(text, TypeError, "Type enum test.ActionChoices is not implicitly convertible to expected type uint256.");
}

BOOST_AUTO_TEST_CASE(enum_implicit_conversion_is_not_okay_64)
{
	char const* text = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test() public {
				b = ActionChoices.Sit;
			}
			uint64 b;
		}
	)";
	CHECK_ERROR(text, TypeError, "Type enum test.ActionChoices is not implicitly convertible to expected type uint64.");
}

BOOST_AUTO_TEST_CASE(enum_to_enum_conversion_is_not_okay)
{
	char const* text = R"(
		contract test {
			enum Paper { Up, Down, Left, Right }
			enum Ground { North, South, West, East }
			function test() public {
				Ground(Paper.Up);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Explicit type conversion not allowed from \"enum test.Paper\" to \"enum test.Ground\".");
}

BOOST_AUTO_TEST_CASE(enum_duplicate_values)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoLeft, Sit }
			}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(enum_name_resolution_under_current_contract_name)
{
	char const* text = R"(
		contract A {
			enum Foo {
				First,
				Second
			}

			function a() public {
				A.Foo;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(private_visibility)
{
	char const* sourceCode = R"(
		contract base {
			function f() private {}
		}
		contract derived is base {
			function g() public { f(); }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(private_visibility_via_explicit_base_access)
{
	char const* sourceCode = R"(
		contract base {
			function f() private {}
		}
		contract derived is base {
			function g() public { base.f(); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Member \"f\" not found or not visible after argument-dependent lookup in type(contract base)");
}

BOOST_AUTO_TEST_CASE(external_visibility)
{
	char const* sourceCode = R"(
		contract c {
			function f() external {}
			function g() public { f(); }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(similar_name_suggestions_expected)
{
	char const* sourceCode = R"(
		contract c {
			function func() {}
			function g() public { fun(); }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier. Did you mean \"func\"?");
}

BOOST_AUTO_TEST_CASE(no_name_suggestion)
{
	char const* sourceCode = R"(
		contract c {
			function g() public { fun(); }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(multiple_similar_suggestions)
{
	char const* sourceCode = R"(
		contract c {
			function g() public {
				uint var1 = 1;
				uint var2 = 1;
				uint var3 = 1;
				uint var4 = 1;
				uint var5 = varx;
			}
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier. Did you mean \"var1\", \"var2\", \"var3\", \"var4\" or \"var5\"?");
}

BOOST_AUTO_TEST_CASE(multiple_scopes_suggestions)
{
	char const* sourceCode = R"(
		contract c {
			uint log9 = 2;
			function g() public {
				uint log8 = 3;
				uint var1 = lgox;
			}
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier. Did you mean \"log8\", \"log9\", \"log0\", \"log1\", \"log2\", \"log3\" or \"log4\"?");
}

BOOST_AUTO_TEST_CASE(inheritence_suggestions)
{
	char const* sourceCode = R"(
		contract a { function func() public {} }
		contract c is a {
			function g() public {
				uint var1 = fun();
			}
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier. Did you mean \"func\"?");
}

BOOST_AUTO_TEST_CASE(no_spurious_identifier_suggestions_with_submatch)
{
	char const* sourceCode = R"(
		contract c {
			function g() public {
				uint va = 1;
				uint vb = vaxyz;
			 }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(no_spurious_identifier_suggestions)
{
	char const* sourceCode = R"(
		contract c {
			function g() public {
				uint va = 1;
				uint vb = x;
			 }
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Undeclared identifier.");
}

BOOST_AUTO_TEST_CASE(external_base_visibility)
{
	char const* sourceCode = R"(
		contract base {
			function f() external {}
		}
		contract derived is base {
			function g() public { base.f(); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Member \"f\" not found or not visible after argument-dependent lookup in type(contract base)");
}

BOOST_AUTO_TEST_CASE(external_argument_assign)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a = 1; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Expression has to be an lvalue.");
}

BOOST_AUTO_TEST_CASE(external_argument_increment)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a++; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Expression has to be an lvalue.");
}

BOOST_AUTO_TEST_CASE(external_argument_delete)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { delete a; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Expression has to be an lvalue.");
}

BOOST_AUTO_TEST_CASE(test_for_bug_override_function_with_bytearray_type)
{
	char const* sourceCode = R"(
		contract Vehicle {
			function f(bytes) external returns (uint256 r) {r = 1;}
		}
		contract Bike is Vehicle {
			function f(bytes) external returns (uint256 r) {r = 42;}
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(array_with_nonconstant_length)
{
	char const* text = R"(
		contract c {
			function f(uint a) public { uint8[a] x; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(array_with_negative_length)
{
	char const* text = R"(
		contract c {
			function f(uint a) public { uint8[-1] x; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Array with negative length specified");
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types1)
{
	char const* text = R"(
		contract c {
			bytes a;
			uint[] b;
			function f() public { b = a; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Type bytes storage ref is not implicitly convertible to expected type uint256[] storage ref.");
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types2)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() public { b = a; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Type uint32[] storage ref is not implicitly convertible to expected type uint8[] storage ref.");
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_conversion_possible)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() public { a = b; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_static_dynamic)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[80] b;
			function f() public { a = b; }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_dynamic_static)
{
	char const* text = R"(
		contract c {
			uint[] a;
			uint[80] b;
			function f() public { b = a; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Type uint256[] storage ref is not implicitly convertible to expected type uint256[80] storage ref.");
}

BOOST_AUTO_TEST_CASE(array_of_undeclared_type)
{
	char const* text = R"(
		contract c {
			a[] public foo;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found or not unique.");
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_int)
{
	char const* text = R"(
		contract c {
			uint8 a = 1000;
		}
	)";
	CHECK_ERROR(text, TypeError, "Type int_const 1000 is not implicitly convertible to expected type uint8.");
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_string)
{
	char const* text = R"(
		contract c {
			uint a = "abc";
		}
	)";
	CHECK_ERROR(text, TypeError, "Type literal_string \"abc\" is not implicitly convertible to expected type uint256.");
}

BOOST_AUTO_TEST_CASE(test_byte_is_alias_of_byte1)
{
	char const* text = R"(
		contract c {
			bytes arr;
			function f() public { byte a = arr[0];}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(warns_assigning_decimal_to_bytesxx)
{
	char const* text = R"(
		contract Foo {
			bytes32 a = 7;
		}
	)";
	CHECK_WARNING(text, "Decimal literal assigned to bytesXX variable will be left-aligned.");
}

BOOST_AUTO_TEST_CASE(does_not_warn_assigning_hex_number_to_bytesxx)
{
	char const* text = R"(
		contract Foo {
			bytes32 a = 0x1234;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(explicit_conversion_from_decimal_to_bytesxx)
{
	char const* text = R"(
		contract Foo {
			bytes32 a = bytes32(7);
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(assigning_value_to_const_variable)
{
	char const* text = R"(
		contract Foo {
			function changeIt() public { x = 9; }
			uint constant x = 56;
		}
	)";
	CHECK_ERROR(text, TypeError, "Cannot assign to a constant variable.");
}

BOOST_AUTO_TEST_CASE(assigning_state_to_const_variable)
{
	char const* text = R"(
		contract C {
			address constant x = msg.sender;
		}
	)";
	CHECK_WARNING(text, "Initial value for constant variable has to be compile-time constant.");
}

BOOST_AUTO_TEST_CASE(assigning_state_to_const_variable_050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";

		contract C {
			address constant x = msg.sender;
		}
	)";
	CHECK_ERROR(text, TypeError, "Initial value for constant variable has to be compile-time constant.");
}

BOOST_AUTO_TEST_CASE(constant_string_literal_disallows_assignment)
{
	char const* text = R"(
		contract Test {
			string constant x = "abefghijklmnopqabcdefghijklmnopqabcdefghijklmnopqabca";
			function f() public {
				// Even if this is made possible in the future, we should not allow assignment
				// to elements of constant arrays.
				x[0] = "f";
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Index access for string is not possible.");
}

BOOST_AUTO_TEST_CASE(assignment_to_const_var_involving_conversion)
{
	char const* text = R"(
		contract C {
			C constant x = C(0x123);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(assignment_to_const_var_involving_expression)
{
	char const* text = R"(
		contract C {
			uint constant x = 0x123 + 0x456;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(assignment_to_const_var_involving_keccak)
{
	char const* text = R"(
		contract C {
			bytes32 constant x = keccak256("abc");
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(assignment_to_const_array_vars)
{
	char const* text = R"(
		contract C {
			uint[3] constant x = [uint(1), 2, 3];
		}
	)";
	CHECK_ERROR(text, TypeError, "implemented");
}

BOOST_AUTO_TEST_CASE(assignment_to_const_string_bytes)
{
	char const* text = R"(
		contract C {
			bytes constant a = "\x00\x01\x02";
			bytes constant b = hex"000102";
			string constant c = "hello";
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(constant_struct)
{
	char const* text = R"(
		contract C {
			struct S { uint x; uint[] y; }
			S constant x = S(5, new uint[](4));
		}
	)";
	CHECK_ERROR(text, TypeError, "implemented");
}

BOOST_AUTO_TEST_CASE(address_is_constant)
{
	char const* text = R"(
		contract C {
			address constant x = 0x1212121212121212121212121212121212121212;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(uninitialized_const_variable)
{
	char const* text = R"(
		contract Foo {
			uint constant y;
		}
	)";
	CHECK_ERROR(text, TypeError, "Uninitialized \"constant\" variable.");
}

BOOST_AUTO_TEST_CASE(overloaded_function_cannot_resolve)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (uint) { return 1; }
			function f(uint a) public returns (uint) { return a; }
			function g() public returns (uint) { return f(3, 5); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "No matching declaration found after argument-dependent lookup.");
}

BOOST_AUTO_TEST_CASE(ambiguous_overloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint8 a) public returns (uint) { return a; }
			function f(uint a) public returns (uint) { return 2 * a; }
			// literal 1 can be both converted to uint and uint8, so the call is ambiguous.
			function g() public returns (uint) { return f(1); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "No unique declaration found after argument-dependent lookup.");
}

BOOST_AUTO_TEST_CASE(assignment_of_nonoverloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint) { return 2 * a; }
			function g() public returns (uint) { var x = f; return x(7); }
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(assignment_of_overloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (uint) { return 1; }
			function f(uint a) public returns (uint) { return 2 * a; }
			function g() public returns (uint) { var x = f; return x(7); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "No matching declaration found after variable lookup.");
}

BOOST_AUTO_TEST_CASE(external_types_clash)
{
	char const* sourceCode = R"(
		contract base {
			enum a { X }
			function f(a) public { }
		}
		contract test is base {
			function f(uint8 a) public { }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Function overload clash during conversion to external types for arguments.");
}

BOOST_AUTO_TEST_CASE(override_changes_return_types)
{
	char const* sourceCode = R"(
		contract base {
			function f(uint a) public returns (uint) { }
		}
		contract test is base {
			function f(uint a) public returns (uint8) { }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Overriding function return types differ");
}

BOOST_AUTO_TEST_CASE(equal_overload)
{
	char const* sourceCode = R"(
		contract C {
			function test(uint a) public returns (uint b) { }
			function test(uint a) external {}
		}
	)";
	CHECK_ALLOW_MULTI(sourceCode, (vector<pair<Error::Type, string>>{
		{Error::Type::DeclarationError, "Function with same name and arguments defined twice."},
		{Error::Type::TypeError, "Overriding function visibility differs"}
	}));
}

BOOST_AUTO_TEST_CASE(uninitialized_var)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) { var x; return 2; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Assignment necessary for type detection.");
}

BOOST_AUTO_TEST_CASE(string)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f(string x) external { s = x; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(invalid_utf8_implicit)
{
	char const* sourceCode = R"(
		contract C {
			string s = "\xa0\x00";
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "invalid UTF-8");
}

BOOST_AUTO_TEST_CASE(invalid_utf8_explicit)
{
	char const* sourceCode = R"(
		contract C {
			string s = string("\xa0\x00");
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Explicit type conversion not allowed");
}

BOOST_AUTO_TEST_CASE(large_utf8_codepoint)
{
	char const* sourceCode = R"(
		contract C {
			string s = "\xf0\x9f\xa6\x84";
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(string_index)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() public { var a = s[2]; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Index access for string is not possible.");
}

BOOST_AUTO_TEST_CASE(string_length)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() public { var a = s.length; }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Member \"length\" not found or not visible after argument-dependent lookup in string storage ref");
}

BOOST_AUTO_TEST_CASE(negative_integers_to_signed_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			int8 public i = -129;
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type int_const -129 is not implicitly convertible to expected type int8.");
}

BOOST_AUTO_TEST_CASE(negative_integers_to_signed_min)
{
	char const* sourceCode = R"(
		contract test {
			int8 public i = -128;
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(positive_integers_to_signed_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			int8 public j = 128;
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type int_const 128 is not implicitly convertible to expected type int8.");
}

BOOST_AUTO_TEST_CASE(positive_integers_to_signed_out_of_bound_max)
{
	char const* sourceCode = R"(
		contract test {
			int8 public j = 127;
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(negative_integers_to_unsigned)
{
	char const* sourceCode = R"(
		contract test {
			uint8 public x = -1;
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type int_const -1 is not implicitly convertible to expected type uint8.");
}

BOOST_AUTO_TEST_CASE(positive_integers_to_unsigned_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			uint8 public x = 700;
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type int_const 700 is not implicitly convertible to expected type uint8.");
}

BOOST_AUTO_TEST_CASE(integer_boolean_or)
{
	char const* sourceCode = R"(
		contract test { function() public { uint x = 1; uint y = 2; x || y; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator || not compatible with types uint256 and uint256");
}

BOOST_AUTO_TEST_CASE(integer_boolean_and)
{
	char const* sourceCode = R"(
		contract test { function() public { uint x = 1; uint y = 2; x && y; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator && not compatible with types uint256 and uint256");
}

BOOST_AUTO_TEST_CASE(integer_boolean_not)
{
	char const* sourceCode = R"(
		contract test { function() public { uint x = 1; !x; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Unary operator ! cannot be applied to type uint256");
}

BOOST_AUTO_TEST_CASE(integer_unsigned_exp_signed)
{
	char const* sourceCode = R"(
		contract test { function() public { uint x = 3; int y = -4; x ** y; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator ** not compatible with types uint256 and int256");
}

BOOST_AUTO_TEST_CASE(integer_signed_exp_unsigned)
{
	char const* sourceCode = R"(
		contract test { function() public { uint x = 3; int y = -4; y ** x; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator ** not compatible with types int256 and uint256");
}

BOOST_AUTO_TEST_CASE(integer_signed_exp_signed)
{
	char const* sourceCode = R"(
		contract test { function() public { int x = -3; int y = -4; x ** y; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator ** not compatible with types int256 and int256");
}

BOOST_AUTO_TEST_CASE(bytes_reference_compare_operators)
{
	char const* sourceCode = R"(
		contract test { bytes a; bytes b; function() public { a == b; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator == not compatible with types bytes storage ref and bytes storage ref");
}

BOOST_AUTO_TEST_CASE(struct_reference_compare_operators)
{
	char const* sourceCode = R"(
		contract test { struct s {uint a;} s x; s y; function() public { x == y; } }
	)";
	CHECK_ERROR(sourceCode, TypeError, "Operator == not compatible with types struct test.s storage ref and struct test.s storage ref");
}

BOOST_AUTO_TEST_CASE(overwrite_memory_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] memory a) external {}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Location has to be calldata for external functions (remove the \"memory\" or \"storage\" keyword).");
}

BOOST_AUTO_TEST_CASE(overwrite_storage_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage a) external {}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Location has to be calldata for external functions (remove the \"memory\" or \"storage\" keyword).");
}

BOOST_AUTO_TEST_CASE(storage_location_local_variables)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				uint[] storage x;
				uint[] memory y;
				uint[] memory z;
				x;y;z;
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(no_mappings_in_memory_array)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				mapping(uint=>uint)[] memory x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type mapping(uint256 => uint256)[] memory is only valid in storage.");
}

BOOST_AUTO_TEST_CASE(assignment_mem_to_local_storage_variable)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f(uint[] x) public {
				var dataRef = data;
				dataRef = x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type uint256[] memory is not implicitly convertible to expected type uint256[] storage pointer.");
}

BOOST_AUTO_TEST_CASE(storage_assign_to_different_local_variable)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			uint8[] otherData;
			function f() public {
				uint8[] storage x = otherData;
				uint[] storage y = data;
				y = x;
				// note that data = otherData works
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type uint8[] storage pointer is not implicitly convertible to expected type uint256[] storage pointer.");
}

BOOST_AUTO_TEST_CASE(uninitialized_mapping_variable)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				mapping(uint => uint) x;
				x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Uninitialized mapping. Mappings cannot be created dynamically, you have to assign them from a state variable");
}

BOOST_AUTO_TEST_CASE(uninitialized_mapping_array_variable)
{
	char const* sourceCode = R"(
		contract C {
			function f() pure public {
				mapping(uint => uint)[] storage x;
				x;
			}
		}
	)";
	CHECK_WARNING(sourceCode, "Uninitialized storage pointer");
}

BOOST_AUTO_TEST_CASE(uninitialized_mapping_array_variable_050)
{
	char const* sourceCode = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				mapping(uint => uint)[] storage x;
				x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, DeclarationError, "Uninitialized storage pointer");
}

BOOST_AUTO_TEST_CASE(no_delete_on_storage_pointers)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f() public {
				var x = data;
				delete x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Unary operator delete cannot be applied to type uint256[] storage pointer");
}

BOOST_AUTO_TEST_CASE(assignment_mem_storage_variable_directly)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f(uint[] x) public {
				data = x;
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(function_argument_mem_to_storage)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage x) private {
			}
			function g(uint[] x) public {
				f(x);
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Invalid type for argument in function call. Invalid implicit conversion from uint256[] memory to uint256[] storage pointer requested.");
}

BOOST_AUTO_TEST_CASE(function_argument_storage_to_mem)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage x) private {
				g(x);
			}
			function g(uint[] x) public {
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(mem_array_assignment_changes_base_type)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint8[] memory x) private {
				// Such an assignment is possible in storage, but not in memory
				// (because it would incur an otherwise unnecessary copy).
				// This requirement might be lifted, though.
				uint[] memory y = x;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Type uint8[] memory is not implicitly convertible to expected type uint256[] memory.");
}

BOOST_AUTO_TEST_CASE(dynamic_return_types_not_possible)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint) public returns (string);
			function g() public {
				var x = this.f(2);
				// we can assign to x but it is not usable.
				bytes(x).length;
			}
		}
	)";
	if (dev::test::Options::get().evmVersion() == EVMVersion::homestead())
		CHECK_ERROR(sourceCode, TypeError, "Explicit type conversion not allowed from \"inaccessible dynamic type\" to \"bytes storage pointer\".");
	else
		CHECK_WARNING(sourceCode, "Use of the \"var\" keyword is deprecated");
}

BOOST_AUTO_TEST_CASE(memory_arrays_not_resizeable)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				uint[] memory x;
				x.length = 2;
			}
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Expression has to be an lvalue.");
}

BOOST_AUTO_TEST_CASE(struct_constructor)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint a; bool x; }
			function f() public {
				S memory s = S(1, true);
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(struct_constructor_nested)
{
	char const* sourceCode = R"(
		contract C {
			struct X { uint x1; uint x2; }
			struct S { uint s1; uint[3] s2; X s3; }
			function f() public {
				uint[3] memory s2;
				S memory s = S(1, s2, X(4, 5));
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(struct_named_constructor)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint a; bool x; }
			function f() public {
				S memory s = S({a: 1, x: true});
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
}

BOOST_AUTO_TEST_CASE(literal_strings)
{
	char const* text = R"(
		contract Foo {
			function f() public {
				string memory long = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
				string memory short = "123";
				long; short;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(string_bytes_conversion)
{
	char const* text = R"(
		contract Test {
			string s;
			bytes b;
			function h(string _s) external { bytes(_s).length; }
			function i(string _s) internal { bytes(_s).length; }
			function j() internal { bytes(s).length; }
			function k(bytes _b) external { string(_b); }
			function l(bytes _b) internal { string(_b); }
			function m() internal { string(b); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inheriting_from_library)
{
	char const* text = R"(
		library Lib {}
		contract Test is Lib {}
	)";
	CHECK_ERROR(text, TypeError, "Libraries cannot be inherited from.");
}

BOOST_AUTO_TEST_CASE(inheriting_library)
{
	char const* text = R"(
		contract Test {}
		library Lib is Test {}
	)";
	CHECK_ERROR(text, TypeError, "Library is not allowed to inherit.");
}

BOOST_AUTO_TEST_CASE(library_having_variables)
{
	char const* text = R"(
		library Lib { uint x; }
	)";
	CHECK_ERROR(text, TypeError, "Library cannot have non-constant state variables");
}

BOOST_AUTO_TEST_CASE(valid_library)
{
	char const* text = R"(
		library Lib { uint constant x = 9; }
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(call_to_library_function)
{
	char const* text = R"(
		library Lib {
			function min(uint, uint) public returns (uint);
		}
		contract Test {
			function f() public {
				uint t = Lib.min(12, 7);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(creating_contract_within_the_contract)
{
	char const* sourceCode = R"(
		contract Test {
			function f() public { var x = new Test(); }
		}
	)";
	CHECK_ERROR(sourceCode, TypeError, "Circular reference for contract creation (cannot create instance of derived or same contract).");
}

BOOST_AUTO_TEST_CASE(array_out_of_bound_access)
{
	char const* text = R"(
		contract c {
			uint[2] dataArray;
			function set5th() public returns (bool) {
				dataArray[5] = 2;
				return true;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Out of bounds array access.");
}

BOOST_AUTO_TEST_CASE(literal_string_to_storage_pointer)
{
	char const* text = R"(
		contract C {
			function f() public { string x = "abc"; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Type literal_string \"abc\" is not implicitly convertible to expected type string storage pointer.");
}

BOOST_AUTO_TEST_CASE(non_initialized_references)
{
	char const* text = R"(
		contract C {
			struct s {
				uint a;
			}
			function f() public {
				s storage x;
				x.a = 2;
			}
		}
	)";

	CHECK_WARNING(text, "Uninitialized storage pointer");
}

BOOST_AUTO_TEST_CASE(non_initialized_references_050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			struct s {
				uint a;
			}
			function f() public {
				s storage x;
			}
		}
	)";

	CHECK_ERROR(text, DeclarationError, "Uninitialized storage pointer");
}

BOOST_AUTO_TEST_CASE(keccak256_with_large_integer_constant)
{
	char const* text = R"(
		contract C {
			function f() public { keccak256(2**500); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid rational number (too large or division by zero).");
}

BOOST_AUTO_TEST_CASE(cyclic_binary_dependency)
{
	char const* text = R"(
		contract A { function f() public { new B(); } }
		contract B { function f() public { new C(); } }
		contract C { function f() public { new A(); } }
	)";
	CHECK_ERROR(text, TypeError, "Circular reference for contract creation (cannot create instance of derived or same contract).");
}

BOOST_AUTO_TEST_CASE(cyclic_binary_dependency_via_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B { function f() public { new C(); } }
		contract C { function f() public { new A(); } }
	)";
	CHECK_ERROR(text, TypeError, "Definition of base has to precede definition of derived contract");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_fail)
{
	char const* text = R"(
		contract C { function f() public { var (x,y); x = 1; y = 1;} }
	)";
	CHECK_ERROR(text, TypeError, "Assignment necessary for type detection.");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fine)
{
	char const* text = R"(
		contract C {
			function three() public returns (uint, uint, uint);
			function two() public returns (uint, uint);
			function none();
			function f() public {
				var (a,) = three();
				var (b,c,) = two();
				var (,d) = three();
				var (,e,g) = two();
				var (,,) = three();
				var () = none();
				a;b;c;d;e;g;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_1)
{
	char const* text = R"(
		contract C {
			function one() public returns (uint);
			function f() public { var (a, b, ) = one(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Not enough components (1) in value to assign all variables (2).");
}
BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_2)
{
	char const* text = R"(
		contract C {
			function one() public returns (uint);
			function f() public { var (a, , ) = one(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Not enough components (1) in value to assign all variables (2).");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_3)
{
	char const* text = R"(
		contract C {
			function one() public returns (uint);
			function f() public { var (, , a) = one(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Not enough components (1) in value to assign all variables (2).");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_4)
{
	char const* text = R"(
		contract C {
			function one() public returns (uint);
			function f() public { var (, a, b) = one(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Not enough components (1) in value to assign all variables (2).");
}

BOOST_AUTO_TEST_CASE(tuples)
{
	char const* text = R"(
		contract C {
			function f() public {
				uint a = (1);
				var (b,) = (uint8(1),);
				var (c,d) = (uint32(1), 2 + a);
				var (e,) = (uint64(1), 2, b);
				a;b;c;d;e;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(tuples_empty_components)
{
	char const* text = R"(
		contract C {
			function f() public {
				(1,,2);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Tuple component cannot be empty.");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_5)
{
	char const* text = R"(
		contract C {
			function one() public returns (uint);
			function f() public { var (,) = one(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Wildcard both at beginning and end of variable declaration list is only allowed if the number of components is equal.");
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_6)
{
	char const* text = R"(
		contract C {
			function two() public returns (uint, uint);
			function f() public { var (a, b, c) = two(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Not enough components (2) in value to assign all variables (3)");
}

BOOST_AUTO_TEST_CASE(tuple_assignment_from_void_function)
{
	char const* text = R"(
		contract C {
			function f() public { }
			function g() public {
				var (x,) = (f(), f());
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Cannot declare variable with void (empty tuple) type.");
}

BOOST_AUTO_TEST_CASE(tuple_compound_assignment)
{
	char const* text = R"(
		contract C {
			function f() public returns (uint a, uint b) {
				(a, b) += (1, 1);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Compound assignment is not allowed for tuple types.");
}

BOOST_AUTO_TEST_CASE(member_access_parser_ambiguity)
{
	char const* text = R"(
		contract C {
			struct R { uint[10][10] y; }
			struct S { uint a; uint b; uint[20][20][20] c; R d; }
			S data;
			function f() public {
				C.S x = data;
				C.S memory y;
				C.S[10] memory z;
				C.S[10];
				y.a = 2;
				x.c[1][2][3] = 9;
				x.d.y[2][2] = 3;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_library)
{
	char const* text = R"(
		library D { }
		contract C {
			using D for uint;
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_not_library)
{
	char const* text = R"(
		contract D { }
		contract C {
			using D for uint;
		}
	)";
	CHECK_ERROR(text, TypeError, "Library name expected.");
}

BOOST_AUTO_TEST_CASE(using_for_function_exists)
{
	char const* text = R"(
		library D { function double(uint self) public returns (uint) { return 2*self; } }
		contract C {
			using D for uint;
			function f(uint a) public {
				a.double;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_function_on_int)
{
	char const* text = R"(
		library D { function double(uint self) public returns (uint) { return 2*self; } }
		contract C {
			using D for uint;
			function f(uint a) public returns (uint) {
				return a.double();
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_function_on_struct)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) public returns (uint) {
				return x.mul(a);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_overload)
{
	char const* text = R"(
		library D {
			struct s { uint a; }
			function mul(s storage self, uint x) public returns (uint) { return self.a *= x; }
			function mul(s storage, bytes32) public returns (bytes32) { }
		}
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) public returns (uint) {
				return x.mul(a);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_by_name)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) public returns (uint) {
				return x.mul({x: a});
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_mismatch)
{
	char const* text = R"(
		library D { function double(bytes32 self) public returns (uint) { return 2; } }
		contract C {
			using D for uint;
			function f(uint a) public returns (uint) {
				return a.double();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"double\" not found or not visible after argument-dependent lookup in uint256");
}

BOOST_AUTO_TEST_CASE(using_for_not_used)
{
	char const* text = R"(
		library D { function double(uint self) public returns (uint) { return 2; } }
		contract C {
			using D for uint;
			function f(uint16 a) public returns (uint) {
				// This is an error because the function is only bound to uint.
				// Had it been bound to *, it would have worked.
				return a.double();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"double\" not found or not visible after argument-dependent lookup in uint16");
}

BOOST_AUTO_TEST_CASE(library_memory_struct)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;
		library c {
			struct S { uint x; }
			function f() public returns (S ) {}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_for_arbitrary_mismatch)
{
	char const* text = R"(
		library D { function double(bytes32 self) public returns (uint) { return 2; } }
		contract C {
			using D for *;
			function f(uint a) public returns (uint) {
				// Bound to a, but self type does not match.
				return a.double();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"double\" not found or not visible after argument-dependent lookup in uint256");
}

BOOST_AUTO_TEST_CASE(bound_function_in_var)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) public returns (uint) {
				var g = x.mul;
				return g({x: a});
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(create_memory_arrays)
{
	char const* text = R"(
		library L {
			struct R { uint[10][10] y; }
			struct S { uint a; uint b; uint[20][20][20] c; R d; }
		}
		contract C {
			function f(uint size) public {
				L.S[][] memory x = new L.S[][](10);
				var y = new uint[](20);
				var z = new bytes(size);
				x;y;z;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(mapping_in_memory_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) public {
				var x = new mapping(uint => uint)[](4);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type cannot live outside storage.");
}

BOOST_AUTO_TEST_CASE(new_for_non_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) public {
				var x = new uint(7);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Contract or array type expected.");
}

BOOST_AUTO_TEST_CASE(invalid_args_creating_memory_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) public {
				var x = new uint[]();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Wrong argument count for function call: 0 arguments given but expected 1.");
}

BOOST_AUTO_TEST_CASE(invalid_args_creating_struct)
{
	char const* text = R"(
		contract C {
			struct S { uint a; uint b; }

			function f() public {
				var s = S({a: 1});
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Wrong argument count for struct constructor: 1 arguments given but expected 2.");
}

BOOST_AUTO_TEST_CASE(function_overload_array_type)
{
	char const* text = R"(
			contract M {
				function f(uint[]);
				function f(int[]);
			}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_and_passing_implicit_conversion)
{
	char const* text = R"(
			contract C {
				function f() public returns (uint) {
					uint8 x = 7;
					uint16 y = 8;
					uint32 z = 9;
					uint32[3] memory ending = [x, y, z];
					return (ending[1]);
				}
			}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_and_passing_implicit_conversion_strings)
{
	char const* text = R"(
		contract C {
			function f() public returns (string) {
				string memory x = "Hello";
				string memory y = "World";
				string[2] memory z = [x, y];
				return (z[0]);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_const_int_conversion)
{
	char const* text = R"(
		contract C {
			function f() public returns (uint) {
				uint8[4] memory z = [1,2,3,5];
				return (z[0]);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_const_string_conversion)
{
	char const* text = R"(
		contract C {
			function f() public returns (string) {
				string[2] memory z = ["Hello", "World"];
				return (z[0]);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_no_type)
{
	char const* text = R"(
		contract C {
			function f() public returns (uint) {
				return ([4,5,6][1]);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_no_type_strings)
{
	char const* text = R"(
		contract C {
			function f() public returns (string) {
				return (["foo", "man", "choo"][1]);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_struct_declaration_arrays)
{
	char const* text = R"(
		contract C {
			struct S {
				uint a;
				string b;
			}
			function f() {
				S[2] memory x = [S({a: 1, b: "fish"}), S({a: 2, b: "fish"})];
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(invalid_types_in_inline_array)
{
	char const* text = R"(
		contract C {
			function f() public {
				uint[3] x = [45, 'foo', true];
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Unable to deduce common type for array elements.");
}

BOOST_AUTO_TEST_CASE(dynamic_inline_array)
{
	char const* text = R"(
		contract C {
			function f() public {
				uint8[4][4] memory dyn = [[1, 2, 3, 4], [2, 3, 4, 5], [3, 4, 5, 6], [4, 5, 6, 7]];
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(lvalues_as_inline_array)
{
	char const* text = R"(
		contract C {
			function f() public {
				[1, 2, 3]++;
				[1, 2, 3] = [4, 5, 6];
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Inline array type cannot be declared as LValue.");
}

BOOST_AUTO_TEST_CASE(break_not_in_loop)
{
	char const* text = R"(
		contract C {
			function f() public {
				if (true)
					break;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "\"break\" has to be in a \"for\" or \"while\" loop.");
}

BOOST_AUTO_TEST_CASE(continue_not_in_loop)
{
	char const* text = R"(
		contract C {
			function f() public {
				if (true)
					continue;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "\"continue\" has to be in a \"for\" or \"while\" loop.");
}

BOOST_AUTO_TEST_CASE(continue_not_in_loop_2)
{
	char const* text = R"(
		contract C {
			function f() public {
				while (true)
				{
				}
				continue;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "\"continue\" has to be in a \"for\" or \"while\" loop.");
}

BOOST_AUTO_TEST_CASE(invalid_different_types_for_conditional_expression)
{
	char const* text = R"(
		contract C {
			function f() public {
				true ? true : 2;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "True expression's type bool doesn't match false expression's type uint8.");
}

BOOST_AUTO_TEST_CASE(left_value_in_conditional_expression_not_supported_yet)
{
	char const* text = R"(
		contract C {
			function f() public {
				uint x;
				uint y;
				(true ? x : y) = 1;
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (std::vector<std::string>{
		"Conditional expression as left value is not supported yet.",
		"Expression has to be an lvalue"
	}));
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_struct)
{
	char const* text = R"(
		contract C {
			struct s1 {
				uint x;
			}
			struct s2 {
				uint x;
			}
			function f() public {
				s1 memory x;
				s2 memory y;
				true ? x : y;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "True expression's type struct C.s1 memory doesn't match false expression's type struct C.s2 memory.");
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_function_type)
{
	char const* text = R"(
		contract C {
			function x(bool) public {}
			function y() public {}

			function f() public {
				true ? x : y;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "True expression's type function (bool) doesn't match false expression's type function ().");
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_enum)
{
	char const* text = R"(
		contract C {
			enum small { A, B, C, D }
			enum big { A, B, C, D }

			function f() public {
				small x;
				big y;

				true ? x : y;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "True expression's type enum C.small doesn't match false expression's type enum C.big.");
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_mapping)
{
	char const* text = R"(
		contract C {
			mapping(uint8 => uint8) table1;
			mapping(uint32 => uint8) table2;

			function f() public {
				true ? table1 : table2;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "True expression's type mapping(uint8 => uint8) doesn't match false expression's type mapping(uint32 => uint8).");
}

BOOST_AUTO_TEST_CASE(conditional_with_all_types)
{
	char const* text = R"(
		contract C {
			struct s1 {
				uint x;
			}
			s1 struct_x;
			s1 struct_y;

			function fun_x() public {}
			function fun_y() public {}

			enum small { A, B, C, D }

			mapping(uint8 => uint8) table1;
			mapping(uint8 => uint8) table2;

			function f() public {
				// integers
				uint x;
				uint y;
				uint g = true ? x : y;
				g += 1; // Avoid unused var warning

				// integer constants
				uint h = true ? 1 : 3;
				h += 1; // Avoid unused var warning

				// string literal
				var i = true ? "hello" : "world";
				i = "used"; //Avoid unused var warning
			}
			function f2() public {
				// bool
				bool j = true ? true : false;
				j = j && true; // Avoid unused var warning

				// real is not there yet.

				// array
				byte[2] memory a;
				byte[2] memory b;
				var k = true ? a : b;
				k[0] = byte(0); //Avoid unused var warning

				bytes memory e;
				bytes memory f;
				var l = true ? e : f;
				l[0] = byte(0); // Avoid unused var warning

				// fixed bytes
				bytes2 c;
				bytes2 d;
				var m = true ? c : d;
				m &= m;

			}
			function f3() public {
				// contract doesn't fit in here

				// struct
				struct_x = true ? struct_x : struct_y;

				// function
				var r = true ? fun_x : fun_y;
				r(); // Avoid unused var warning
				// enum
				small enum_x;
				small enum_y;
				enum_x = true ? enum_x : enum_y;

				// tuple
				var (n, o) = true ? (1, 2) : (3, 4);
				(n, o) = (o, n); // Avoid unused var warning
				// mapping
				var p = true ? table1 : table2;
				p[0] = 0; // Avoid unused var warning
				// typetype
				var q = true ? uint32(1) : uint32(2);
				q += 1; // Avoid unused var warning
				// modifier doesn't fit in here

				// magic doesn't fit in here

				// module doesn't fit in here
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(uint7_and_uintM_as_identifier)
{
	char const* text = R"(
		contract test {
		string uintM = "Hello 4 you";
			function f() public {
				uint8 uint7 = 3;
				uint7 = 5;
				string memory intM;
				uint bytesM = 21;
				intM; bytesM;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(varM_disqualified_as_keyword)
{
	char const* text = R"(
		contract test {
			function f() public {
				uintM something = 3;
				intM should = 4;
				bytesM fail = "now";
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, DeclarationError, (std::vector<std::string>{
		"Identifier not found or not unique.",
		"Identifier not found or not unique.",
		"Identifier not found or not unique."
	}));
}

BOOST_AUTO_TEST_CASE(modifier_is_not_a_valid_typename)
{
	char const* text = R"(
		contract test {
			modifier mod() { _; }

			function f() public {
				mod g;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Name has to refer to a struct, enum or contract.");
}

BOOST_AUTO_TEST_CASE(modifier_is_not_a_valid_typename_is_not_fatal)
{
	char const* text = R"(
		contract test {
			modifier mod() { _; }

			function f() public {
				mod g;
				g = f;
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (std::vector<std::string>{"Name has to refer to a struct, enum or contract."}));
}

BOOST_AUTO_TEST_CASE(function_is_not_a_valid_typename)
{
	char const* text = R"(
		contract test {
			function foo() public {
			}

			function f() public {
				foo g;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Name has to refer to a struct, enum or contract.");
}

BOOST_AUTO_TEST_CASE(long_uint_variable_fails)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint99999999999999999999999999 something = 3;
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found or not unique.");
}

BOOST_AUTO_TEST_CASE(bytes10abc_is_identifier)
{
	char const* text = R"(
		contract test {
			function f() public {
				bytes32 bytes10abc = "abc";
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(int10abc_is_identifier)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint uint10abc = 3;
				int int10abc = 4;
				uint10abc; int10abc;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(library_functions_do_not_have_value)
{
	char const* text = R"(
		library L { function l() public {} }
		contract test {
			function f() public {
				L.l.value;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup in function ()");
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_0x7_mxn)
{
	char const* text = R"(
		contract test {
			fixed0x7 a = .3;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found");
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_long_invalid_identifier)
{
	char const* text = R"(
		contract test {
			fixed99999999999999999999999999999999999999x7 b = 9.5;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found");
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_7x8_mxn)
{
	char const* text = R"(
		contract test {
			fixed7x8 c = 3.12345678;
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found");
}

BOOST_AUTO_TEST_CASE(library_instances_cannot_be_used)
{
	char const* text = R"(
		library L { function l() public {} }
		contract test {
			function f() public {
				L x;
				x.l();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"l\" not found or not visible after argument-dependent lookup in library L");
}

BOOST_AUTO_TEST_CASE(invalid_fixed_type_long)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed8x888888888888888888888888888888888888888888888888888 b;
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found");
}

BOOST_AUTO_TEST_CASE(fixed_type_int_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint64 a = 3;
				int64 b = 4;
				fixed c = b;
				ufixed d = a;
				c; d;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_type_rational_int_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed c = 3;
				ufixed d = 4;
				c; d;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_type_rational_fraction_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed a = 4.5;
				ufixed d = 2.5;
				a; d;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(invalid_int_implicit_conversion_from_fixed)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed a = 4.5;
				int b = a;
				a; b;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type fixed128x18 is not implicitly convertible to expected type int256");
}

BOOST_AUTO_TEST_CASE(rational_unary_minus_operation)
{
	char const* text = R"(
		contract test {
			function f() pure public {
				ufixed16x2 a = 3.25;
				fixed16x2 b = -3.25;
				a; b;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(rational_unary_plus_operation)
{
	char const* text = R"(
		contract test {
			function f() pure public {
				ufixed16x2 a = +3.25;
				fixed16x2 b = -3.25;
				a; b;
			}
		}
	)";
	CHECK_WARNING(text, "Use of unary + is deprecated");
}

BOOST_AUTO_TEST_CASE(rational_unary_plus_assignment)
{
	char const* text = R"(
		contract test {
			function f(uint x) pure public {
				uint y = +x;
				y;
			}
		}
	)";
	CHECK_WARNING(text, "Use of unary + is deprecated");
}

BOOST_AUTO_TEST_CASE(rational_unary_plus_operation_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			function f() pure public {
				ufixed16x2 a = +3.25;
				fixed16x2 b = -3.25;
				a; b;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "Use of unary + is deprecated");
}

BOOST_AUTO_TEST_CASE(rational_unary_plus_assignment_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			function f(uint x) pure public {
				uint y = +x;
				y;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "Use of unary + is deprecated");
}

BOOST_AUTO_TEST_CASE(leading_zero_rationals_convert)
{
	char const* text = R"(
		contract A {
			function f() pure public {
				ufixed16x2 a = 0.5;
				ufixed256x52 b = 0.0000000000000006661338147750939242541790008544921875;
				fixed16x2 c = -0.5;
				fixed256x52 d = -0.0000000000000006661338147750939242541790008544921875;
				a; b; c; d;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_type_size_capabilities)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed256x1 a = 123456781234567979695948382928485849359686494864095409282048094275023098123.5;
				ufixed256x77 b = 0.920890746623327805482905058466021565416131529487595827354393978494366605267637;
				ufixed224x78 c = 0.000000000001519884736399797998492268541131529487595827354393978494366605267646;
				fixed256x1 d = -123456781234567979695948382928485849359686494864095409282048094275023098123.5;
				fixed256x76 e = -0.93322335481643744342575580035176794825198893968114429702091846411734101080123;
				fixed256x79 g = -0.0001178860664374434257558003517679482519889396811442970209184641173410108012309;
				a; b; c; d; e; g;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_type_zero_handling)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed16x2 a = 0; a;
				ufixed32x1 b = 0; b;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_type_invalid_implicit_conversion_size)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed a = 11/4;
				ufixed248x8 b = a; b;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type ufixed128x18 is not implicitly convertible to expected type ufixed248x8");
}

BOOST_AUTO_TEST_CASE(fixed_type_invalid_implicit_conversion_lost_data)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed256x1 a = 1/3; a;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type ufixed256x1");
}

BOOST_AUTO_TEST_CASE(fixed_type_valid_explicit_conversions)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed256x80 a = ufixed256x80(1/3); a;
				ufixed248x80 b = ufixed248x80(1/3); b;
				ufixed8x1 c = ufixed8x1(1/3); c;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(invalid_array_declaration_with_rational)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint[3.5] a; a;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Array with fractional length specified.");
}

BOOST_AUTO_TEST_CASE(invalid_array_declaration_with_signed_fixed_type)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint[fixed(3.5)] a; a;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(invalid_array_declaration_with_unsigned_fixed_type)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint[ufixed(3.5)] a; a;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(rational_to_bytes_implicit_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				bytes32 c = 3.2; c;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type bytes32");
}

BOOST_AUTO_TEST_CASE(fixed_to_bytes_implicit_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed a = 3.25;
				bytes32 c = a; c;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "fixed128x18 is not implicitly convertible to expected type bytes32");
}

BOOST_AUTO_TEST_CASE(mapping_with_fixed_literal)
{
	char const* text = R"(
		contract test {
			mapping(ufixed8x1 => string) fixedString;
			function f() public {
				fixedString[0.5] = "Half";
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(fixed_points_inside_structs)
{
	char const* text = R"(
		contract test {
			struct myStruct {
				ufixed a;
				int b;
			}
			myStruct a = myStruct(3.125, 3);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_fixed_types)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed[3] memory a = [fixed(3.5), fixed(-4.25), fixed(967.125)];
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_array_rationals)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed128x3[4] memory a = [ufixed128x3(3.5), 4.125, 2.5, 4.0];
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(rational_index_access)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint[] memory a;
				a[.5];
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "rational_const 1 / 2 is not implicitly convertible to expected type uint256");
}

BOOST_AUTO_TEST_CASE(rational_to_fixed_literal_expression)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed64x8 a = 3.5 * 3;
				ufixed64x8 b = 4 - 2.5;
				ufixed64x8 c = 11 / 4;
				ufixed240x5 d = 599 + 0.21875;
				ufixed256x80 e = ufixed256x80(35.245 % 12.9);
				ufixed256x80 f = ufixed256x80(1.2 % 2);
				fixed g = 2 ** -2;
				a; b; c; d; e; f; g;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(rational_as_exponent_value_signed)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed g = 2 ** -2.2;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(rational_as_exponent_value_unsigned)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed b = 3 ** 2.5;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(rational_as_exponent_half)
{
	char const* text = R"(
		contract test {
			function f() public {
				2 ** (1/2);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(rational_as_exponent_value_neg_quarter)
{
	char const* text = R"(
		contract test {
			function f() public {
				42 ** (-1/4);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(fixed_point_casting_exponents_15)
{
	char const* text = R"(
		contract test {
			function f() public {
				var a = 3 ** ufixed(1.5);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(fixed_point_casting_exponents_neg)
{
	char const* text = R"(
		contract test {
			function f() public {
				var c = 42 ** fixed(-1/4);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(var_capable_of_holding_constant_rationals)
{
	char const* text = R"(
		contract test {
			function f() public {
				var a = 0.12345678;
				var b = 12345678.352;
				var c = 0.00000009;
				a; b; c;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(var_and_rational_with_tuple)
{
	char const* text = R"(
		contract test {
			function f() public {
				var (a, b) = (.5, 1/3);
				a; b;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(var_handle_divided_integers)
{
	char const* text = R"(
		contract test {
			function f() public {
				var x = 1/3;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(rational_bitnot_unary_operation)
{
	char const* text = R"(
		contract test {
			function f() public {
				~fixed(3.5);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "cannot be applied");
}

BOOST_AUTO_TEST_CASE(rational_bitor_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed(1.5) | 3;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(rational_bitxor_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed(1.75) ^ 3;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(rational_bitand_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() public {
				fixed(1.75) & 3;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "not compatible with types");
}

BOOST_AUTO_TEST_CASE(missing_bool_conversion)
{
	char const* text = R"(
		contract test {
			function b(uint a) public {
				bool(a == 1);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(integer_and_fixed_interaction)
{
	char const* text = R"(
		contract test {
			function f() public {
				ufixed a = uint64(1) + ufixed(2);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(one_divided_by_three_integer_conversion)
{
	char const* text = R"(
		contract test {
			function f() public {
				uint a = 1/3;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type uint256. Try converting to type ufixed256x77");
}

BOOST_AUTO_TEST_CASE(unused_return_value)
{
	char const* text = R"(
		contract test {
			function g() public returns (uint) {}
			function f() public {
				g();
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(unused_return_value_send)
{
	char const* text = R"(
		contract test {
			function f() public {
				address(0x12).send(1);
			}
		}
	)";
	CHECK_WARNING(text, "Failure condition of 'send' ignored. Consider using 'transfer' instead.");
}

BOOST_AUTO_TEST_CASE(unused_return_value_call)
{
	char const* text = R"(
		contract test {
			function f() public {
				address(0x12).call("abc");
			}
		}
	)";
	CHECK_WARNING(text, "Return value of low-level calls not used");
}

BOOST_AUTO_TEST_CASE(unused_return_value_call_value)
{
	char const* text = R"(
		contract test {
			function f() public {
				address(0x12).call.value(2)("abc");
			}
		}
	)";
	CHECK_WARNING(text, "Return value of low-level calls not used");
}

BOOST_AUTO_TEST_CASE(unused_return_value_callcode)
{
	char const* text = R"(
		contract test {
			function f() public {
				address(0x12).callcode("abc");
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(text, (std::vector<std::string>{
		"Return value of low-level calls not used",
		"\"callcode\" has been deprecated"
	}));
}

BOOST_AUTO_TEST_CASE(unused_return_value_delegatecall)
{
	char const* text = R"(
		contract test {
			function f() public {
				address(0x12).delegatecall("abc");
			}
		}
	)";
	CHECK_WARNING(text, "Return value of low-level calls not used");
}

BOOST_AUTO_TEST_CASE(callcode_deprecated)
{
	char const* text = R"(
		contract test {
			function f() pure public {
				address(0x12).callcode;
			}
		}
	)";
	CHECK_WARNING(text, "\"callcode\" has been deprecated in favour of \"delegatecall\"");
}

BOOST_AUTO_TEST_CASE(callcode_deprecated_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			function f() pure public {
				address(0x12).callcode;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "\"callcode\" has been deprecated in favour of \"delegatecall\"");
}

BOOST_AUTO_TEST_CASE(callcode_not_deprecated_as_function)
{
	char const* text = R"(
		contract test {
			function callcode() pure public {
				test.callcode();
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(payable_in_library)
{
	char const* text = R"(
		library test {
			function f() payable public {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Library functions cannot be payable.");
}

BOOST_AUTO_TEST_CASE(payable_external)
{
	char const* text = R"(
		contract test {
			function f() payable external {}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(payable_internal)
{
	char const* text = R"(
		contract test {
			function f() payable internal {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal functions cannot be payable.");
}

BOOST_AUTO_TEST_CASE(payable_private)
{
	char const* text = R"(
		contract test {
			function f() payable private {}
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal functions cannot be payable.");
}

BOOST_AUTO_TEST_CASE(illegal_override_payable)
{
	char const* text = R"(
		contract B { function f() payable public {} }
		contract C is B { function f() public {} }
	)";
	CHECK_ERROR(text, TypeError, "Overriding function changes state mutability from \"payable\" to \"nonpayable\".");
}

BOOST_AUTO_TEST_CASE(illegal_override_payable_nonpayable)
{
	char const* text = R"(
		contract B { function f() public {} }
		contract C is B { function f() payable public {} }
	)";
	CHECK_ERROR(text, TypeError, "Overriding function changes state mutability from \"nonpayable\" to \"payable\".");
}

BOOST_AUTO_TEST_CASE(function_variable_mixin)
{
	char const* text = R"(
		// bug #1798 (cpp-ethereum), related to #1286 (solidity)
		contract attribute {
			bool ok = false;
		}
		contract func {
			function ok() public returns (bool) { return true; }
		}
		contract attr_func is attribute, func {
			function checkOk() public returns (bool) { return ok(); }
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier already declared.");
}

BOOST_AUTO_TEST_CASE(calling_payable)
{
	char const* text = R"(
		contract receiver { function pay() payable public {} }
		contract test {
			function f() public { (new receiver()).pay.value(10)(); }
			receiver r = new receiver();
			function g() public { r.pay.value(10)(); }
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(calling_nonpayable)
{
	char const* text = R"(
		contract receiver { function nopay() public {} }
		contract test {
			function f() public { (new receiver()).nopay.value(10)(); }
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup in function () external - did you forget the \"payable\" modifier?");
}

BOOST_AUTO_TEST_CASE(non_payable_constructor)
{
	char const* text = R"(
		contract C {
			function C() { }
		}
		contract D {
			function f() public returns (uint) {
				(new C).value(2)();
				return 2;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup in function () returns (contract C) - did you forget the \"payable\" modifier?");
}

BOOST_AUTO_TEST_CASE(warn_nonpresent_pragma)
{
	char const* text = "contract C {}";
	auto sourceAndError = parseAnalyseAndReturnError(text, true, false);
	BOOST_REQUIRE(!sourceAndError.second.empty());
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_CHECK(searchErrorMessage(*sourceAndError.second.front(), "Source file does not specify required compiler version!"));
}

BOOST_AUTO_TEST_CASE(unsatisfied_version)
{
	char const* text = R"(
		pragma solidity ^99.99.0;
	)";
	auto sourceAndError = parseAnalyseAndReturnError(text, false, false, false);
	BOOST_REQUIRE(!sourceAndError.second.empty());
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_CHECK(sourceAndError.second.front()->type() == Error::Type::SyntaxError);
	BOOST_CHECK(searchErrorMessage(*sourceAndError.second.front(), "Source file requires different compiler version"));
}

BOOST_AUTO_TEST_CASE(invalid_array_as_statement)
{
	char const* text = R"(
		contract test {
			struct S { uint x; }
			function test(uint k) public { S[k]; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Integer constant expected.");
}

BOOST_AUTO_TEST_CASE(using_directive_for_missing_selftype)
{
	char const* text = R"(
		library B {
			function b() public {}
		}

		contract A {
			using B for bytes;

			function a() public {
				bytes memory x;
				x.b();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"b\" not found or not visible after argument-dependent lookup in bytes memory");
}

BOOST_AUTO_TEST_CASE(shift_constant_left_negative_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 << -8;
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator << not compatible with types int_const 66 and int_const -8");
}

BOOST_AUTO_TEST_CASE(shift_constant_right_negative_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 >> -8;
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator >> not compatible with types int_const 66 and int_const -8");
}

BOOST_AUTO_TEST_CASE(shift_constant_left_excessive_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 << 0x100000000;
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator << not compatible with types int_const 66 and int_const 4294967296");
}

BOOST_AUTO_TEST_CASE(shift_constant_right_excessive_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 >> 0x100000000;
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator >> not compatible with types int_const 66 and int_const 4294967296");
}

BOOST_AUTO_TEST_CASE(shift_constant_right_fractional)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 >> (1 / 2);
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator >> not compatible with types int_const 66 and rational_const 1 / 2");
}

BOOST_AUTO_TEST_CASE(inline_assembly_unbalanced_positive_stack)
{
	char const* text = R"(
		contract test {
			function f() public {
				assembly {
					1
				}
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Unbalanced stack at the end of a block: 1 surplus item(s).");
}

BOOST_AUTO_TEST_CASE(inline_assembly_unbalanced_negative_stack)
{
	char const* text = R"(
		contract test {
			function f() public {
				assembly {
					pop
				}
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Unbalanced stack at the end of a block: 1 missing item(s).");
}

BOOST_AUTO_TEST_CASE(inline_assembly_unbalanced_two_stack_load)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract c {
			uint8 x;
			function f() public {
				assembly { pop(x) }
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Only local variables are supported. To access storage variables,");
}

BOOST_AUTO_TEST_CASE(inline_assembly_in_modifier)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			modifier m {
				uint a = 1;
				assembly {
					a := 2
				}
				_;
			}
			function f() public m {
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint x = 1;
			function f() public {
				assembly {
					x := 2
				}
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Only local variables are supported. To access storage variables,");
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_in_modifiers)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint x = 1;
			modifier m {
				assembly {
					x := 2
				}
				_;
			}
			function f() public m {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Only local variables are supported. To access storage variables,");
}

BOOST_AUTO_TEST_CASE(inline_assembly_constant_assign)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint constant x = 1;
			function f() public {
				assembly {
					x := 2
				}
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Constant variables not supported by inline assembly");
}

BOOST_AUTO_TEST_CASE(inline_assembly_constant_access)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint constant x = 1;
			function f() public {
				assembly {
					let y := x
				}
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Constant variables not supported by inline assembly");
}

BOOST_AUTO_TEST_CASE(inline_assembly_local_variable_access_out_of_functions)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			function f() public {
				uint a;
				assembly {
					function g() -> x { x := a }
				}
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Cannot access local Solidity variables from inside an inline assembly function.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_local_variable_access_out_of_functions_storage_ptr)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint[] r;
			function f() public {
				uint[] storage a = r;
				assembly {
					function g() -> x { x := a_offset }
				}
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Cannot access local Solidity variables from inside an inline assembly function.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_variable_access_out_of_functions)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract test {
			uint a;
			function f() pure public {
				assembly {
					function g() -> x { x := a_slot }
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(inline_assembly_constant_variable_via_offset)
{
	char const* text = R"(
		contract test {
			uint constant x = 2;
			function f() pure public {
				assembly {
					let r := x_offset
				}
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Constant variables not supported by inline assembly.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_calldata_variables)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f(bytes bytesAsCalldata) external {
				assembly {
					let x := bytesAsCalldata
				}
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Call data elements cannot be accessed directly.");
}

BOOST_AUTO_TEST_CASE(inline_assembly_050_literals_on_stack)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				assembly {
					1
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::SyntaxError, "are not supposed to return"},
		{Error::Type::DeclarationError, "Unbalanced stack"},
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_literals_on_stack)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				assembly {
					1
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "are not supposed to return"},
		{Error::Type::DeclarationError, "Unbalanced stack"},
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_050_bare_instructions)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() view public {
				assembly {
					address
					pop
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::SyntaxError, "The use of non-functional"},
		{Error::Type::SyntaxError, "The use of non-functional"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_bare_instructions)
{
	char const* text = R"(
		contract C {
			function f() view public {
				assembly {
					address
					pop
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "The use of non-functional"},
		{Error::Type::Warning, "The use of non-functional"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_050_labels)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				assembly {
					label:
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::SyntaxError, "Jump instructions and labels are low-level"},
		{Error::Type::SyntaxError, "The use of labels is deprecated"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_labels)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				assembly {
					label:
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Jump instructions and labels are low-level"},
		{Error::Type::Warning, "The use of labels is deprecated"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_050_jump)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				assembly {
					jump(2)
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::SyntaxError, "Jump instructions and labels are low-level"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_jump)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				assembly {
					jump(2)
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::TypeError, "Function declared as pure"},
		{Error::Type::Warning, "Jump instructions and labels are low-level"}
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_050_leave_items_on_stack)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				assembly {
					mload(0)
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::SyntaxError, "are not supposed to return"},
		{Error::Type::DeclarationError, "Unbalanced stack"},
	}));
}

BOOST_AUTO_TEST_CASE(inline_assembly_leave_items_on_stack)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				assembly {
					mload(0)
				}
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "are not supposed to return"},
		{Error::Type::DeclarationError, "Unbalanced stack"},
	}));
}

BOOST_AUTO_TEST_CASE(invalid_mobile_type)
{
	char const* text = R"(
			contract C {
				function f() public {
					// Invalid number
					[1, 78901234567890123456789012345678901234567890123456789345678901234567890012345678012345678901234567];
				}
			}
	)";
	CHECK_ERROR(text, TypeError, "Invalid rational number.");
}

BOOST_AUTO_TEST_CASE(warns_msg_value_in_non_payable_public_function)
{
	char const* text = R"(
		contract C {
			function f() view public {
				msg.value;
			}
		}
	)";
	CHECK_WARNING(text, "\"msg.value\" used in non-payable function. Do you want to add the \"payable\" modifier to this function?");
}

BOOST_AUTO_TEST_CASE(does_not_warn_msg_value_in_payable_function)
{
	char const* text = R"(
		contract C {
			function f() payable public {
				msg.value;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(does_not_warn_msg_value_in_internal_function)
{
	char const* text = R"(
		contract C {
			function f() view internal {
				msg.value;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(does_not_warn_msg_value_in_library)
{
	char const* text = R"(
		library C {
			function f() view public {
				msg.value;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(does_not_warn_msg_value_in_modifier_following_non_payable_public_function)
{
	char const* text = R"(
		contract c {
			function f() pure public { }
			modifier m() { msg.value; _; }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(assignment_to_constant)
{
	char const* text = R"(
		contract c {
			uint constant a = 1;
			function f() public { a = 2; }
		}
	)";
	CHECK_ERROR(text, TypeError, "Cannot assign to a constant variable.");
}

BOOST_AUTO_TEST_CASE(return_structs)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; T[] sub; }
			struct T { uint[] x; }
			function f() returns (uint, S) {
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(read_returned_struct)
{
	char const* text = R"(
		pragma experimental ABIEncoderV2;
		contract A {
			struct T {
				int x;
				int y;
			}
			function g() public returns (T) {
				return this.g();
			}
		}
	)";
	CHECK_WARNING(text, "Experimental features");
}
BOOST_AUTO_TEST_CASE(address_checksum_type_deduction)
{
	char const* text = R"(
		contract C {
			function f() public {
				var x = 0xfA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
				x.send(2);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(invalid_address_checksum)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				address x = 0xFA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
				x;
			}
		}
	)";
	CHECK_WARNING(text, "This looks like an address but has an invalid checksum.");
}

BOOST_AUTO_TEST_CASE(invalid_address_no_checksum)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				address x = 0xfa0bfc97e48458494ccd857e1a85dc91f7f0046e;
				x;
			}
		}
	)";
	CHECK_WARNING(text, "This looks like an address but has an invalid checksum.");
}

BOOST_AUTO_TEST_CASE(invalid_address_length_short)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				address x = 0xA0bFc97E48458494Ccd857e1A85DC91F7F0046E;
				x;
			}
		}
	)";
	CHECK_WARNING(text, "This looks like an address but has an invalid checksum.");
}

BOOST_AUTO_TEST_CASE(invalid_address_length_long)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				address x = 0xFA0bFc97E48458494Ccd857e1A85DC91F7F0046E0;
				x;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "This looks like an address but has an invalid checksum."},
		{Error::Type::TypeError, "not implicitly convertible"}
	}));
}

BOOST_AUTO_TEST_CASE(string_literal_not_convertible_to_address_as_assignment)
{
	char const* text = R"(
		// A previous implementation claimed the string would be an address
		contract AddrString {
			address public test = "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c";
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type address");
}

BOOST_AUTO_TEST_CASE(string_literal_not_convertible_to_address_as_return_value)
{
	char const* text = R"(
		// A previous implementation claimed the string would be an address
		contract AddrString {
			function f() public returns (address) {
				return "0xCA35b7d915458EF540aDe6068dFe2F44E8fa733c";
		   }
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type");
}

BOOST_AUTO_TEST_CASE(early_exit_on_fatal_errors)
{
	char const* text = R"(
		// This tests a crash that occured because we did not stop for fatal errors.
		contract C {
			struct S {
				ftring a;
			}
			S public s;
			function s() s {
			}
		}
	)";
	CHECK_ERROR(text, DeclarationError, "Identifier not found or not unique");
}

BOOST_AUTO_TEST_CASE(address_methods)
{
	char const* text = R"(
		contract C {
			function f() public {
				address addr;
				uint balance = addr.balance;
				bool callRet = addr.call();
				bool callcodeRet = addr.callcode();
				bool delegatecallRet = addr.delegatecall();
				bool sendRet = addr.send(1);
				addr.transfer(1);
				callRet; callcodeRet; delegatecallRet; sendRet;
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface)
{
	char const* text = R"(
		interface I {
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface_functions)
{
	char const* text = R"(
		interface I {
			function();
			function f();
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface_function_bodies)
{
	char const* text = R"(
		interface I {
			function f() public {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Functions in interfaces cannot have an implementation");
}

BOOST_AUTO_TEST_CASE(interface_events)
{
	char const* text = R"(
		interface I {
			event E();
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface_inheritance)
{
	char const* text = R"(
		interface A {
		}
		interface I is A {
		}
	)";
	CHECK_ERROR(text, TypeError, "Interfaces cannot inherit");
}


BOOST_AUTO_TEST_CASE(interface_structs)
{
	char const* text = R"(
		interface I {
			struct A {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Structs cannot be defined in interfaces");
}

BOOST_AUTO_TEST_CASE(interface_variables)
{
	char const* text = R"(
		interface I {
			uint a;
		}
	)";
	CHECK_ERROR(text, TypeError, "Variables cannot be declared in interfaces");
}

BOOST_AUTO_TEST_CASE(interface_function_parameters)
{
	char const* text = R"(
		interface I {
			function f(uint a) public returns (bool);
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface_enums)
{
	char const* text = R"(
		interface I {
			enum A { B, C }
		}
	)";
	CHECK_ERROR(text, TypeError, "Enumerable cannot be declared in interfaces");
}

BOOST_AUTO_TEST_CASE(using_interface)
{
	char const* text = R"(
		interface I {
			function f();
		}
		contract C is I {
			function f() public {
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(using_interface_complex)
{
	char const* text = R"(
		interface I {
			event A();
			function f();
			function g();
			function();
		}
		contract C is I {
			function f() public {
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(interface_implement_public_contract)
{
	char const* text = R"(
		interface I {
			function f() external;
		}
		contract C is I {
			function f() public {
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(throw_is_deprecated)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				throw;
			}
		}
	)";
	CHECK_WARNING(text, "\"throw\" is deprecated in favour of \"revert()\", \"require()\" and \"assert()\"");
}

BOOST_AUTO_TEST_CASE(throw_is_deprecated_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure public {
				throw;
			}
		}
	)";
	CHECK_ERROR(text, SyntaxError, "\"throw\" is deprecated in favour of \"revert()\", \"require()\" and \"assert()\"");
}

BOOST_AUTO_TEST_CASE(bare_revert)
{
	char const* text = R"(
		contract C {
			function f(uint x) pure public {
				if (x > 7)
					revert;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "No matching declaration found");
}

BOOST_AUTO_TEST_CASE(revert_with_reason)
{
	char const* text = R"(
		contract C {
			function f(uint x) pure public {
				if (x > 7)
					revert("abc");
				else
					revert();
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(bare_selfdestruct)
{
	char const* text = R"(
		contract C {
			function f() pure public { selfdestruct; }
		}
	)";
	CHECK_WARNING(text, "Statement has no effect.");
}

BOOST_AUTO_TEST_CASE(bare_assert)
{
	char const* text = R"(
		contract C {
			function f() pure public { assert; }
		}
	)";
	CHECK_WARNING(text, "Statement has no effect.");
}

BOOST_AUTO_TEST_CASE(bare_require)
{
	char const* text = R"(
		contract C {
			// This is different because it does have overloads.
			function f() pure public { require; }
		}
	)";
	CHECK_ERROR(text, TypeError, "No matching declaration found after variable lookup.");
}

BOOST_AUTO_TEST_CASE(pure_statement_in_for_loop)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				for (uint x = 0; x < 10; true)
					x++;
			}
		}
	)";
	CHECK_WARNING(text, "Statement has no effect.");
}

BOOST_AUTO_TEST_CASE(pure_statement_check_for_regular_for_loop)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				for (uint x = 0; true; x++)
				{}
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(warn_unused_local)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				uint a;
			}
		}
	)";
	CHECK_WARNING(text, "Unused local variable.");
}

BOOST_AUTO_TEST_CASE(warn_unused_local_assigned)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				uint a = 1;
			}
		}
	)";
	CHECK_WARNING(text, "Unused local variable.");
}

BOOST_AUTO_TEST_CASE(warn_unused_function_parameter)
{
	char const* text = R"(
		contract C {
			function f(uint a) pure public {
			}
		}
	)";
	CHECK_WARNING(text, "Unused function parameter. Remove or comment out the variable name to silence this warning.");
}

BOOST_AUTO_TEST_CASE(unused_unnamed_function_parameter)
{
	char const* text = R"(
		contract C {
			function f(uint) pure public {
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(warn_unused_return_parameter)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint a) {
			}
		}
	)";
	CHECK_WARNING(text, "Unused function parameter. Remove or comment out the variable name to silence this warning.");
}

BOOST_AUTO_TEST_CASE(warn_unused_return_parameter_with_explicit_return)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint a) {
				return;
			}
		}
	)";
	CHECK_WARNING(text, "Unused function parameter. Remove or comment out the variable name to silence this warning.");
}

BOOST_AUTO_TEST_CASE(unused_unnamed_return_parameter)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint) {
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(named_return_parameter)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint a) {
				a = 1;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(named_return_parameter_with_explicit_return)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint a) {
				return 1;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(unnamed_return_parameter_with_explicit_return)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint) {
				return 1;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(no_unused_warning_interface_arguments)
{
	char const* text = R"(
		interface I {
			function f(uint a) pure external returns (uint b);
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(no_unused_warning_abstract_arguments)
{
	char const* text = R"(
		contract C {
			function f(uint a) pure public returns (uint b);
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(no_unused_warnings)
{
	char const* text = R"(
		contract C {
			function f(uint a) pure public returns (uint b) {
				uint c = 1;
				b = a + c;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(no_unused_dec_after_use)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				a = 7;
				uint a;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(no_unused_inline_asm)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				uint a;
				assembly {
					a := 1
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_functions)
{
	char const* text = R"(
		contract C {
			function keccak256() pure public {}
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_variables)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				uint msg;
				msg;
			}
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_storage_variables)
{
	char const* text = R"(
		contract C {
			uint msg;
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtin_at_global_scope)
{
	char const* text = R"(
		contract msg {
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_parameters)
{
	char const* text = R"(
		contract C {
			function f(uint require) pure public {
				require = 2;
			}
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_return_parameters)
{
	char const* text = R"(
		contract C {
			function f() pure public returns (uint require) {
				require = 2;
			}
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_events)
{
	char const* text = R"(
		contract C {
			event keccak256();
		}
	)";
	CHECK_WARNING(text, "shadows a builtin symbol");
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_ignores_struct)
{
	char const* text = R"(
		contract C {
			struct a {
				uint msg;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_ignores_constructor)
{
	char const* text = R"(
		contract C {
			constructor() public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_overload_is_not_shadowing)
{
	char const* text = R"(
		contract C {
			function f() pure public {}
			function f(uint) pure public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_override_is_not_shadowing)
{
	char const* text = R"(
		contract D { function f() pure public {} }
		contract C is D {
			function f(uint) pure public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(event_parameter_cannot_shadow_state_variable)
{
	char const* text = R"(
		contract C {
			address a;
			event E(address a);
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(callable_crash)
{
	char const* text = R"(
		contract C {
			struct S { uint a; bool x; }
			S public s;
			function C() public {
				3({a: 1, x: true});
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type is not callable");
}

BOOST_AUTO_TEST_CASE(error_transfer_non_payable_fallback)
{
	char const* text = R"(
		// This used to be a test for a.transfer to generate a warning
		// because A's fallback function is not payable.

		contract A {
			function() public {}
		}

		contract B {
			A a;

			function() public {
				a.transfer(100);
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"transfer\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(error_transfer_no_fallback)
{
	char const* text = R"(
		// This used to be a test for a.transfer to generate a warning
		// because A does not have a payable fallback function.

		contract A {}

		contract B {
			A a;

			function() public {
				a.transfer(100);
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"transfer\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(error_send_non_payable_fallback)
{
	char const* text = R"(
		// This used to be a test for a.send to generate a warning
		// because A does not have a payable fallback function.

		contract A {
			function() public {}
		}

		contract B {
			A a;

			function() public {
				require(a.send(100));
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"send\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(does_not_error_transfer_payable_fallback)
{
	char const* text = R"(
		// This used to be a test for a.transfer to generate a warning
		// because A does not have a payable fallback function.

		contract A {
			function() payable public {}
		}

		contract B {
			A a;

			function() public {
				a.transfer(100);
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"transfer\" inherited from the address type is deprecated.");
}

BOOST_AUTO_TEST_CASE(does_not_error_transfer_regular_function)
{
	char const* text = R"(
		contract A {
			function transfer() pure public {}
		}

		contract B {
			A a;

			function() public {
				a.transfer();
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(returndatasize_as_variable)
{
	char const* text = R"(
		contract c { function f() public { uint returndatasize; assembly { returndatasize }}}
	)";
	vector<pair<Error::Type, std::string>> expectations(vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Variable is shadowed in inline assembly by an instruction of the same name"},
		{Error::Type::Warning, "The use of non-functional instructions is deprecated."},
		{Error::Type::DeclarationError, "Unbalanced stack"}
	});
	if (!dev::test::Options::get().evmVersion().supportsReturndata())
		expectations.emplace_back(make_pair(Error::Type::Warning, std::string("\"returndatasize\" instruction is only available for Byzantium-compatible")));
	CHECK_ALLOW_MULTI(text, expectations);
}

BOOST_AUTO_TEST_CASE(create2_as_variable)
{
	char const* text = R"(
		contract c { function f() public { uint create2; assembly { create2(0, 0, 0, 0) } }}
	)";
	CHECK_ALLOW_MULTI(text, (std::vector<std::pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Variable is shadowed in inline assembly by an instruction of the same name"},
		{Error::Type::Warning, "The \"create2\" instruction is not supported by the VM version"},
		{Error::Type::DeclarationError, "Unbalanced stack"},
		{Error::Type::Warning, "not supposed to return values"}
	}));
}

BOOST_AUTO_TEST_CASE(specified_storage_no_warn)
{
	char const* text = R"(
		contract C {
			struct S { uint a; string b; }
			S x;
			function f() view public {
				S storage y = x;
				y;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(unspecified_storage_warn)
{
	char const* text = R"(
		contract C {
			struct S { uint a; }
			S x;
			function f() view public {
				S y = x;
				y;
			}
		}
	)";
	CHECK_WARNING(text, "Variable is declared as a storage pointer. Use an explicit \"storage\" keyword to silence this warning");
}

BOOST_AUTO_TEST_CASE(unspecified_storage_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			struct S { uint a; }
			S x;
			function f() view public {
				S y = x;
				y;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Data location must be specified as either \"memory\" or \"storage\".");
}

BOOST_AUTO_TEST_CASE(storage_location_non_array_or_struct_disallowed)
{
	char const* text = R"(
		contract C {
			function f(uint storage a) public { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Data location can only be given for array or struct types.");
}

BOOST_AUTO_TEST_CASE(storage_location_non_array_or_struct_disallowed_is_not_fatal)
{
	char const* text = R"(
		contract C {
			function f(uint storage a) public {
				a = f;
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (std::vector<std::string>{"Data location can only be given for array or struct types."}));
}

BOOST_AUTO_TEST_CASE(implicit_conversion_disallowed)
{
	char const* text = R"(
		contract C {
			function f() public returns (bytes4) {
				uint32 tmp = 1;
				return tmp;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Return argument type uint32 is not implicitly convertible to expected type (type of first return variable) bytes4.");
}

BOOST_AUTO_TEST_CASE(too_large_arrays_for_calldata_external)
{
	char const* text = R"(
		contract C {
			function f(uint[85678901234] a) pure external {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Array is too large to be encoded.");
}

BOOST_AUTO_TEST_CASE(too_large_arrays_for_calldata_internal)
{
	char const* text = R"(
		contract C {
			function f(uint[85678901234] a) pure internal {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Array is too large to be encoded.");
}

BOOST_AUTO_TEST_CASE(too_large_arrays_for_calldata_public)
{
	char const* text = R"(
		contract C {
			function f(uint[85678901234] a) pure public {
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Array is too large to be encoded.");
}

BOOST_AUTO_TEST_CASE(explicit_literal_to_memory_string_assignment)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				string memory x = "abc";
				x;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(explicit_literal_to_storage_string_assignment)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				string storage x = "abc";
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type literal_string \"abc\" is not implicitly convertible to expected type string storage pointer.");
}

BOOST_AUTO_TEST_CASE(explicit_literal_to_unspecified_string_assignment)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				string x = "abc";
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Type literal_string \"abc\" is not implicitly convertible to expected type string storage pointer.");
}

BOOST_AUTO_TEST_CASE(explicit_literal_to_unspecified_string)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				string("abc");
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Explicit type conversion not allowed from \"literal_string \"abc\"\" to \"string storage pointer\"");
}

BOOST_AUTO_TEST_CASE(modifiers_access_storage_pointer)
{
	char const* text = R"(
		contract C {
			struct S { uint a; }
			modifier m(S storage x) {
				x;
				_;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_types_selector_1)
{
	char const* text = R"(
		contract C {
			function f() view returns (bytes4) {
				return f.selector;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"selector\" not found");
}

BOOST_AUTO_TEST_CASE(function_types_selector_2)
{
	char const* text = R"(
		contract C {
			function g() pure internal {
			}
			function f() view returns (bytes4) {
				return g.selector;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"selector\" not found");
}

BOOST_AUTO_TEST_CASE(function_types_selector_3)
{
	char const* text = R"(
		contract C {
			function f() view returns (bytes4) {
				function () g;
				return g.selector;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"selector\" not found");
}

BOOST_AUTO_TEST_CASE(function_types_selector_4)
{
	char const* text = R"(
		contract C {
			function f() pure external returns (bytes4) {
				return this.f.selector;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_types_selector_5)
{
	char const* text = R"(
		contract C {
			function h() pure external {
			}
			function f() view external returns (bytes4) {
				var g = this.h;
				return g.selector;
			}
		}
	)";
	CHECK_WARNING(text, "Use of the \"var\" keyword is deprecated.");
}

BOOST_AUTO_TEST_CASE(function_types_selector_6)
{
	char const* text = R"(
		contract C {
			function h() pure external {
			}
			function f() view external returns (bytes4) {
				function () pure external g = this.h;
				return g.selector;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_types_selector_7)
{
	char const* text = R"(
		contract C {
			function h() pure external {
			}
			function f() view external returns (bytes4) {
				function () pure external g = this.h;
				var i = g;
				return i.selector;
			}
		}
	)";
	CHECK_WARNING(text, "Use of the \"var\" keyword is deprecated.");
}

BOOST_AUTO_TEST_CASE(using_this_in_constructor)
{
	char const* text = R"(
		contract C {
			constructor() public {
				this.f();
			}
			function f() pure public {
			}
		}
	)";
	CHECK_WARNING(text, "\"this\" used in constructor");
}

BOOST_AUTO_TEST_CASE(do_not_crash_on_not_lvalue)
{
	char const* text = R"(
		// This checks for a bug that caused a crash because of continued analysis.
		contract C {
			mapping (uint => uint) m;
			function f() public {
				m(1) = 2;
			}
		}
	)";
	CHECK_ERROR_ALLOW_MULTI(text, TypeError, (std::vector<std::string>{
		"is not callable",
		"Expression has to be an lvalue",
		"Type int_const 2 is not implicitly"
	}));
}

BOOST_AUTO_TEST_CASE(builtin_keccak256_reject_gas)
{
	char const* text = R"(
		contract C {
			function f() public {
				keccak256.gas();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"gas\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_sha256_reject_gas)
{
	const char* text = R"(
		contract C {
			function f() public {
				sha256.gas();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"gas\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_ripemd160_reject_gas)
{
	const char* text = R"(
		contract C {
			function f() public {
				ripemd160.gas();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"gas\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_ecrecover_reject_gas)
{
	const char* text = R"(
		contract C {
			function f() public {
				ecrecover.gas();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"gas\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(gasleft)
{
	char const* text = R"(
		contract C {
			function f() public view returns (uint256 val) { return gasleft(); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(msg_gas_deprecated)
{
	char const* text = R"(
		contract C {
			function f() public view returns (uint256 val) { return msg.gas; }
		}
	)";
	CHECK_WARNING(text, "\"msg.gas\" has been deprecated in favor of \"gasleft()\"");
}

BOOST_AUTO_TEST_CASE(msg_gas_deprecated_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public returns (uint256 val) { return msg.gas; }
		}
	)";
	CHECK_ERROR(text, TypeError, "\"msg.gas\" has been deprecated in favor of \"gasleft()\"");
}

BOOST_AUTO_TEST_CASE(gasleft_shadowing_1)
{
	char const* text = R"(
		contract C {
			function gasleft() public pure returns (bytes32 val) { return "abc"; }
			function f() public pure returns (bytes32 val) { return gasleft(); }
		}
	)";
	CHECK_WARNING(text, "This declaration shadows a builtin symbol.");
}

BOOST_AUTO_TEST_CASE(gasleft_shadowing_2)
{
	char const* text = R"(
		contract C {
			uint gasleft;
			function f() public { gasleft = 42; }
		}
	)";
	CHECK_WARNING(text, "This declaration shadows a builtin symbol.");
}

BOOST_AUTO_TEST_CASE(builtin_keccak256_reject_value)
{
	char const* text = R"(
		contract C {
			function f() public {
				keccak256.value();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_sha256_reject_value)
{
	const char* text = R"(
		contract C {
			function f() public {
				sha256.value();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_ripemd160_reject_value)
{
	const char* text = R"(
		contract C {
			function f() public {
				ripemd160.value();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(builtin_ecrecover_reject_value)
{
	const char* text = R"(
		contract C {
			function f() public {
				ecrecover.value();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"value\" not found or not visible after argument-dependent lookup");
}

BOOST_AUTO_TEST_CASE(large_storage_array_fine)
{
	char const* text = R"(
		contract C {
			uint[2**64 - 1] x;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(large_storage_array_simple)
{
	char const* text = R"(
		contract C {
			uint[2**64] x;
		}
	)";
	CHECK_WARNING(text, "covers a large part of storage and thus makes collisions likely");
}

BOOST_AUTO_TEST_CASE(large_storage_arrays_combined)
{
	char const* text = R"(
		contract C {
			uint[200][200][2**30][][2**30] x;
		}
	)";
	CHECK_WARNING(text, "covers a large part of storage and thus makes collisions likely");
}

BOOST_AUTO_TEST_CASE(large_storage_arrays_struct)
{
	char const* text = R"(
		contract C {
			struct S { uint[2**30] x; uint[2**50] y; }
			S[2**20] x;
		}
	)";
	CHECK_WARNING(text, "covers a large part of storage and thus makes collisions likely");
}

BOOST_AUTO_TEST_CASE(large_storage_array_mapping)
{
	char const* text = R"(
		contract C {
			mapping(uint => uint[2**100]) x;
		}
	)";
	CHECK_WARNING(text, "covers a large part of storage and thus makes collisions likely");
}

BOOST_AUTO_TEST_CASE(library_function_without_implementation_public)
{
	char const* text = R"(
		library L {
			// This can be used as an "interface", hence it is allowed.
			function f() public;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(library_function_without_implementation_internal)
{
	char const* text = R"(
		library L {
			function f() internal;
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal library function must be implemented if declared.");
}

BOOST_AUTO_TEST_CASE(library_function_without_implementation_private)
{
	char const* text = R"(
		library L {
			function f() private;
		}
	)";
	CHECK_ERROR(text, TypeError, "Internal library function must be implemented if declared.");
}

BOOST_AUTO_TEST_CASE(using_for_with_non_library)
{
	char const* text = R"(
		// This tests a crash that was resolved by making the first error fatal.
		library L {
			struct S { uint d; }
			using S for S;
			function f(S _s) internal {
				_s.d = 1;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Library name expected.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_empty)
{
	char const* text = R"(
		pragma experimental;
	)";
	CHECK_ERROR(text, SyntaxError, "Experimental feature name is missing.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_unknown_number_literal)
{
	char const* text = R"(
		pragma experimental 123;
	)";
	CHECK_ERROR(text, SyntaxError, "Unsupported experimental feature name.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_unknown_string_literal)
{
	char const* text = R"(
		pragma experimental unsupportedName;
	)";
	CHECK_ERROR(text, SyntaxError, "Unsupported experimental feature name.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_unknown_quoted_string_literal)
{
	char const* text = R"(
		pragma experimental "unsupportedName";
	)";
	CHECK_ERROR(text, SyntaxError, "Unsupported experimental feature name.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_empy_string_literal)
{
	char const* text = R"(
		pragma experimental "";
	)";
	CHECK_ERROR(text, SyntaxError, "Empty experimental feature name is invalid.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_multiple_same_line)
{
	char const* text = R"(
		pragma experimental unsupportedName unsupportedName;
	)";
	CHECK_ERROR(text, SyntaxError, "Stray arguments.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_test_warning)
{
	char const* text = R"(
		pragma experimental __test;
	)";
	CHECK_WARNING(text, "Experimental features are turned on. Do not use experimental features on live deployments.");
}

BOOST_AUTO_TEST_CASE(experimental_pragma_duplicate)
{
	char const* text = R"(
		pragma experimental __test;
		pragma experimental __test;
	)";
	CHECK_ERROR_ALLOW_MULTI(text, SyntaxError, (std::vector<std::string>{"Duplicate experimental feature name."}));
}

BOOST_AUTO_TEST_CASE(reject_interface_creation)
{
	char const* text = R"(
		interface I {}
		contract C {
			function f() public {
				new I();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Cannot instantiate an interface.");
}

BOOST_AUTO_TEST_CASE(accept_library_creation)
{
	char const* text = R"(
		library L {}
		contract C {
			function f() public {
				new L();
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(reject_interface_constructors)
{
	char const* text = R"(
		interface I {}
		contract C is I(2) {}
	)";
	CHECK_ERROR(text, TypeError, "Wrong argument count for constructor call: 1 arguments given but expected 0.");
}

BOOST_AUTO_TEST_CASE(fallback_marked_external_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function () external { }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(fallback_marked_internal_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function () internal { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Fallback function must be defined as \"external\".");
}

BOOST_AUTO_TEST_CASE(fallback_marked_private_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function () private { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Fallback function must be defined as \"external\".");
}

BOOST_AUTO_TEST_CASE(fallback_marked_public_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function () public { }
		}
	)";
	CHECK_ERROR(text, TypeError, "Fallback function must be defined as \"external\".");
}

BOOST_AUTO_TEST_CASE(tuple_invalid_literal_too_large_for_uint)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				uint x;
				(x, ) = (1E111);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "is not implicitly convertible to expected type");
}

BOOST_AUTO_TEST_CASE(tuple_invalid_literal_too_large_unassigned)
{
	const char* text = R"(
		contract C {
			function f() pure public {
				uint x;
				(x, ) = (1, 1E111);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid rational number.");
}

BOOST_AUTO_TEST_CASE(tuple_invalid_literal_too_large_for_uint_multi)
{
	const char* text = R"(
		contract C {
			function f() pure public {
				uint x;
				(x, ) = (1E111, 1);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid rational number.");
}

BOOST_AUTO_TEST_CASE(tuple_invalid_literal_too_large_exp)
{
	const char* text = R"(
		contract C {
			function f() pure public {
				(2**270, 1);
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid rational number.");
}

BOOST_AUTO_TEST_CASE(tuple_invalid_literal_too_large_expression)
{
	const char* text = R"(
		contract C {
			function f() pure public {
				((2**270) / 2**100, 1);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(address_overload_resolution)
{
	char const* text = R"(
		contract C {
			function balance() returns (uint) {
				this.balance; // to avoid pureness warning
				return 1;
			}
			function transfer(uint amount) {
				address(this).transfer(amount); // to avoid pureness warning
			}
		}
		contract D {
			function f() {
				var x = (new C()).balance();
				x;
				(new C()).transfer(5);
			}
		}
	)";
	CHECK_SUCCESS(text);
}

BOOST_AUTO_TEST_CASE(array_length_invalid_expression_negative_bool)
{
	char const* text = R"(
		contract C {
			uint[-true] ids;
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(array_length_invalid_expression_int_divides_bool)
{
	char const* text = R"(
		contract C {
			uint[true/1] ids;
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(array_length_invalid_expression_bool_divides_int)
{
	char const* text = R"(
		contract C {
			uint[1/true] ids;
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(array_length_invalid_expression_scientific_literal)
{
	char const* text = R"(
		contract C {
			uint[1.111111E1111111111111] ids;
		}
	)";
	CHECK_ERROR(text, TypeError, "Invalid array length, expected integer literal or constant expression.");
}

BOOST_AUTO_TEST_CASE(array_length_invalid_expression_division_by_zero)
{
	char const* text = R"(
		contract C {
			uint[3/0] ids;
		}
	)";
	CHECK_ERROR(text, TypeError, "Operator / not compatible with types int_const 3 and int_const 0");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_balance)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.balance;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"balance\" inherited from the address type is deprecated.");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_transfer)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.transfer;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"transfer\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_send)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.send;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"send\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_call)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.call;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"call\" inherited from the address type is deprecated.");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_callcode)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.callcode;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"callcode\" inherited from the address type is deprecated"},
		{Error::Type::Warning, "\"callcode\" has been deprecated in favour of \"delegatecall\""}
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_contract_delegatecall)
{
	char const* text = R"(
		contract C {
			function f() view public {
				this.delegatecall;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"delegatecall\" inherited from the address type is deprecated.");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_balance)
{
	char const* text = R"(
		contract C {
			function f() view public {
				C c;
				c.balance;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"balance\" inherited from the address type is deprecated");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_transfer)
{
	char const* text = R"(
		contract C {
			function f() view public {
				C c;
				c.transfer;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"transfer\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_send)
{
	char const* text = R"(
		contract C {
			function f() view public {
				C c;
				c.send;
			}
		}
	)";
	CHECK_ALLOW_MULTI(text, (vector<pair<Error::Type, std::string>>{
		{Error::Type::Warning, "Using contract member \"send\" inherited from the address type is deprecated"},
		{Error::Type::TypeError, "Value transfer to a contract without a payable fallback function"}
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_call)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				C c;
				c.call;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"call\" inherited from the address type is deprecated");
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_callcode)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				C c;
				c.callcode;
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(text, (std::vector<std::string>{
		"Using contract member \"callcode\" inherited from the address type is deprecated",
		"\"callcode\" has been deprecated in favour of \"delegatecall\""
	}));
}

BOOST_AUTO_TEST_CASE(warn_about_address_members_on_non_this_contract_delegatecall)
{
	char const* text = R"(
		contract C {
			function f() pure public {
				C c;
				c.delegatecall;
			}
		}
	)";
	CHECK_WARNING(text, "Using contract member \"delegatecall\" inherited from the address type is deprecated");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_balance_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.balance;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"balance\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_transfer_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.transfer;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"transfer\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_send_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.send;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"send\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_call_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.call;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"call\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_callcode_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.callcode;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"callcode\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_address_members_on_contract_delegatecall_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public {
				this.delegatecall;
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "Member \"delegatecall\" not found or not visible after argument-dependent lookup in contract");
}

BOOST_AUTO_TEST_CASE(no_warning_for_using_members_that_look_like_address_members)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function transfer(uint) public;
			function f() public {
				this.transfer(10);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(event_emit_simple)
{
	char const* text = R"(
		contract C {
			event e();
			function f() public {
				emit e();
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(event_emit_complex)
{
	char const* text = R"(
		contract C {
			event e(uint a, string b);
			function f() public {
				emit e(2, "abc");
				emit e({b: "abc", a: 8});
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(event_emit_foreign_class)
{
	char const* text = R"(
		contract A { event e(uint a, string b); }
		contract C is A {
			function f() public {
				emit A.e(2, "abc");
				emit A.e({b: "abc", a: 8});
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(event_without_emit_deprecated)
{
	char const* text = R"(
		contract C {
			event e();
			function f() public {
				e();
			}
		}
	)";
	CHECK_WARNING(text, "without \"emit\" prefix");
}

BOOST_AUTO_TEST_CASE(events_without_emit_deprecated_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			event e();
			function f() public {
				e();
			}
		}
	)";
	CHECK_ERROR(text, TypeError, "have to be prefixed");
}

BOOST_AUTO_TEST_CASE(getter_is_memory_type)
{
	char const* text = R"(
		contract C {
			struct S { string m; }
			string[] public x;
			S[] public y;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Check that the getters return a memory strings, not a storage strings.
	ContractDefinition const& c = dynamic_cast<ContractDefinition const&>(*m_compiler.ast("").nodes().at(1));
	BOOST_CHECK(c.interfaceFunctions().size() == 2);
	for (auto const& f: c.interfaceFunctions())
	{
		auto const& retType = f.second->returnParameterTypes().at(0);
		BOOST_CHECK(retType->dataStoredIn(DataLocation::Memory));
	}
}

BOOST_AUTO_TEST_CASE(require_visibility_specifiers)
{
	char const* text = R"(
		contract C {
			function f() pure { }
		}
	)";
	CHECK_WARNING(text, "No visibility specified. Defaulting to");
}

BOOST_AUTO_TEST_CASE(require_visibility_specifiers_v050)
{
	char const* text = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() pure { }
		}
	)";
	CHECK_ERROR(text, SyntaxError, "No visibility specified.");
}

BOOST_AUTO_TEST_CASE(blockhash)
{
	char const* code = R"(
		contract C {
			function f() public view returns (bytes32) { return blockhash(3); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(code);
}

BOOST_AUTO_TEST_CASE(block_blockhash_deprecated)
{
	char const* code = R"(
		contract C {
			function f() public view returns (bytes32) {
				return block.blockhash(3);
			}
		}
	)";
	CHECK_WARNING(code, "\"block.blockhash()\" has been deprecated in favor of \"blockhash()\"");
}

BOOST_AUTO_TEST_CASE(block_blockhash_deprecated_v050)
{
	char const* code = R"(
		pragma experimental "v0.5.0";
		contract C {
			function f() public returns (bytes32) { return block.blockhash(3); }
		}
	)";
	CHECK_ERROR(code, TypeError, "\"block.blockhash()\" has been deprecated in favor of \"blockhash()\"");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
