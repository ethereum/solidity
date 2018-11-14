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
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2014
 * Unit tests for the solidity compiler JSON Interface output.
 */

#include <test/Options.h>
#include <string>
#include <libdevcore/JSON.h>
#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/Exceptions.h>
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
	DocumentationChecker(): m_compilerStack() {}

	void checkNatspec(
		std::string const& _code,
		std::string const& _contractName,
		std::string const& _expectedDocumentationString,
		bool _userDocumentation
	)
	{
		m_compilerStack.reset(false);
		m_compilerStack.addSource("", "pragma solidity >=0.0;\n" + _code);
		m_compilerStack.setEVMVersion(dev::test::Options::get().evmVersion());
		BOOST_REQUIRE_MESSAGE(m_compilerStack.parseAndAnalyze(), "Parsing contract failed");

		Json::Value generatedDocumentation;
		if (_userDocumentation)
			generatedDocumentation = m_compilerStack.natspecUser(_contractName);
		else
			generatedDocumentation = m_compilerStack.natspecDev(_contractName);
		Json::Value expectedDocumentation;
		jsonParseStrict(_expectedDocumentationString, expectedDocumentation);
		BOOST_CHECK_MESSAGE(
			expectedDocumentation == generatedDocumentation,
			"Expected:\n" << expectedDocumentation.toStyledString() <<
			"\n but got:\n" << generatedDocumentation.toStyledString()
		);
	}

	void expectNatspecError(std::string const& _code)
	{
		m_compilerStack.reset(false);
		m_compilerStack.addSource("", "pragma solidity >=0.0;\n" + _code);
		m_compilerStack.setEVMVersion(dev::test::Options::get().evmVersion());
		BOOST_CHECK(!m_compilerStack.parseAndAnalyze());
		BOOST_REQUIRE(Error::containsErrorOfType(m_compilerStack.errors(), Error::Type::DocstringParsingError));
	}

private:
	CompilerStack m_compilerStack;
};

BOOST_FIXTURE_TEST_SUITE(SolidityNatspecJSON, DocumentationChecker)

BOOST_AUTO_TEST_CASE(user_basic_test)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			function mul(uint a) public returns(uint d) { return a * 7; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \"notice\": \"Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_basic_test)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// @dev Multiplies a number by 7
			function mul(uint a) public returns (uint d) { return a * 7; }
		}
	)";

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

	checkNatspec(sourceCode, "test", devNatspec, false);
	checkNatspec(sourceCode, "test", userNatspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiline_comment)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// and then adds `b`
			function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
				return (a * 7) + b;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7 and then adds `b`
			function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
				return (a * 7) + b;
			}

			/// @notice Divides `input` by `div`
			function divide(uint input, uint div) public returns (uint d) {
				return input / div;
			}

			/// @notice Subtracts 3 from `input`
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"},"
	"    \"divide(uint256,uint256)\":{ \"notice\": \"Divides `input` by `div`\"},"
	"    \"sub(int256)\":{ \"notice\": \"Subtracts 3 from `input`\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";

	char const* natspec = "{\"methods\":{} }";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			function mul(uint a) public returns (uint d) {
				return a * 7;
			}
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

	char const* devNatspec = "{\"methods\":{}}";
	char const* userNatspec = "{\"methods\":{}}";

	checkNatspec(sourceCode, "test", devNatspec, false);
	checkNatspec(sourceCode, "test", userNatspec, true);
}

BOOST_AUTO_TEST_CASE(dev_desc_after_nl)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev
			/// Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params_mixed_whitespace)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev	 Multiplies a number by 7 and adds second parameter\n"
	"  /// @param 	 a Documentation for the first parameter\n"
	"  /// @param	 second			 Documentation for the second parameter\n"
	"  function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }\n"
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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_mutiline_param_description)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
			/// @dev Divides 2 numbers
			/// @param input Documentation for the input parameter
			/// @param div Documentation for the div parameter
			function divide(uint input, uint div) public returns (uint d) {
				return input / div;
			}
			/// @dev Subtracts 3 from `input`
			/// @param input Documentation for the input parameter
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return The result of the multiplication
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}
BOOST_AUTO_TEST_CASE(dev_return_desc_after_nl)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return
			/// The result of the multiplication
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}


BOOST_AUTO_TEST_CASE(dev_multiline_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return The result of the multiplication
			/// and cookies with nutella
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiline_comment)
{
	char const* sourceCode = R"(
		contract test {
			/**
			 * @dev Multiplies a number by 7 and adds second parameter
			 * @param a Documentation for the first parameter starts here.
			 * Since it's a really complicated parameter we need 2 lines
			 * @param second Documentation for the second parameter
			 * @return The result of the multiplication
			 * and cookies with nutella
			 */
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_contract_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_contract_doc)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"    \"author\": \"Lefteris\","
	"    \"title\": \"Just a test contract\","
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_author_at_function)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @author John Doe
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

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

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(natspec_notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			function mul(uint a) public returns (uint d) { return a * 7; }
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

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(natspec_multiline_notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			/// which requires two lines to explain
			function mul(uint a) public returns (uint d) { return a * 7; }
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

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(empty_comment)
{
	char const* sourceCode = R"(
		//
		contract test
		{}
	)";
	char const* natspec = R"ABCDEF(
	{
	   "methods" : {}
	}
	)ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_title_at_function_error)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @title I really should not be here
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_nonexistent_param)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param not_existing Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_paramname)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_paramname_end)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param se
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_param_description)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(user_constructor)
{
	char const *sourceCode = R"(
		contract test {
			/// @notice this is a really nice constructor
			constructor(uint a, uint second) public { }
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods" : {
		"constructor" : "this is a really nice constructor"
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_constructor_and_function)
{
	char const *sourceCode = R"(
		contract test {
			/// @notice this is a really nice constructor
			constructor(uint a, uint second) public { }
			/// another multiplier
			function mul(uint a, uint second) public returns(uint d) { return a * 7 + second; }
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods" : {
		"mul(uint256,uint256)" : {
			"notice" : "another multiplier"
		},
		"constructor" : "this is a really nice constructor"
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_constructor)
{
	char const *sourceCode = R"(
		contract test {
			/// @author Alex
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			constructor(uint a, uint second) public { }
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods" : {
		"constructor" : {
			"author" : "Alex",
			"params" : {
				"a" : "the parameter a is really nice and very useful",
				"second" : "the second parameter is not very useful, it just provides additional confusion"
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_constructor_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @author Alex
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			/// @return return should not work within constructors
			constructor(uint a, uint second) public { }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_constructor_and_function)
{
	char const *sourceCode = R"(
		contract test {
			/// @author Alex
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			constructor(uint a, uint second) public { }
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return The result of the multiplication
			/// and cookies with nutella
			function mul(uint a, uint second) public returns(uint d) {
				return a * 7 + second;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods" : {
		"mul(uint256,uint256)" : {
			"details" : "Multiplies a number by 7 and adds second parameter",
			"params" : {
				"a" : "Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines",
				"second" : "Documentation for the second parameter"
			},
			"return" : "The result of the multiplication and cookies with nutella"
		},
		"constructor" : {
			"author" : "Alex",
			"params" : {
				"a" : "the parameter a is really nice and very useful",
				"second" : "the second parameter is not very useful, it just provides additional confusion"
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
