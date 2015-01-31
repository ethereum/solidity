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
	InterfaceChecker(): m_compilerStack(false) {}

	void checkInterface(std::string const& _code, std::string const& _expectedInterfaceString, std::string const& expectedSolidityInterface)
	{
		try
		{
			m_compilerStack.parse(_code);
		}
		catch(boost::exception const& _e)
		{
			auto msg = std::string("Parsing contract failed with: ") + boost::diagnostic_information(_e);
			BOOST_FAIL(msg);
		}
		std::string generatedInterfaceString = m_compilerStack.getMetadata("", DocumentationType::ABI_INTERFACE);
		std::string generatedSolidityInterfaceString = m_compilerStack.getMetadata("", DocumentationType::ABI_SOLIDITY_INTERFACE);
		Json::Value generatedInterface;
		m_reader.parse(generatedInterfaceString, generatedInterface);
		Json::Value expectedInterface;
		m_reader.parse(_expectedInterfaceString, expectedInterface);
		BOOST_CHECK_MESSAGE(expectedInterface == generatedInterface,
							"Expected:\n" << expectedInterface.toStyledString() <<
							"\n but got:\n" << generatedInterface.toStyledString());
		BOOST_CHECK_MESSAGE(expectedSolidityInterface == generatedSolidityInterfaceString,
							"Expected:\n" << expectedSolidityInterface <<
							"\n but got:\n" << generatedSolidityInterfaceString);
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

	char const* sourceInterface = "contract test{function f(uint256 a)returns(uint256 d){}}";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"type": "function",
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

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = "contract test {\n"
	"}\n";
	char const* sourceInterface = "contract test{}";

	char const* interface = "[]";

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(multiple_methods)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  function g(uint b) returns(uint e) { return b * 8; }\n"
	"}\n";
	char const* sourceInterface = "contract test{function f(uint256 a)returns(uint256 d){}function g(uint256 b)returns(uint256 e){}}";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"type": "function",
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
		"type": "function",
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

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(multiple_params)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a, uint b) returns(uint d) { return a + b; }\n"
	"}\n";
	char const* sourceInterface = "contract test{function f(uint256 a,uint256 b)returns(uint256 d){}}";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"type": "function",
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

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(multiple_methods_order)
{
	// methods are expected to be in alpabetical order
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  function c(uint b) returns(uint e) { return b * 8; }\n"
	"}\n";
	char const* sourceInterface = "contract test{function c(uint256 b)returns(uint256 e){}function f(uint256 a)returns(uint256 d){}}";

	char const* interface = R"([
	{
		"name": "c",
		"constant": false,
		"type": "function",
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
		"type": "function",
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

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(const_function)
{
	char const* sourceCode = "contract test {\n"
	"  function foo(uint a, uint b) returns(uint d) { return a + b; }\n"
	"  function boo(uint32 a) constant returns(uint b) { return a * 4; }\n"
	"}\n";
	char const* sourceInterface = "contract test{function foo(uint256 a,uint256 b)returns(uint256 d){}function boo(uint32 a)constant returns(uint256 b){}}";

	char const* interface = R"([
	{
		"name": "foo",
		"constant": false,
		"type": "function",
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
		"type": "function",
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

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(exclude_fallback_function)
{
	char const* sourceCode = "contract test { function() {} }";
	char const* sourceInterface = "contract test{}";

	char const* interface = "[]";

	checkInterface(sourceCode, interface, sourceInterface);
}

BOOST_AUTO_TEST_CASE(events)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  event e1(uint b, address indexed c); \n"
	"  event e2(); \n"
	"}\n";
	char const* sourceInterface = "contract test{function f(uint256 a)returns(uint256 d){}event e1(uint256 b,address indexed c);event e2;}";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"type": "function",
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
		"name": "e1",
		"type": "event",
		"inputs": [
		{
			"indexed": false,
			"name": "b",
			"type": "uint256"
		},
		{
			"indexed": true,
			"name": "c",
			"type": "address"
		}
		]
	},
	{
		"name": "e2",
		"type": "event",
		"inputs": []
	}

	])";

	checkInterface(sourceCode, interface, sourceInterface);
}


BOOST_AUTO_TEST_CASE(inherited)
{
	char const* sourceCode =
	"	contract Base { \n"
	"		function baseFunction(uint p) returns (uint i) { return p; } \n"
	"		event baseEvent(string32 indexed evtArgBase); \n"
	"	} \n"
	"	contract Derived is Base { \n"
	"		function derivedFunction(string32 p) returns (string32 i) { return p; } \n"
	"		event derivedEvent(uint indexed evtArgDerived); \n"
	"	}";
	char const* sourceInterface = "contract Derived{function baseFunction(uint256 p)returns(uint256 i){}function derivedFunction(string32 p)returns(string32 i){}event derivedEvent(uint256 indexed evtArgDerived);event baseEvent(string32 indexed evtArgBase);}";

	char const* interface = R"([
	{
		"name": "baseFunction",
		"constant": false,
		"type": "function",
		"inputs":
		[{
			"name": "p",
			"type": "uint256"
		}],
		"outputs":
		[{
			"name": "i",
			"type": "uint256"
		}]
	},
	{
		"name": "derivedFunction",
		"constant": false,
		"type": "function",
		"inputs":
		[{
			"name": "p",
			"type": "string32"
		}],
		"outputs":
		[{
			"name": "i",
			"type": "string32"
		}]
	},
	{
		"name": "derivedEvent",
		"type": "event",
		"inputs":
		[{
			"indexed": true,
			"name": "evtArgDerived",
			"type": "uint256"
		}]
	},
	{
		"name": "baseEvent",
		"type": "event",
		"inputs":
		[{
			"indexed": true,
			"name": "evtArgBase",
			"type": "string32"
		}]
	}])";


	checkInterface(sourceCode, interface, sourceInterface);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
}
