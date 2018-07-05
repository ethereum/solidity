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

BOOST_AUTO_TEST_SUITE(SolidityASTJSON)

BOOST_AUTO_TEST_CASE(short_type_name)
{
	CompilerStack c;
	c.addSource("a", "contract c { function f() public { uint[] memory x; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(false, sourceIndices).toJson(c.ast("a"));
	Json::Value varDecl = astJson["nodes"][0]["nodes"][0]["body"]["statements"][0]["declarations"][0];
	BOOST_CHECK_EQUAL(varDecl["storageLocation"], "memory");
	BOOST_CHECK_EQUAL(varDecl["typeDescriptions"]["typeIdentifier"], "t_array$_t_uint256_$dyn_memory_ptr");
	BOOST_CHECK_EQUAL(varDecl["typeDescriptions"]["typeString"], "uint256[]");
}

BOOST_AUTO_TEST_CASE(short_type_name_ref)
{
	CompilerStack c;
	c.addSource("a", "contract c { function f() public { uint[][] memory rows; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(false, sourceIndices).toJson(c.ast("a"));
	Json::Value varDecl = astJson["nodes"][0]["nodes"][0]["body"]["statements"][0]["declarations"][0];
	BOOST_CHECK_EQUAL(varDecl["storageLocation"], "memory");
	BOOST_CHECK_EQUAL(varDecl["typeName"]["typeDescriptions"]["typeIdentifier"], "t_array$_t_array$_t_uint256_$dyn_storage_$dyn_storage_ptr");
	BOOST_CHECK_EQUAL(varDecl["typeName"]["typeDescriptions"]["typeString"], "uint256[][]");
}

BOOST_AUTO_TEST_CASE(long_type_name_binary_operation)
{
	CompilerStack c;
	c.addSource("a", "contract c { function f() public { uint a = 2 + 3; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(false, sourceIndices).toJson(c.ast("a"));
	Json::Value varDecl = astJson["nodes"][0]["nodes"][0]["body"]["statements"][0]["initialValue"]["commonType"];
	BOOST_CHECK_EQUAL(varDecl["typeIdentifier"], "t_rational_5_by_1");
	BOOST_CHECK_EQUAL(varDecl["typeString"], "int_const 5");
}

BOOST_AUTO_TEST_CASE(long_type_name_identifier)
{
	CompilerStack c;
	c.addSource("a", "contract c { uint[] a; function f() public { uint[] b = a; } }");
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	c.parseAndAnalyze();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(false, sourceIndices).toJson(c.ast("a"));
	Json::Value varDecl = astJson["nodes"][0]["nodes"][1]["body"]["statements"][0]["initialValue"];
	BOOST_CHECK_EQUAL(varDecl["typeDescriptions"]["typeIdentifier"], "t_array$_t_uint256_$dyn_storage");
	BOOST_CHECK_EQUAL(varDecl["typeDescriptions"]["typeString"], "uint256[] storage ref");
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
	//same tests for non-legacy mode
	Json::Value astJsonA = ASTJsonConverter(false, sourceIndices).toJson(c.ast("a"));
	Json::Value documentationA = astJsonA["nodes"][0]["documentation"];
	BOOST_CHECK_EQUAL(documentationA, "This contract is empty");
	Json::Value astJsonB = ASTJsonConverter(false, sourceIndices).toJson(c.ast("b"));
	Json::Value documentationB = astJsonB["nodes"][0]["documentation"];
	BOOST_CHECK_EQUAL(documentationB, "This contract is empty and has a line-breaking comment.");
	Json::Value astJsonC = ASTJsonConverter(false, sourceIndices).toJson(c.ast("c"));
	Json::Value documentationC0 = astJsonC["nodes"][0]["nodes"][0]["documentation"];
	Json::Value documentationC1 = astJsonC["nodes"][0]["nodes"][1]["documentation"];
	Json::Value documentationC2 = astJsonC["nodes"][0]["nodes"][2]["documentation"];
	BOOST_CHECK_EQUAL(documentationC0, "Some comment on Evt.");
	BOOST_CHECK_EQUAL(documentationC1, "Some comment on mod.");
	BOOST_CHECK_EQUAL(documentationC2, "Some comment on fn.");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
