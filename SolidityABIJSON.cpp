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
#include <jsoncpp/json/json.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace solidity
{
namespace test
{

class InterfaceChecker
{
public:
	void checkInterface(std::string const& _code, std::string const& _expectedInterfaceString)
	{
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
		std::string generatedInterfaceString = m_compilerStack.getMetadata("", DocumentationType::ABI_INTERFACE);
		Json::Value generatedInterface;
		m_reader.parse(generatedInterfaceString, generatedInterface);
		Json::Value expectedInterface;
		m_reader.parse(_expectedInterfaceString, expectedInterface);
		BOOST_CHECK_MESSAGE(expectedInterface == generatedInterface,
							"Expected " << _expectedInterfaceString <<
							"\n but got:\n" << generatedInterfaceString);
	}

private:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityABIJSON, InterfaceChecker)

BOOST_AUTO_TEST_CASE(basic_test)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"}\n";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = "contract test {\n"
	"}\n";

	char const* interface = "[]";

	checkInterface(sourceCode, interface);
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
		"constant": false,
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
		"constant": false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_params)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a, uint b) returns(uint d) { return a + b; }\n"
	"}\n";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
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

	checkInterface(sourceCode, interface);
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
		"constant": false,
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
		"constant": false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(const_function)
{
	char const* sourceCode = "contract test {\n"
	"  function foo(uint a, uint b) returns(uint d) { return a + b; }\n"
	"  function boo(uint32 a) constant returns(uint b) { return a * 4; }\n"
	"}\n";

	char const* interface = R"([
	{
		"name": "foo",
		"constant": false,
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
	},
	{
		"name": "boo",
		"constant": true,
		"inputs": [{
			"name": "a",
			"type": "uint32"
		}],
		"outputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
