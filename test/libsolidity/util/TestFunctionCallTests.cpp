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

#include <functional>
#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <liblangutil/Exceptions.h>
#include <test/ExecutionFramework.h>

#include <test/libsolidity/util/TestFunctionCall.h>

using namespace std;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(TestFunctionCallTest)

BOOST_AUTO_TEST_CASE(format_unsigned_singleline)
{
	bytes expectedBytes = toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(uint8): 1 -> 1");

	test.setRawBytes(toBigEndian(u256{2}));
	test.setFailure(false);

	BOOST_REQUIRE_EQUAL(test.format("", true), "// f(uint8): 1 -> 2");
}

BOOST_AUTO_TEST_CASE(format_unsigned_singleline_signed_encoding)
{
	bytes expectedBytes = toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(uint8): 1 -> 1");

	test.setRawBytes(toBigEndian(u256{-1}));
	test.setFailure(false);

	BOOST_REQUIRE_EQUAL(test.format("", true), "// f(uint8): 1 -> -1");
}

BOOST_AUTO_TEST_CASE(format_unsigned_multiline)
{
	bytes expectedBytes = toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter result{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{result}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{}, string{}};
	FunctionCall call{"f(uint8)", 0, arguments, expectations};
	call.displayMode = FunctionCall::DisplayMode::MultiLine;
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(),	"// f(uint8)\n// -> 1");
}

BOOST_AUTO_TEST_CASE(format_multiple_unsigned_singleline)
{
	bytes expectedBytes = toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param, param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param, param}, string{}};
	FunctionCall call{"f(uint8, uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(uint8, uint8): 1, 1 -> 1, 1");
}

BOOST_AUTO_TEST_CASE(format_signed_singleline)
{
	bytes expectedBytes = toBigEndian(u256{-1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "-1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(int8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(int8): -1 -> -1");

	test.setRawBytes(toBigEndian(u256{-2}));
	test.setFailure(false);

	BOOST_REQUIRE_EQUAL(test.format("", true), "// f(int8): -1 -> -2");
}

BOOST_AUTO_TEST_CASE(format_hex_singleline)
{
	bytes result = fromHex("0x31");
	bytes expectedBytes = result + bytes(32 - result.size(), 0);
	ABIType abiType{ABIType::Hex, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "0x31", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(bytes32)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(bytes32): 0x31 -> 0x31");

	bytes actualResult = fromHex("0x32");
	bytes actualBytes = actualResult + bytes(32 - actualResult.size(), 0);
	test.setRawBytes(actualBytes);
	test.setFailure(false);

	BOOST_REQUIRE_EQUAL(test.format("", true), "// f(bytes32): 0x31 -> 0x32");
}

BOOST_AUTO_TEST_CASE(format_hex_string_singleline)
{
	bytes expectedBytes = fromHex("4200ef");
	ABIType abiType{ABIType::HexString, ABIType::AlignLeft, 3};
	Parameter param{expectedBytes, "hex\"4200ef\"", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(string)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(string): hex\"4200ef\" -> hex\"4200ef\"");
}

BOOST_AUTO_TEST_CASE(format_bool_true_singleline)
{
	bytes expectedBytes = toBigEndian(u256{true});
	ABIType abiType{ABIType::Boolean, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "true", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(bool)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(bool): true -> true");

	bytes actualResult = bytes{false};
	bytes actualBytes = actualResult + bytes(32 - actualResult.size(), 0);
	test.setRawBytes(actualBytes);
	test.setFailure(false);

	BOOST_REQUIRE_EQUAL(test.format("", true), "// f(bool): true -> false");
}

BOOST_AUTO_TEST_CASE(format_bool_false_singleline)
{
	bytes expectedBytes = toBigEndian(u256{false});
	ABIType abiType{ABIType::Boolean, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "false", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(bool)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(bool): false -> false");
}

BOOST_AUTO_TEST_CASE(format_bool_left_singleline)
{
	bytes expectedBytes = toBigEndian(u256{false});
	ABIType abiType{ABIType::Boolean, ABIType::AlignLeft, 32};
	Parameter param{expectedBytes, "left(false)", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(bool)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(bool): left(false) -> left(false)");
}

BOOST_AUTO_TEST_CASE(format_hex_number_right_singleline)
{
	bytes result = fromHex("0x42");
	bytes expectedBytes = result + bytes(32 - result.size(), 0);
	ABIType abiType{ABIType::Hex, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "right(0x42)", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(bool)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(bool): right(0x42) -> right(0x42)");
}

BOOST_AUTO_TEST_CASE(format_empty_byte_range)
{
	bytes expectedBytes;
	ABIType abiType{ABIType::None, ABIType::AlignNone, 0};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{}, string{}};
	FunctionCall call{"f()", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f() -> 1");
}

BOOST_AUTO_TEST_CASE(format_failure_singleline)
{
	bytes expectedBytes = toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{}, true, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param}, string{}};
	FunctionCall call{"f(uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f(uint8): 1 -> FAILURE");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
