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
 * Unit tests for the iulia interpreter.
 */

#include <test/libjulia/Common.h>

#include <libjulia/interpreter/Interpreter.h>

#include <boost/test/unit_test.hpp>

#include <tuple>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;


namespace
{
pair<bool, string> check(string const& _source, vector<string> const& _expectation)
{
	Block b = *parse(_source, false).first;
	InterpreterState state;
	Interpreter interpreter(state);
	try
	{
		interpreter(b);
	}
	catch (InterpreterTerminated const&)
	{
	}
	if (state.trace == _expectation)
		return std::make_pair(true, std::string{});
	std::string message =
			"Invalid trace\n"
			"   Result                                                           Expectation\n";
	for (size_t i = 0; i < std::max(_expectation.size(), state.trace.size()); ++i)
	{
		std::string result{i >= state.trace.size() ? string{"STOPPED"} : state.trace.at(i)};
		std::string expected{i >= _expectation.size() ? string{"STOPPED"} : _expectation.at(i)};
		message +=
			(result == expected ? "RES   " : "RES X ") +
			result +
			"\n" +
			"EXP   " +
			expected +
			"\n";
	}
	message += "\n\n{\n";
	for (size_t i = 0; i < state.trace.size(); ++i)
	{
		message += "\"" + state.trace[i] + "\"";
		if (i + 1 < state.trace.size())
			message += ",";
		message += "\n";
	}
	message += "}\n";
	return make_pair(false, message);
}
}

#define CHECK(_source, _expectation)\
do\
{\
	auto result = check(_source, _expectation);\
	BOOST_CHECK_MESSAGE(result.first, result.second);\
}\
while(false)

BOOST_AUTO_TEST_SUITE(IuliaInterpreter)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", {});
}

BOOST_AUTO_TEST_CASE(simple_mstore)
{
	CHECK("{ mstore(10, 11) }", {
		"MSTORE_AT_SIZE(10, 32) [000000000000000000000000000000000000000000000000000000000000000b]"
	});
}

BOOST_AUTO_TEST_CASE(loop)
{
	CHECK("{ for { let x := 2 } lt(x, 10) { x := add(x, 1) } { mstore(x, mul(x, 0x1000)) } }", (strings{
		"MSTORE_AT_SIZE(2, 32) [0000000000000000000000000000000000000000000000000000000000002000]",
		"MSTORE_AT_SIZE(3, 32) [0000000000000000000000000000000000000000000000000000000000003000]",
		"MSTORE_AT_SIZE(4, 32) [0000000000000000000000000000000000000000000000000000000000004000]",
		"MSTORE_AT_SIZE(5, 32) [0000000000000000000000000000000000000000000000000000000000005000]",
		"MSTORE_AT_SIZE(6, 32) [0000000000000000000000000000000000000000000000000000000000006000]",
		"MSTORE_AT_SIZE(7, 32) [0000000000000000000000000000000000000000000000000000000000007000]",
		"MSTORE_AT_SIZE(8, 32) [0000000000000000000000000000000000000000000000000000000000008000]",
		"MSTORE_AT_SIZE(9, 32) [0000000000000000000000000000000000000000000000000000000000009000]"
	}));
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	CHECK("{ function f(a, b) -> x, y { x := add(a, b) y := mul(a, b) } let r, t := f(6, 7) sstore(r, t) }", {
		"SSTORE(13, 42)"
	});
}

BOOST_AUTO_TEST_CASE(switch_statement)
{
	CHECK("{ switch 7 case 7 { mstore(1, 2) } case 3 { mstore(6, 7) } default { mstore(8, 9) } }", {
		"MSTORE_AT_SIZE(1, 32) [0000000000000000000000000000000000000000000000000000000000000002]"
	});
}

BOOST_AUTO_TEST_CASE(external_call)
{
	CHECK("{ let x := call(gas(), 0x45, 0x5, 0, 0x20, 0x30, 0x20) sstore(0, x) }", (strings{
		"GAS()",
		"MLOAD_FROM_SIZE(0, 32)",
		"MSTORE_AT_SIZE(48, 32)",
		"CALL(153, 69, 5, 0, 32, 48, 32)",
		"SSTORE(0, 1)"
	}));
}

BOOST_AUTO_TEST_SUITE_END()
