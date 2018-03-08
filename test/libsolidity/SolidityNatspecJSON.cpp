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
#include <libsolidity/interface/Exceptions.h>
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

	enum class NatspecType
	{
		DEVELOPER,
		USER
	};

	void checkNatspec(
		std::string const& _code,
		std::string const& _expectedDocumentationString,
		NatspecType _natspecType
	)
	{
		m_compilerStack.reset(false);
		m_compilerStack.addSource("", "pragma solidity >=0.0;\n" + _code);
		m_compilerStack.setEVMVersion(dev::test::Options::get().evmVersion());
		bool parseResult = m_compilerStack.parseAndAnalyze();
		if (!parseResult) {
			for (auto &error :  m_compilerStack.errors()) {
				std::cout << error->what() << std::endl;
			}
		}
		BOOST_REQUIRE_MESSAGE(parseResult, "Parsing contract failed");

		Json::Value generatedDocumentation;
		if (_natspecType == NatspecType::USER)
			generatedDocumentation = m_compilerStack.natspecUser(m_compilerStack.lastContractName());
		else if (_natspecType == NatspecType::DEVELOPER)
			generatedDocumentation = m_compilerStack.natspecDev(m_compilerStack.lastContractName());

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
			function mul(uint a) returns(uint d) { return a * 7; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \"notice\": \"Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, natspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(dev_and_user_basic_test)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// @dev Multiplies a number by 7
			function mul(uint a) returns(uint d) { return a * 7; }
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

	checkNatspec(sourceCode, devNatspec, NatspecType::DEVELOPER);
	checkNatspec(sourceCode, userNatspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(user_multiline_comment)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// and then adds `b`
			function mul_and_add(uint a, uint256 b) returns(uint256 d) {
				return (a * 7) + b;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"}"
	"}}";

	checkNatspec(sourceCode, natspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(user_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7 and then adds `b`
			function mul_and_add(uint a, uint256 b) returns(uint256 d) {
				return (a * 7) + b;
			}

			/// @notice Divides `input` by `div`
			function divide(uint input, uint div) returns(uint d) {
				return input / div;
			}

			/// @notice Subtracts 3 from `input`
			function sub(int input) returns(int d) {
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

	checkNatspec(sourceCode, natspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(user_empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";

	char const* natspec = "{\"methods\":{} }";

	checkNatspec(sourceCode, natspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(dev_and_user_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			function mul(uint a) returns(uint d) {
				return a * 7;
			}
			function sub(int input) returns(int d) {
				return input - 3;
			}
		}
	)";

	char const* devNatspec = "{\"methods\":{}}";
	char const* userNatspec = "{\"methods\":{}}";

	checkNatspec(sourceCode, devNatspec, NatspecType::DEVELOPER);
	checkNatspec(sourceCode, userNatspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(dev_desc_after_nl)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev
			/// Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params_mixed_whitespace)
{
	char const* sourceCode = "contract test {\n"
	"  /// @dev	 Multiplies a number by 7 and adds second parameter\n"
	"  /// @param 	 a Documentation for the first parameter\n"
	"  /// @param	 second			 Documentation for the second parameter\n"
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_mutiline_param_description)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) returns(uint d) {
				return a * 7 + second;
			}
			/// @dev Divides 2 numbers
			/// @param input Documentation for the input parameter
			/// @param div Documentation for the div parameter
			function divide(uint input, uint div) returns(uint d) {
				return input / div;
			}
			/// @dev Subtracts 3 from `input`
			/// @param input Documentation for the input parameter
			function sub(int input) returns(int d) {
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
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
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
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
			function mul(uint a, uint second) returns(uint d) {
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
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
			function mul(uint a, uint second) returns(uint d) {
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
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
			function mul(uint a, uint second) returns(uint d) {
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_contract_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_contract_doc)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(dev_author_at_function)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @author John Doe
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
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

	checkNatspec(sourceCode, natspec, NatspecType::USER);
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

	checkNatspec(sourceCode, natspec, NatspecType::USER);
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

	checkNatspec(sourceCode, natspec, NatspecType::USER);
}

BOOST_AUTO_TEST_CASE(dev_title_at_function_error)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @title I really should not be here
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
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
			function mul(uint a, uint second) returns(uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_contract)
{
	char const* sourceCode = R"(/// @external:testA LINE-1
		contract test {
			uint256 stateVar;
			function functionName1(bytes32 input) returns (bytes32 out) {}
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n",
			"line" : 1
		}
	},
	"methods" : {}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_author)
{
	char const* sourceCode = R"(/// @author Alex
		/// @external:testA LINE-2
		contract test {
			uint256 stateVar;
			function functionName1(bytes32 input) returns (bytes32 out) {}
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"author" : "Alex",
	"external" : {
		"testA" : {
			"content" : " LINE-2\n",
			"line" : 2
		}
	},
	"methods" : {}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_author_notes_author_title)
{
	char const* sourceCode = R"(/// hello!
		/// this is an interesting note
		/// that uses
		/// multiple
		/// lines. we need to check
		/// whether the line counting still works.
		/// @author Alex
		/// @external:testA LINE-8
		/// @title My Contract
		/// @external:testB LINE-10
		contract test {
			uint256 stateVar;
			function functionName1(bytes32 input) returns (bytes32 out) {}
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"author" : "Alex",
		"external" : {
		"testA" : {
			"content" : " LINE-8\n",
			"line" : 8
		},
		"testB" : {
			"content" : " LINE-10\n",
			"line" : 10
		}
	},
	"methods" : {},
	"title" : "My Contract"
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_notes_author_title_dev)
{
	char const* sourceCode = R"(/// hello!
		/// this is an interesting note
		/// that uses
		/// multiple
		/// lines. we need to check
		/// whether the line counting still works.
		/// @author Alex
		/// @external:testA LINE-8
		/// @title My Contract
		/// @dev this
		/// is
        /// awesome!
		/// @external:testB LINE-13
		contract test {
			uint256 stateVarA;
			uint256 stateVarB;
			uint256 stateVarC;
			uint256 stateVarD;
			function functionName1(bytes32 input) returns (bytes32 out) {}
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"author" : "Alex",
		"external" : {
		"testA" : {
			"content" : " LINE-8\n",
				"line" : 8
		},
		"testB" : {
			"content" : " LINE-13\n",
				"line" : 13
		}
	},
	"methods" : {},
	"title" : "My Contract"
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_methods)
{
	char const* sourceCode = R"(/// @ext:testA LINE-1
		contract test {
			uint256 stateVar;
			/// @external:testB LINE-4
			function functionName1(bytes32 input) returns (bytes32 out) {}
			/// @ext:testC LINE-6
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n",
			"line" : 1
		}
	},
	"methods" : {
		"functionName1(bytes32)" : {
			"external" : {
				"testB" : {
					"content" : " LINE-4\n",
					"line" : 4
				}
			}
		},
		"functionName2(bytes32)" : {
			"external" : {
				"testC" : {
					"content" : " LINE-6\n",
					"line" : 6
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_methods_notes_params)
{
	char const* sourceCode = R"(/// @ext:testA LINE-1
		/// what is that?
		/// @ext:testB LINE-3
		contract test {
			uint256 stateVar;
			/// bla bla
			/// BLAH BLAAH!
			/// @param input attention! the input is not the input.
			/// @external:testB LINE-9
			/// @param p2 is also not the
			/// input.
			/// yep.
			/// @external:testC LINE-13
			function functionName1(bytes32 input, bytes32 p2) returns (bytes32 out) {}
			/// @ext:testC LINE-15
			/// LINE-16
			/// @param input as the name says.
			/// @ext:testD LINE-18
			/// LINE-19
			/// @dev dev dev
			/// dev dev dev dev dev
			/// d d d d d - dev - v!!!
			/// @ext:testE LINE-23
			/// LINE-24
			/// LINE-25
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n what is that?\n",
			"line" : 1
		},
		"testB" : {
			"content" : " LINE-3\n",
			"line" : 3
		}
	},
	"methods" : {
		"functionName1(bytes32,bytes32)" : {
			"external" : {
				"testB" : {
					"content" : " LINE-9\n",
					"line" : 9
				},
				"testC" : {
					"content" : " LINE-13\n",
					"line" : 13
				}
			},
			"params" : {
				"input" : "attention! the input is not the input.",
				"p2" : "is also not the input. yep."
			}
		},
		"functionName2(bytes32)" : {
			"details" : "dev dev dev dev dev dev dev d d d d d - dev - v!!!",
				"external" : {
				"testC" : {
					"content" : " LINE-15\n LINE-16\n",
					"line" : 15
				},
				"testD" : {
					"content" : " LINE-18\n LINE-19\n",
					"line" : 18
				},
				"testE" : {
					"content" : " LINE-23\n LINE-24\n LINE-25\n",
					"line" : 23
				}
			},
			"params" : {
				"input" : "as the name says."
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_functions_multiline)
{
	char const* sourceCode = R"(/// @external:testA LINE-1
		/// LINE-2
		/// LINE-3
		contract test {
			uint256 stateVar;
			/// @external:testB LINE-6
			/// LINE-7
			/// LINE-8
			function functionName1(bytes32 input) returns (bytes32 out) {}
			/// @ext:testC LINE-10
			/// LINE-11
			/// LINE-12
			function functionName2(bytes32 input) returns (bytes32 out) {}
		}
	)";
	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n LINE-2\n LINE-3\n",
			"line" : 1
		}
	},
	"methods" : {
		"functionName1(bytes32)" : {
			"external" : {
				"testB" : {
					"content" : " LINE-6\n LINE-7\n LINE-8\n",
					"line" : 6
				}
			}
		},
		"functionName2(bytes32)" : {
			"external" : {
				"testC" : {
					"content" : " LINE-10\n LINE-11\n LINE-12\n",
					"line" : 10
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_functions_multiline_second)
{
	char const* sourceCode = R"(/// @external:ModuleA fancy moduleA annotation
	/// multiline annotation for moduleA
	contract Contract {
		uint256 stateVar;
		/// @external:ModuleB awesome moduleB annotation
		/// fancy multiline annotation for moduleB
		/// @external:ModuleC moduleC
		function functionName1(bytes32 input) returns (bytes32 out) {}
		/// @ext:ModuleD fancy annotation for moduleD
		function functionName2(bytes32 input) returns (bytes32 out) {}
	}
	)";
	char const* natspec = R"ABCDEF({
	"external" : {
		"ModuleA" : {
			"content" : " fancy moduleA annotation\n multiline annotation for moduleA\n",
			"line" : 1
		}
	},
	"methods" : {
		"functionName1(bytes32)" : {
			"external" : {
				"ModuleB" : {
					"content" : " awesome moduleB annotation\n fancy multiline annotation for moduleB\n",
						"line" : 5
				},
				"ModuleC" : {
					"content" : " moduleC\n",
					"line" : 7
				}
			}
		},
		"functionName2(bytes32)" : {
			"external" : {
				"ModuleD" : {
					"content" : " fancy annotation for moduleD\n",
					"line" : 9
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_multiline)
{
	char const* sourceCode = R"(/// @external:testA LINE-1
		/// LINE-2
		/// LINE-3
		/// @ext:testB LINE-4
		/// LINE-5
		/// LINE-6
		contract test {
			uint256 stateVar;
			function functionName1(bytes32 input) returns (bytes32 out) {}
			function functionName2(bytes32 input) returns (bytes32 out) {}
			function functionName3(bytes32 input) returns (bytes32 out) {}
			function functionName4(bytes32 input) returns (bytes32 out) {}
		}
	)";

	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n LINE-2\n LINE-3\n",
			"line" : 1
		},
		"testB" : {
			"content" : " LINE-4\n LINE-5\n LINE-6\n",
			"line" : 4
		}
	},
	"methods" : {}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_function)
{
	char const* sourceCode = R"(contract test {
			uint256 stateVar;
			/// @external:testA LINE-3
			function functionName1(bytes32 input) returns (bytes32 out) {}
			/// @ext:testB LINE-5
			function functionName2(bytes32 input) returns (bytes32 out) {}
			/// @external:testC LINE-7
			function functionName3(bytes32 input) returns (bytes32 out) {}
			/// @ext:testD LINE-9
			function functionName4(bytes32 input) returns (bytes32 out) {}
		}
	)";

	char const* natspec = R"ABCDEF({
	"methods" : {
		"functionName1(bytes32)" : {
			"external" : {
				"testA" : {
					"content" : " LINE-3\n",
					"line" : 3
				}
			}
		},
		"functionName2(bytes32)" : {
			"external" : {
				"testB" : {
					"content" : " LINE-5\n",
					"line" : 5
				}
			}
		},
		"functionName3(bytes32)" : {
			"external" : {
				"testC" : {
					"content" : " LINE-7\n",
					"line" : 7
				}
			}
		},
		"functionName4(bytes32)" : {
			"external" : {
				"testD" : {
					"content" : " LINE-9\n",
					"line" : 9
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_function_multiline)
{
	char const* sourceCode = R"(contract test {
			uint256 stateVar;
			/// @ext:testA LINE-3
			/// LINE-4
			/// LINE-5
			/// @external:testB LINE-6
			/// LINE-7
			/// LINE-8
			/// LINE-9
			function functionName1(bytes32 input) returns (bytes32 out) {}
		}
	)";

	char const* natspec = R"ABCDEF({
	"methods" : {
		"functionName1(bytes32)" : {
			"external" : {
				"testA" : {
					"content" : " LINE-3\n LINE-4\n LINE-5\n",
					"line" : 3
				},
				"testB" : {
					"content" : " LINE-6\n LINE-7\n LINE-8\n LINE-9\n",
					"line" : 6
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_multi)
{
	char const* sourceCode = R"(/// @ext:testA LINE-1
		/// LINE-2
		/// @external:testB LINE-3
		/// LINE-4
		contract test {
			uint256 stateVar;
			function functionName1(bytes32 input) returns (bytes32 out) {}
		}
	)";

	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n LINE-2\n",
			"line" : 1
		},
		"testB" : {
			"content" : " LINE-3\n LINE-4\n",
			"line" : 3
		}
	},
	"methods" : {}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_function_multi)
{
	char const* sourceCode = R"(/// @ext:testA LINE-1
		/// LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA LINE-5
			/// @external:testB LINE-6
			/// LINE-7
			/// LINE-8
			/// LINE-9
			/// @external:testC LINE-10
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";

	char const* natspec = R"ABCDEF({
	"external" : {
		"testA" : {
			"content" : " LINE-1\n LINE-2\n",
			"line" : 1
		}
	},
	"methods" : {
		"functionName1(bytes32,bytes32,bytes32)" : {
			"external" : {
				"testA" : {
					"content" : " LINE-5\n",
					"line" : 5
				},
				"testB" : {
					"content" : " LINE-6\n LINE-7\n LINE-8\n LINE-9\n",
					"line" : 6
				},
				"testC" : {
					"content" : " LINE-10\n",
					"line" : 10
				}
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, natspec, NatspecType::DEVELOPER);
}

BOOST_AUTO_TEST_CASE(ext_contract_atomicity)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @external:testB testB-LINE-1
			/// testB-LINE-2
			/// testB-LINE-4
			/// testB-LINE-3
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_contract_atomicity_second)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		/// @ext:testB testA-LINE-3
		/// @ext:testA testA-LINE-4
		/// testA-LINE-5
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @external:testB testB-LINE-1
			/// testB-LINE-2
			/// testB-LINE-4
			/// testB-LINE-3
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_contract_atomicity_third)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		/// @author Alex
		/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @external:testB testB-LINE-1
			/// testB-LINE-2
			/// testB-LINE-4
			/// testB-LINE-3
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_function_atomicity_first)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @external:testA testA-LINE-2
			/// testA-LINE-2
			/// testA-LINE-4
			/// testA-LINE-3
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_function_atomicity_second)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @external:testB testA-LINE-2
			/// testA-LINE-2
			/// testA-LINE-4
			/// testA-LINE-3
			/// @ext:testA testA-LINE-1
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(ext_function_atomicity_third)
{
	char const* sourceCode = R"(/// @ext:testA testA-LINE-1
		/// testA-LINE-2
		contract test {
			uint256 stateVar;
			/// @ext:testA testA-LINE-1
			/// @author Alex
			/// @ext:testA testA-LINE-1
			function functionName1(bytes32 input1, bytes32 input2, bytes32 input3) returns (bytes32 out) {}
		}
	)";
	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
