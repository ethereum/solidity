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

namespace dev
{
namespace solidity
{
namespace test
{

class DocumentationChecker
{
public:
	void checkNatspec(std::string const& _code, std::string const& _expectedDocumentationString)
	{
		m_compilerStack.parse(_code);
		auto generatedDocumentationString = m_compilerStack.getDocumentation();
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

BOOST_AUTO_TEST_CASE(basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  /// Multiplies `a` by 7\n"
	"  function mul(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul\":{ \"user\": \" Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, natspec);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
}
