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

#include <libdevcore/SHA3.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/SyntaxChecker.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/TypeChecker.h>
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

pair<ASTPointer<SourceUnit>, std::shared_ptr<Error::Type const>>
parseAnalyseAndReturnError(string const& _source, bool _reportWarnings = false, bool _insertVersionPragma = true)
{
	// Silence compiler version warning
	string source = _insertVersionPragma ? "pragma solidity >=0.0;\n" + _source : _source;
	ErrorList errors;
	Parser parser(errors);
	ASTPointer<SourceUnit> sourceUnit;
	// catch exceptions for a transition period
	try
	{
		sourceUnit = parser.parse(std::make_shared<Scanner>(CharStream(source)));
		if(!sourceUnit)
			BOOST_FAIL("Parsing failed in type checker test.");

		SyntaxChecker syntaxChecker(errors);
		if (!syntaxChecker.checkSyntax(*sourceUnit))
			return make_pair(sourceUnit, std::make_shared<Error::Type const>(errors[0]->type()));

		std::shared_ptr<GlobalContext> globalContext = make_shared<GlobalContext>();
		NameAndTypeResolver resolver(globalContext->declarations(), errors);
		solAssert(Error::containsOnlyWarnings(errors), "");
		resolver.registerDeclarations(*sourceUnit);

		bool success = true;
		for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*globalContext->currentThis());
				resolver.updateDeclaration(*globalContext->currentSuper());
				if (!resolver.resolveNamesAndTypes(*contract))
					success = false;
			}
		if (success)
			for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
				{
					globalContext->setCurrentContract(*contract);
					resolver.updateDeclaration(*globalContext->currentThis());

					TypeChecker typeChecker(errors);
					bool success = typeChecker.checkTypeRequirements(*contract);
					BOOST_CHECK(success || !errors.empty());

				}
		for (auto const& currentError: errors)
		{
			if (
				(_reportWarnings && currentError->type() == Error::Type::Warning) ||
				(!_reportWarnings && currentError->type() != Error::Type::Warning)
			)
				return make_pair(sourceUnit, std::make_shared<Error::Type const>(currentError->type()));
		}
	}
	catch (InternalCompilerError const& _e)
	{
		string message("Internal compiler error");
		if (string const* description = boost::get_error_info<errinfo_comment>(_e))
			message += ": " + *description;
		BOOST_FAIL(message);
	}
	catch (Error const& _e)
	{
		return make_pair(sourceUnit, std::make_shared<Error::Type const>(_e.type()));
	}
	catch (...)
	{
		BOOST_FAIL("Unexpected exception.");
	}
	return make_pair(sourceUnit, nullptr);
}

ASTPointer<SourceUnit> parseAndAnalyse(string const& _source)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source);
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_REQUIRE(!sourceAndError.second);
	return sourceAndError.first;
}

bool success(string const& _source)
{
	return !parseAnalyseAndReturnError(_source).second;
}

Error::Type expectError(std::string const& _source, bool _warning = false)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source, _warning);
	BOOST_REQUIRE(!!sourceAndError.second);
	BOOST_REQUIRE(!!sourceAndError.first);
	return *sourceAndError.second;
}

static ContractDefinition const* retrieveContract(ASTPointer<SourceUnit> _source, unsigned index)
{
	ContractDefinition* contract;
	unsigned counter = 0;
	for (ASTPointer<ASTNode> const& node: _source->nodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && counter == index)
			return contract;

	return nullptr;
}

static FunctionTypePointer retrieveFunctionBySignature(
	ContractDefinition const* _contract,
	std::string const& _signature
)
{
	FixedHash<4> hash(dev::keccak256(_signature));
	return _contract->interfaceFunctions()[hash];
}

}

BOOST_AUTO_TEST_SUITE(SolidityNameAndTypeResolution)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = "contract test {\n"
					   "  uint256 stateVariable1;\n"
					   "  function fun(uint256 arg1) { uint256 y; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(double_stateVariable_declaration)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  uint128 variable;\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(double_function_declaration)
{
	char const* text = "contract test {\n"
					   "  function fun() { uint x; }\n"
					   "  function fun() { uint x; }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(double_variable_declaration)
{
	char const* text = "contract test {\n"
					   "  function f() { uint256 x; if (true)  { uint256 x; } }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(name_shadowing)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f() { uint32 variable ; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(name_references)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f(uint256 arg) returns (uint out) { f(variable); test; out; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(undeclared_name)
{
	char const* text = "contract test {\n"
					   "  uint256 variable;\n"
					   "  function f(uint256 arg) { f(notfound); }"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(reference_to_later_declaration)
{
	char const* text = "contract test {\n"
					   "  function g() { f(); }"
					   "  function f() {  }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(struct_definition_directly_recursive)
{
	char const* text = "contract test {\n"
					   "  struct MyStructName {\n"
					   "    address addr;\n"
					   "    MyStructName x;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(struct_definition_not_really_recursive)
{
	char const* text = R"(
		contract test {
			struct s1 { uint a; }
			struct s2 { s1 x; s1 y; }
		}
	)";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(type_inference_smoke_test)
{
	char const* text = "contract test {\n"
					   "  function f(uint256 arg1, uint32 arg2) returns (bool ret) { var x = arg1 + arg2 == 8; ret = x; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(type_checking_return)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_number)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r1, bool r2) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(type_checking_return_wrong_type)
{
	char const* text = "contract test {\n"
					   "  function f() returns (uint256 r) { return 1 >= 2; }"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(type_checking_function_call)
{
	char const* text = "contract test {\n"
					   "  function f() returns (bool r) { return g(12, true) == 3; }\n"
					   "  function g(uint256 a, bool b) returns (uint256 r) { }\n"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison)
{
	char const* text = "contract test {\n"
					   "  function f() { uint32(2) == int64(2); }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(type_conversion_for_comparison_invalid)
{
	char const* text = "contract test {\n"
					   "  function f() { int32(2) == uint64(2); }"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(type_inference_explicit_conversion)
{
	char const* text = "contract test {\n"
					   "  function f() returns (int256 r) { var x = int256(uint32(2)); return x; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(large_string_literal)
{
	char const* text = "contract test {\n"
					   "  function f() { var x = \"123456789012345678901234567890123\"; }"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(balance)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    uint256 x = address(0).balance;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(balance_invalid)
{
	char const* text = "contract test {\n"
					   "  function fun() {\n"
					   "    address(0).balance = 7;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(returns_in_constructor)
{
	char const* text = "contract test {\n"
					   "  function test() returns (uint a) {\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(comparison_bitop_precedence)
{
	char const* text = "contract First {\n"
					   "  function fun() returns (bool ret) {\n"
					   "    return 1 & 2 == 8 & 9 && 1 ^ 2 < 4 | 6;\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(function_no_implementation)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract test {\n"
		"  function functionName(bytes32 input) returns (bytes32 out);\n"
		"}\n";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* contract = dynamic_cast<ContractDefinition*>(nodes[1].get());
	BOOST_REQUIRE(contract);
	BOOST_CHECK(!contract->annotation().isFullyImplemented);
	BOOST_CHECK(!contract->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() {} }
		)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().isFullyImplemented);
	BOOST_CHECK(!base->definedFunctions()[0]->isImplemented());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(derived->annotation().isFullyImplemented);
	BOOST_CHECK(derived->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract_with_overload)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(bool); }
		contract derived is base { function foo(uint) {} }
		)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().isFullyImplemented);
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().isFullyImplemented);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	ETH_TEST_REQUIRE_NO_THROW(parseAndAnalyse(text), "Parsing and name resolving failed");
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
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	BOOST_CHECK_EQUAL(nodes.size(), 4);
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[3].get());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().isFullyImplemented);
}

BOOST_AUTO_TEST_CASE(redeclare_implemented_abstract_function_as_abstract)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract derived is base { function foo() {} }
		contract wrong is derived { function foo(); }
		)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(implement_abstract_via_constructor)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract base { function foo(); }
		contract foo is base { function foo() {} }
	)";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name resolving failed");
	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	BOOST_CHECK_EQUAL(nodes.size(), 3);
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().isFullyImplemented);
}

BOOST_AUTO_TEST_CASE(function_canonical_signature)
{
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = "contract Test {\n"
					   "  function foo(uint256 arg1, uint64 arg2, bool arg3) returns (uint256 ret) {\n"
					   "    ret = arg1 + arg2;\n"
					   "  }\n"
					   "}\n";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
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
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
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
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
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
	// bug #1801
	ASTPointer<SourceUnit> sourceUnit;
	char const* text = R"(
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function boo(ActionChoices enumArg) external returns (uint ret) {
				ret = 5;
			}
		})";
	ETH_TEST_REQUIRE_NO_THROW(sourceUnit = parseAndAnalyse(text), "Parsing and name Resolving failed");
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(hash_collision_in_interface)
{
	char const* text = "contract test {\n"
					   "  function gsf() {\n"
					   "  }\n"
					   "  function tgeo() {\n"
					   "  }\n"
					   "}\n";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(cyclic_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B is A { }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(legal_override_direct)
{
	char const* text = R"(
		contract B { function f() {} }
		contract C is B { function f(uint i) {} }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(legal_override_indirect)
{
	char const* text = R"(
		contract A { function f(uint a) {} }
		contract B { function f() {} }
		contract C is A, B { }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(illegal_override_visibility)
{
	char const* text = R"(
		contract B { function f() internal {} }
		contract C is B { function f() public {} }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_constness)
{
	char const* text = R"(
		contract B { function f() constant {} }
		contract C is B { function f() {} }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(complex_inheritance)
{
	char const* text = R"(
		contract A { function f() { uint8 x = C(0).g(); } }
		contract B { function f() {} function g() returns (uint8 r) {} }
		contract C is A, B { }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(constructor_visibility)
{
	// The constructor of a base class should not be visible in the derived class
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function f() { A x = A(0); } }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(overriding_constructor)
{
	// It is fine to "override" constructor of a base class since it is invisible
	char const* text = R"(
		contract A { function A() { } }
		contract B is A { function A() returns (uint8 r) {} }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(missing_base_constructor_arguments)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(base_constructor_arguments_override)
{
	char const* text = R"(
		contract A { function A(uint a) { } }
		contract B is A { }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(implicit_derived_to_base_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() { A a = B(1); }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(implicit_base_to_derived_conversion)
{
	char const* text = R"(
		contract A { }
		contract B is A {
			function f() { B b = A(1); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(super_excludes_current_contract)
{
	char const* text = R"(
		contract A {
			function b() {}
		}

		contract B is A {
			function f() {
				super.f();
			}
		}
	)";

	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation)
{
	char const* text = R"(
		contract B {
			function f() mod1(2, true) mod2("0123456") { }
			modifier mod1(uint a, bool b) { if (b) _; }
			modifier mod2(bytes7 a) { while (a == "1234567") _; }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(invalid_function_modifier_type)
{
	char const* text = R"(
		contract B {
			function f() mod1(true) { }
			modifier mod1(uint a) { if (a > 0) _; }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation_parameters)
{
	char const* text = R"(
		contract B {
			function f(uint8 a) mod1(a, true) mod2(r) returns (bytes7 r) { }
			modifier mod1(uint a, bool b) { if (b) _; }
			modifier mod2(bytes7 a) { while (a == "1234567") _; }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(function_modifier_invocation_local_variables)
{
	char const* text = R"(
		contract B {
			function f() mod(x) { uint x = 7; }
			modifier mod(uint a) { if (a > 0) _; }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(legal_modifier_override)
{
	char const* text = R"(
		contract A { modifier mod(uint a) { _; } }
		contract B is A { modifier mod(uint a) { _; } }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(illegal_modifier_override)
{
	char const* text = R"(
		contract A { modifier mod(uint a) { _; } }
		contract B is A { modifier mod(uint8 a) { _; } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(modifier_overrides_function)
{
	char const* text = R"(
		contract A { modifier mod(uint a) { _; } }
		contract B is A { function mod(uint a) { } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(function_overrides_modifier)
{
	char const* text = R"(
		contract A { function mod(uint a) { } }
		contract B is A { modifier mod(uint a) { _; } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(modifier_returns_value)
{
	char const* text = R"(
		contract A {
			function f(uint a) mod(2) returns (uint r) { }
			modifier mod(uint a) { _; return 7; }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	ETH_TEST_CHECK_NO_THROW(source = parseAndAnalyse(text), "Parsing and Resolving names failed");
	BOOST_REQUIRE((contract = retrieveContract(source, 0)) != nullptr);
	FunctionTypePointer function = retrieveFunctionBySignature(contract, "foo()");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto returnParams = function->returnParameterTypeNames(false);
	BOOST_CHECK_EQUAL(returnParams.at(0), "uint256");
	BOOST_CHECK(function->isConstant());

	function = retrieveFunctionBySignature(contract, "map(uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	auto params = function->parameterTypeNames(false);
	BOOST_CHECK_EQUAL(params.at(0), "uint256");
	returnParams = function->returnParameterTypeNames(false);
	BOOST_CHECK_EQUAL(returnParams.at(0), "bytes4");
	BOOST_CHECK(function->isConstant());

	function = retrieveFunctionBySignature(contract, "multiple_map(uint256,uint256)");
	BOOST_REQUIRE(function && function->hasDeclaration());
	params = function->parameterTypeNames(false);
	BOOST_CHECK_EQUAL(params.at(0), "uint256");
	BOOST_CHECK_EQUAL(params.at(1), "uint256");
	returnParams = function->returnParameterTypeNames(false);
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
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
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
	ETH_TEST_CHECK_NO_THROW(source = parseAndAnalyse(text), "Parsing and Resolving names failed");
	BOOST_CHECK((contract = retrieveContract(source, 0)) != nullptr);
	FunctionTypePointer function;
	function = retrieveFunctionBySignature(contract, "foo()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a private variable should not exist");
	function = retrieveFunctionBySignature(contract, "bar()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of an internal variable should not exist");
}

BOOST_AUTO_TEST_CASE(missing_state_variable)
{
	char const* text = R"(
		contract Scope {
			function getStateVar() constant returns (uint stateVar) {
				stateVar = Scope.stateVar; // should fail.
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(struct_accessor_one_array_only)
{
	char const* sourceCode = R"(
		contract test {
			struct Data {  uint[15] m_array; }
			Data public data;
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(base_class_state_variable_internal_member)
{
	char const* text = "contract Parent {\n"
					   "    uint256 internal m_aMember;\n"
					   "}\n"
					   "contract Child is Parent{\n"
					   "    function foo() returns (uint256) { return Parent.m_aMember; }\n"
					   "}\n";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* text = R"(
		contract C {
			uint x;
			function() { x = 2; }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(fallback_function_with_arguments)
{
	char const* text = R"(
		contract C {
			uint x;
			function(uint a) { x = 2; }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function_in_library)
{
	char const* text = R"(
		library C {
			function() {}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function_with_return_parameters)
{
	char const* text = R"(
		contract C {
			function() returns (uint) { }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(fallback_function_with_constant_modifier)
{
	char const* text = R"(
		contract C {
			uint x;
			function() constant { x = 2; }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d);
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(anonymous_event_four_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d) anonymous;
		})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(anonymous_event_too_many_indexed)
{
	char const* text = R"(
		contract c {
			event e(uint indexed a, bytes3 indexed b, bool indexed c, uint indexed d, uint indexed e) anonymous;
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(event_call)
{
	char const* text = R"(
		contract c {
			event e(uint a, bytes3 indexed s, bool indexed b);
			function f() { e(2, "abc", true); }
		})";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(multiple_events_argument_clash)
{
	char const* text = R"(
		contract c {
			event e1(uint a, uint e1, uint e2);
			event e2(uint a, uint e1, uint e2);
		})";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(error_count_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1}); }\n"
							 "}\n";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(empty_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({}); }\n"
							 "}\n";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(duplicate_parameter_names_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1, a: 2}); }\n"
							 "}\n";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_parameter_names_in_named_args)
{
	char const* sourceCode = "contract test {\n"
							 "  function a(uint a, uint b) returns (uint r) { r = a + b; }\n"
							 "  function b() returns (uint r) { r = a({a: 1, c: 2}); }\n"
							 "}\n";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter)
{
	char const* text = R"(
		contract test {
			function f(uint){
		}
	})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* text = R"(
		contract test {
			function f() returns(bool){
		}
		})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f(uint, uint k) returns(uint ret_k){
				return k;
		}
	})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter_with_named_one)
{
	char const* text = R"(
		contract test {
			function f() returns(uint ret_k, uint){
				return 5;
		}
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(disallow_declaration_of_void_type)
{
	char const* sourceCode = "contract c { function f() { var (x) = f(); } }";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	ETH_TEST_CHECK_NO_THROW(parseAndAnalyse(sourceCodeFine),
		"Parsing and Resolving names failed");
	char const* sourceCode = R"(
		contract c {
			function c ()
			{
				 a = 115792089237316195423570985008687907853269984665640564039458 ether;
			}
			uint256 a;
		})";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(exp_operator_exponent_too_big)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns(uint d) { return 2 ** 10000000000; }
		})";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(enum_member_access_accross_contracts)
{
	char const* text = R"(
			contract Interface {
				enum MyEnum { One, Two }
			}
			contract Impl {
				function test() returns (Interface.MyEnum) {
					return Interface.MyEnum.One;
				}
			}
	)";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(enum_invalid_direct_member_access)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				function test()
				{
					choices = Sit;
				}
				ActionChoices choices;
			}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(enum_to_enum_conversion_is_not_okay)
{
	char const* text = R"(
		contract test {
			enum Paper { Up, Down, Left, Right }
			enum Ground { North, South, West, East }
			function test()
			{
				Ground(Paper.Up);
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(enum_duplicate_values)
{
	char const* text = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoLeft, Sit }
			}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(enum_name_resolution_under_current_contract_name)
{
	char const* text = R"(
		contract A {
			enum Foo {
				First,
				Second
			}

			function a() {
				A.Foo;
			}
		}
	)";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::DeclarationError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(external_visibility)
{
	char const* sourceCode = R"(
		contract c {
			function f() external {}
			function g() { f(); }
		}
		)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::DeclarationError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_assign)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a = 1; }
		}
		)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_increment)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { a++; }
		}
		)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(external_argument_delete)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) external { delete a; }
		}
		)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	ETH_TEST_CHECK_NO_THROW(parseAndAnalyse(sourceCode), "Parsing and Name Resolving failed");
}

BOOST_AUTO_TEST_CASE(array_with_nonconstant_length)
{
	char const* text = R"(
		contract c {
			function f(uint a) { uint8[a] x; }
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types1)
{
	char const* text = R"(
		contract c {
			bytes a;
			uint[] b;
			function f() { b = a; }
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types2)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() { b = a; }
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_conversion_possible)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[] b;
			function f() { a = b; }
		})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_static_dynamic)
{
	char const* text = R"(
		contract c {
			uint32[] a;
			uint8[80] b;
			function f() { a = b; }
		})";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(array_copy_with_different_types_dynamic_static)
{
	char const* text = R"(
		contract c {
			uint[] a;
			uint[80] b;
			function f() { b = a; }
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_int)
{
	char const* text = R"(
		contract c {
			uint8 a = 1000;
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(storage_variable_initialization_with_incorrect_type_string)
{
	char const* text = R"(
		contract c {
			uint a = "abc";
		})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(test_fromElementaryTypeName)
{

	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::Int, 0, 0)) == *make_shared<IntegerType>(256, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 8, 0)) == *make_shared<IntegerType>(8, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 16, 0)) == *make_shared<IntegerType>(16, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 24, 0)) == *make_shared<IntegerType>(24, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 32, 0)) == *make_shared<IntegerType>(32, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 40, 0)) == *make_shared<IntegerType>(40, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 48, 0)) == *make_shared<IntegerType>(48, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 56, 0)) == *make_shared<IntegerType>(56, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 64, 0)) == *make_shared<IntegerType>(64, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 72, 0)) == *make_shared<IntegerType>(72, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 80, 0)) == *make_shared<IntegerType>(80, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 88, 0)) == *make_shared<IntegerType>(88, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 96, 0)) == *make_shared<IntegerType>(96, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 104, 0)) == *make_shared<IntegerType>(104, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 112, 0)) == *make_shared<IntegerType>(112, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 120, 0)) == *make_shared<IntegerType>(120, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 128, 0)) == *make_shared<IntegerType>(128, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 136, 0)) == *make_shared<IntegerType>(136, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 144, 0)) == *make_shared<IntegerType>(144, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 152, 0)) == *make_shared<IntegerType>(152, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 160, 0)) == *make_shared<IntegerType>(160, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 168, 0)) == *make_shared<IntegerType>(168, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 176, 0)) == *make_shared<IntegerType>(176, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 184, 0)) == *make_shared<IntegerType>(184, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 192, 0)) == *make_shared<IntegerType>(192, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 200, 0)) == *make_shared<IntegerType>(200, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 208, 0)) == *make_shared<IntegerType>(208, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 216, 0)) == *make_shared<IntegerType>(216, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 224, 0)) == *make_shared<IntegerType>(224, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 232, 0)) == *make_shared<IntegerType>(232, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 240, 0)) == *make_shared<IntegerType>(240, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 248, 0)) == *make_shared<IntegerType>(248, IntegerType::Modifier::Signed));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::IntM, 256, 0)) == *make_shared<IntegerType>(256, IntegerType::Modifier::Signed));

	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UInt, 0, 0)) == *make_shared<IntegerType>(256, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 8, 0)) == *make_shared<IntegerType>(8, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 16, 0)) == *make_shared<IntegerType>(16, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 24, 0)) == *make_shared<IntegerType>(24, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 32, 0)) == *make_shared<IntegerType>(32, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 40, 0)) == *make_shared<IntegerType>(40, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 48, 0)) == *make_shared<IntegerType>(48, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 56, 0)) == *make_shared<IntegerType>(56, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 64, 0)) == *make_shared<IntegerType>(64, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 72, 0)) == *make_shared<IntegerType>(72, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 80, 0)) == *make_shared<IntegerType>(80, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 88, 0)) == *make_shared<IntegerType>(88, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 96, 0)) == *make_shared<IntegerType>(96, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 104, 0)) == *make_shared<IntegerType>(104, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 112, 0)) == *make_shared<IntegerType>(112, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 120, 0)) == *make_shared<IntegerType>(120, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 128, 0)) == *make_shared<IntegerType>(128, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 136, 0)) == *make_shared<IntegerType>(136, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 144, 0)) == *make_shared<IntegerType>(144, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 152, 0)) == *make_shared<IntegerType>(152, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 160, 0)) == *make_shared<IntegerType>(160, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 168, 0)) == *make_shared<IntegerType>(168, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 176, 0)) == *make_shared<IntegerType>(176, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 184, 0)) == *make_shared<IntegerType>(184, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 192, 0)) == *make_shared<IntegerType>(192, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 200, 0)) == *make_shared<IntegerType>(200, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 208, 0)) == *make_shared<IntegerType>(208, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 216, 0)) == *make_shared<IntegerType>(216, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 224, 0)) == *make_shared<IntegerType>(224, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 232, 0)) == *make_shared<IntegerType>(232, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 240, 0)) == *make_shared<IntegerType>(240, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 248, 0)) == *make_shared<IntegerType>(248, IntegerType::Modifier::Unsigned));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::UIntM, 256, 0)) == *make_shared<IntegerType>(256, IntegerType::Modifier::Unsigned));

	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::Byte, 0, 0)) == *make_shared<FixedBytesType>(1));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 1, 0)) == *make_shared<FixedBytesType>(1));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 2, 0)) == *make_shared<FixedBytesType>(2));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 3, 0)) == *make_shared<FixedBytesType>(3));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 4, 0)) == *make_shared<FixedBytesType>(4));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 5, 0)) == *make_shared<FixedBytesType>(5));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 6, 0)) == *make_shared<FixedBytesType>(6));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 7, 0)) == *make_shared<FixedBytesType>(7));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 8, 0)) == *make_shared<FixedBytesType>(8));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 9, 0)) == *make_shared<FixedBytesType>(9));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 10, 0)) == *make_shared<FixedBytesType>(10));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 11, 0)) == *make_shared<FixedBytesType>(11));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 12, 0)) == *make_shared<FixedBytesType>(12));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 13, 0)) == *make_shared<FixedBytesType>(13));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 14, 0)) == *make_shared<FixedBytesType>(14));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 15, 0)) == *make_shared<FixedBytesType>(15));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 16, 0)) == *make_shared<FixedBytesType>(16));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 17, 0)) == *make_shared<FixedBytesType>(17));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 18, 0)) == *make_shared<FixedBytesType>(18));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 19, 0)) == *make_shared<FixedBytesType>(19));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 20, 0)) == *make_shared<FixedBytesType>(20));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 21, 0)) == *make_shared<FixedBytesType>(21));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 22, 0)) == *make_shared<FixedBytesType>(22));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 23, 0)) == *make_shared<FixedBytesType>(23));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 24, 0)) == *make_shared<FixedBytesType>(24));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 25, 0)) == *make_shared<FixedBytesType>(25));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 26, 0)) == *make_shared<FixedBytesType>(26));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 27, 0)) == *make_shared<FixedBytesType>(27));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 28, 0)) == *make_shared<FixedBytesType>(28));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 29, 0)) == *make_shared<FixedBytesType>(29));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 30, 0)) == *make_shared<FixedBytesType>(30));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 31, 0)) == *make_shared<FixedBytesType>(31));
	BOOST_CHECK(*Type::fromElementaryTypeName(ElementaryTypeNameToken(Token::BytesM, 32, 0)) == *make_shared<FixedBytesType>(32));
}

BOOST_AUTO_TEST_CASE(test_byte_is_alias_of_byte1)
{
	char const* text = R"(
		contract c {
			bytes arr;
			function f() { byte a = arr[0];}
		})";
	ETH_TEST_REQUIRE_NO_THROW(parseAndAnalyse(text), "Type resolving failed");
}

BOOST_AUTO_TEST_CASE(assigning_value_to_const_variable)
{
	char const* text = R"(
		contract Foo {
			function changeIt() { x = 9; }
			uint constant x = 56;
	})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(complex_const_variable)
{
	//for now constant specifier is valid only for uint bytesXX and enums
	char const* text = R"(
		contract Foo {
			mapping(uint => bool) constant mapVar;
	})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(uninitialized_const_variable)
{
	char const* text = R"(
		contract Foo {
			uint constant y;
	})";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(assignment_of_nonoverloaded_function)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint) { return 2 * a; }
			function g() returns(uint) { var x = f; return x(7); }
		}
	)";
	ETH_TEST_REQUIRE_NO_THROW(parseAndAnalyse(sourceCode), "Type resolving failed");
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multiple_constructors)
{
	char const* sourceCode = R"(
		contract test {
			function test(uint a) { }
			function test() {}
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(equal_overload)
{
	char const* sourceCode = R"(
		contract test {
			function test(uint a) returns (uint b) { }
			function test(uint a) external {}
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(uninitialized_var)
{
	char const* sourceCode = R"(
		contract C {
			function f() returns (uint) { var x; return 2; }
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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

BOOST_AUTO_TEST_CASE(string_index)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() { var a = s[2]; }
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(string_length)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f() { var a = s.length; }
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(negative_integers_to_signed_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			int8 public i = -129;
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(positive_integers_to_unsigned_out_of_bound)
{
	char const* sourceCode = R"(
		contract test {
			uint8 public x = 700;
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(integer_boolean_operators)
{
	char const* sourceCode1 = R"(
		contract test { function() { uint x = 1; uint y = 2; x || y; } }
	)";
	BOOST_CHECK(expectError(sourceCode1) == Error::Type::TypeError);
	char const* sourceCode2 = R"(
		contract test { function() { uint x = 1; uint y = 2; x && y; } }
	)";
	BOOST_CHECK(expectError(sourceCode2) == Error::Type::TypeError);
	char const* sourceCode3 = R"(
		contract test { function() { uint x = 1; !x; } }
	)";
	BOOST_CHECK(expectError(sourceCode3) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(exp_signed_variable)
{
	char const* sourceCode1 = R"(
		contract test { function() { uint x = 3; int y = -4; x ** y; } }
	)";
	BOOST_CHECK(expectError(sourceCode1) == Error::Type::TypeError);
	char const* sourceCode2 = R"(
		contract test { function() { uint x = 3; int y = -4; y ** x; } }
	)";
	BOOST_CHECK(expectError(sourceCode2) == Error::Type::TypeError);
	char const* sourceCode3 = R"(
		contract test { function() { int x = -3; int y = -4; x ** y; } }
	)";
	BOOST_CHECK(expectError(sourceCode3) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(reference_compare_operators)
{
	char const* sourceCode1 = R"(
		contract test { bytes a; bytes b; function() { a == b; } }
	)";
	BOOST_CHECK(expectError(sourceCode1) == Error::Type::TypeError);
	char const* sourceCode2 = R"(
		contract test { struct s {uint a;} s x; s y; function() { x == y; } }
	)";
	BOOST_CHECK(expectError(sourceCode2) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(overwrite_memory_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] memory a) external {}
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(overwrite_storage_location_external)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint[] storage a) external {}
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(dynamic_return_types_not_possible)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint) returns (string);
			function g() {
				var (x,) = this.f(2);
				// we can assign to x but it is not usable.
				bytes(x).length;
			}
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK_NO_THROW(parseAndAnalyse(sourceCode));
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
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inheriting_from_library)
{
	char const* text = R"(
		library Lib {}
		contract Test is Lib {}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(inheriting_library)
{
	char const* text = R"(
		contract Test {}
		library Lib is Test {}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(library_having_variables)
{
	char const* text = R"(
		library Lib { uint x; }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(valid_library)
{
	char const* text = R"(
		library Lib { uint constant x = 9; }
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(call_to_library_function)
{
	char const* text = R"(
		library Lib {
			function min(uint x, uint y) returns (uint);
		}
		contract Test {
			function f() {
				uint t = Lib.min(12, 7);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(creating_contract_within_the_contract)
{
	char const* sourceCode = R"(
		contract Test {
			function f() { var x = new Test(); }
		}
	)";
	BOOST_CHECK(expectError(sourceCode) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(array_out_of_bound_access)
{
	char const* text = R"(
		contract c {
			uint[2] dataArray;
			function set5th() returns (bool) {
				dataArray[5] = 2;
				return true;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(literal_string_to_storage_pointer)
{
	char const* text = R"(
		contract C {
			function f() { string x = "abc"; }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(non_initialized_references)
{
	char const* text = R"(
		contract c
		{
			struct s{
				uint a;
			}
			function f()
			{
				s x;
				x.a = 2;
			}
		}
	)";

	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(sha3_with_large_integer_constant)
{
	char const* text = R"(
		contract c
		{
			function f() { sha3(2**500); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(cyclic_binary_dependency)
{
	char const* text = R"(
		contract A { function f() { new B(); } }
		contract B { function f() { new C(); } }
		contract C { function f() { new A(); } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(cyclic_binary_dependency_via_inheritance)
{
	char const* text = R"(
		contract A is B { }
		contract B { function f() { new C(); } }
		contract C { function f() { new A(); } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_fail)
{
	char const* text = R"(
		contract C { function f() { var (x,y); } }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fine)
{
	char const* text = R"(
		contract C {
			function three() returns (uint, uint, uint);
			function two() returns (uint, uint);
			function none();
			function f() {
				var (a,) = three();
				var (b,c,) = two();
				var (,d) = three();
				var (,e,g) = two();
				var (,,) = three();
				var () = none();
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_1)
{
	char const* text = R"(
		contract C {
			function one() returns (uint);
			function f() { var (a, b, ) = one(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}
BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_2)
{
	char const* text = R"(
		contract C {
			function one() returns (uint);
			function f() { var (a, , ) = one(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_3)
{
	char const* text = R"(
		contract C {
			function one() returns (uint);
			function f() { var (, , a) = one(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_4)
{
	char const* text = R"(
		contract C {
			function one() returns (uint);
			function f() { var (, a, b) = one(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(tuples_empty_components)
{
	char const* text = R"(
		contract C {
			function f() {
				(1,,2);
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_5)
{
	char const* text = R"(
		contract C {
			function one() returns (uint);
			function f() { var (,) = one(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration_wildcards_fail_6)
{
	char const* text = R"(
		contract C {
			function two() returns (uint, uint);
			function f() { var (a, b, c) = two(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(member_access_parser_ambiguity)
{
	char const* text = R"(
		contract C {
			struct R { uint[10][10] y; }
			struct S { uint a; uint b; uint[20][20][20] c; R d; }
			S data;
			function f() {
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_library)
{
	char const* text = R"(
		library D { }
		contract C {
			using D for uint;
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_not_library)
{
	char const* text = R"(
		contract D { }
		contract C {
			using D for uint;
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(using_for_function_exists)
{
	char const* text = R"(
		library D { function double(uint self) returns (uint) { return 2*self; } }
		contract C {
			using D for uint;
			function f(uint a) {
				a.double;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_function_on_int)
{
	char const* text = R"(
		library D { function double(uint self) returns (uint) { return 2*self; } }
		contract C {
			using D for uint;
			function f(uint a) returns (uint) {
				return a.double();
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_function_on_struct)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) returns (uint) {
				return x.mul(a);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_overload)
{
	char const* text = R"(
		library D {
			struct s { uint a; }
			function mul(s storage self, uint x) returns (uint) { return self.a *= x; }
			function mul(s storage self, bytes32 x) returns (bytes32) { }
		}
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) returns (uint) {
				return x.mul(a);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_by_name)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) returns (uint) {
				return x.mul({x: a});
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(using_for_mismatch)
{
	char const* text = R"(
		library D { function double(bytes32 self) returns (uint) { return 2; } }
		contract C {
			using D for uint;
			function f(uint a) returns (uint) {
				return a.double();
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(using_for_not_used)
{
	// This is an error because the function is only bound to uint.
	// Had it been bound to *, it would have worked.
	char const* text = R"(
		library D { function double(uint self) returns (uint) { return 2; } }
		contract C {
			using D for uint;
			function f(uint16 a) returns (uint) {
				return a.double();
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(using_for_arbitrary_mismatch)
{
	// Bound to a, but self type does not match.
	char const* text = R"(
		library D { function double(bytes32 self) returns (uint) { return 2; } }
		contract C {
			using D for *;
			function f(uint a) returns (uint) {
				return a.double();
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(bound_function_in_var)
{
	char const* text = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s x;
			function f(uint a) returns (uint) {
				var g = x.mul;
				return g({x: a});
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(create_memory_arrays)
{
	char const* text = R"(
		library L {
			struct R { uint[10][10] y; }
			struct S { uint a; uint b; uint[20][20][20] c; R d; }
		}
		contract C {
			function f(uint size) {
				L.S[][] memory x = new L.S[][](10);
				var y = new uint[](20);
				var z = new bytes(size);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(mapping_in_memory_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) {
				var x = new mapping(uint => uint)[](4);
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(new_for_non_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) {
				var x = new uint(7);
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_args_creating_memory_array)
{
	char const* text = R"(
		contract C {
			function f(uint size) {
				var x = new uint[]();
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(function_overload_array_type)
{
	char const* text = R"(
			contract M {
				function f(uint[] values);
				function f(int[] values);
			}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_and_passing_implicit_conversion)
{
	char const* text = R"(
			contract C {
				function f() returns (uint) {
					uint8 x = 7;
					uint16 y = 8;
					uint32 z = 9;
					uint32[3] memory ending = [x, y, z];
					return (ending[1]);
				}
			}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_and_passing_implicit_conversion_strings)
{
	char const* text = R"(
		contract C {
			function f() returns (string) {
				string memory x = "Hello";
				string memory y = "World";
				string[2] memory z = [x, y];
				return (z[0]);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_const_int_conversion)
{
	char const* text = R"(
		contract C {
			function f() returns (uint) {
				uint8[4] memory z = [1,2,3,5];
				return (z[0]);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_const_string_conversion)
{
	char const* text = R"(
		contract C {
			function f() returns (string) {
				string[2] memory z = ["Hello", "World"];
				return (z[0]);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_no_type)
{
	char const* text = R"(
		contract C {
			function f() returns (uint) {
				return ([4,5,6][1]);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_declaration_no_type_strings)
{
	char const* text = R"(
		contract C {
			function f() returns (string) {
				return (["foo", "man", "choo"][1]);
			}
		}
	)";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(invalid_types_in_inline_array)
{
	char const* text = R"(
		contract C {
			function f() {
				uint[3] x = [45, 'foo', true];
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(dynamic_inline_array)
{
	char const* text = R"(
		contract C {
			function f() {
				uint8[4][4] memory dyn = [[1, 2, 3, 4], [2, 3, 4, 5], [3, 4, 5, 6], [4, 5, 6, 7]];
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(lvalues_as_inline_array)
{
	char const* text = R"(
		contract C {
			function f() {
				[1, 2, 3]++;
				[1, 2, 3] = [4, 5, 6];
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(break_not_in_loop)
{
	char const* text = R"(
		contract C {
			function f() {
				if (true)
					break;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::SyntaxError);
}

BOOST_AUTO_TEST_CASE(continue_not_in_loop)
{
	char const* text = R"(
		contract C {
			function f() {
				if (true)
					continue;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::SyntaxError);
}

BOOST_AUTO_TEST_CASE(continue_not_in_loop_2)
{
	char const* text = R"(
		contract C {
			function f() {
				while (true)
				{
				}
				continue;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::SyntaxError);
}

BOOST_AUTO_TEST_CASE(invalid_different_types_for_conditional_expression)
{
	char const* text = R"(
		contract C {
			function f() {
				true ? true : 2;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(left_value_in_conditional_expression_not_supported_yet)
{
	char const* text = R"(
		contract C {
			function f() {
				uint x;
				uint y;
				(true ? x : y) = 1;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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
			function f() {
				s1 x;
				s2 y;
				true ? x : y;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_function_type)
{
	char const* text = R"(
		contract C {
			function x(bool) {}
			function y() {}

			function f() {
				true ? x : y;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_enum)
{
	char const* text = R"(
		contract C {
			enum small { A, B, C, D }
			enum big { A, B, C, D }

			function f() {
				small x;
				big y;

				true ? x : y;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_different_mapping)
{
	char const* text = R"(
		contract C {
			mapping(uint8 => uint8) table1;
			mapping(uint32 => uint8) table2;

			function f() {
				true ? table1 : table2;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
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

			function fun_x() {}
			function fun_y() {}

			enum small { A, B, C, D }

			mapping(uint8 => uint8) table1;
			mapping(uint8 => uint8) table2;

			function f() {
				// integers
				uint x;
				uint y;
				true ? x : y;

				// integer constants
				true ? 1 : 3;

				// string literal
				true ? "hello" : "world";

				// bool
				true ? true : false;

				// real is not there yet.

				// array
				byte[2] memory a;
				byte[2] memory b;
				true ? a : b;

				bytes memory e;
				bytes memory f;
				true ? e : f;

				// fixed bytes
				bytes2 c;
				bytes2 d;
				true ? c : d;

				// contract doesn't fit in here

				// struct
				true ? struct_x : struct_y;

				// function
				true ? fun_x : fun_y;

				// enum
				small enum_x;
				small enum_y;
				true ? enum_x : enum_y;

				// tuple
				true ? (1, 2) : (3, 4);

				// mapping
				true ? table1 : table2;

				// typetype
				true ? uint32(1) : uint32(2);

				// modifier doesn't fit in here

				// magic doesn't fit in here

				// module doesn't fit in here
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(constructor_call_invalid_arg_count)
{
	// This caused a segfault in an earlier version
	char const* text = R"(
		contract C {
			function C(){}
		}
		contract D is C {
			function D() C(5){}
		}
	)";

	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(index_access_for_bytes)
{
	char const* text = R"(
		contract C {
			bytes20 x;
			function f(bytes16 b) {
				b[uint(x[2])];
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(uint7_and_uintM_as_identifier)
{
	char const* text = R"(
		contract test {
		string uintM = "Hello 4 you";
			function f() {
				uint8 uint7 = 3;
				uint7 = 5;
				string memory intM;
				uint bytesM = 21;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(varM_disqualified_as_keyword)
{
	char const* text = R"(
		contract test {
			function f() {
				uintM something = 3;
				intM should = 4;
				bytesM fail = "now";
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(long_uint_variable_fails)
{
	char const* text = R"(
		contract test {
			function f() {
				uint99999999999999999999999999 something = 3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(bytes10abc_is_identifier)
{
	char const* text = R"(
		contract test {
			function f() {
				bytes32 bytes10abc = "abc";
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(int10abc_is_identifier)
{
	char const* text = R"(
		contract test {
			function f() {
				uint uint10abc = 3;
				int int10abc = 4;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(library_functions_do_not_have_value)
{
	char const* text = R"(
		library L { function l() {} }
		contract test {
			function f() {
				L.l.value;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_0x7_mxn)
{
	char const* text = R"(
		contract test {
			fixed0x7 a = .3;
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_long_invalid_identifier)
{
	char const* text = R"(
		contract test {
			fixed99999999999999999999999999999999999999x7 b = 9.5;
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(invalid_fixed_types_7x8_mxn)
{
	char const* text = R"(
		contract test {
			fixed7x8 c = 3.12345678;
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(library_instances_cannot_be_used)
{
	char const* text = R"(
		library L { function l() {} }
		contract test {
			function f() {
				L x;
				x.l();
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(invalid_fixed_type_long)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed8x888888888888888888888888888888888888888888888888888 b;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_int_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				uint128 a = 3;
				int128 b = 4;
				fixed c = b;
				ufixed d = a;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_rational_int_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed c = 3;
				ufixed d = 4;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_rational_fraction_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 4.5;
				ufixed d = 2.5;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(invalid_int_implicit_conversion_from_fixed)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 4.5;
				int b = a;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_unary_operation)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed8x16 a = +3.25;
				fixed8x16 b = -3.25;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(leading_zero_rationals_convert)
{
	char const* text = R"(
		contract A {
			function f() {
				ufixed0x8 a = 0.5;
				ufixed0x56 b = 0.0000000000000006661338147750939242541790008544921875;
				fixed0x8 c = -0.5;
				fixed0x56 d = -0.0000000000000006661338147750939242541790008544921875;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(size_capabilities_of_fixed_point_types)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed248x8 a = 123456781234567979695948382928485849359686494864095409282048094275023098123.5;
				ufixed0x256 b = 0.920890746623327805482905058466021565416131529487595827354393978494366605267637829135688384325135165352082715782143655824815685807141335814463015972119819459298455224338812271036061391763384038070334798471324635050876128428143374549108557403087615966796875;
				ufixed0x256 c = 0.0000000000015198847363997979984922685411315294875958273543939784943666052676464653042434787697605517039455161817147718251801220885263595179331845639229818863564267318422845592626219390573301877339317935702714669975697814319204326238832436501979827880859375;
				fixed248x8 d = -123456781234567979695948382928485849359686494864095409282048094275023098123.5;
				fixed0x256 e = -0.93322335481643744342575580035176794825198893968114429702091846411734101080123092162893656820177312738451291806995868682861328125;
				fixed0x256 g = -0.00011788606643744342575580035176794825198893968114429702091846411734101080123092162893656820177312738451291806995868682861328125;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_invalid_implicit_conversion_size)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed a = 11/4;
				ufixed248x8 b = a;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_invalid_implicit_conversion_lost_data)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed0x256 a = 1/3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(fixed_type_valid_explicit_conversions)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed0x256 a = ufixed0x256(1/3);
				ufixed0x248 b = ufixed0x248(1/3);
				ufixed0x8 c = ufixed0x8(1/3);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(invalid_array_declaration_with_rational)
{
	char const* text = R"(
		contract test {
			function f() {
				uint[3.5] a;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(invalid_array_declaration_with_fixed_type)
{
	char const* text = R"(
		contract test {
			function f() {
				uint[fixed(3.5)] a;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_to_bytes_implicit_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				bytes32 c = 3.2;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(fixed_to_bytes_implicit_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 3.2;
				bytes32 c = a;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(mapping_with_fixed_literal)
{
	char const* text = R"(
		contract test {
			mapping(ufixed8x248 => string) fixedString;
			function f() {
				fixedString[0.5] = "Half";
			}
		}
	)";
	BOOST_CHECK(success(text));
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
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_fixed_types)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed[3] memory a = [fixed(3.5), fixed(-4.25), fixed(967.125)];
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_array_rationals)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed8x8[4] memory a = [3.5, 4.125, 2.5, 4.0];
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(rational_index_access)
{
	char const* text = R"(
		contract test {
			function f() {
				uint[] memory a;
				a[.5];
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_to_fixed_literal_expression)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed8x8 a = 3.5 * 3;
				ufixed8x8 b = 4 - 2.5;
				ufixed8x8 c = 11 / 4;
				ufixed16x240 d = 599 + 0.21875;
				ufixed8x248 e = ufixed8x248(35.245 % 12.9);
				ufixed8x248 f = ufixed8x248(1.2 % 2);
				fixed g = 2 ** -2;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(rational_as_exponent_value)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed g = 2 ** -2.2;
				ufixed b = 3 ** 2.5;
				ufixed24x24 b = 2 ** (1/2);
				fixed40x40 c = 42 ** (-1/4);
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(fixed_point_casting_exponents)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed a = 3 ** ufixed(1.5);
				ufixed b = 2 ** ufixed(1/2);
				fixed c = 42 ** fixed(-1/4);
				fixed d = 16 ** fixed(-0.33);
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(var_capable_of_holding_constant_rationals)
{
	char const* text = R"(
		contract test {
			function f() {
				var a = 0.12345678;
				var b = 12345678.352;
				var c = 0.00000009;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(var_and_rational_with_tuple)
{
	char const* text = R"(
		contract test {
			function f() {
				var (a, b) = (.5, 1/3);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(var_handle_divided_integers)
{
	char const* text = R"(
		contract test {
			function f() {
				var x = 1/3;
			}
		}
	)";
	BOOST_CHECK(success(text));	
}

BOOST_AUTO_TEST_CASE(rational_bitnot_unary_operation)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = ~3.56;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_bitor_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 1.56 | 3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_bitxor_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 1.56 ^ 3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(rational_bitand_binary_operation)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 1.56 & 3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(zero_handling)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed8x8 a = 0;
				ufixed8x8 b = 0;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(missing_bool_conversion)
{
	char const* text = R"(
		contract test {
			function b(uint a) {
				bool(a == 1);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(integer_and_fixed_interaction)
{
	char const* text = R"(
		contract test {
			function f() {
				ufixed a = uint128(1) + ufixed(2);
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(signed_rational_modulus)
{
	char const* text = R"(
		contract test {
			function f() {
				fixed a = 0.42578125 % -0.4271087646484375;
				fixed b = .5 % a;
				fixed c = a % b;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(one_divided_by_three_integer_conversion)
{
	char const* text = R"(
		contract test {
			function f() {
				uint a = 1/3;
			}
		}
	)";
	BOOST_CHECK(!success(text));
}

BOOST_AUTO_TEST_CASE(unused_return_value)
{
	char const* text = R"(
		contract test {
			function g() returns (uint) {}
			function f() {
				g();
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(unused_return_value_send)
{
	char const* text = R"(
		contract test {
			function f() {
				address(0x12).send(1);
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(unused_return_value_call)
{
	char const* text = R"(
		contract test {
			function f() {
				address(0x12).call("abc");
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(unused_return_value_call_value)
{
	char const* text = R"(
		contract test {
			function f() {
				address(0x12).call.value(2)("abc");
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(unused_return_value_callcode)
{
	char const* text = R"(
		contract test {
			function f() {
				address(0x12).callcode("abc");
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(unused_return_value_delegatecall)
{
	char const* text = R"(
		contract test {
			function f() {
				address(0x12).delegatecall("abc");
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(modifier_without_underscore)
{
	char const* text = R"(
		contract test {
			modifier m() {}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::SyntaxError);
}

BOOST_AUTO_TEST_CASE(payable_in_library)
{
	char const* text = R"(
		library test {
			function f() payable {}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(payable_external)
{
	char const* text = R"(
		contract test {
			function f() payable external {}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(payable_internal)
{
	char const* text = R"(
		contract test {
			function f() payable internal {}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(payable_private)
{
	char const* text = R"(
		contract test {
			function f() payable private {}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_payable)
{
	char const* text = R"(
		contract B { function f() payable {} }
		contract C is B { function f() {} }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(illegal_override_payable_nonpayable)
{
	char const* text = R"(
		contract B { function f() {} }
		contract C is B { function f() payable {} }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(payable_constant_conflict)
{
	char const* text = R"(
		contract C { function f() payable constant {} }
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(calling_payable)
{
	char const* text = R"(
		contract receiver { function pay() payable {} }
		contract test {
			function f() { (new receiver()).pay.value(10)(); }
			receiver r = new receiver();
			function g() { r.pay.value(10)(); }
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(calling_nonpayable)
{
	char const* text = R"(
		contract receiver { function nopay() {} }
		contract test {
			function f() { (new receiver()).nopay.value(10)(); }
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(non_payable_constructor)
{
	char const* text = R"(
		contract C {
			function C() { }
		}
		contract D {
			function f() returns (uint) {
				(new C).value(2)();
				return 2;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(warn_nonpresent_pragma)
{
	char const* text = "contract C {}";
	auto sourceAndError = parseAnalyseAndReturnError(text, true, false);
	BOOST_REQUIRE(!!sourceAndError.second);
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_CHECK(*sourceAndError.second == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(unsatisfied_version)
{
	char const* text = R"(
		pragma solidity ^99.99.0;
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::SyntaxError);
}

BOOST_AUTO_TEST_CASE(constant_constructor)
{
	char const* text = R"(
		contract test {
			function test() constant {}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(external_constructor)
{
	char const* text = R"(
		contract test {
			function test() external {}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_array_as_statement)
{
	char const* text = R"(
		contract test {
			struct S { uint x; }
			function test(uint k)  { S[k]; }
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(using_directive_for_missing_selftype)
{
	char const* text = R"(
		library B {
			function b() {}
		}

		contract A {
			using B for bytes;

			function a() {
				bytes memory x;
				x.b();
			}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(function_type)
{
	char const* text = R"(
		contract C {
			function f() {
				function(uint) returns (uint) x;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(function_type_parameter)
{
	char const* text = R"(
		contract C {
			function f(function(uint) external returns (uint) g) returns (function(uint) external returns (uint)) {
				return g;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(function_type_returned)
{
	char const* text = R"(
		contract C {
			function f() returns (function(uint) external returns (uint) g) {
				return g;
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(private_function_type)
{
	char const* text = R"(
		contract C {
			function f() {
				function(uint) private returns (uint) x;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(public_function_type)
{
	char const* text = R"(
		contract C {
			function f() {
				function(uint) public returns (uint) x;
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(internal_function_as_external_parameter)
{
	// It should not be possible to give internal functions
	// as parameters to external functions.
	char const* text = R"(
		contract C {
			function f(function(uint) internal returns (uint) x) {
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(internal_function_returned_from_public_function)
{
	// It should not be possible to return internal functions from external functions.
	char const* text = R"(
		contract C {
			function f() returns (function(uint) internal returns (uint) x) {
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(internal_function_as_external_parameter_in_library_internal)
{
	char const* text = R"(
		library L {
			function f(function(uint) internal returns (uint) x) internal {
			}
		}
	)";
	BOOST_CHECK(success(text));
}
BOOST_AUTO_TEST_CASE(internal_function_as_external_parameter_in_library_external)
{
	char const* text = R"(
		library L {
			function f(function(uint) internal returns (uint) x) {
			}
		}
	)";
	BOOST_CHECK(expectError(text) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(invalid_fixed_point_literal)
{
	char const* text = R"(
		contract A {
			function a() {
				.8E0;
			}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(shift_constant_left_negative_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 << -8;
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(shift_constant_right_negative_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 >> -8;
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(shift_constant_left_excessive_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 << 0x100000000;
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(shift_constant_right_excessive_rvalue)
{
	char const* text = R"(
		contract C {
			uint public a = 0x42 >> 0x100000000;
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_CASE(inline_assembly_unbalanced_positive_stack)
{
	char const* text = R"(
		contract test {
			function f() {
				assembly {
					1
				}
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(inline_assembly_unbalanced_negative_stack)
{
	char const* text = R"(
		contract test {
			function f() {
				assembly {
					pop
				}
			}
		}
	)";
	BOOST_CHECK(expectError(text, true) == Error::Type::Warning);
}

BOOST_AUTO_TEST_CASE(inline_assembly_in_modifier)
{
	char const* text = R"(
		contract test {
			modifier m {
				uint a = 1;
				assembly {
					a := 2
				}
				_;
			}
			function f() m {
			}
		}
	)";
	BOOST_CHECK(success(text));
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage)
{
	char const* text = R"(
		contract test {
			uint x = 1;
			function f() {
				assembly {
					x := 2
				}
			}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_in_modifiers)
{
	char const* text = R"(
		contract test {
			uint x = 1;
			modifier m {
				assembly {
					x := 2
				}
				_;
			}
			function f() m {
			}
		}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::DeclarationError);
}

BOOST_AUTO_TEST_CASE(invalid_mobile_type)
{
	char const* text = R"(
			contract C {
				function f() {
					// Invalid number
					[1, 78901234567890123456789012345678901234567890123456789345678901234567890012345678012345678901234567];
				}
			}
	)";
	BOOST_CHECK(expectError(text, false) == Error::Type::TypeError);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
