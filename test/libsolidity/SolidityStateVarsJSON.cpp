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

class StateVarsDocumentationChecker
{
public:
	StateVarsDocumentationChecker(): m_compilerStack(false) {}

	void checkStateVarsDocumentation(
		std::string const& _code,
		std::string const& _expectedStateVarsDocumentationString
	)
	{
		std::string generatedStateVarsDocumentationString;
		ETH_TEST_REQUIRE_NO_THROW(m_compilerStack.parse(_code), "Parsing failed");

		generatedStateVarsDocumentationString = m_compilerStack.metadata("", DocumentationType::StateVariables);
		Json::Value generatedStateVarsDocumentation;
		m_reader.parse(generatedStateVarsDocumentationString, generatedStateVarsDocumentation);
		Json::Value expectedStateVarsDocumentation;
		m_reader.parse(_expectedStateVarsDocumentationString, expectedStateVarsDocumentation);
		BOOST_CHECK_MESSAGE(
				expectedStateVarsDocumentation == generatedStateVarsDocumentation,
			"Expected " << _expectedStateVarsDocumentationString <<
			"\n but got:\n" << generatedStateVarsDocumentationString
		);
	}

private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityStateVarsJSON, StateVarsDocumentationChecker)

BOOST_AUTO_TEST_CASE(statevars_basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  uint a;\n"
	"  uint8 b;\n"
	"  uint256 c;\n"
	"  bytes d;\n"
	"  bytes32 e;\n"
	"  string f;\n"
	"  uint[] g;\n"
	"  string[] h;\n"
	"  address i;\n"
	"}\n";

	char const* statevarsDocumentation = "["
	"    {"
	"        \"name\": \"a\","
	"        \"type\": \"uint256\""
	"    },"
	"    {"
	"        \"name\": \"b\","
	"        \"type\": \"uint8\""
	"    },"
	"    {"
	"        \"name\": \"c\","
	"        \"type\": \"uint256\""
	"    },"
	"    {"
	"        \"name\": \"d\","
	"        \"type\": \"bytes\""
	"    },"
	"    {"
	"        \"name\": \"e\","
	"        \"type\": \"bytes32\""
	"    },"
	"    {"
	"        \"name\": \"f\","
	"        \"type\": \"string\""
	"    },"
	"    {"
	"        \"name\": \"g\","
	"        \"type\": \"uint256[]\""
	"    },"
	"    {"
	"        \"name\": \"h\","
	"        \"type\": \"string[]\""
	"    },"
	"    {"
	"        \"name\": \"i\","
	"        \"type\": \"address\""
	"    }"
	"]";

	checkStateVarsDocumentation(sourceCode, statevarsDocumentation);
}



BOOST_AUTO_TEST_CASE(statevars_structs_test)
{
	char const* sourceCode = "contract test {\n"
	"  struct X {\n"
	"    uint a;\n"
	"  }\n"
	"\n"
	"  X a;\n"
	"  X[] b;\n"
	"}\n";

	char const* statevarsDocumentation = "["
	"    {"
	"        \"name\": \"a\","
	"        \"type\": \"test.X\""
	"    },"
	"    {"
	"        \"name\": \"b\","
	"        \"type\": \"test.X[]\""
	"    }"
	"]";

	checkStateVarsDocumentation(sourceCode, statevarsDocumentation);
}

BOOST_AUTO_TEST_CASE(statevars_mapping_test)
{
	char const* sourceCode = "contract test {\n"
	"  struct X {\n"
	"    uint a;\n"
	"  }\n"
	"\n"
	"  mapping(uint => uint) data1;\n"
	"  mapping(string => string) data2;\n"
	"  mapping(uint => string) data3;\n"
	"  mapping(uint => X) data4;\n"
	"}\n";

	char const* statevarsDocumentation = "["
	"    {"
	"        \"name\": \"data1\","
	"        \"type\": \"mapping(uint256 => uint256)\""
	"    },"
	"    {"
	"        \"name\": \"data2\","
	"        \"type\": \"mapping(string => string)\""
	"    },"
	"    {"
	"        \"name\": \"data3\","
	"        \"type\": \"mapping(uint256 => string)\""
	"    },"
	"    {"
	"        \"name\": \"data4\","
	"        \"type\": \"mapping(uint256 => test.X)\""
	"    }"
	"]";

	checkStateVarsDocumentation(sourceCode, statevarsDocumentation);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
