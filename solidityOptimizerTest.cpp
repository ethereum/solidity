
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
 * @date 2014
 * Tests for the Solidity optimizer.
 */

#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <test/solidityExecutionFramework.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

class OptimizerTestFramework: public ExecutionFramework
{
public:
	OptimizerTestFramework() { }
	/// Compiles the source code with and without optimizing.
	void compileBothVersions(unsigned _expectedSizeDecrease, std::string const& _sourceCode, u256 const& _value = 0, std::string const& _contractName = "") {
		m_optimize = false;
		bytes nonOptimizedBytecode = compileAndRun(_sourceCode, _value, _contractName);
		m_nonOptimizedContract = m_contractAddress;
		m_optimize = true;
		bytes optimizedBytecode = compileAndRun(_sourceCode, _value, _contractName);
		int sizeDiff = nonOptimizedBytecode.size() - optimizedBytecode.size();
		BOOST_CHECK_MESSAGE(sizeDiff >= _expectedSizeDecrease, "Bytecode did only shrink by "
							+ boost::lexical_cast<string>(sizeDiff) + " bytes, expected: "
							+ boost::lexical_cast<string>(_expectedSizeDecrease));
		m_optimizedContract = m_contractAddress;
	}

	template <class... Args>
	void compareVersions(byte _index, Args const&... _arguments)
	{
		m_contractAddress = m_nonOptimizedContract;
		bytes nonOptimizedOutput = callContractFunction(_index, _arguments...);
		m_contractAddress = m_optimizedContract;
		bytes optimizedOutput = callContractFunction(_index, _arguments...);
		BOOST_CHECK_MESSAGE(nonOptimizedOutput == optimizedOutput, "Computed values do not match."
							"\nNon-Optimized: " + toHex(nonOptimizedOutput) +
							"\nOptimized:     " + toHex(optimizedOutput));
	}

protected:
	Address m_optimizedContract;
	Address m_nonOptimizedContract;
};

BOOST_FIXTURE_TEST_SUITE(SolidityOptimizerTest, OptimizerTestFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns (uint b) {
				return a;
			}
		})";
	compileBothVersions(4, sourceCode);
	compareVersions(0, u256(7));
}

BOOST_AUTO_TEST_CASE(large_integers)
{
	char const* sourceCode = R"(
		contract test {
			function f() returns (uint a, uint b) {
				a = 0x234234872642837426347000000;
				b = 0x110000000000000000000000002;
			}
		})";
	compileBothVersions(28, sourceCode);
	compareVersions(0);
}

BOOST_AUTO_TEST_CASE(invariants)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns (uint b) {
				return (((a + (1 - 1)) ^ 0) | 0) & (uint(0) - 1);
			}
		})";
	compileBothVersions(19, sourceCode);
	compareVersions(0, u256(0x12334664));
}

BOOST_AUTO_TEST_CASE(unused_expressions)
{
	char const* sourceCode = R"(
		contract test {
			uint data;
			function f() returns (uint a, uint b) {
				10 + 20;
				data;
			}
		})";
	compileBothVersions(11, sourceCode);
	compareVersions(0);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
