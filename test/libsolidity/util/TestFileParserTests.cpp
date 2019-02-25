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
 * Unit tests for Solidity's test expectation parser.
 */

#include <functional>
#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <liblangutil/Exceptions.h>
#include <test/ExecutionFramework.h>

#include <test/libsolidity/util/TestFileParser.h>

using namespace std;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

using fmt = ExecutionFramework;
using Mode = FunctionCall::DisplayMode;

vector<FunctionCall> parse(string const& _source)
{
	istringstream stream{_source, ios_base::out};
	TestFileParser parser{stream};
	return parser.parseFunctionCalls();
}

void testFunctionCall(
		FunctionCall const& _call,
		FunctionCall::DisplayMode _mode,
		string _signature = "",
		bool _failure = true,
		bytes _arguments = bytes{},
		bytes _expectations = bytes{},
		u256 _value = 0,
		string _argumentComment = "",
		string _expectationComment = "",
		vector<string> _rawArguments = vector<string>{}
)
{
	BOOST_REQUIRE_EQUAL(_call.expectations.failure, _failure);
	BOOST_REQUIRE_EQUAL(_call.signature, _signature);
	ABI_CHECK(_call.arguments.rawBytes(), _arguments);
	ABI_CHECK(_call.expectations.rawBytes(), _expectations);
	BOOST_REQUIRE_EQUAL(_call.displayMode, _mode);
	BOOST_REQUIRE_EQUAL(_call.value, _value);
	BOOST_REQUIRE_EQUAL(_call.arguments.comment, _argumentComment);
	BOOST_REQUIRE_EQUAL(_call.expectations.comment, _expectationComment);

	if (!_rawArguments.empty())
	{
		BOOST_REQUIRE_EQUAL(_call.arguments.parameters.size(), _rawArguments.size());
		size_t index = 0;
		for (Parameter const& param: _call.arguments.parameters)
		{
			BOOST_REQUIRE_EQUAL(param.rawString, _rawArguments[index]);
			++index;
		}
	}
}

BOOST_AUTO_TEST_SUITE(TestFileParserTest)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* source = R"()";
	BOOST_REQUIRE_EQUAL(parse(source).size(), 0);
}

BOOST_AUTO_TEST_CASE(call_succees)
{
	char const* source = R"(
		// success() ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::SingleLine, "success()", false);
}

BOOST_AUTO_TEST_CASE(non_existent_call_revert_single_line)
{
	char const* source = R"(
		// i_am_not_there() -> FAILURE
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::SingleLine, "i_am_not_there()", true);
}

BOOST_AUTO_TEST_CASE(call_arguments_success)
{
	char const* source = R"(
		// f(uint256): 1
		// ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::MultiLine, "f(uint256)", false, fmt::encodeArgs(u256{1}));
}

BOOST_AUTO_TEST_CASE(call_arguments_comments_success)
{
	char const* source = R"(
		// f(uint256, uint256): 1, 1
		// # Comment on the parameters. #
		// ->
		// # This call should not return a value, but still succeed. #
		// f()
		// # Comment on no parameters. #
		// -> 1
		// # This comment should be parsed. #
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"f(uint256,uint256)",
		false,
		fmt::encodeArgs(1, 1),
		fmt::encodeArgs(),
		0,
		" Comment on the parameters. ",
		" This call should not return a value, but still succeed. "
	);
	testFunctionCall(
		calls.at(1),
		Mode::MultiLine,
		"f()",
		false,
		fmt::encodeArgs(),
		fmt::encodeArgs(1),
		0,
		" Comment on no parameters. ",
		" This comment should be parsed. "
	);
}

BOOST_AUTO_TEST_CASE(simple_single_line_call_comment_success)
{
	char const* source = R"(
		// f(uint256): 1 -> # f(uint256) does not return a value. #
		// f(uint256): 1 -> 1
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);

	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f(uint256)",
		false,
		fmt::encodeArgs(1),
		fmt::encodeArgs(),
		0,
		"",
		" f(uint256) does not return a value. "
	);
	testFunctionCall(calls.at(1), Mode::SingleLine, "f(uint256)", false, fmt::encode(1), fmt::encode(1));
}

BOOST_AUTO_TEST_CASE(multiple_single_line)
{
	char const* source = R"(
		// f(uint256): 1 -> 1
		// g(uint256): 1 ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);

	testFunctionCall(calls.at(0), Mode::SingleLine, "f(uint256)", false, fmt::encodeArgs(1), fmt::encodeArgs(1));
	testFunctionCall(calls.at(1), Mode::SingleLine, "g(uint256)", false, fmt::encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(multiple_single_line_swapped)
{
	char const* source = R"(
		// f(uint256): 1 ->
		// g(uint256): 1 -> 1
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);

	testFunctionCall(calls.at(0), Mode::SingleLine, "f(uint256)", false, fmt::encodeArgs(1));
	testFunctionCall(calls.at(1), Mode::SingleLine, "g(uint256)", false, fmt::encodeArgs(1), fmt::encodeArgs(1));

}

BOOST_AUTO_TEST_CASE(non_existent_call_revert)
{
	char const* source = R"(
		// i_am_not_there()
		// -> FAILURE
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::MultiLine, "i_am_not_there()", true);
}

BOOST_AUTO_TEST_CASE(call_expectations_empty_single_line)
{
	char const* source = R"(
		// _exp_() ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::SingleLine, "_exp_()", false);
}

BOOST_AUTO_TEST_CASE(call_expectations_empty_multiline)
{
	char const* source = R"(
		// _exp_()
		// ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(calls.at(0), Mode::MultiLine, "_exp_()", false);
}

BOOST_AUTO_TEST_CASE(call_comments)
{
	char const* source = R"(
		// f() # Parameter comment # -> 1 # Expectation comment #
		// f() # Parameter comment #
		// -> 1 # Expectation comment #
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f()",
		false,
		fmt::encodeArgs(),
		fmt::encodeArgs(1),
		0,
		" Parameter comment ",
		" Expectation comment "
	);
	testFunctionCall(
		calls.at(1),
		Mode::MultiLine,
		"f()",
		false,
		fmt::encodeArgs(),
		fmt::encodeArgs(1),
		0,
		" Parameter comment ",
		" Expectation comment "
	);
}

BOOST_AUTO_TEST_CASE(call_arguments)
{
	char const* source = R"(
		// f(uint256), 314 ether: 5 # optional ether value #
		// -> 4
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"f(uint256)",
		false,
		fmt::encodeArgs(5),
		fmt::encodeArgs(4),
		314,
		" optional ether value "
	);
}

BOOST_AUTO_TEST_CASE(call_arguments_bool)
{
	char const* source = R"(
		// f(bool): true -> false
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f(bool)",
		false,
		fmt::encodeArgs(true),
		fmt::encodeArgs(false)
	);
}

BOOST_AUTO_TEST_CASE(call_arguments_tuple)
{
	char const* source = R"(
		// f((uint256, bytes32), uint256) ->
		// f((uint8), uint8) ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(calls.at(0), Mode::SingleLine, "f((uint256,bytes32),uint256)", false);
	testFunctionCall(calls.at(1), Mode::SingleLine, "f((uint8),uint8)", false);
}

BOOST_AUTO_TEST_CASE(call_arguments_left_aligned)
{
	char const* source = R"(
		// f(bytes32, bytes32): 0x6161, 0x420000EF -> 1
		// g(bytes32, bytes32): 0x0616, 0x0042EF00 -> 1
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f(bytes32,bytes32)",
		false,
		fmt::encodeArgs(
			u256("0x6161000000000000000000000000000000000000000000000000000000000000"),
			u256("0x420000EF00000000000000000000000000000000000000000000000000000000")
		),
		fmt::encodeArgs(1)
	);
	testFunctionCall(
		calls.at(1),
		Mode::SingleLine,
		"g(bytes32,bytes32)",
		false,
		fmt::encodeArgs(
			u256("0x0616000000000000000000000000000000000000000000000000000000000000"),
			u256("0x0042EF0000000000000000000000000000000000000000000000000000000000")
		),
		fmt::encodeArgs(1)
	);
}

BOOST_AUTO_TEST_CASE(call_arguments_tuple_of_tuples)
{
	char const* source = R"(
		// f(((uint256, bytes32), bytes32), uint256)
		// # f(S memory s, uint256 b) #
		// ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"f(((uint256,bytes32),bytes32),uint256)",
		false,
		fmt::encodeArgs(),
		fmt::encodeArgs(),
		0,
		" f(S memory s, uint256 b) "
	);
}

BOOST_AUTO_TEST_CASE(call_arguments_recursive_tuples)
{
	char const* source = R"(
		// f(((((bytes, bytes, bytes), bytes), bytes), bytes), bytes) ->
		// f(((((bytes, bytes, (bytes)), bytes), bytes), (bytes, bytes)), (bytes, bytes)) ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f(((((bytes,bytes,bytes),bytes),bytes),bytes),bytes)",
		false
	);
	testFunctionCall(
		calls.at(1),
		Mode::SingleLine,
		"f(((((bytes,bytes,(bytes)),bytes),bytes),(bytes,bytes)),(bytes,bytes))",
		false
	);
}

BOOST_AUTO_TEST_CASE(call_arguments_mismatch)
{
	char const* source = R"(
		// f(uint256):
		// 1, 2
		// # This only throws at runtime #
		// -> 1
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"f(uint256)",
		false,
		fmt::encodeArgs(1, 2),
		fmt::encodeArgs(1),
		0,
		" This only throws at runtime "
	);
}

BOOST_AUTO_TEST_CASE(call_multiple_arguments)
{
	char const* source = R"(
		// test(uint256, uint256):
		// 1,
		// 2
		// -> 1,
		// 1
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"test(uint256,uint256)",
		false,
		fmt::encodeArgs(1, 2),
		fmt::encodeArgs(1, 1)
	);
}

BOOST_AUTO_TEST_CASE(call_multiple_arguments_mixed_format)
{
	char const* source = R"(
		// test(uint256, uint256), 314 ether:
		// 1, -2
		// -> -1, 2
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::MultiLine,
		"test(uint256,uint256)",
		false,
		fmt::encodeArgs(1, -2),
		fmt::encodeArgs(-1, 2),
		314
	);
}

BOOST_AUTO_TEST_CASE(call_signature_valid)
{
	char const* source = R"(
		// f(uint256, uint8, string) -> FAILURE
		// f(invalid, xyz, foo) -> FAILURE
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 2);
	testFunctionCall(calls.at(0), Mode::SingleLine, "f(uint256,uint8,string)", true);
	testFunctionCall(calls.at(1), Mode::SingleLine, "f(invalid,xyz,foo)", true);
}

BOOST_AUTO_TEST_CASE(call_raw_arguments)
{
	char const* source = R"(
		// f(): 1, -2, -3 ->
	)";
	auto const calls = parse(source);
	BOOST_REQUIRE_EQUAL(calls.size(), 1);
	testFunctionCall(
		calls.at(0),
		Mode::SingleLine,
		"f()",
		false,
		fmt::encodeArgs(1, -2, -3),
		fmt::encodeArgs(),
		0,
		"",
		"",
		{"1", "-2", "-3"}
	);
}

BOOST_AUTO_TEST_CASE(call_newline_invalid)
{
	char const* source = R"(
		/
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_invalid)
{
	char const* source = R"(
		/ f() ->
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_signature_invalid)
{
	char const* source = R"(
		// f(uint8,) -> FAILURE
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_tuple_invalid)
{
	char const* source = R"(
		// f((uint8,) -> FAILURE
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_tuple_invalid_empty)
{
	char const* source = R"(
		// f(uint8, ()) -> FAILURE
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_tuple_invalid_parantheses)
{
	char const* source = R"(
		// f((uint8,() -> FAILURE
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_expectations_missing)
{
	char const* source = R"(
		// f())";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_value_expectations_missing)
{
	char const* source = R"(
		// f(), 0)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_invalid)
{
	char const* source = R"(
		// f(uint256): abc -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_invalid_decimal)
{
	char const* source = R"(
		// sig(): 0.h3 ->
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_value_invalid)
{
	char const* source = R"(
		// f(uint256), abc : 1 -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_value_invalid_decimal)
{
	char const* source = R"(
		// sig(): 0.1hd ether ->
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_ether_type_invalid)
{
	char const* source = R"(
		// f(uint256), 2 btc : 1 -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_hex_number_invalid)
{
	char const* source = R"(
		// f(bytes32, bytes32): 0x616, 0x042 -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_colon)
{
	char const* source = R"(
		// h256():
		// -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arguments_newline_colon)
{
	char const* source = R"(
		// h256()
		// :
		// -> 1
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_CASE(call_arrow_missing)
{
	char const* source = R"(
		// h256()
	)";
	BOOST_REQUIRE_THROW(parse(source), langutil::Error);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
