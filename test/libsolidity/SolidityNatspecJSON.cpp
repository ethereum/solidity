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
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2014
 * Unit tests for the solidity compiler JSON Interface output.
 */

#include "../TestHelper.h"
#include <string>
#include <json/json.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/Exceptions.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace solidity
{
namespace test
{

class DocumentationChecker
{
public:
	DocumentationChecker(): m_compilerStack(false) {}

	void checkNatspec(
		std::string const& _code,
		std::string const& _expectedDocumentationString,
		bool _userDocumentation
	)
	{
		std::string generatedDocumentationString;
		ETH_TEST_REQUIRE_NO_THROW(m_compilerStack.parse(_code), "Parsing failed");

		if (_userDocumentation)
			generatedDocumentationString = m_compilerStack.getMetadata("", DocumentationType::NatspecUser);
		else
			generatedDocumentationString = m_compilerStack.getMetadata("", DocumentationType::NatspecDev);
		Json::Value generatedDocumentation;
		m_reader.parse(generatedDocumentationString, generatedDocumentation);
		Json::Value expectedDocumentation;
		m_reader.parse(_expectedDocumentationString, expectedDocumentation);
		BOOST_CHECK_MESSAGE(
			expectedDocumentation == generatedDocumentation,
			"Expected " << _expectedDocumentationString <<
			"\n but got:\n" << generatedDocumentationString
		);
	}

private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityNatspecJSON, DocumentationChecker)

BOOST_AUTO_TEST_CASE(user_basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  /// @notice Multiplies `a` by 7\n"
	"  function mul(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \"notice\": \"Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  /// @notice Multiplies `a` by 7\n"
	"  /// @dev Multiplies a number by 7\n"
	"  function mul(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* devNatspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7\"\n"
	"        }\n"
	"    }\n"
	"}}";

	char const* userNatspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \"notice\": \"Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, devNatspec, false);
	checkNatspec(sourceCode, userNatspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiline_comment)
{
	char const* sourceCode = "contract test {\n"
	"  /// @notice Multiplies `a` by 7\n"
	"  /// and then adds `b`\n"
	"  function mul_and_add(uint a, uint256 b) returns(uint256 d)\n"
	"  {\n"
	"      return (a * 7) + b;\n"
	"  }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"}"
	"}}";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiple_functions)
{
	char const* sourceCode = "contract test {\n"
	"  /// @notice Multiplies `a` by 7 and then adds `b`\n"
	"  function mul_and_add(uint a, uint256 b) returns(uint256 d)\n"
	"  {\n"
	"      return (a * 7) + b;\n"
	"  }\n"
	"\n"
	"  /// @notice Divides `input` by `div`\n"
	"  function divide(uint input, uint div) returns(uint d)\n"
	"  {\n"
	"      return input / div;\n"
	"  }\n"
	"  /// @notice Subtracts 3 from `input`\n"
	"  function sub(int input) returns(int d)\n"
	"  {\n"
	"      return input - 3;\n"
	"  }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"},"
	"    \"divide(uint256,uint256)\":{ \"notice\": \"Divides `input` by `div`\"},"
	"    \"sub(int256)\":{ \"notice\": \"Subtracts 3 from `input`\"}"
	"}}";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(user_empty_contract)
{
	char const* sourceCode = "contract test {\n"
	"}\n";

	char const* natspec = "{\"methods\":{} }";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_no_doc)
{
	char const* sourceCode = "contract test {\n"
	"  function mul(uint a) returns(uint d) { return a * 7; }\n"
	"  function sub(int input) returns(int d)\n"
	"  {\n"
	"      return input - 3;\n"
	"  }\n"
	"}\n";

	char const* devNatspec = "{\"methods\":{}}";
	char const* userNatspec = "{\"methods\":{}}";

	checkNatspec(sourceCode, devNatspec, false);
	checkNatspec(sourceCode, userNatspec, true);
}

BOOST_AUTO_TEST_CASE(dev_desc_after_nl)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev\n"
	"  /// Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter\n"
	"  /// @param second Documentation for the second parameter\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \" Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter\n"
	"  /// @param second Documentation for the second parameter\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_mutiline_param_description)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter starts here.\n"
	"  /// Since it's a really complicated parameter we need 2 lines\n"
	"  /// @param second Documentation for the second parameter\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_functions)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter\n"
	"  /// @param second Documentation for the second parameter\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"  \n"
	"  /// @dev Divides 2 numbers\n"
	"  /// @param input Documentation for the input parameter\n"
	"  /// @param div Documentation for the div parameter\n"
	"  function divide(uint input, uint div) returns(uint d)\n"
	"  {\n"
	"      return input / div;\n"
	"  }\n"
	"  /// @dev Subtracts 3 from `input`\n"
	"  /// @param input Documentation for the input parameter\n"
	"  function sub(int input) returns(int d)\n"
	"  {\n"
	"      return input - 3;\n"
	"  }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    },\n"
	"    \"divide(uint256,uint256)\":{ \n"
	"        \"details\": \"Divides 2 numbers\",\n"
	"        \"params\": {\n"
	"            \"input\": \"Documentation for the input parameter\",\n"
	"            \"div\": \"Documentation for the div parameter\"\n"
	"        }\n"
	"    },\n"
	"    \"sub(int256)\":{ \n"
	"        \"details\": \"Subtracts 3 from `input`\",\n"
	"        \"params\": {\n"
	"            \"input\": \"Documentation for the input parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter starts here.\n"
	"  /// Since it's a really complicated parameter we need 2 lines\n"
	"  /// @param second Documentation for the second parameter\n"
	"  /// @return The result of the multiplication\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"return\": \"The result of the multiplication\"\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}
BOOST_AUTO_TEST_CASE(dev_return_desc_after_nl)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter starts here.\n"
	"  /// Since it's a really complicated parameter we need 2 lines\n"
	"  /// @param second Documentation for the second parameter\n"
	"  /// @return\n"
	"  /// The result of the multiplication\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"return\": \" The result of the multiplication\"\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}


BOOST_AUTO_TEST_CASE(dev_multiline_return)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Multiplies a number by 7 and adds second parameter\n"
	"  /// @param a Documentation for the first parameter starts here.\n"
	"  /// Since it's a really complicated parameter we need 2 lines\n"
	"  /// @param second Documentation for the second parameter\n"
	"  /// @return The result of the multiplication\n"
	"  /// and cookies with nutella\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"return\": \"The result of the multiplication and cookies with nutella\"\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiline_comment)
{
	char const* sourceCode = "contract test {\n"
	"  /**\n"
	"   * @dev Multiplies a number by 7 and adds second parameter\n"
	"   * @param a Documentation for the first parameter starts here.\n"
	"   * Since it's a really complicated parameter we need 2 lines\n"
	"   * @param second Documentation for the second parameter\n"
	"   * @return The result of the multiplication\n"
	"   * and cookies with nutella\n"
	"   */"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"return\": \"The result of the multiplication and cookies with nutella\"\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_contract_no_doc)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev Mul function\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_contract_doc)
{
	char const* sourceCode = " /// @author Lefteris\n"
	" /// @title Just a test contract\n"
	"contract test {\n"
	"  /// @dev Mul function\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"    \"author\": \"Lefteris\","
	"    \"title\": \"Just a test contract\","
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_author_at_function)
{
	char const* sourceCode = " /// @author Lefteris\n"
	" /// @title Just a test contract\n"
	"contract test {\n"
	"  /// @dev Mul function\n"
	"  /// @author John Doe\n"
	"  function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }\n"
	"}\n";

	char const* natspec = "{"
	"    \"author\": \"Lefteris\","
	"    \"title\": \"Just a test contract\","
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\",\n"
	"            \"author\": \"John Doe\",\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, natspec, false);
}

BOOST_AUTO_TEST_CASE(natspec_notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			function mul(uint a) returns(uint d) { return a * 7; }
		}
	)";


	char const* natspec = R"ABCDEF(
	{
	   "methods" : {
		  "mul(uint256)" : {
			 "notice" : "I do something awesome"
		  }
	   }
	}
	)ABCDEF";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(natspec_multiline_notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			/// which requires two lines to explain
			function mul(uint a) returns(uint d) { return a * 7; }
		}
	)";

	char const* natspec = R"ABCDEF(
	{
	   "methods" : {
		  "mul(uint256)" : {
			 "notice" : "I do something awesome which requires two lines to explain"
		  }
	   }
	}
	)ABCDEF";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
