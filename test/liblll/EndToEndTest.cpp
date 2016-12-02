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
 * @author Alex Beregszaszi
 * @date 2016
 * End to end tests for LLL.
 */

#include <string>
#include <memory>
#include <boost/test/unit_test.hpp>
#include <test/liblll/ExecutionFramework.h>

using namespace std;

namespace dev
{
namespace lll
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(LLLEndToEndTest, LLLExecutionFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
        char const* sourceCode = "(returnlll { (return \"test\") })";
        compileAndRun(sourceCode);
        BOOST_CHECK(callFallback() == encodeArgs(string("test", 4)));
}

BOOST_AUTO_TEST_CASE(bare_panic)
{
	char const* sourceCode = "(panic)";
	compileAndRunWithoutCheck(sourceCode);
	BOOST_REQUIRE(m_output.empty());
}

BOOST_AUTO_TEST_CASE(exp_operator_const)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp 2 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const_signed)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp (- 0 2) 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(-8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_on_range)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xb3de648b)
					(return (exp 2 (calldataload 0x04))))
				(jump 0x02)))
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const& a) -> u256 { return u256(1 << a.convert_to<int>()); }, 0, 16);
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_numeric)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 65535)
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(65535)));
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_string)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 "test")
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs("test"));
}

BOOST_AUTO_TEST_CASE(constructor_arguments_external)
{
	char const* sourceCode = R"(
		(seq
			(codecopy 0x00 (bytecodesize) 64)
			(sstore 0x00 @0x00)
			(sstore 0x01 @0x20)
			(returnlll
				(seq
					(when (= (div (calldataload 0x00) (exp 2 224)) 0xf2c9ecd8)
						(return @@0x00))
					(when (= (div (calldataload 0x00) (exp 2 224)) 0x89ea642f)
						(return @@0x01))
					(jump 0x02))))
	)";
	compileAndRun(sourceCode, 0, "", encodeArgs(u256(65535), "test"));
	BOOST_CHECK(callContractFunction("getNumber()") == encodeArgs(u256(65535)));
	BOOST_CHECK(callContractFunction("getString()") == encodeArgs("test"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
