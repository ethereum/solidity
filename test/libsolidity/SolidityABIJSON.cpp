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
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2014
 * Unit tests for the solidity compiler JSON Interface output.
 */

#include "../TestHelper.h"
#include <libsolidity/interface/CompilerStack.h>

#include <libdevcore/Exceptions.h>
#include <libdevcore/SwarmHash.h>

#include <json/json.h>

namespace dev
{
namespace solidity
{
namespace test
{

class JSONInterfaceChecker
{
public:
	JSONInterfaceChecker(): m_compilerStack() {}

	void checkInterface(std::string const& _code, std::string const& _expectedInterfaceString)
	{
		ETH_TEST_REQUIRE_NO_THROW(m_compilerStack.parseAndAnalyze("pragma solidity >=0.0;\n" + _code), "Parsing contract failed");

		Json::Value generatedInterface = m_compilerStack.contractABI("");
		Json::Value expectedInterface;
		m_reader.parse(_expectedInterfaceString, expectedInterface);
		BOOST_CHECK_MESSAGE(
			expectedInterface == generatedInterface,
			"Expected:\n" << expectedInterface.toStyledString() <<
			"\n but got:\n" << generatedInterface.toStyledString()
		);
	}

protected:
	CompilerStack m_compilerStack;
	Json::Reader m_reader;
};

BOOST_FIXTURE_TEST_SUITE(SolidityABIJSON, JSONInterfaceChecker)

BOOST_AUTO_TEST_CASE(basic_test)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint d) { return a * 7; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";
	char const* interface = "[]";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_methods)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint d) { return a * 7; }
			function g(uint b) returns(uint e) { return b * 8; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
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
		"payable" : false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_params)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a, uint b) returns(uint d) { return a + b; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(multiple_methods_order)
{
	// methods are expected to be in alpabetical order
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint d) { return a * 7; }
			function c(uint b) returns(uint e) { return b * 8; }
		}
	)";

	char const* interface = R"([
	{
		"name": "c",
		"constant": false,
		"payable" : false,
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
		"payable" : false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(const_function)
{
	char const* sourceCode = R"(
		contract test {
			function foo(uint a, uint b) returns(uint d) { return a + b; }
			function boo(uint32 a) constant returns(uint b) { return a * 4; }
		}
	)";

	char const* interface = R"([
	{
		"name": "foo",
		"constant": false,
		"payable" : false,
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
		"payable" : false,
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

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(events)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns(uint d) { return a * 7; }
			event e1(uint b, address indexed c);
			event e2();
		}
	)";
	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
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
		"anonymous": false,
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
		"anonymous": false,
		"inputs": []
	}

	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(events_anonymous)
{
	char const* sourceCode = R"(
		contract test {
			event e() anonymous;
		}
	)";
	char const* interface = R"([
	{
		"name": "e",
		"type": "event",
		"anonymous": true,
		"inputs": []
	}

	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(inherited)
{
	char const* sourceCode = R"(
		contract Base {
			function baseFunction(uint p) returns (uint i) { return p; }
			event baseEvent(bytes32 indexed evtArgBase);
		}
		contract Derived is Base {
			function derivedFunction(bytes32 p) returns (bytes32 i) { return p; }
			event derivedEvent(uint indexed evtArgDerived);
		}
	)";

	char const* interface = R"([
	{
		"name": "baseFunction",
		"constant": false,
		"payable" : false,
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
		"payable" : false,
		"type": "function",
		"inputs":
		[{
			"name": "p",
			"type": "bytes32"
		}],
		"outputs":
		[{
			"name": "i",
			"type": "bytes32"
		}]
	},
	{
		"name": "derivedEvent",
		"type": "event",
		"anonymous": false,
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
		"anonymous": false,
		"inputs":
		[{
			"indexed": true,
			"name": "evtArgBase",
			"type": "bytes32"
		}]
	}])";


	checkInterface(sourceCode, interface);
}
BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint, uint k) returns(uint ret_k, uint ret_g) {
				uint g = 8;
				ret_k = k;
				ret_g = g;
			}
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"type": "function",
		"inputs": [
		{
			"name": "",
			"type": "uint256"
		},
		{
			"name": "k",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "ret_k",
			"type": "uint256"
		},
		{
			"name": "ret_g",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint k) returns(uint) {
				return k;
			}
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"type": "function",
		"inputs": [
		{
			"name": "k",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "",
			"type": "uint256"
		}
		]
	}
	])";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(constructor_abi)
{
	char const* sourceCode = R"(
		contract test {
			function test(uint param1, test param2, bool param3) {}
		}
	)";

	char const* interface = R"([
	{
		"inputs": [
			{
				"name": "param1",
				"type": "uint256"
			},
			{
				"name": "param2",
				"type": "address"
			},
			{
				"name": "param3",
				"type": "bool"
			}
		],
		"payable": false,
		"type": "constructor"
	}
	])";
	checkInterface(sourceCode, interface);
}


BOOST_AUTO_TEST_CASE(return_param_in_abi)
{
	// bug #1801
	char const* sourceCode = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			function test(ActionChoices param) {}
			function ret() returns(ActionChoices) {
				ActionChoices action = ActionChoices.GoLeft;
				return action;
			}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable" : false,
			"inputs" : [],
			"name" : "ret",
			"outputs" : [
				{
					"name" : "",
					"type" : "uint8"
				}
			],
			"type" : "function"
		},
		{
			"inputs": [
				{
					"name": "param",
					"type": "uint8"
				}
			],
			"payable": false,
			"type": "constructor"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(strings_and_arrays)
{
	// bug #1801
	char const* sourceCode = R"(
		contract test {
			function f(string a, bytes b, uint[] c) external {}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable" : false,
			"name": "f",
			"inputs": [
				{ "name": "a", "type": "string" },
				{ "name": "b", "type": "bytes" },
				{ "name": "c", "type": "uint256[]" }
			],
			"outputs": [],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(library_function)
{
	char const* sourceCode = R"(
		library test {
			struct StructType { uint a; }
			function f(StructType storage b, uint[] storage c, test d) returns (uint[] e, StructType storage f) {}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable" : false,
			"name": "f",
			"inputs": [
				{ "name": "b", "type": "test.StructType storage" },
				{ "name": "c", "type": "uint256[] storage" },
				{ "name": "d", "type": "test" }
			],
			"outputs": [
				{ "name": "e", "type": "uint256[]" },
				{ "name": "f", "type": "test.StructType storage" }
			],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(include_fallback_function)
{
	char const* sourceCode = R"(
		contract test {
			function() {}
		}
	)";

	char const* interface = R"(
	[
		{
			"payable": false,
			"type" : "fallback"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(payable_function)
{
	char const* sourceCode = R"(
		contract test {
			function f() {}
			function g() payable {}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable": false,
			"inputs": [],
			"name": "f",
			"outputs": [],
			"type" : "function"
		},
		{
			"constant" : false,
			"payable": true,
			"inputs": [],
			"name": "g",
			"outputs": [],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(payable_fallback_function)
{
	char const* sourceCode = R"(
		contract test {
			function () payable {}
		}
	)";

	char const* interface = R"(
	[
		{
			"payable": true,
			"type" : "fallback"
		}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_CASE(function_type)
{
	char const* sourceCode = R"(
		contract test {
			function g(function(uint) external returns (uint) x) {}
		}
	)";

	char const* interface = R"(
	[
	{
		"constant" : false,
		"payable": false,
		"inputs": [{
			"name": "x",
			"type": "function"
		}],
		"name": "g",
		"outputs": [],
		"type" : "function"
	}
	]
	)";
	checkInterface(sourceCode, interface);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
