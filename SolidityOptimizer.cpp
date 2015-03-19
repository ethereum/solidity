
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
#include <libevmcore/CommonSubexpressionEliminator.h>
#include <libevmcore/Assembly.h>

using namespace std;
using namespace dev::eth;

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
	void compileBothVersions(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = ""
	)
	{
		m_optimize = false;
		bytes nonOptimizedBytecode = compileAndRun(_sourceCode, _value, _contractName);
		m_nonOptimizedContract = m_contractAddress;
		m_optimize = true;
		bytes optimizedBytecode = compileAndRun(_sourceCode, _value, _contractName);
		BOOST_CHECK_MESSAGE(
			nonOptimizedBytecode.size() > optimizedBytecode.size(),
			"Optimizer did not reduce bytecode size."
		);
		m_optimizedContract = m_contractAddress;
	}

	template <class... Args>
	void compareVersions(std::string _sig, Args const&... _arguments)
	{
		m_contractAddress = m_nonOptimizedContract;
		bytes nonOptimizedOutput = callContractFunction(_sig, _arguments...);
		m_contractAddress = m_optimizedContract;
		bytes optimizedOutput = callContractFunction(_sig, _arguments...);
		BOOST_CHECK_MESSAGE(nonOptimizedOutput == optimizedOutput, "Computed values do not match."
							"\nNon-Optimized: " + toHex(nonOptimizedOutput) +
							"\nOptimized:     " + toHex(optimizedOutput));
	}

protected:
	Address m_optimizedContract;
	Address m_nonOptimizedContract;
};

BOOST_FIXTURE_TEST_SUITE(SolidityOptimizer, OptimizerTestFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) returns (uint b) {
				return a;
			}
		})";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", u256(7));
}

BOOST_AUTO_TEST_CASE(identities)
{
	char const* sourceCode = R"(
		contract test {
			function f(int a) returns (int b) {
				return int(0) | (int(1) * (int(0) ^ (0 + a)));
			}
		})";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", u256(0x12334664));
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
	compileBothVersions(sourceCode);
	compareVersions("f()");
}

BOOST_AUTO_TEST_CASE(constant_folding_both_sides)
{
	// if constants involving the same associative and commutative operator are applied from both
	// sides, the operator should be applied only once, because the expression compiler pushes
	// literals as late as possible
	char const* sourceCode = R"(
		contract test {
			function f(uint x) returns (uint y) {
				return 98 ^ (7 * ((1 | (x | 1000)) * 40) ^ 102);
			}
		})";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)");
}

BOOST_AUTO_TEST_CASE(storage_access)
{
	char const* sourceCode = R"(
		contract test {
			uint8[40] data;
			function f(uint x) returns (uint y) {
				data[2] = data[7] = uint8(x);
				data[4] = data[2] * 10 + data[3];
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)");
}

BOOST_AUTO_TEST_CASE(array_copy)
{
	char const* sourceCode = R"(
		contract test {
			bytes2[] data1;
			bytes5[] data2;
			function f(uint x) returns (uint l, uint y) {
				for (uint i = 0; i < msg.data.length; ++i)
					data1[i] = msg.data[i];
				data2 = data1;
				l = data2.length;
				y = uint(data2[x]);
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", 0);
	compareVersions("f(uint256)", 10);
	compareVersions("f(uint256)", 36);
}

BOOST_AUTO_TEST_CASE(function_calls)
{
	char const* sourceCode = R"(
		contract test {
			function f1(uint x) returns (uint) { return x*x; }
			function f(uint x) returns (uint) { return f1(7+x) - this.f1(x**9); }
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", 0);
	compareVersions("f(uint256)", 10);
	compareVersions("f(uint256)", 36);
}

BOOST_AUTO_TEST_CASE(cse_intermediate_swap)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{
		Instruction::SWAP1, Instruction::POP, Instruction::ADD, u256(0), Instruction::SWAP1,
		Instruction::SLOAD, Instruction::SWAP1, u256(100), Instruction::EXP, Instruction::SWAP1,
		Instruction::DIV, u256(0xff), Instruction::AND
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK(!output.empty());
}

BOOST_AUTO_TEST_CASE(cse_negative_stack_access)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{AssemblyItem(Instruction::DUP2), AssemblyItem(u256(0))};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(cse_negative_stack_end)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{
		AssemblyItem(Instruction::ADD)
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(cse_intermediate_negative_stack)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{
		AssemblyItem(Instruction::ADD),
		AssemblyItem(u256(1)),
		AssemblyItem(Instruction::DUP2)
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(cse_pop)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{
		AssemblyItem(Instruction::POP)
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(cse_unneeded_items)
{
	eth::CommonSubexpressionEliminator cse;
	AssemblyItems input{
		AssemblyItem(Instruction::ADD),
		AssemblyItem(Instruction::SWAP1),
		AssemblyItem(Instruction::POP),
		AssemblyItem(u256(7)),
		AssemblyItem(u256(8)),
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
