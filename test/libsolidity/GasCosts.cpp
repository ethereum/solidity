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
// SPDX-License-Identifier: GPL-3.0
/**
 * Tests that check that the cost of certain operations stay within range.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>
#include <liblangutil/EVMVersion.h>
#include <libsolutil/IpfsHash.h>
#include <libevmasm/GasMeter.h>

#include <cmath>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::langutil;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::test;

namespace solidity::frontend::test
{

#define CHECK_DEPLOY_GAS(_gasNoOpt, _gasOpt, _evmVersion) \
	do \
	{ \
		u256 ipfsCost = GasMeter::dataGas(util::ipfsHash(m_compiler.metadata(m_compiler.lastContractName())), true, _evmVersion); \
		u256 gasOpt{_gasOpt}; \
		u256 gasNoOpt{_gasNoOpt}; \
		u256 gas = m_optimiserSettings == OptimiserSettings::minimal() ? gasNoOpt : gasOpt; \
		BOOST_CHECK_MESSAGE( \
			m_gasUsed >= ipfsCost, \
			"Gas used: " + \
			m_gasUsed.str() + \
			" is less than the data cost for the IPFS hash: " + \
			u256(ipfsCost).str() \
		); \
		u256 gasUsed = m_gasUsed - ipfsCost; \
		BOOST_CHECK_MESSAGE( \
			gas == gasUsed, \
			"Gas used: " + \
			gasUsed.str() + \
			" - expected: " + \
			gas.str() \
		); \
	} while(0)

#define CHECK_GAS(_gasNoOpt, _gasOpt, _tolerance) \
	do \
	{ \
		u256 gasOpt{_gasOpt}; \
		u256 gasNoOpt{_gasNoOpt}; \
		u256 tolerance{_tolerance}; \
		u256 gas = m_optimiserSettings == OptimiserSettings::minimal() ? gasNoOpt : gasOpt; \
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
	m_compiler.overwriteReleaseFlag(true);
	compileAndRun(sourceCode);

	auto evmVersion = solidity::test::CommonOptions::get().evmVersion();

	if (evmVersion <= EVMVersion::byzantium())
		CHECK_DEPLOY_GAS(134145, 130831, evmVersion);
	// This is only correct on >=Constantinople.
	else if (CommonOptions::get().useABIEncoderV2)
	{
		if (CommonOptions::get().optimize)
		{
			// Costs with 0 are cases which cannot be triggered in tests.
			if (evmVersion < EVMVersion::istanbul())
				CHECK_DEPLOY_GAS(0, 123969, evmVersion);
			else
				CHECK_DEPLOY_GAS(0, 110969, evmVersion);
		}
		else
		{
			if (evmVersion < EVMVersion::istanbul())
				CHECK_DEPLOY_GAS(147835, 123969, evmVersion);
			else
				CHECK_DEPLOY_GAS(131871, 110969, evmVersion);
		}
	}
	else if (evmVersion < EVMVersion::istanbul())
		CHECK_DEPLOY_GAS(126929, 119659, evmVersion);
	else
		CHECK_DEPLOY_GAS(114345, 107335, evmVersion);

	if (evmVersion >= EVMVersion::byzantium())
	{
		callContractFunction("f()");
		if (evmVersion == EVMVersion::byzantium())
			CHECK_GAS(21545, 21526, 20);
		// This is only correct on >=Constantinople.
		else if (CommonOptions::get().useABIEncoderV2)
		{
			if (CommonOptions::get().optimize)
			{
				if (evmVersion < EVMVersion::istanbul())
					CHECK_GAS(0, 21567, 20);
				else
					CHECK_GAS(0, 21351, 20);
			}
			else
			{
				if (evmVersion < EVMVersion::istanbul())
					CHECK_GAS(21707, 21559, 20);
				else
					CHECK_GAS(21499, 21351, 20);
			}
		}
		else if (evmVersion < EVMVersion::istanbul())
			CHECK_GAS(21546, 21526, 20);
		else
			CHECK_GAS(21332, 21322, 20);
	}
}

BOOST_AUTO_TEST_CASE(single_callvaluecheck)
{
	string sourceCode = R"(
		// All functions nonpayable, we can check callvalue at the beginning
		contract Nonpayable {
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
		// At least on payable function, we cannot do the optimization.
		contract Payable {
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
	size_t bytecodeSizeNonpayable = m_compiler.object("Nonpayable").bytecode.size();
	size_t bytecodeSizePayable = m_compiler.object("Payable").bytecode.size();

	BOOST_CHECK_EQUAL(bytecodeSizePayable - bytecodeSizeNonpayable, 26);
}

BOOST_AUTO_TEST_SUITE_END()

}
