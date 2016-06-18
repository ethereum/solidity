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
 * @author Ruben de Vries <ruben@rubensayshi.com>
 * @date 2015
 * Unit tests for the solidity compiler JSON Interface output.
 */

#include "../TestHelper.h"
#include "../../libsolidity/interface/CompilerStack.h"
#include <string>
#include <json/json.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/Exceptions.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace solidity
{
namespace test
{

class StructsDocumentationChecker
{
public:
	StructsDocumentationChecker(): m_compilerStack(false) {}

	void checkStructsDocumentation(
		std::string const& _code,
		std::string const& _expectedStructsDocumentationString
	)
	{
		std::string generatedStructsDocumentationString;
		ETH_TEST_REQUIRE_NO_THROW(m_compilerStack.parse(_code), "Parsing failed");

		generatedStructsDocumentationString = m_compilerStack.metadata("", DocumentationType::Structs);
		Json::Value generatedStructsDocumentation;
		m_reader.parse(generatedStructsDocumentationString, generatedStructsDocumentation);
		Json::Value expectedStructsDocumentation;
		m_reader.parse(_expectedStructsDocumentationString, expectedStructsDocumentation);
		BOOST_CHECK_MESSAGE(
				expectedStructsDocumentation == generatedStructsDocumentation,
			"Expected " << _expectedStructsDocumentationString <<
			"\n but got:\n" << generatedStructsDocumentationString
		);
	}

private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityStructsJSON, StructsDocumentationChecker)

BOOST_AUTO_TEST_CASE(structs_basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  struct X {\n"
	"    uint a;\n"
	"    uint8 b;\n"
	"    uint256 c;\n"
	"    bytes d;\n"
	"    bytes32 e;\n"
	"    string f;\n"
	"    uint[] g;\n"
	"    string[] h;\n"
	"    address i;\n"
	"  }\n"
	"}\n";

	char const* structsDocumentation = "["
	"    {"
	"        \"members\": ["
	"            {"
	"                \"name\": \"a\","
	"                \"type\": \"uint256\""
	"            },"
	"            {"
	"                \"name\": \"b\","
	"                \"type\": \"uint8\""
	"            },"
	"            {"
	"                \"name\": \"c\","
	"                \"type\": \"uint256\""
	"            },"
	"            {"
	"                \"name\": \"d\","
	"                \"type\": \"bytes\""
	"            },"
	"            {"
	"                \"name\": \"e\","
	"                \"type\": \"bytes32\""
	"            },"
	"            {"
	"                \"name\": \"f\","
	"                \"type\": \"string\""
	"            },"
	"            {"
	"                \"name\": \"g\","
	"                \"type\": \"uint256[]\""
	"            },"
	"            {"
	"                \"name\": \"h\","
	"                \"type\": \"string[]\""
	"            },"
	"            {"
	"                \"name\": \"i\","
	"                \"type\": \"address\""
	"            }"
	"        ],"
	"        \"name\": \"X\","
	"        \"type\": \"struct\""
	"    }"
	"]";

	checkStructsDocumentation(sourceCode, structsDocumentation);
}


BOOST_AUTO_TEST_CASE(structs_nested_test)
{
	char const* sourceCode = "contract test {\n"
	"  struct X {\n"
	"    uint a;\n"
	"    Y b;\n"
	"    Y[] c;\n"
	"  }\n"
	"  struct Y {\n"
	"    uint a;\n"
	"    uint8 b;\n"
	"  }\n"
	"}\n";

	char const* structsDocumentation = "["
	"    {"
	"        \"members\": ["
	"            {"
	"                \"name\": \"a\","
	"                \"type\": \"uint256\""
	"            },"
	"            {"
	"                \"name\": \"b\","
	"                \"type\": \"test.Y\""
	"            },"
	"            {"
	"                \"name\": \"c\","
	"                \"type\": \"test.Y[]\""
	"            }"
	"        ],"
	"        \"name\": \"X\","
	"        \"type\": \"struct\""
	"    },"
	"    {"
	"        \"members\": ["
	"            {"
	"                \"name\": \"a\","
	"                \"type\": \"uint256\""
	"            },"
	"            {"
	"                \"name\": \"b\","
	"                \"type\": \"uint8\""
	"            }"
	"        ],"
	"        \"name\": \"Y\","
	"        \"type\": \"struct\""
	"    }"
	"]";

	checkStructsDocumentation(sourceCode, structsDocumentation);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
