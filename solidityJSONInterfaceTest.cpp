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
 * @author Marek Kotewicz <marek@ethdev.com>
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

class InterfaceChecker
{
public:
	bool checkInterface(std::string const& _code, std::string const& _expectedInterfaceString)
	{
		m_compilerStack.parse(_code);
		std::string generatedInterfaceString = m_compilerStack.getInterface();
		Json::Value generatedInterface;
		m_reader.parse(generatedInterfaceString, generatedInterface);
		Json::Value expectedInterface;
		m_reader.parse(_expectedInterfaceString, expectedInterface);
		return expectedInterface == generatedInterface;
	}
	
private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityCompilerJSONInterfaceOutput, InterfaceChecker)

BOOST_AUTO_TEST_CASE(basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* interface = R"([
	{
		"name": "f",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";

	BOOST_CHECK(checkInterface(sourceCode, interface));
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = "contract test {\n"
	"}\n";

	char const* interface = "[]";

	BOOST_CHECK(checkInterface(sourceCode, interface));
}

BOOST_AUTO_TEST_CASE(multiple_methods)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  function g(uint b) returns(uint e) { return b * 8; }\n"
	"}\n";
	
	char const* interface = R"([
	{
		"name": "f",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	},
	{
		"name": "g",
		"inputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "e",
			"type": "uint256"
		}
		]
	}
	])";

	BOOST_CHECK(checkInterface(sourceCode, interface));
}

BOOST_AUTO_TEST_CASE(multiple_params)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a, uint b) returns(uint d) { return a + b; }\n"
	"}\n";

	char const* interface = R"([
	{
		"name": "f",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		},
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";

	BOOST_CHECK(checkInterface(sourceCode, interface));
}

BOOST_AUTO_TEST_CASE(multiple_methods_order)
{
	// methods are expected to be in alpabetical order
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  function c(uint b) returns(uint e) { return b * 8; }\n"
	"}\n";
		
	char const* interface = R"([
	{
		"name": "c",
		"inputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "e",
			"type": "uint256"
		}
		]
	},
	{
		"name": "f",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";
	
	BOOST_CHECK(checkInterface(sourceCode, interface));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
