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
 * Tests that check that the cost of certain operations stay within range.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>

#include <cmath>

using namespace std;
using namespace langutil;
using namespace dev::eth;
using namespace dev::solidity;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

#define CHECK_GAS(_gasNoOpt, _gasOpt, _tolerance) \
	do \
	{ \
		u256 gasOpt{_gasOpt}; \
		u256 gasNoOpt{_gasNoOpt}; \
		u256 tolerance{_tolerance}; \
		u256 gas = m_optimize ? gasOpt : gasNoOpt; \
		u256 diff = gas < m_gasUsed ? m_gasUsed - gas : gas - m_gasUsed; \
		BOOST_CHECK_MESSAGE( \
			diff <= tolerance, \
			"Gas used: " + \
			m_gasUsed.str() + \
			" - expected: " + \
			gas.str() + \
			" (tolerance: " + \
			tolerance.str() + \
			")" \
		); \
	} while(0)

BOOST_FIXTURE_TEST_SUITE(GasCostTests, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(string_storage)
{
	char const* sourceCode = R"(
		contract C {
			function f() pure public {
				require(false, "Not Authorized. This function can only be called by the custodian or owner of this contract");
			}
		}
	)";
	compileAndRun(sourceCode);

	if (Options::get().evmVersion() <= EVMVersion::byzantium())
		CHECK_GAS(134435, 130591, 100);
	else
		CHECK_GAS(127225, 124873, 100);
	if (Options::get().evmVersion() >= EVMVersion::byzantium())
	{
		callContractFunction("f()");
		if (Options::get().evmVersion() == EVMVersion::byzantium())
			CHECK_GAS(21551, 21526, 20);
		else
			CHECK_GAS(21546, 21526, 20);
	}
}

BOOST_AUTO_TEST_CASE(single_callvaluecheck_no_payable)
{
	char const* sourceCode = R"(
		contract C {
			address a;
			function f(address b) public {
				a = b;
			}
			function f1(address b) public pure returns (uint c) {
				return uint(b) + 2;
			}
			function f2(address b) public pure returns (uint) {
				return uint(b) + 8;
			}
			function f3(address, uint c) pure public returns (uint) {
				return c - 5;
			}
		}
	)";
	compileAndRun(sourceCode);

	// With no payable functions we run the callvalue check only once
	if (Options::get().evmVersion() <= EVMVersion::byzantium())
		CHECK_GAS(225293, 163547, 0);
	else
		CHECK_GAS(219243, 157765, 0);
}

// With at least one payable function we run the check for each
BOOST_AUTO_TEST_CASE(single_callvaluecheck_payable)
{
	char const* sourceCode = R"(
		contract C {
			address a;
			function f(address b) public {
				a = b;
			}
			function f1(address b) public pure returns (uint c) {
				return uint(b) + 2;
			}
			function f2(address b) public pure returns (uint) {
				return uint(b) + 8;
			}
			function f3(address, uint c) payable public returns (uint) {
				return c - 5;
			}
		}
	)";
	compileAndRun(sourceCode);

	if (Options::get().evmVersion() <= EVMVersion::byzantium())
		CHECK_GAS(232139, 170387, 0);
	else
		CHECK_GAS(226089, 164541, 0);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
