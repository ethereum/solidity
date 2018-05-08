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
 * Tests for the json ast output.
 */

#include <test/Options.h>

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/ast/ASTJsonConverter.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SolidityASTLegacyJSON)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CompilerStack c;
	c.addSource("a", "contract C {}");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	BOOST_CHECK_EQUAL(astJson["name"], "SourceUnit");
}

BOOST_AUTO_TEST_CASE(source_location)
{
	CompilerStack c;
	c.addSource("a", "contract C { function f() { var x = 2; x++; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	BOOST_CHECK_EQUAL(astJson["name"], "SourceUnit");
	BOOST_CHECK_EQUAL(astJson["children"][0]["name"], "ContractDefinition");
	BOOST_CHECK_EQUAL(astJson["children"][0]["children"][0]["name"], "FunctionDefinition");
	BOOST_CHECK_EQUAL(astJson["children"][0]["children"][0]["src"], "13:32:1");

}

BOOST_AUTO_TEST_CASE(inheritance_specifier)
{
	CompilerStack c;
	c.addSource("a", "contract C1 {} contract C2 is C1 {}");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	BOOST_CHECK_EQUAL(astJson["children"][1]["attributes"]["name"], "C2");
	BOOST_CHECK_EQUAL(astJson["children"][1]["children"][0]["name"], "InheritanceSpecifier");
	BOOST_CHECK_EQUAL(astJson["children"][1]["children"][0]["src"], "30:2:1");
	BOOST_CHECK_EQUAL(astJson["children"][1]["children"][0]["children"][0]["name"], "UserDefinedTypeName");
	BOOST_CHECK_EQUAL(astJson["children"][1]["children"][0]["children"][0]["attributes"]["name"], "C1");
}

BOOST_AUTO_TEST_CASE(using_for_directive)
{
	CompilerStack c;
	c.addSource("a", "library L {} contract C { using L for uint; }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value usingFor = astJson["children"][1]["children"][0];
	BOOST_CHECK_EQUAL(usingFor["name"], "UsingForDirective");
	BOOST_CHECK_EQUAL(usingFor["src"], "26:17:1");
	BOOST_CHECK_EQUAL(usingFor["children"][0]["name"], "UserDefinedTypeName");
	BOOST_CHECK_EQUAL(usingFor["children"][0]["attributes"]["name"], "L");
	BOOST_CHECK_EQUAL(usingFor["children"][1]["name"], "ElementaryTypeName");
	BOOST_CHECK_EQUAL(usingFor["children"][1]["attributes"]["name"], "uint");
}

BOOST_AUTO_TEST_CASE(enum_value)
{
	CompilerStack c;
	c.addSource("a", "contract C { enum E { A, B } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value enumDefinition = astJson["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(enumDefinition["children"][0]["name"], "EnumValue");
	BOOST_CHECK_EQUAL(enumDefinition["children"][0]["attributes"]["name"], "A");
	BOOST_CHECK_EQUAL(enumDefinition["children"][0]["src"], "22:1:1");
	BOOST_CHECK_EQUAL(enumDefinition["children"][1]["name"], "EnumValue");
	BOOST_CHECK_EQUAL(enumDefinition["children"][1]["attributes"]["name"], "B");
	BOOST_CHECK_EQUAL(enumDefinition["children"][1]["src"], "25:1:1");
}

BOOST_AUTO_TEST_CASE(modifier_definition)
{
	CompilerStack c;
	c.addSource("a", "contract C { modifier M(uint i) { _; } function F() M(1) {} }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value modifier = astJson["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(modifier["name"], "ModifierDefinition");
	BOOST_CHECK_EQUAL(modifier["attributes"]["name"], "M");
	BOOST_CHECK_EQUAL(modifier["src"], "13:25:1");
}

BOOST_AUTO_TEST_CASE(modifier_invocation)
{
	CompilerStack c;
	c.addSource("a", "contract C { modifier M(uint i) { _; } function F() M(1) {} }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value modifier = astJson["children"][0]["children"][1]["children"][2];
	BOOST_CHECK_EQUAL(modifier["name"], "ModifierInvocation");
	BOOST_CHECK_EQUAL(modifier["src"], "52:4:1");
	BOOST_CHECK_EQUAL(modifier["children"][0]["attributes"]["type"], "modifier (uint256)");
	BOOST_CHECK_EQUAL(modifier["children"][0]["attributes"]["value"], "M");
	BOOST_CHECK_EQUAL(modifier["children"][1]["attributes"]["value"], "1");
}

BOOST_AUTO_TEST_CASE(event_definition)
{
	CompilerStack c;
	c.addSource("a", "contract C { event E(); }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value event = astJson["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(event["name"], "EventDefinition");
	BOOST_CHECK_EQUAL(event["attributes"]["name"], "E");
	BOOST_CHECK_EQUAL(event["src"], "13:10:1");
}

BOOST_AUTO_TEST_CASE(array_type_name)
{
	CompilerStack c;
	c.addSource("a", "contract C { uint[] i; }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value arrayDecl = astJson["children"][0]["children"][0]["attributes"];
	BOOST_CHECK_EQUAL(arrayDecl["storageLocation"], "default");
	BOOST_CHECK_EQUAL(arrayDecl["type"], "uint256[]");
	Json::Value array = astJson["children"][0]["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(array["name"], "ArrayTypeName");
	BOOST_CHECK_EQUAL(array["src"], "13:6:1");
}

BOOST_AUTO_TEST_CASE(short_type_name)
{
	CompilerStack c;
	c.addSource("a", "contract c { function f() { uint[] memory x; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value arrayDecl = astJson["children"][0]["children"][0]["children"][2]["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(arrayDecl["attributes"]["storageLocation"], "memory");
	BOOST_CHECK_EQUAL(arrayDecl["attributes"]["type"], "uint256[]");
}

BOOST_AUTO_TEST_CASE(short_type_name_ref)
{
	CompilerStack c;
	c.addSource("a", "contract c { function f() { uint[][] memory rows; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value arrayDecl = astJson["children"][0]["children"][0]["children"][2]["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(arrayDecl["attributes"]["storageLocation"], "memory");
	BOOST_CHECK_EQUAL(arrayDecl["attributes"]["type"], "uint256[][]");
}

BOOST_AUTO_TEST_CASE(placeholder_statement)
{
	CompilerStack c;
	c.addSource("a", "contract C { modifier M { _; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value placeholder = astJson["children"][0]["children"][0]["children"][1]["children"][0];
	BOOST_CHECK_EQUAL(placeholder["name"], "PlaceholderStatement");
	BOOST_CHECK_EQUAL(placeholder["src"], "26:1:1");
}

BOOST_AUTO_TEST_CASE(non_utf8)
{
	CompilerStack c;
	c.addSource("a", "contract C { function f() { var x = hex\"ff\"; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value varDecl = astJson["children"][0]["children"][0]["children"][2]["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(varDecl["attributes"]["type"], "string");
	BOOST_CHECK_EQUAL(varDecl["attributes"]["typeName"], Json::nullValue);
	Json::Value literal = astJson["children"][0]["children"][0]["children"][2]["children"][0]["children"][1];
	BOOST_CHECK_EQUAL(literal["name"], "Literal");
	BOOST_CHECK_EQUAL(literal["attributes"]["hexvalue"], "ff");
	BOOST_CHECK_EQUAL(literal["attributes"]["token"], "string");
	BOOST_CHECK_EQUAL(literal["attributes"]["value"], Json::nullValue);
	BOOST_CHECK(literal["attributes"]["type"].asString().find("invalid") != string::npos);
}

BOOST_AUTO_TEST_CASE(function_type)
{
	CompilerStack c;
	c.addSource("a",
		"contract C { function f(function() external payable returns (uint) x) "
		"returns (function() external view returns (uint)) {} }"
	);
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value fun = astJson["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(fun["name"], "FunctionDefinition");
	Json::Value argument = fun["children"][0]["children"][0];
	BOOST_CHECK_EQUAL(argument["name"], "VariableDeclaration");
	BOOST_CHECK_EQUAL(argument["attributes"]["name"], "x");
	BOOST_CHECK_EQUAL(argument["attributes"]["type"], "function () payable external returns (uint256)");
	Json::Value funType = argument["children"][0];
	BOOST_CHECK_EQUAL(funType["attributes"]["constant"], false);
	BOOST_CHECK_EQUAL(funType["attributes"]["payable"], true);
	BOOST_CHECK_EQUAL(funType["attributes"]["visibility"], "external");
	Json::Value retval = fun["children"][1]["children"][0];
	BOOST_CHECK_EQUAL(retval["name"], "VariableDeclaration");
	BOOST_CHECK_EQUAL(retval["attributes"]["name"], "");
	BOOST_CHECK_EQUAL(retval["attributes"]["type"], "function () view external returns (uint256)");
	funType = retval["children"][0];
	BOOST_CHECK_EQUAL(funType["attributes"]["constant"], true);
	BOOST_CHECK_EQUAL(funType["attributes"]["payable"], false);
	BOOST_CHECK_EQUAL(funType["attributes"]["visibility"], "external");
}

BOOST_AUTO_TEST_CASE(documentation)
{
	CompilerStack c;
	c.addSource("a", "/**This contract is empty*/ contract C {}");
	c.addSource("b",
		"/**This contract is empty"
		" and has a line-breaking comment.*/"
		"contract C {}"
	);
	c.addSource("c",
		"contract C {"
		"  /** Some comment on Evt.*/ event Evt();"
		"  /** Some comment on mod.*/ modifier mod() { _; }"
		"  /** Some comment on fn.*/ function fn() public {}"
		"}"
	);
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 0;
	sourceIndices["b"] = 1;
	sourceIndices["c"] = 2;
	Json::Value astJsonA = ASTJsonConverter(true, sourceIndices).toJson(c.ast("a"));
	Json::Value documentationA = astJsonA["children"][0]["attributes"]["documentation"];
	BOOST_CHECK_EQUAL(documentationA, "This contract is empty");
	Json::Value astJsonB = ASTJsonConverter(true, sourceIndices).toJson(c.ast("b"));
	Json::Value documentationB = astJsonB["children"][0]["attributes"]["documentation"];
	BOOST_CHECK_EQUAL(documentationB, "This contract is empty and has a line-breaking comment.");
	Json::Value astJsonC = ASTJsonConverter(true, sourceIndices).toJson(c.ast("c"));
	Json::Value documentationC0 = astJsonC["children"][0]["children"][0]["attributes"]["documentation"];
	Json::Value documentationC1 = astJsonC["children"][0]["children"][1]["attributes"]["documentation"];
	Json::Value documentationC2 = astJsonC["children"][0]["children"][2]["attributes"]["documentation"];
	BOOST_CHECK_EQUAL(documentationC0, "Some comment on Evt.");
	BOOST_CHECK_EQUAL(documentationC1, "Some comment on mod.");
	BOOST_CHECK_EQUAL(documentationC2, "Some comment on fn.");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
