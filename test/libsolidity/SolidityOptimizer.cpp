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
 * @date 2014
 * Tests for the Solidity optimizer.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>

#include <libevmasm/Instruction.h>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <chrono>
#include <string>
#include <tuple>
#include <memory>

using namespace std;
using namespace dev::eth;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

class OptimizerTestFramework: public SolidityExecutionFramework
{
public:
	OptimizerTestFramework() { }

	bytes const& compileAndRunWithOptimizer(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bool const _optimize = true,
		unsigned const _optimizeRuns = 200
	)
	{
		bool const c_optimize = m_optimize;
		unsigned const c_optimizeRuns = m_optimizeRuns;
		m_optimize = _optimize;
		m_optimizeRuns = _optimizeRuns;
		bytes const& ret = compileAndRun(_sourceCode, _value, _contractName);
		m_optimize = c_optimize;
		m_optimizeRuns = c_optimizeRuns;
		return ret;
	}

	/// Compiles the source code with and without optimizing.
	void compileBothVersions(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		unsigned const _optimizeRuns = 200
	)
	{
		m_nonOptimizedBytecode = compileAndRunWithOptimizer(_sourceCode, _value, _contractName, false, _optimizeRuns);
		m_nonOptimizedContract = m_contractAddress;
		m_optimizedBytecode = compileAndRunWithOptimizer(_sourceCode, _value, _contractName, true, _optimizeRuns);
		size_t nonOptimizedSize = numInstructions(m_nonOptimizedBytecode);
		size_t optimizedSize = numInstructions(m_optimizedBytecode);
		BOOST_CHECK_MESSAGE(
			_optimizeRuns < 50 || optimizedSize < nonOptimizedSize,
			string("Optimizer did not reduce bytecode size. Non-optimized size: ") +
			std::to_string(nonOptimizedSize) + " - optimized size: " +
			std::to_string(optimizedSize)
		);
		m_optimizedContract = m_contractAddress;
	}

	template <class... Args>
	void compareVersions(std::string _sig, Args const&... _arguments)
	{
		m_contractAddress = m_nonOptimizedContract;
		bytes nonOptimizedOutput = callContractFunction(_sig, _arguments...);
		m_gasUsedNonOptimized = m_gasUsed;
		m_contractAddress = m_optimizedContract;
		bytes optimizedOutput = callContractFunction(_sig, _arguments...);
		m_gasUsedOptimized = m_gasUsed;
		BOOST_CHECK_MESSAGE(!optimizedOutput.empty(), "No optimized output for " + _sig);
		BOOST_CHECK_MESSAGE(!nonOptimizedOutput.empty(), "No un-optimized output for " + _sig);
		BOOST_CHECK_MESSAGE(nonOptimizedOutput == optimizedOutput, "Computed values do not match."
							"\nNon-Optimized: " + toHex(nonOptimizedOutput) +
							"\nOptimized:     " + toHex(optimizedOutput));
	}

	/// @returns the number of intructions in the given bytecode, not taking the metadata hash
	/// into account.
	size_t numInstructions(bytes const& _bytecode, boost::optional<Instruction> _which = boost::optional<Instruction>{})
	{
		BOOST_REQUIRE(_bytecode.size() > 5);
		size_t metadataSize = (_bytecode[_bytecode.size() - 2] << 8) + _bytecode[_bytecode.size() - 1];
		BOOST_REQUIRE_MESSAGE(metadataSize == 0x29, "Invalid metadata size");
		BOOST_REQUIRE(_bytecode.size() >= metadataSize + 2);
		bytes realCode = bytes(_bytecode.begin(), _bytecode.end() - metadataSize - 2);
		size_t instructions = 0;
		solidity::eachInstruction(realCode, [&](Instruction _instr, u256 const&) {
			if (!_which || *_which == _instr)
				instructions++;
		});
		return instructions;
	}

protected:
	u256 m_gasUsedOptimized;
	u256 m_gasUsedNonOptimized;
	bytes m_nonOptimizedBytecode;
	bytes m_optimizedBytecode;
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
		}
	)";
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
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(int256)", u256(0x12334664));
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
		}
	)";
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
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", 7);
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
	compareVersions("f(uint256)", 7);
}

BOOST_AUTO_TEST_CASE(array_copy)
{
	char const* sourceCode = R"(
		contract test {
			bytes2[] data1;
			bytes5[] data2;
			function f(uint x) returns (uint l, uint y) {
				data1.length = msg.data.length;
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
	compareVersions("f(uint256)", 35);
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

BOOST_AUTO_TEST_CASE(storage_write_in_loops)
{
	char const* sourceCode = R"(
		contract test {
			uint d;
			function f(uint a) returns (uint r) {
				var x = d;
				for (uint i = 1; i < a * a; i++) {
					r = d;
					d = i;
				}

			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256)", 0);
	compareVersions("f(uint256)", 10);
	compareVersions("f(uint256)", 36);
}

// Test disabled with https://github.com/ethereum/solidity/pull/762
// Information in joining branches is not retained anymore.
BOOST_AUTO_TEST_CASE(retain_information_in_branches)
{
	// This tests that the optimizer knows that we already have "z == keccak256(y)" inside both branches.
	char const* sourceCode = R"(
		contract c {
			bytes32 d;
			uint a;
			function f(uint x, bytes32 y) returns (uint r_a, bytes32 r_d) {
				bytes32 z = keccak256(y);
				if (x > 8) {
					z = keccak256(y);
					a = x;
				} else {
					z = keccak256(y);
					a = x;
				}
				r_a = a;
				r_d = d;
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256,bytes32)", 0, "abc");
	compareVersions("f(uint256,bytes32)", 8, "def");
	compareVersions("f(uint256,bytes32)", 10, "ghi");

	bytes optimizedBytecode = compileAndRunWithOptimizer(sourceCode, 0, "c", true);
	size_t numSHA3s = 0;
	eachInstruction(optimizedBytecode, [&](Instruction _instr, u256 const&) {
		if (_instr == Instruction::KECCAK256)
			numSHA3s++;
	});
// TEST DISABLED - OPTIMIZER IS NOT EFFECTIVE ON THIS ONE ANYMORE
//	BOOST_CHECK_EQUAL(1, numSHA3s);
}

BOOST_AUTO_TEST_CASE(store_tags_as_unions)
{
	// This calls the same function from two sources and both calls have a certain Keccak-256 on
	// the stack at the same position.
	// Without storing tags as unions, the return from the shared function would not know where to
	// jump and thus all jumpdests are forced to clear their state and we do not know about the
	// sha3 anymore.
	// Note that, for now, this only works if the functions have the same number of return
	// parameters since otherwise, the return jump addresses are at different stack positions
	// which triggers the "unknown jump target" situation.
	char const* sourceCode = R"(
		contract test {
			bytes32 data;
			function f(uint x, bytes32 y) external returns (uint r_a, bytes32 r_d) {
				r_d = keccak256(y);
				shared(y);
				r_d = keccak256(y);
				r_a = 5;
			}
			function g(uint x, bytes32 y) external returns (uint r_a, bytes32 r_d) {
				r_d = keccak256(y);
				shared(y);
				r_d = bytes32(uint(keccak256(y)) + 2);
				r_a = 7;
			}
			function shared(bytes32 y) internal {
				data = keccak256(y);
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(uint256,bytes32)", 7, "abc");

	bytes optimizedBytecode = compileAndRunWithOptimizer(sourceCode, 0, "test", true);
	size_t numSHA3s = 0;
	eachInstruction(optimizedBytecode, [&](Instruction _instr, u256 const&) {
		if (_instr == Instruction::KECCAK256)
			numSHA3s++;
	});
// TEST DISABLED UNTIL 93693404 IS IMPLEMENTED
//	BOOST_CHECK_EQUAL(2, numSHA3s);
}

BOOST_AUTO_TEST_CASE(incorrect_storage_access_bug)
{
	// This bug appeared because a Keccak-256 operation with too low sequence number was used,
	// resulting in memory not being rewritten before the Keccak-256. The fix was to
	// take the max of the min sequence numbers when merging the states.
	char const* sourceCode = R"(
		contract C
		{
			mapping(uint => uint) data;
			function f() returns (uint)
			{
				if(data[now] == 0)
					data[uint(-7)] = 5;
				return data[now];
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f()");
}

BOOST_AUTO_TEST_CASE(sequence_number_for_calls)
{
	// This is a test for a bug that was present because we did not increment the sequence
	// number for CALLs - CALLs can read and write from memory (and DELEGATECALLs can do the same
	// to storage), so the sequence number should be incremented.
	char const* sourceCode = R"(
		contract test {
			function f(string a, string b) returns (bool) { return sha256(a) == sha256(b); }
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f(string,string)", 0x40, 0x80, 3, "abc", 3, "def");
}

BOOST_AUTO_TEST_CASE(computing_constants)
{
	char const* sourceCode = R"(
		contract C {
			uint m_a;
			uint m_b;
			uint m_c;
			uint m_d;
			function C() {
				set();
			}
			function set() returns (uint) {
				m_a = 0x77abc0000000000000000000000000000000000000000000000000000000001;
				m_b = 0x817416927846239487123469187231298734162934871263941234127518276;
				g();
				return 1;
			}
			function g() {
				m_b = 0x817416927846239487123469187231298734162934871263941234127518276;
				m_c = 0x817416927846239487123469187231298734162934871263941234127518276;
				h();
			}
			function h() {
				m_d = 0xff05694900000000000000000000000000000000000000000000000000000000;
			}
			function get() returns (uint ra, uint rb, uint rc, uint rd) {
				ra = m_a;
				rb = m_b;
				rc = m_c;
				rd = m_d;
			}
		}
	)";
	compileBothVersions(sourceCode, 0, "C", 1);
	compareVersions("get()");
	compareVersions("set()");
	compareVersions("get()");

	bytes optimizedBytecode = compileAndRunWithOptimizer(sourceCode, 0, "C", true, 1);
	bytes complicatedConstant = toBigEndian(u256("0x817416927846239487123469187231298734162934871263941234127518276"));
	unsigned occurrences = 0;
	for (auto iter = optimizedBytecode.cbegin(); iter < optimizedBytecode.cend(); ++occurrences)
	{
		iter = search(iter, optimizedBytecode.cend(), complicatedConstant.cbegin(), complicatedConstant.cend());
		if (iter < optimizedBytecode.cend())
			++iter;
	}
	BOOST_CHECK_EQUAL(2, occurrences);

	bytes constantWithZeros = toBigEndian(u256("0x77abc0000000000000000000000000000000000000000000000000000000001"));
	BOOST_CHECK(search(
		optimizedBytecode.cbegin(),
		optimizedBytecode.cend(),
		constantWithZeros.cbegin(),
		constantWithZeros.cend()
	) == optimizedBytecode.cend());
}


BOOST_AUTO_TEST_CASE(constant_optimization_early_exit)
{
	// This tests that the constant optimizer does not try to find the best representation
	// indefinitely but instead stops after some number of iterations.
	char const* sourceCode = R"(
	pragma solidity ^0.4.0;

	contract HexEncoding {
		function hexEncodeTest(address addr) returns (bytes32 ret) {
			uint x = uint(addr) / 2**32;

			// Nibble interleave
			x = x & 0x00000000000000000000000000000000ffffffffffffffffffffffffffffffff;
			x = (x | (x * 2**64)) & 0x0000000000000000ffffffffffffffff0000000000000000ffffffffffffffff;
			x = (x | (x * 2**32)) & 0x00000000ffffffff00000000ffffffff00000000ffffffff00000000ffffffff;
			x = (x | (x * 2**16)) & 0x0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff;
			x = (x | (x * 2** 8)) & 0x00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff;
			x = (x | (x * 2** 4)) & 0x0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f;

			// Hex encode
			uint h = (x & 0x0808080808080808080808080808080808080808080808080808080808080808) / 8;
			uint i = (x & 0x0404040404040404040404040404040404040404040404040404040404040404) / 4;
			uint j = (x & 0x0202020202020202020202020202020202020202020202020202020202020202) / 2;
			x = x + (h & (i | j)) * 0x27 + 0x3030303030303030303030303030303030303030303030303030303030303030;

			// Store and load next batch
			assembly {
				mstore(0, x)
			}
			x = uint(addr) * 2**96;

			// Nibble interleave
			x = x & 0x00000000000000000000000000000000ffffffffffffffffffffffffffffffff;
			x = (x | (x * 2**64)) & 0x0000000000000000ffffffffffffffff0000000000000000ffffffffffffffff;
			x = (x | (x * 2**32)) & 0x00000000ffffffff00000000ffffffff00000000ffffffff00000000ffffffff;
			x = (x | (x * 2**16)) & 0x0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff0000ffff;
			x = (x | (x * 2** 8)) & 0x00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff00ff;
			x = (x | (x * 2** 4)) & 0x0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f;

			// Hex encode
			h = (x & 0x0808080808080808080808080808080808080808080808080808080808080808) / 8;
			i = (x & 0x0404040404040404040404040404040404040404040404040404040404040404) / 4;
			j = (x & 0x0202020202020202020202020202020202020202020202020202020202020202) / 2;
			x = x + (h & (i | j)) * 0x27 + 0x3030303030303030303030303030303030303030303030303030303030303030;

			// Store and hash
			assembly {
				mstore(32, x)
				ret := keccak256(0, 40)
			}
		}
	}
	)";
	auto start = std::chrono::steady_clock::now();
	compileBothVersions(sourceCode);
	double duration = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	BOOST_CHECK_MESSAGE(duration < 20, "Compilation of constants took longer than 20 seconds.");
	compareVersions("hexEncodeTest(address)", u256(0x123456789));
}

BOOST_AUTO_TEST_CASE(inconsistency)
{
	// This is a test of a bug in the optimizer.
	char const* sourceCode = R"(
		contract Inconsistency {
			struct Value {
				uint badnum;
				uint number;
			}

			struct Container {
				uint[] valueIndices;
				Value[] values;
			}

			Container[] containers;
			uint[] valueIndices;
			uint INDEX_ZERO = 0;
			uint  debug;

			// Called with params: containerIndex=0, valueIndex=0
			function levelIII(uint containerIndex, uint valueIndex) private {
				Container container = containers[containerIndex];
				Value value = container.values[valueIndex];
				debug = container.valueIndices[value.number];
			}
			function levelII() private {
				for (uint i = 0; i < valueIndices.length; i++) {
					levelIII(INDEX_ZERO, valueIndices[i]);
				}
			}

			function trigger() public returns (uint) {
				containers.length++;
				Container container = containers[0];

				container.values.push(Value({
					badnum: 9000,
					number: 0
				}));

				container.valueIndices.length++;
				valueIndices.length++;

				levelII();
				return debug;
			}

			function DoNotCallButDoNotDelete() public {
				levelII();
				levelIII(1, 2);
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("trigger()");
}

BOOST_AUTO_TEST_CASE(dead_code_elimination_across_assemblies)
{
	// This tests that a runtime-function that is stored in storage in the constructor
	// is not removed as part of dead code elimination.
	char const* sourceCode = R"(
		contract DCE {
			function () internal returns (uint) stored;
			function DCE() {
				stored = f;
			}
			function f() internal returns (uint) { return 7; }
			function test() returns (uint) { return stored(); }
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("test()");
}

BOOST_AUTO_TEST_CASE(invalid_state_at_control_flow_join)
{
	char const* sourceCode = R"(
		contract Test {
			uint256 public totalSupply = 100;
			function f() returns (uint r) {
				if (false)
					r = totalSupply;
				totalSupply -= 10;
			}
			function test() returns (uint) {
				f();
				return this.totalSupply();
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("test()");
}

BOOST_AUTO_TEST_CASE(init_empty_dynamic_arrays)
{
	// This is not so much an optimizer test, but rather a test
	// that allocating empty arrays is implemented efficiently.
	// In particular, initializing a dynamic memory array does
	// not use any memory.
	char const* sourceCode = R"(
		contract Test {
			function f() pure returns (uint r) {
				uint[][] memory x = new uint[][](20000);
				return x.length;
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f()");
	BOOST_CHECK_LE(m_gasUsedNonOptimized, 1900000);
	BOOST_CHECK_LE(1600000, m_gasUsedNonOptimized);
}

BOOST_AUTO_TEST_CASE(optimise_multi_stores)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint16 a; uint16 b; uint16[3] c; uint[] dyn; }
			uint padding;
			S[] s;
			function f() public returns (uint16, uint16, uint16[3], uint) {
				uint16[3] memory c;
				c[0] = 7;
				c[1] = 8;
				c[2] = 9;
				s.push(S(1, 2, c, new uint[](4)));
				return (s[0].a, s[0].b, s[0].c, s[0].dyn[2]);
			}
		}
	)";
	compileBothVersions(sourceCode);
	compareVersions("f()");
	BOOST_CHECK_EQUAL(numInstructions(m_nonOptimizedBytecode, Instruction::SSTORE), 9);
	BOOST_CHECK_EQUAL(numInstructions(m_optimizedBytecode, Instruction::SSTORE), 8);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
