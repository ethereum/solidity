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
#include <test/libsolidity/util/SoltestErrors.h>

#include <test/Common.h>

#include <libsolidity/ast/AST.h>

#include <libsolutil/Keccak256.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace solidity::langutil;

namespace solidity::frontend::test
{

BOOST_FIXTURE_TEST_SUITE(SolidityNameAndTypeResolution, AnalysisFramework)

BOOST_AUTO_TEST_CASE(function_no_implementation)
{
	char const* text = R"(
		abstract contract test {
			function functionName(bytes32 input) public virtual returns (bytes32 out);
		}
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* contract = dynamic_cast<ContractDefinition*>(nodes[1].get());
	BOOST_REQUIRE(contract);
	BOOST_CHECK(!contract->annotation().unimplementedDeclarations->empty());
	BOOST_CHECK(!contract->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract)
{
	char const* text = R"(
		abstract contract base { function foo() public virtual; }
		contract derived is base { function foo() public override {} }
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().unimplementedDeclarations->empty());
	BOOST_CHECK(!base->definedFunctions()[0]->isImplemented());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(derived->annotation().unimplementedDeclarations->empty());
	BOOST_CHECK(derived->definedFunctions()[0]->isImplemented());
}

BOOST_AUTO_TEST_CASE(abstract_contract_with_overload)
{
	char const* text = R"(
		abstract contract base { function foo(bool) public virtual; }
		abstract contract derived is base { function foo(uint) public {} }
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	ContractDefinition* base = dynamic_cast<ContractDefinition*>(nodes[1].get());
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(base);
	BOOST_CHECK(!base->annotation().unimplementedDeclarations->empty());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().unimplementedDeclarations->empty());
}

BOOST_AUTO_TEST_CASE(implement_abstract_via_constructor)
{
	char const* text = R"(
		abstract contract base { function foo() public virtual; }
		abstract contract foo is base { constructor() {} }
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	std::vector<ASTPointer<ASTNode>> nodes = sourceUnit->nodes();
	BOOST_CHECK_EQUAL(nodes.size(), 3);
	ContractDefinition* derived = dynamic_cast<ContractDefinition*>(nodes[2].get());
	BOOST_REQUIRE(derived);
	BOOST_CHECK(!derived->annotation().unimplementedDeclarations->empty());
}

BOOST_AUTO_TEST_CASE(function_canonical_signature)
{
	char const* text = R"(
		contract Test {
			function foo(uint256 arg1, uint64 arg2, bool arg3) public returns (uint256 ret) {
				ret = arg1 + arg2;
			}
		}
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_CHECK_EQUAL("foo(uint256,uint64,bool)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(function_canonical_signature_type_aliases)
{
	char const* text = R"(
		contract Test {
			function boo(uint, bytes32, address) public returns (uint ret) {
				ret = 5;
			}
		}
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

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
	char const* text = R"(
		contract C {
			uint a;
		}
		contract Test {
			function boo(uint, bool, bytes8, bool[2] calldata, uint[] calldata, C, address[] calldata) external returns (uint ret) {
				ret = 5;
			}
		}
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

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
	char const* text = R"(
		// test for bug #1801
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function boo(ActionChoices enumArg) external returns (uint ret) {
				ret = 5;
			}
		}
	)";
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			if (functions.empty())
				continue;
			BOOST_CHECK_EQUAL("boo(uint8)", functions[0]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(external_struct_signatures)
{
	char const* text = R"(
		pragma abicoder v2;
		contract Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			struct Simple { uint i; }
			struct Nested { X[2][] a; uint y; }
			struct X { bytes32 x; Test t; Simple[] s; }
			function f(ActionChoices, uint, Simple calldata) external {}
			function g(Test, Nested calldata) external {}
			function h(function(Nested memory) external returns (uint)[] calldata) external {}
			function i(Nested[] calldata) external {}
		}
	)";
	// Ignore analysis errors. This test only checks that correct signatures
	// are generated for external structs, but they are not yet supported
	// in code generation and therefore cause an error in the TypeChecker.
	SourceUnit const* sourceUnit = parseAnalyseAndReturnError(text, false, true, true).first;
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(uint8,uint256,(uint256))", functions[0]->externalSignature());
			BOOST_CHECK_EQUAL("g(address,((bytes32,address,(uint256)[])[2][],uint256))", functions[1]->externalSignature());
			BOOST_CHECK_EQUAL("h(function[])", functions[2]->externalSignature());
			BOOST_CHECK_EQUAL("i(((bytes32,address,(uint256)[])[2][],uint256)[])", functions[3]->externalSignature());
		}
}

BOOST_AUTO_TEST_CASE(external_struct_signatures_in_libraries)
{
	char const* text = R"(
		pragma abicoder v2;
		library Test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			struct Simple { uint i; }
			struct Nested { X[2][] a; uint y; }
			struct X { bytes32 x; Test t; Simple[] s; }
			function f(ActionChoices, uint, Simple calldata) external {}
			function g(Test, Nested calldata) external {}
			function h(function(Nested memory) external returns (uint)[] calldata) external {}
			function i(Nested[] calldata) external {}
		}
	)";
	// Ignore analysis errors. This test only checks that correct signatures
	// are generated for external structs, but calldata structs are not yet supported
	// in code generation and therefore cause an error in the TypeChecker.
	SourceUnit const* sourceUnit = parseAnalyseAndReturnError(text, false, true, true).first;
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(Test.ActionChoices,uint256,Test.Simple)", functions[0]->externalSignature());
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
	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			auto functions = contract->definedFunctions();
			BOOST_REQUIRE(!functions.empty());
			BOOST_CHECK_EQUAL("f(Test.X storage)", functions[0]->externalSignature());
		}
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

	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	ContractDefinition const* contract;
	BOOST_REQUIRE((contract = retrieveContractByName(*sourceUnit, "test")) != nullptr);
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

	auto [sourceUnit, errors] = parseAnalyseAndReturnError(text);
	soltestAssert(sourceUnit);
	soltestAssert(errors.empty(), "Unexpected error: " + formatErrors(errors));

	ContractDefinition const* contract;
	BOOST_CHECK((contract = retrieveContractByName(*sourceUnit, "test")) != nullptr);
	FunctionTypePointer function;
	function = retrieveFunctionBySignature(*contract, "foo()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of a private variable should not exist");
	function = retrieveFunctionBySignature(*contract, "bar()");
	BOOST_CHECK_MESSAGE(function == nullptr, "Accessor function of an internal variable should not exist");
}

BOOST_AUTO_TEST_CASE(string)
{
	char const* sourceCode = R"(
		contract C {
			string s;
			function f(string calldata x) external { s = x; }
		}
	)";
	CHECK_SUCCESS(sourceCode);
}

BOOST_AUTO_TEST_CASE(dynamic_return_types_not_possible)
{
	char const* sourceCode = R"(
		abstract contract C {
			function f(uint) public virtual returns (string memory);
			function g() public {
				string memory x = this.f(2);
				// we can assign to x but it is not usable.
				bytes(x).length;
			}
		}
	)";
	if (solidity::test::CommonOptions::get().evmVersion() == EVMVersion::homestead())
		CHECK_ERROR(sourceCode, TypeError, "Type inaccessible dynamic type is not implicitly convertible to expected type string memory.");
	else
		CHECK_SUCCESS_NO_WARNINGS(sourceCode);
}

BOOST_AUTO_TEST_CASE(warn_nonpresent_pragma)
{
	char const* text = R"(
		// SPDX-License-Identifier: GPL-3.0
		contract C {}
	)";
	auto sourceAndError = parseAnalyseAndReturnError(text, true, false);
	BOOST_REQUIRE(!sourceAndError.second.empty());
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_CHECK(searchErrorMessage(*sourceAndError.second.front(), "Source file does not specify required compiler version!"));
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
	ContractDefinition const& c = dynamic_cast<ContractDefinition const&>(*compiler().ast("").nodes().at(1));
	BOOST_CHECK(c.interfaceFunctions().size() == 2);
	for (auto const& f: c.interfaceFunctions())
	{
		auto const& retType = f.second->returnParameterTypes().at(0);
		BOOST_CHECK(retType->dataStoredIn(DataLocation::Memory));
	}
}

BOOST_AUTO_TEST_CASE(address_staticcall)
{
	char const* sourceCode = R"(
		contract C {
			function f() public view returns(bool) {
				(bool success,) = address(0x4242).staticcall("");
				return success;
			}
		}
	)";

	if (solidity::test::CommonOptions::get().evmVersion().hasStaticCall())
		CHECK_SUCCESS_NO_WARNINGS(sourceCode);
	else
		CHECK_ERROR(sourceCode, TypeError, "\"staticcall\" is not supported by the VM version.");
}

BOOST_AUTO_TEST_CASE(address_staticcall_value)
{
	if (solidity::test::CommonOptions::get().evmVersion().hasStaticCall())
	{
		char const* sourceCode = R"(
			contract C {
				function f() public view {
					address(0x4242).staticcall.value;
				}
			}
		)";
		CHECK_ERROR(sourceCode, TypeError, "Member \"value\" is only available for payable functions.");
	}
}

BOOST_AUTO_TEST_CASE(address_call_full_return_type)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				(bool success, bytes memory m) = address(0x4242).call("");
				success; m;
			}
		}
	)";

	if (solidity::test::CommonOptions::get().evmVersion().supportsReturndata())
		CHECK_SUCCESS_NO_WARNINGS(sourceCode);
	else
		CHECK_ERROR(sourceCode, TypeError, "Type inaccessible dynamic type is not implicitly convertible to expected type bytes memory.");
}

BOOST_AUTO_TEST_CASE(address_delegatecall_full_return_type)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				(bool success, bytes memory m) = address(0x4242).delegatecall("");
				success; m;
			}
		}
	)";

	if (solidity::test::CommonOptions::get().evmVersion().supportsReturndata())
		CHECK_SUCCESS_NO_WARNINGS(sourceCode);
	else
		CHECK_ERROR(sourceCode, TypeError, "Type inaccessible dynamic type is not implicitly convertible to expected type bytes memory.");
}


BOOST_AUTO_TEST_CASE(address_staticcall_full_return_type)
{
	if (solidity::test::CommonOptions::get().evmVersion().hasStaticCall())
	{
		char const* sourceCode = R"(
			contract C {
				function f() public view {
					(bool success, bytes memory m) = address(0x4242).staticcall("");
					success; m;
				}
			}
		)";

		CHECK_SUCCESS_NO_WARNINGS(sourceCode);
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
