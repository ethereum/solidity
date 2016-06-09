/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
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

namespace dev
{
namespace solidity
{
namespace test
{

class GasMeterTestFramework: public ExecutionFramework
{
public:
	GasMeterTestFramework() { }
	void compile(string const& _sourceCode)
	{
		m_compiler.setSource(_sourceCode);
		ETH_TEST_REQUIRE_NO_THROW(m_compiler.compile(), "Compiling contract failed");

		AssemblyItems const* items = m_compiler.runtimeAssemblyItems("");
		ASTNode const& sourceUnit = m_compiler.ast();
		BOOST_REQUIRE(items != nullptr);
		m_gasCosts = GasEstimator::breakToStatementLevel(
			GasEstimator::structuralEstimation(*items, vector<ASTNode const*>({&sourceUnit})),
			{&sourceUnit}
		);
	}

	void testCreationTimeGas(string const& _sourceCode)
	{
		EVMSchedule schedule;

		compileAndRun(_sourceCode);
		auto state = make_shared<KnownState>();
		PathGasMeter meter(*m_compiler.assemblyItems());
		GasMeter::GasConsumption gas = meter.estimateMax(0, state);
		u256 bytecodeSize(m_compiler.runtimeObject().bytecode.size());
		gas += bytecodeSize * schedule.createDataGas;
		BOOST_REQUIRE(!gas.isInfinite);
		BOOST_CHECK(gas.value == m_gasUsed);
	}

	/// Compares the gas computed by PathGasMeter for the given signature (but unknown arguments)
	/// against the actual gas usage computed by the VM on the given set of argument variants.
	void testRunTimeGas(string const& _sig, vector<bytes> _argumentVariants)
	{
		u256 gasUsed = 0;
		FixedHash<4> hash(dev::sha3(_sig));
		for (bytes const& arguments: _argumentVariants)
		{
			sendMessage(hash.asBytes() + arguments, false, 0);
			gasUsed = max(gasUsed, m_gasUsed);
		}

		GasMeter::GasConsumption gas = GasEstimator::functionalEstimation(
			*m_compiler.runtimeAssemblyItems(),
			_sig
		);
		BOOST_REQUIRE(!gas.isInfinite);
		BOOST_CHECK(gas.value == m_gasUsed);
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
			function f(uint a) returns (uint b) {
				x.length = a;
				for (; a < 200; ++a) {
					x[a] = 9;
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
				auto scannerFromSource = [&](string const&) -> Scanner const& { return m_compiler.scanner(); };
				SourceReferenceFormatter::printSourceLocation(cout, &first->first->location(), scannerFromSource);
				SourceReferenceFormatter::printSourceLocation(cout, &second->first->location(), scannerFromSource);
			}
	}
}

BOOST_AUTO_TEST_CASE(simple_contract)
{
	// Tests a simple "deploy contract" code without constructor. The actual contract is not relevant.
	char const* sourceCode = R"(
		contract test {
			bytes32 public shaValue;
			function f(uint a) {
				shaValue = sha3(a);
			}
		}
	)";
	testCreationTimeGas(sourceCode);
}

BOOST_AUTO_TEST_CASE(store_sha3)
{
	char const* sourceCode = R"(
		contract test {
			bytes32 public shaValue;
			function test(uint a) {
				shaValue = sha3(a);
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
			function test() {
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
			function f(uint x) {
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
			function f(uint x) {
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
			function f(uint x) {
				if (x > 7)
					data2 = g(x**8) + 1;
				else
					data = 1;
			}
			function g(uint x) returns (uint) {
				return data2;
			}
		}
	)";
	testCreationTimeGas(sourceCode);
	testRunTimeGas("f(uint256)", vector<bytes>{encodeArgs(2), encodeArgs(8)});
	testRunTimeGas("g(uint256)", vector<bytes>{encodeArgs(2)});
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
