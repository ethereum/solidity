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
}

BOOST_AUTO_TEST_CASE(format_empty_byte_range)
{
	bytes expectedBytes;
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{}, string{}};
	FunctionCall call{"f()", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_EQUAL(test.format(), "// f() -> ");
}

BOOST_AUTO_TEST_CASE(format_parameter_encoding_too_short)
{
	bytes expectedBytes = toBigEndian(u256{1}) + toBigEndian(u256{1});
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param, param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param, param}, string{}};
	FunctionCall call{"f(uint8, uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_THROW(test.format(), runtime_error);
}

BOOST_AUTO_TEST_CASE(format_byte_range_too_short)
{
	bytes expectedBytes{0};
	ABIType abiType{ABIType::UnsignedDec, ABIType::AlignRight, 32};
	Parameter param{expectedBytes, "1", abiType, FormatInfo{}};
	FunctionCallExpectations expectations{vector<Parameter>{param, param}, false, string{}};
	FunctionCallArgs arguments{vector<Parameter>{param, param}, string{}};
	FunctionCall call{"f(uint8, uint8)", 0, arguments, expectations};
	TestFunctionCall test{call};

	BOOST_REQUIRE_THROW(test.format(), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
