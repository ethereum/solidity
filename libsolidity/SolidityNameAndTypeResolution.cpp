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
#include <libdevcore/SHA3.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/GlobalContext.h>
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

ASTPointer<SourceUnit> parseTextAndResolveNames(std::string const& _source)
{
	Parser parser;
	ASTPointer<SourceUnit> sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(_source)));
	NameAndTypeResolver resolver({});
	resolver.registerDeclarations(*sourceUnit);
	std::shared_ptr<GlobalContext> globalContext = make_shared<GlobalContext>();

	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			globalContext->setCurrentContract(*contract);
			resolver.updateDeclaration(*globalContext->getCurrentThis());
			resolver.updateDeclaration(*globalContext->getCurrentSuper());
			resolver.resolveNamesAndTypes(*contract);
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			globalContext->setCurrentContract(*contract);
			resolver.updateDeclaration(*globalContext->getCurrentThis());
			resolver.checkTypeRequirements(*contract);
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
					   "  function fun(uint256 arg1) { uint256 y; }"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(name_references)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f(uint256 arg) returns (uint out) { f(variable); test; out; }"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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

BOOST_AUTO_TEST_CASE(struct_definition_not_really_recursive)
{
	char const* text = R"(
		contract test {
			struct s1 { uint a; }
			struct s2 { s1 x; s1 y; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(type_inference_smoke_test)
{
	char const* text = "contract test {\n"
					   "  function f(uint256 arg1, uint32 arg2) returns (bool ret) { var x = arg1 + arg2 == 8; ret = x; }"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(type_checking_return)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r) { return 1 >= 2; }"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison)
{
	char const* text = "contract test {\n"
					   "  function f() { uint32(2) == int64(2); }"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(large_string_literal)
{
	char const* text = "contract test {\n"
					   "  function f() { var x = \"123456789012345678901234567890123\"; }"
					   "}\n";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(balance)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint256 x = address(0).balance;\n"
					   "  }\n"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(comparison_bitop_precedence)
{
	char const* text = "contract First {\n"
					   "  function fun() returns (bool ret) {\n"
					   "    return 1 & 2 == 8 & 9 && 1 ^ 2 < 4 | 6;\n"
					   "  }\n"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(function_no_implementation)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract test {\n"
		"  function functionName(bytes32 input) returns (bytes32 out);\n"
		"}\n";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->getNodes();
	ContractDefinition* contract = dynamic_cast<ContractDefinition*>(nodes[0].get());
	BOOST_CHECK(contract);
	BOOST_CHECK(!contract->isFullyImplemented());
	BOOST_CHECK(!contract->getDefinedFunctions()[0]->isFullyImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() {} }
		)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->getNodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[0].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[1].get());
	BOOST_CHECK(base);
	BOOST_CHECK(!base->isFullyImplemented());
	BOOST_CHECK(!base->getDefinedFunctions()[0]->isFullyImplemented());
	BOOST_CHECK(derived);
	BOOST_CHECK(derived->isFullyImplemented());
	BOOST_CHECK(derived->getDefinedFunctions()[0]->isFullyImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract_with_overload)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(bool); }
		contract derived is base { function foo(uint) {} }
		)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->getNodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[0].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[1].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->isFullyImplemented());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->isFullyImplemented());
}

BOOST_AUTO_TEST_CASE(create_abstract_contract)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract derived {
			base b;
			function foo() { b = new base();}
			}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(abstract_contract_constructor_args_optional)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract BaseBase { function BaseBase(uint j); }
		contract base is BaseBase { function foo(); }
		contract derived is base {
			function derived(uint i) BaseBase(i){}
			function foo() {}
		}
		)";
	ETH_TEST_REQUIRE_NO_THROW(parseTextAndResolveNames(text), "Parsing and name resolving failed");
}

BOOST_AUTO_TEST_CASE(abstract_contract_constructor_args_not_provided)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract BaseBase { function BaseBase(uint j); }
		contract base is BaseBase { function foo(); }
		contract derived is base {
			function derived(uint i) {}
			function foo() {}
		}
		)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->getNodes();
	BOOST_CHECK_EQUAL(nodes.size(), 3);
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_CHECK(derived);
	BOOST_CHECK(!derived->isFullyImplemented());
}

BOOST_AUTO_TEST_CASE(redeclare_implemented_abstract_function_as_abstract)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() {} }
		contract wrong is derived { function foo(); }
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(function_canonical_signature)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract Test {\n"
					   "  function foo(uint256 arg1, uint64 arg2, bool arg3) returns (uint256 ret) {\n"
					   "    ret = arg1 + arg2;\n"
					   "  }\n"
					   "}\n";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			BOOST_CHECK_EQUAL("foo(uint256,uint64,bool)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_canonical_signature_type_aliases)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract Test {\n"
					   "  function boo(uint arg1, bytes32 arg2, address arg3) returns (uint ret) {\n"
					   "    ret = 5;\n"
					   "  }\n"
					   "}\n";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint256,bytes32,address)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_external_types)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			function boo(uint arg2, bool arg3, bytes8 arg4, bool[2] pairs, uint[] dynamic, C carg, address[] addresses) external returns (uint ret) {
			   ret = 5;
			}
		})";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint256,bool,bytes8,bool[2],uint256[],address,address[])", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(enum_external_type)
{
	// bug #1801
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function boo(ActionChoices enumArg) external returns (uint ret) {
				ret = 5;
			}
		})";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseTextAndResolveNames(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->getNodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->getDefinedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint8)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_external_call_allowed_conversion)
{
	char const* text = R"(
		contract C {}
		contract Test {
			function externalCall()	{
				C arg;
				this.g(arg);
			}
			function g (C c) external {}
	})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(function_external_call_not_allowed_conversion)
{
	char const* text = R"(
		contract C {}
		contract Test {
			function externalCall()	{
				address arg;
				this.g(arg);
			}
			function g (C c) external {}
	})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(function_internal_allowed_conversion)
{
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			C a;
			function g (C c) {}
			function internalCall() {
				g(a);
			}
	})";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(function_internal_not_allowed_conversion)
{
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			address a;
			function g (C c) {}
			function internalCall() {
				g(a);
			}
	})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(cyclic_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B is A { }
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(legal_override_direct)
{
	char const* text = R"(
		contract B { function f() {} }
		contract C is B { function f(uint i) {} }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(legal_override_indirect)
{
	char const* text = R"(
		contract A { function f(uint a) {} }
		contract B { function f() {} }
		contract C is A, B { }
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(illegal_override_visibility)
{
	char const* text = R"(
		contract B { function f() internal {} }
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(constructor_visibility)
{
	// The constructor of a base class should not be visible in the derived class
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function f() { A x = A(0); } }
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(overriding_constructor)
{
	// It is fine to "override" constructor of a base class since it is invisible
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function A() returns (uint8 r) {} }
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(missing_base_constructor_arguments)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(base_constructor_arguments_override)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(implicit_derived_to_base_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() { A a = B(1); }
		}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
			modifier mod2(bytes7 a) { while (a == "1234567") _ }
		}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
			function f(uint8 a) mod1(a, true) mod2(r) returns (bytes7 r) { }
			modifier mod1(uint a, bool b) { if (b) _ }
			modifier mod2(bytes7 a) { while (a == "1234567") _ }
		}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation_local_variables)
{
	char const* text = R"(
		contract B {
			function f() mod(x) { uint x = 7; }
			modifier mod(uint a) { if (a > 0) _ }
		}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(legal_modifier_override)
{
	char const* text = R"(
		contract A { modifier mod(uint a) {} }
		contract B is A { modifier mod(uint a) {} }
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
					   "mapping(uint=>bytes4) public map;\n"
					   "mapping(uint=>mapping(uint=>bytes4)) public multiple_map;\n"
					   "}\n";

	ASTPointer<SourceUnit> source;
	ContractDefinition const* contract;
	ETH_TEST_CHECK_NO_THROW(source = parseTextAndResolveNames(text), "Parsing and Resolving names failed");
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
	BOOST_CHECK_EQUAL(returnParams.at(0), "bytes4");
	BOOST_CHECK(function->isConstant());

	function = retrieveFunctionBySignature(contract, "multiple_map(uint256,uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	params = function->getParameterTypeNames();
	BOOST_CHECK_EQUAL(params.at(0), "uint256");
	BOOST_CHECK_EQUAL(params.at(1), "uint256");
	returnParams = function->getReturnParameterTypeNames();
	BOOST_CHECK_EQUAL(returnParams.at(0), "bytes4");
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
					   "uint256 internal bar;\n"
					   "}\n";

	ASTPointer<SourceUnit> source;
	ContractDefinition const* contract;
	ETH_TEST_CHECK_NO_THROW(source = parseTextAndResolveNames(text), "Parsing and Resolving names failed");
	BOOST_CHECK((contract = retrieveContract(source, 0)) != nullptr);
	FunctionTypePointer function;
	function = retrieveFunctionBySignature(contract, "foo()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a private variable should not exist");
	function = retrieveFunctionBySignature(contract, "bar()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of an internal variable should not exist");
}

BOOST_AUTO_TEST_CASE(base_class_state_variable_accessor)
{
	// test for issue #1126 https://github.com/ethereum/cpp-ethereum/issues/1126
	char const* text = "contract Parent {\n"
					   "    uint256 public m_aMember;\n"
					   "}\n"
					   "contract Child is Parent{\n"
					   "    function foo() returns (uint256) { return Parent.m_aMember; }\n"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(base_class_state_variable_internal_member)
{
	char const* text = "contract Parent {\n"
					   "    uint256 internal m_aMember;\n"
					   "}\n"
					   "contract Child is Parent{\n"
					   "    function foo() returns (uint256) { return Parent.m_aMember; }\n"
					   "}\n";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(state_variable_member_of_wrong_class1)
{
	char const* text = "contract Parent1 {\n"
					   "    uint256 internal m_aMember1;\n"
					   "}\n"
					   "contract Parent2 is Parent1{\n"
					   "    uint256 internal m_aMember2;\n"
					   "}\n"
					   "contract Child is Parent2{\n"
					   "    function foo() returns (uint256) { return Parent2.m_aMember1; }\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(state_variable_member_of_wrong_class2)
{
	char const* text = "contract Parent1 {\n"
					   "    uint256 internal m_aMember1;\n"
					   "}\n"
					   "contract Parent2 is Parent1{\n"
					   "    uint256 internal m_aMember2;\n"
					   "}\n"
					   "contract Child is Parent2{\n"
					   "    function foo() returns (uint256) { return Child.m_aMember2; }\n"
					   "    uint256 public m_aMember3;\n"
					   "}\n";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = R"(
		contract C {
			uint x;
			function() { x = 2; }
		}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(event_call)
{
	char const* text = R"(
		contract c {
			event e(uint a, bytes3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(event_inheritance)
{
	char const* text = R"(
		contract base {
			event e(uint a, bytes3 indexed s, bool indexed b);
		}
		contract c is base {
			function f() { e(2, "abc", true); }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(multiple_events_argument_clash)
{
	char const* text = R"(
		contract c {
			event e1(uint a, uint e1, uint e2);
			event e2(uint a, uint e1, uint e2);
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(access_to_internal_function)
{
	char const* text = R"(
		contract c {
			function f() internal {}
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

BOOST_AUTO_TEST_CASE(access_to_internal_state_variable)
{
	char const* text = R"(
		contract c {
			uint public a;
		}
		contract d {
			function g() { c(0).a(); }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* text = R"(
		contract test {
			function f() returns(bool){
		}
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f(uint, uint k) returns(uint ret_k){
				return k;
		}
	})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCodeFine),
		"Parsing and Resolving names failed");
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
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(enum_invalid_member_access)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				function test()
				{
					choices = ActionChoices.RunAroundWavingYourHands;
				}
				ActionChoices choices;
			}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(enum_explicit_conversion_is_okay)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				function test()
				{
					a = uint256(ActionChoices.GoStraight);
					b = uint64(ActionChoices.Sit);
				}
				uint256 a;
				uint64 b;
			}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(int_to_enum_explicit_conversion_is_okay)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				function test()
				{
					a = 2;
					b = ActionChoices(a);
				}
				uint256 a;
				ActionChoices b;
			}
	)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(enum_implicit_conversion_is_not_okay)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				function test()
				{
					a = ActionChoices.GoStraight;
					b = ActionChoices.Sit;
				}
				uint256 a;
				uint64 b;
			}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(enum_duplicate_values)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoLeft, Sit }
			}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), DeclarationError);
}

BOOST_AUTO_TEST_CASE(private_visibility)
{
	char const* sourceCode = R"(
		contract base {
			function f() private {}
		}
		contract derived is base {
			function g() { f(); }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), DeclarationError);
}

BOOST_AUTO_TEST_CASE(private_visibility_via_explicit_base_access)
{
	char const* sourceCode = R"(
		contract base {
			function f() private {}
		}
		contract derived is base {
			function g() { base.f(); }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(external_visibility)
{
	char const* sourceCode = R"(
		contract c {
			function f() external {}
			function g() { f(); }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), DeclarationError);
}

BOOST_AUTO_TEST_CASE(external_base_visibility)
{
	char const* sourceCode = R"(
		contract base {
			function f() external {}
		}
		contract derived is base {
			function g() { base.f(); }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_assign)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a = 1; }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_increment)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a++; }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_delete)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { delete a; }
		}
		)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(test_for_bug_override_function_with_bytearray_type)
{
	char const* sourceCode = R"(
		contract Vehicle {
			function f(bytes _a) external returns (uint256 r) {r = 1;}
		}
		contract Bike is Vehicle {
			function f(bytes _a) external returns (uint256 r) {r = 42;}
		}
		)";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode), "Parsing and Name Resolving failed");
}

BOOST_AUTO_TEST_CASE(array_with_nonconstant_length)
{
	char const* text = R"(
		contract c {
			function f(uint a) { uint8[a] x; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types1)
{
	char const* text = R"(
		contract c {
			bytes a;
			uint[] b;
			function f() { b = a; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types2)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() { b = a; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_conversion_possible)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() { a = b; }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_static_dynamic)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[80] b;
			function f() { a = b; }
		})";
	ETH_TEST_CHECK_NO_THROW(parseTextAndResolveNames(text), "Parsing and Name Resolving Failed");
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_dynamic_static)
{
	char const* text = R"(
		contract c {
			uint[] a;
			uint[80] b;
			function f() { b = a; }
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_int)
{
	char const* text = R"(
		contract c {
			uint8 a = 1000;
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_string)
{
	char const* text = R"(
		contract c {
			uint a = "abc";
		})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(test_fromElementaryTypeName)
{
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int) == *make_shared<IntegerType>(256, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int8) == *make_shared<IntegerType>(8, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int16) == *make_shared<IntegerType>(16, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int24) == *make_shared<IntegerType>(24, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int32) == *make_shared<IntegerType>(32, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int40) == *make_shared<IntegerType>(40, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int48) == *make_shared<IntegerType>(48, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int56) == *make_shared<IntegerType>(56, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int64) == *make_shared<IntegerType>(64, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int72) == *make_shared<IntegerType>(72, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int80) == *make_shared<IntegerType>(80, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int88) == *make_shared<IntegerType>(88, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int96) == *make_shared<IntegerType>(96, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int104) == *make_shared<IntegerType>(104, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int112) == *make_shared<IntegerType>(112, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int120) == *make_shared<IntegerType>(120, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int128) == *make_shared<IntegerType>(128, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int136) == *make_shared<IntegerType>(136, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int144) == *make_shared<IntegerType>(144, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int152) == *make_shared<IntegerType>(152, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int160) == *make_shared<IntegerType>(160, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int168) == *make_shared<IntegerType>(168, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int176) == *make_shared<IntegerType>(176, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int184) == *make_shared<IntegerType>(184, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int192) == *make_shared<IntegerType>(192, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int200) == *make_shared<IntegerType>(200, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int208) == *make_shared<IntegerType>(208, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int216) == *make_shared<IntegerType>(216, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int224) == *make_shared<IntegerType>(224, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int232) == *make_shared<IntegerType>(232, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int240) == *make_shared<IntegerType>(240, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int248) == *make_shared<IntegerType>(248, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Int256) == *make_shared<IntegerType>(256, IntegerType::Modifier::Signed));

	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt) == *make_shared<IntegerType>(256, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt8) == *make_shared<IntegerType>(8, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt16) == *make_shared<IntegerType>(16, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt24) == *make_shared<IntegerType>(24, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt32) == *make_shared<IntegerType>(32, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt40) == *make_shared<IntegerType>(40, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt48) == *make_shared<IntegerType>(48, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt56) == *make_shared<IntegerType>(56, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt64) == *make_shared<IntegerType>(64, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt72) == *make_shared<IntegerType>(72, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt80) == *make_shared<IntegerType>(80, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt88) == *make_shared<IntegerType>(88, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt96) == *make_shared<IntegerType>(96, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt104) == *make_shared<IntegerType>(104, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt112) == *make_shared<IntegerType>(112, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt120) == *make_shared<IntegerType>(120, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt128) == *make_shared<IntegerType>(128, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt136) == *make_shared<IntegerType>(136, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt144) == *make_shared<IntegerType>(144, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt152) == *make_shared<IntegerType>(152, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt160) == *make_shared<IntegerType>(160, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt168) == *make_shared<IntegerType>(168, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt176) == *make_shared<IntegerType>(176, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt184) == *make_shared<IntegerType>(184, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt192) == *make_shared<IntegerType>(192, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt200) == *make_shared<IntegerType>(200, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt208) == *make_shared<IntegerType>(208, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt216) == *make_shared<IntegerType>(216, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt224) == *make_shared<IntegerType>(224, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt232) == *make_shared<IntegerType>(232, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt240) == *make_shared<IntegerType>(240, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt248) == *make_shared<IntegerType>(248, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::UInt256) == *make_shared<IntegerType>(256, IntegerType::Modifier::Unsigned));

	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Byte) == *make_shared<FixedBytesType>(1));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes1) == *make_shared<FixedBytesType>(1));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes2) == *make_shared<FixedBytesType>(2));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes3) == *make_shared<FixedBytesType>(3));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes4) == *make_shared<FixedBytesType>(4));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes5) == *make_shared<FixedBytesType>(5));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes6) == *make_shared<FixedBytesType>(6));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes7) == *make_shared<FixedBytesType>(7));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes8) == *make_shared<FixedBytesType>(8));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes9) == *make_shared<FixedBytesType>(9));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes10) == *make_shared<FixedBytesType>(10));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes11) == *make_shared<FixedBytesType>(11));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes12) == *make_shared<FixedBytesType>(12));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes13) == *make_shared<FixedBytesType>(13));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes14) == *make_shared<FixedBytesType>(14));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes15) == *make_shared<FixedBytesType>(15));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes16) == *make_shared<FixedBytesType>(16));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes17) == *make_shared<FixedBytesType>(17));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes18) == *make_shared<FixedBytesType>(18));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes19) == *make_shared<FixedBytesType>(19));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes20) == *make_shared<FixedBytesType>(20));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes21) == *make_shared<FixedBytesType>(21));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes22) == *make_shared<FixedBytesType>(22));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes23) == *make_shared<FixedBytesType>(23));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes24) == *make_shared<FixedBytesType>(24));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes25) == *make_shared<FixedBytesType>(25));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes26) == *make_shared<FixedBytesType>(26));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes27) == *make_shared<FixedBytesType>(27));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes28) == *make_shared<FixedBytesType>(28));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes29) == *make_shared<FixedBytesType>(29));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes30) == *make_shared<FixedBytesType>(30));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes31) == *make_shared<FixedBytesType>(31));
	BOOST_CHECK(*Type::fromElementaryTypeName(Token::Bytes32) == *make_shared<FixedBytesType>(32));
}

BOOST_AUTO_TEST_CASE(test_byte_is_alias_of_byte1)
{
	char const* text = R"(
		contract c {
			bytes arr;
			function f() { byte a = arr[0];}
		})";
	ETH_TEST_REQUIRE_NO_THROW(parseTextAndResolveNames(text), "Type resolving failed");
}

BOOST_AUTO_TEST_CASE(assigning_value_to_const_variable)
{
	char const* text = R"(
		contract Foo {
			function changeIt() { x = 9; }
			uint constant x = 56;
	})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(complex_const_variable)
{
	//for now constant specifier is valid only for uint bytesXX and enums
	char const* text = R"(
		contract Foo {
			mapping(uint => bool) constant mapVar;
	})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(uninitialized_const_variable)
{
	char const* text = R"(
		contract Foo {
			uint constant y;
	})";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
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
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), ParserError);
}

BOOST_AUTO_TEST_CASE(overloaded_function_cannot_resolve)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns(uint) { return 1; }
			function f(uint a) returns(uint) { return a; }
			function g() returns(uint) { return f(3, 5); }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(ambiguous_overloaded_function)
{
	// literal 1 can be both converted to uint and uint8, so the call is ambiguous.
	char const* sourceCode = R"(
		contract test {
			function f(uint8 a) returns(uint) { return a; }
			function f(uint a) returns(uint) { return 2*a; }
			function g() returns(uint) { return f(1); }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_of_nonoverloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint) { return 2 * a; }
			function g() returns(uint) { var x = f; return x(7); }
		}
	)";
	ETH_TEST_REQUIRE_NO_THROW(parseTextAndResolveNames(sourceCode), "Type resolving failed");
}

BOOST_AUTO_TEST_CASE(assignment_of_overloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns(uint) { return 1; }
			function f(uint a) returns(uint) { return 2 * a; }
			function g() returns(uint) { var x = f; return x(7); }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(external_types_clash)
{
	char const* sourceCode = R"(
		contract base {
			enum a { X }
			function f(a) { }
		}
		contract test is base {
			function f(uint8 a) { }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(override_changes_return_types)
{
	char const* sourceCode = R"(
		contract base {
			function f(uint a) returns (uint) { }
		}
		contract test is base {
			function f(uint a) returns (uint8) { }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(multiple_constructors)
{
	char const* sourceCode = R"(
		contract test {
			function test(uint a) { }
			function test() {}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), DeclarationError);
}

BOOST_AUTO_TEST_CASE(equal_overload)
{
	char const* sourceCode = R"(
		contract test {
			function test(uint a) returns (uint b) { }
			function test(uint a) external {}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), DeclarationError);
}

BOOST_AUTO_TEST_CASE(uninitialized_var)
{
	char const* sourceCode = R"(
		contract C {
			function f() returns (uint) { var x; return 2; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(string)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f(string x) external { s = x; }
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(string_index)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() { var a = s[2]; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(string_length)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() { var a = s.length; }
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(negative_integers_to_signed_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			int8 public i = -129;
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(negative_integers_to_signed_min)
{
	char const* sourceCode = R"(
		contract test {
			int8 public i = -128;
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(positive_integers_to_signed_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			int8 public j = 128;
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(positive_integers_to_signed_out_of_bound_max)
{
	char const* sourceCode = R"(
		contract test {
			int8 public j = 127;
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(negative_integers_to_unsigned)
{
	char const* sourceCode = R"(
		contract test {
			uint8 public x = -1;
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(positive_integers_to_unsigned_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			uint8 public x = 700;
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(overwrite_memory_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] memory a) external {}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(overwrite_storage_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage a) external {}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(storage_location_local_variables)
{
	char const* sourceCode = R"(
		contract C {
			function f() {
				uint[] storage x;
				uint[] memory y;
				uint[] memory z;
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(no_mappings_in_memory_array)
{
	char const* sourceCode = R"(
		contract C {
			function f() {
				mapping(uint=>uint)[] memory x;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_mem_to_local_storage_variable)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f(uint[] x) {
				var dataRef = data;
				dataRef = x;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(storage_assign_to_different_local_variable)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			uint8[] otherData;
			function f() {
				uint8[] storage x = otherData;
				uint[] storage y = data;
				y = x;
				// note that data = otherData works
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(no_delete_on_storage_pointers)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f() {
				var x = data;
				delete x;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_mem_storage_variable_directly)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			function f(uint[] x) {
				data = x;
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(function_argument_mem_to_storage)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage x) private {
			}
			function g(uint[] x) {
				f(x);
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(function_argument_storage_to_mem)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage x) private {
				g(x);
			}
			function g(uint[] x) {
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(mem_array_assignment_changes_base_type)
{
	// Such an assignment is possible in storage, but not in memory
	// (because it would incur an otherwise unnecessary copy).
	// This requirement might be lifted, though.
	char const* sourceCode = R"(
		contract C {
			function f(uint8[] memory x) private {
				uint[] memory y = x;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(dynamic_return_types_not_possible)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint) returns (string);
			function g() {
				var x = this.f(2);
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(memory_arrays_not_resizeable)
{
	char const* sourceCode = R"(
		contract C {
			function f() {
				uint[] memory x;
				x.length = 2;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(sourceCode), TypeError);
}

BOOST_AUTO_TEST_CASE(struct_constructor)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint a; bool x; }
			function f() {
				S memory s = S(1, true);
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(struct_constructor_nested)
{
	char const* sourceCode = R"(
		contract C {
			struct X { uint x1; uint x2; }
			struct S { uint s1; uint[3] s2; X s3; }
			function f() {
				uint[3] memory s2;
				S memory s = S(1, s2, X(4, 5));
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(struct_named_constructor)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint a; bool x; }
			function f() {
				S memory s = S({a: 1, x: true});
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(sourceCode));
}

BOOST_AUTO_TEST_CASE(literal_strings)
{
	char const* text = R"(
		contract Foo {
			function f() {
				string memory long = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
				string memory short = "123";
			}
		}
	)";
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_CASE(invalid_integer_literal_fraction)
{
	char const* text = R"(
		contract Foo {
			function f() {
				var x = 1.20;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_integer_literal_exp)
{
	char const* text = R"(
		contract Foo {
			function f() {
				var x = 1e2;
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
}

BOOST_AUTO_TEST_CASE(memory_structs_with_mappings)
{
	char const* text = R"(
		contract Test {
			struct S { uint8 a; mapping(uint => uint) b; uint8 c; }
			S s;
			function f() {
				S memory x;
				x.b[1];
			}
		}
	)";
	BOOST_CHECK_THROW(parseTextAndResolveNames(text), TypeError);
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
	BOOST_CHECK_NO_THROW(parseTextAndResolveNames(text));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
