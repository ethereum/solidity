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

BOOST_AUTO_TEST_CASE(enclosed_panic)
{
	char const* sourceCode = "(seq (panic))";
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

BOOST_AUTO_TEST_CASE(exp_operator_parameter)
{
	char const* sourceCode = R"(
		(seq
			(def 'function (function-hash code-body)
				(when (= (div (calldataload 0x00) (exp 2 224)) function-hash)
					code-body))
			(returnlll
				(seq
					(function 0xb3de648b
						(return (exp 2 (calldataload 0x04))))
					(jump 0x02))))
    )";
	compileAndRun(sourceCode);
    BOOST_CHECK(callContractFunction("f(uint256)", u256(16)) == toBigEndian(u256(65536)));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
