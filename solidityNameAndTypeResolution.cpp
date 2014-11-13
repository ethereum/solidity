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
void parseTextAndResolveNames(std::string const& _source)
{
	Parser parser;
	ASTPointer<ContractDefinition> contract = parser.parse(
										   std::make_shared<Scanner>(CharStream(_source)));
	NameAndTypeResolver resolver;
	resolver.resolveNamesAndTypes(*contract);
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

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

