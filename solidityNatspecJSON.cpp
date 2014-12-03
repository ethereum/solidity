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

#include <boost/test/unit_test.hpp>
#include <libsolidity/CompilerStack.h>
#include <jsonrpc/json/json.h>
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
	void checkNatspec(std::string const& _code,
					  std::string const& _expectedDocumentationString,
					  bool _userDocumentation)
	{
		std::string generatedDocumentationString;
		try
		{
			m_compilerStack.parse(_code);
		}
		catch (const std::exception& e)
		{
			std::string const* extra = boost::get_error_info<errinfo_comment>(e);
			std::string msg = std::string("Parsing contract failed with: ") +
				e.what() + std::string("\n");
			if (extra)
				msg += *extra;
			BOOST_FAIL(msg);
		}

		if (_userDocumentation)
			generatedDocumentationString = *m_compilerStack.getJsonDocumentation(NATSPEC_USER);
		else
			generatedDocumentationString = *m_compilerStack.getJsonDocumentation(NATSPEC_DEV);
		Json::Value generatedDocumentation;
		m_reader.parse(generatedDocumentationString, generatedDocumentation);
		Json::Value expectedDocumentation;
		m_reader.parse(_expectedDocumentationString, expectedDocumentation);
		BOOST_CHECK_MESSAGE(expectedDocumentation == generatedDocumentation,
							"Expected " << _expectedDocumentationString <<
							"\n but got:\n" << generatedDocumentationString);
	}

private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityNatspecJSON, DocumentationChecker)

BOOST_AUTO_TEST_CASE(user_basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  /// Multiplies `a` by 7\n"
	"  function mul(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul\":{ \"notice\": \" Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiline_comment)
{
	char const* sourceCode = "contract test {\n"
	"  /// Multiplies `a` by 7\n"
	"  /// and then adds `b`\n"
	"  function mul_and_add(uint a, uint256 b) returns(uint256 d)\n"
	"  {\n"
	"      return (a * 7) + b;\n"
	"  }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add\":{ \"notice\": \" Multiplies `a` by 7\n and then adds `b`\"}"
	"}}";

	checkNatspec(sourceCode, natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiple_functions)
{
	char const* sourceCode = "contract test {\n"
	"  /// Multiplies `a` by 7\n"
	"  /// and then adds `b`\n"
	"  function mul_and_add(uint a, uint256 b) returns(uint256 d)\n"
	"  {\n"
	"      return (a * 7) + b;\n"
	"  }\n"
	"\n"
	"  /// Divides `input` by `div`\n"
	"  function divide(uint input, uint div) returns(uint d)\n"
	"  {\n"
	"      return input / div;\n"
	"  }\n"
	"  /// Subtracts 3 from `input`\n"
	"  function sub(int input) returns(int d)\n"
	"  {\n"
	"      return input - 3;\n"
	"  }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add\":{ \"notice\": \" Multiplies `a` by 7\n and then adds `b`\"},"
	"    \"divide\":{ \"notice\": \" Divides `input` by `div`\"},"
	"    \"sub\":{ \"notice\": \" Subtracts 3 from `input`\"}"
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


BOOST_AUTO_TEST_SUITE_END()

}
}
}
