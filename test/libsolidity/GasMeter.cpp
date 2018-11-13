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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Unit tests for the gas estimator.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>
#include <libevmasm/GasMeter.h>
#include <libevmasm/KnownState.h>
#include <libevmasm/PathGasMeter.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>

using namespace std;
using namespace dev::eth;
using namespace dev::solidity;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

class GasMeterTestFramework: public SolidityExecutionFramework
{
public:
	GasMeterTestFramework() { }
	void compile(string const& _sourceCode)
	{
		m_compiler.reset(false);
		m_compiler.addSource("", "pragma solidity >=0.0;\n" + _sourceCode);
		m_compiler.setOptimiserSettings(dev::test::Options::get().optimize);
		m_compiler.setEVMVersion(m_evmVersion);
		BOOST_REQUIRE_MESSAGE(m_compiler.compile(), "Compiling contract failed");

		AssemblyItems const* items = m_compiler.runtimeAssemblyItems(m_compiler.lastContractName());
		ASTNode const& sourceUnit = m_compiler.ast("");
		BOOST_REQUIRE(items != nullptr);
		m_gasCosts = GasEstimator::breakToStatementLevel(
			GasEstimator(dev::test::Options::get().evmVersion()).structuralEstimation(*items, vector<ASTNode const*>({&sourceUnit})),
			{&sourceUnit}
		);
	}

	void testCreationTimeGas(string const& _sourceCode)
	{
		compileAndRun(_sourceCode);
		auto state = make_shared<KnownState>();
		PathGasMeter meter(*m_compiler.assemblyItems(m_compiler.lastContractName()), dev::test::Options::get().evmVersion());
		GasMeter::GasConsumption gas = meter.estimateMax(0, state);
		u256 bytecodeSize(m_compiler.runtimeObject(m_compiler.lastContractName()).bytecode.size());
		// costs for deployment
		gas += bytecodeSize * GasCosts::createDataGas;
		// costs for transaction
		gas += gasForTransaction(m_compiler.object(m_compiler.lastContractName()).bytecode, true);

		BOOST_REQUIRE(!gas.isInfinite);
		BOOST_CHECK_EQUAL(gas.value, m_gasUsed);
	}

	/// Compares the gas computed by PathGasMeter for the given signature (but unknown arguments)
	/// against the actual gas usage computed by the VM on the given set of argument variants.
	void testRunTimeGas(string const& _sig, vector<bytes> _argumentVariants)
	{
		u256 gasUsed = 0;
		GasMeter::GasConsumption gas;
		FixedHash<4> hash(dev::keccak256(_sig));
		for (bytes const& arguments: _argumentVariants)
		{
			sendMessage(hash.asBytes() + arguments, false, 0);
			BOOST_CHECK(m_transactionSuccessful);
			gasUsed = max(gasUsed, m_gasUsed);
			gas = max(gas, gasForTransaction(hash.asBytes() + arguments, false));
		}

		gas += GasEstimator(dev::test::Options::get().evmVersion()).functionalEstimation(
			*m_compiler.runtimeAssemblyItems(m_compiler.lastContractName()),
			_sig
		);
		BOOST_REQUIRE(!gas.isInfinite);
		BOOST_CHECK_EQUAL(gas.value, m_gasUsed);
	}

	static GasMeter::GasConsumption gasForTransaction(bytes const& _data, bool _isCreation)
	{
		GasMeter::GasConsumption gas = _isCreation ? GasCosts::txCreateGas : GasCosts::txGas;
		for (auto i: _data)
			gas += i != 0 ? GasCosts::txDataNonZeroGas : GasCosts::txDataZeroGas;
		return gas;
	}

protected:
	map<ASTNode const*, eth::GasMeter::GasConsumption> m_gasCosts;
};

BOOST_FIXTURE_TEST_SUITE(GasMeterTests, GasMeterTestFramework)

BOOST_AUTO_TEST_CASE(non_overlapping_filtered_costs)
{
	char const* sourceCode = R"(
		contract test {
			bytes x;
			function f(uint a) public returns (uint b) {
				x.length = a;
				for (; a < 200; ++a) {
					x[a] = 0x09;
					b = a * a;
				}
				return f(a - 1);
			}
		}
	)";
	compile(sourceCode);
	for (auto first = m_gasCosts.cbegin(); first != m_gasCosts.cend(); ++first)
	{
		auto second = first;
		for (++second; second != m_gasCosts.cend(); ++second)
			if (first->first->location().intersects(second->first->location()))
			{
				BOOST_CHECK_MESSAGE(false, "Source locations should not overlap!");
				auto scannerFromSource = [&](string const& _sourceName) -> Scanner const& { return m_compiler.scanner(_sourceName); };
				SourceReferenceFormatter formatter(cout, scannerFromSource);

				formatter.printSourceLocation(&first->first->location());
				formatter.printSourceLocation(&second->first->location());
			}
	}
}

BOOST_AUTO_TEST_CASE(simple_contract)
{
	// Tests a simple "deploy contract" code without constructor. The actual contract is not relevant.
	char const* sourceCode = R"(
		contract test {
			bytes32 public shaValue;
			function f(uint a) public {
				shaValue = keccak256(abi.encodePacked(a));
			}
		}
	)";
	testCreationTimeGas(sourceCode);
}

BOOST_AUTO_TEST_CASE(store_keccak256)
{
	char const* sourceCode = R"(
		contract test {
			bytes32 public shaValue;
			constructor(uint a) public {
				shaValue = keccak256(abi.encodePacked(a));
			}
		}
	)";
	testCreationTimeGas(sourceCode);
}

BOOST_AUTO_TEST_CASE(updating_store)
{
	char const* sourceCode = R"(
		contract test {
			uint data;
			uint data2;
			constructor() public {
				data = 1;
				data = 2;
				data2 = 0;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
}

BOOST_AUTO_TEST_CASE(branches)
{
	char const* sourceCode = R"(
		contract test {
			uint data;
			uint data2;
			function f(uint x) public {
				if (x > 7)
					data2 = 1;
				else
					data = 1;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("f(uint256)", vector<bytes>{encodeArgs(2), encodeArgs(8)});
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	char const* sourceCode = R"(
		contract test {
			uint data;
			uint data2;
			function f(uint x) public {
				if (x > 7)
					data2 = g(x**8) + 1;
				else
					data = 1;
			}
			function g(uint x) internal returns (uint) {
				return data2;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("f(uint256)", vector<bytes>{encodeArgs(2), encodeArgs(8)});
}

BOOST_AUTO_TEST_CASE(multiple_external_functions)
{
	char const* sourceCode = R"(
		contract test {
			uint data;
			uint data2;
			function f(uint x) public {
				if (x > 7)
					data2 = g(x**8) + 1;
				else
					data = 1;
			}
			function g(uint x) public returns (uint) {
				return data2;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("f(uint256)", vector<bytes>{encodeArgs(2), encodeArgs(8)});
	testRunTimeGas("g(uint256)", vector<bytes>{encodeArgs(2)});
}

BOOST_AUTO_TEST_CASE(exponent_size)
{
	char const* sourceCode = R"(
		contract A {
			function g(uint x) public returns (uint) {
				return x ** 0x100;
			}
			function h(uint x) public returns (uint) {
				return x ** 0x10000;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("g(uint256)", vector<bytes>{encodeArgs(2)});
	testRunTimeGas("h(uint256)", vector<bytes>{encodeArgs(2)});
}

BOOST_AUTO_TEST_CASE(balance_gas)
{
	char const* sourceCode = R"(
		contract A {
			function lookup_balance(address a) public returns (uint) {
				return a.balance;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("lookup_balance(address)", vector<bytes>{encodeArgs(2), encodeArgs(100)});
}

BOOST_AUTO_TEST_CASE(extcodesize_gas)
{
	char const* sourceCode = R"(
		contract A {
			function f() public returns (uint _s) {
				assembly {
					_s := extcodesize(0x30)
				}
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("f()", vector<bytes>{encodeArgs()});
}

BOOST_AUTO_TEST_CASE(regular_functions_exclude_fallback)
{
	// A bug in the estimator caused the costs for a specific function
	// to always include the costs for the fallback.
	char const* sourceCode = R"(
		contract A {
			uint public x;
			function() external { x = 2; }
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("x()", vector<bytes>{encodeArgs()});
}

BOOST_AUTO_TEST_CASE(complex_control_flow)
{
	// This crashed the gas estimator previously (or took a very long time).
	// Now we do not follow branches if they start out with lower gas costs than the ones
	// we previously considered. This of course reduces accuracy.
	char const* sourceCode = R"(
		contract log {
			function ln(int128 x) public pure returns (int128 result) {
				int128 t = x / 256;
				int128 y = 5545177;
				x = t;
				t = x * 16; if (t <= 1000000) { x = t; y = y - 2772588; }
				t = x * 4; if (t <= 1000000) { x = t; y = y - 1386294; }
				t = x * 2; if (t <= 1000000) { x = t; y = y - 693147; }
				t = x + x / 2; if (t <= 1000000) { x = t; y = y - 405465; }
				t = x + x / 4; if (t <= 1000000) { x = t; y = y - 223144; }
				t = x + x / 8; if (t <= 1000000) { x = t; y = y - 117783; }
				t = x + x / 16; if (t <= 1000000) { x = t; y = y - 60624; }
				t = x + x / 32; if (t <= 1000000) { x = t; y = y - 30771; }
				t = x + x / 64; if (t <= 1000000) { x = t; y = y - 15504; }
				t = x + x / 128; if (t <= 1000000) { x = t; y = y - 7782; }
				t = x + x / 256; if (t <= 1000000) { x = t; y = y - 3898; }
				t = x + x / 512; if (t <= 1000000) { x = t; y = y - 1951; }
				t = x + x / 1024; if (t <= 1000000) { x = t; y = y - 976; }
				t = x + x / 2048; if (t <= 1000000) { x = t; y = y - 488; }
				t = x + x / 4096; if (t <= 1000000) { x = t; y = y - 244; }
				t = x + x / 8192; if (t <= 1000000) { x = t; y = y - 122; }
				t = x + x / 16384; if (t <= 1000000) { x = t; y = y - 61; }
				t = x + x / 32768; if (t <= 1000000) { x = t; y = y - 31; }
				t = x + x / 65536; if (t <= 1000000) { y = y - 15; }
				return y;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	// max gas is used for small x
	testRunTimeGas("ln(int128)", vector<bytes>{encodeArgs(0), encodeArgs(10), encodeArgs(105), encodeArgs(30000)});
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
