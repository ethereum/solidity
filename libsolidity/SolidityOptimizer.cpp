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
#include <memory>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <test/libsolidity/solidityExecutionFramework.h>
#include <libevmcore/CommonSubexpressionEliminator.h>
#include <libevmcore/ControlFlowGraph.h>
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
		size_t nonOptimizedSize = 0;
		eth::eachInstruction(nonOptimizedBytecode, [&](Instruction, u256 const&) {
			nonOptimizedSize++;
		});
		size_t optimizedSize = 0;
		eth::eachInstruction(optimizedBytecode, [&](Instruction, u256 const&) {
			optimizedSize++;
		});
		BOOST_CHECK_MESSAGE(
			nonOptimizedSize > optimizedSize,
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

	AssemblyItems getCSE(AssemblyItems const& _input)
	{
		// add dummy locations to each item so that we can check that they are not deleted
		AssemblyItems input = _input;
		for (AssemblyItem& item: input)
			item.setLocation(SourceLocation(1, 3, make_shared<string>("")));

		eth::CommonSubexpressionEliminator cse;
		BOOST_REQUIRE(cse.feedItems(input.begin(), input.end()) == input.end());
		AssemblyItems output = cse.getOptimizedItems();

		for (AssemblyItem const& item: output)
		{
			BOOST_CHECK(item == Instruction::POP || !item.getLocation().isEmpty());
		}
		return output;
	}

	void checkCSE(AssemblyItems const& _input, AssemblyItems const& _expectation)
	{
		AssemblyItems output = getCSE(_input);
		BOOST_CHECK_EQUAL_COLLECTIONS(_expectation.begin(), _expectation.end(), output.begin(), output.end());
	}

	void checkCFG(AssemblyItems const& _input, AssemblyItems const& _expectation)
	{
		AssemblyItems output = _input;
		// Running it four times should be enough for these tests.
		for (unsigned i = 0; i < 4; ++i)
		{
			eth::ControlFlowGraph cfg(output);
			output = cfg.optimisedItems();
		}
		BOOST_CHECK_EQUAL_COLLECTIONS(_expectation.begin(), _expectation.end(), output.begin(), output.end());
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
	AssemblyItems input{Instruction::DUP2, u256(0)};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_negative_stack_end)
{
	AssemblyItems input{Instruction::ADD};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_intermediate_negative_stack)
{
	AssemblyItems input{Instruction::ADD, u256(1), Instruction::DUP1};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_pop)
{
	checkCSE({Instruction::POP}, {Instruction::POP});
}

BOOST_AUTO_TEST_CASE(cse_unneeded_items)
{
	AssemblyItems input{
		Instruction::ADD,
		Instruction::SWAP1,
		Instruction::POP,
		u256(7),
		u256(8),
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_constant_addition)
{
	AssemblyItems input{u256(7), u256(8), Instruction::ADD};
	checkCSE(input, {u256(7 + 8)});
}

BOOST_AUTO_TEST_CASE(cse_invariants)
{
	AssemblyItems input{
		Instruction::DUP1,
		Instruction::DUP1,
		u256(0),
		Instruction::OR,
		Instruction::OR
	};
	checkCSE(input, {Instruction::DUP1});
}

BOOST_AUTO_TEST_CASE(cse_subself)
{
	checkCSE({Instruction::DUP1, Instruction::SUB}, {Instruction::POP, u256(0)});
}

BOOST_AUTO_TEST_CASE(cse_subother)
{
	checkCSE({Instruction::SUB}, {Instruction::SUB});
}

BOOST_AUTO_TEST_CASE(cse_double_negation)
{
	checkCSE({Instruction::DUP5, Instruction::NOT, Instruction::NOT}, {Instruction::DUP5});
}

BOOST_AUTO_TEST_CASE(cse_associativity)
{
	AssemblyItems input{
		Instruction::DUP1,
		Instruction::DUP1,
		u256(0),
		Instruction::OR,
		Instruction::OR
	};
	checkCSE(input, {Instruction::DUP1});
}

BOOST_AUTO_TEST_CASE(cse_associativity2)
{
	AssemblyItems input{
		u256(0),
		Instruction::DUP2,
		u256(2),
		u256(1),
		Instruction::DUP6,
		Instruction::ADD,
		u256(2),
		Instruction::ADD,
		Instruction::ADD,
		Instruction::ADD,
		Instruction::ADD
	};
	checkCSE(input, {Instruction::DUP2, Instruction::DUP2, Instruction::ADD, u256(5), Instruction::ADD});
}

BOOST_AUTO_TEST_CASE(cse_storage)
{
	AssemblyItems input{
		u256(0),
		Instruction::SLOAD,
		u256(0),
		Instruction::SLOAD,
		Instruction::ADD,
		u256(0),
		Instruction::SSTORE
	};
	checkCSE(input, {
		u256(0),
		Instruction::DUP1,
		Instruction::SLOAD,
		Instruction::DUP1,
		Instruction::ADD,
		Instruction::SWAP1,
		Instruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_noninterleaved_storage)
{
	// two stores to the same location should be replaced by only one store, even if we
	// read in the meantime
	AssemblyItems input{
		u256(7),
		Instruction::DUP2,
		Instruction::SSTORE,
		Instruction::DUP1,
		Instruction::SLOAD,
		u256(8),
		Instruction::DUP3,
		Instruction::SSTORE
	};
	checkCSE(input, {
		u256(8),
		Instruction::DUP2,
		Instruction::SSTORE,
		u256(7)
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage)
{
	// stores and reads to/from two unknown locations, should not optimize away the first store
	AssemblyItems input{
		u256(7),
		Instruction::DUP2,
		Instruction::SSTORE, // store to "DUP1"
		Instruction::DUP2,
		Instruction::SLOAD, // read from "DUP2", might be equal to "DUP1"
		u256(0),
		Instruction::DUP3,
		Instruction::SSTORE // store different value to "DUP1"
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_same_value)
{
	// stores and reads to/from two unknown locations, should not optimize away the first store
	// but it should optimize away the second, since we already know the value will be the same
	AssemblyItems input{
		u256(7),
		Instruction::DUP2,
		Instruction::SSTORE, // store to "DUP1"
		Instruction::DUP2,
		Instruction::SLOAD, // read from "DUP2", might be equal to "DUP1"
		u256(6),
		u256(1),
		Instruction::ADD,
		Instruction::DUP3,
		Instruction::SSTORE // store same value to "DUP1"
	};
	checkCSE(input, {
		u256(7),
		Instruction::DUP2,
		Instruction::SSTORE,
		Instruction::DUP2,
		Instruction::SLOAD
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_at_known_location)
{
	// stores and reads to/from two known locations, should optimize away the first store,
	// because we know that the location is different
	AssemblyItems input{
		u256(0x70),
		u256(1),
		Instruction::SSTORE, // store to 1
		u256(2),
		Instruction::SLOAD, // read from 2, is different from 1
		u256(0x90),
		u256(1),
		Instruction::SSTORE // store different value at 1
	};
	checkCSE(input, {
		u256(2),
		Instruction::SLOAD,
		u256(0x90),
		u256(1),
		Instruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_at_known_location_offset)
{
	// stores and reads to/from two locations which are known to be different,
	// should optimize away the first store, because we know that the location is different
	AssemblyItems input{
		u256(0x70),
		Instruction::DUP2,
		u256(1),
		Instruction::ADD,
		Instruction::SSTORE, // store to "DUP1"+1
		Instruction::DUP1,
		u256(2),
		Instruction::ADD,
		Instruction::SLOAD, // read from "DUP1"+2, is different from "DUP1"+1
		u256(0x90),
		Instruction::DUP3,
		u256(1),
		Instruction::ADD,
		Instruction::SSTORE // store different value at "DUP1"+1
	};
	checkCSE(input, {
		u256(2),
		Instruction::DUP2,
		Instruction::ADD,
		Instruction::SLOAD,
		u256(0x90),
		u256(1),
		Instruction::DUP4,
		Instruction::ADD,
		Instruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_memory_at_known_location_offset)
{
	// stores and reads to/from two locations which are known to be different,
	// should not optimize away the first store, because the location overlaps with the load,
	// but it should optimize away the second, because we know that the location is different by 32
	AssemblyItems input{
		u256(0x50),
		Instruction::DUP2,
		u256(2),
		Instruction::ADD,
		Instruction::MSTORE, // ["DUP1"+2] = 0x50
		u256(0x60),
		Instruction::DUP2,
		u256(32),
		Instruction::ADD,
		Instruction::MSTORE, // ["DUP1"+32] = 0x60
		Instruction::DUP1,
		Instruction::MLOAD, // read from "DUP1"
		u256(0x70),
		Instruction::DUP3,
		u256(32),
		Instruction::ADD,
		Instruction::MSTORE, // ["DUP1"+32] = 0x70
		u256(0x80),
		Instruction::DUP3,
		u256(2),
		Instruction::ADD,
		Instruction::MSTORE, // ["DUP1"+2] = 0x80
	};
	// If the actual code changes too much, we could also simply check that the output contains
	// exactly 3 MSTORE and exactly 1 MLOAD instruction.
	checkCSE(input, {
		u256(0x50),
		u256(2),
		Instruction::DUP3,
		Instruction::ADD,
		Instruction::SWAP1,
		Instruction::DUP2,
		Instruction::MSTORE, // ["DUP1"+2] = 0x50
		Instruction::DUP2,
		Instruction::MLOAD, // read from "DUP1"
		u256(0x70),
		u256(32),
		Instruction::DUP5,
		Instruction::ADD,
		Instruction::MSTORE, // ["DUP1"+32] = 0x70
		u256(0x80),
		Instruction::SWAP1,
		Instruction::SWAP2,
		Instruction::MSTORE // ["DUP1"+2] = 0x80
	});
}

BOOST_AUTO_TEST_CASE(cse_deep_stack)
{
	AssemblyItems input{
		Instruction::ADD,
		Instruction::SWAP1,
		Instruction::POP,
		Instruction::SWAP8,
		Instruction::POP,
		Instruction::SWAP8,
		Instruction::POP,
		Instruction::SWAP8,
		Instruction::SWAP5,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
	};
	checkCSE(input, {
		Instruction::SWAP4,
		Instruction::SWAP12,
		Instruction::SWAP3,
		Instruction::SWAP11,
		Instruction::POP,
		Instruction::SWAP1,
		Instruction::SWAP3,
		Instruction::ADD,
		Instruction::SWAP8,
		Instruction::POP,
		Instruction::SWAP6,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
		Instruction::POP,
	});
}

BOOST_AUTO_TEST_CASE(cse_jumpi_no_jump)
{
	AssemblyItems input{
		u256(0),
		u256(1),
		Instruction::DUP2,
		AssemblyItem(PushTag, 1),
		Instruction::JUMPI
	};
	checkCSE(input, {
		u256(0),
		u256(1)
	});
}

BOOST_AUTO_TEST_CASE(cse_jumpi_jump)
{
	AssemblyItems input{
		u256(1),
		u256(1),
		Instruction::DUP2,
		AssemblyItem(PushTag, 1),
		Instruction::JUMPI
	};
	checkCSE(input, {
		u256(1),
		Instruction::DUP1,
		AssemblyItem(PushTag, 1),
		Instruction::JUMP
	});
}

BOOST_AUTO_TEST_CASE(cse_empty_sha3)
{
	AssemblyItems input{
		u256(0),
		Instruction::DUP2,
		Instruction::SHA3
	};
	checkCSE(input, {
		u256(sha3(bytesConstRef()))
	});
}

BOOST_AUTO_TEST_CASE(cse_partial_sha3)
{
	AssemblyItems input{
		u256(0xabcd) << (256 - 16),
		u256(0),
		Instruction::MSTORE,
		u256(2),
		u256(0),
		Instruction::SHA3
	};
	checkCSE(input, {
		u256(0xabcd) << (256 - 16),
		u256(0),
		Instruction::MSTORE,
		u256(sha3(bytes{0xab, 0xcd}))
	});
}

BOOST_AUTO_TEST_CASE(cse_sha3_twice_same_location)
{
	// sha3 twice from same dynamic location
	AssemblyItems input{
		Instruction::DUP2,
		Instruction::DUP1,
		Instruction::MSTORE,
		u256(64),
		Instruction::DUP2,
		Instruction::SHA3,
		u256(64),
		Instruction::DUP3,
		Instruction::SHA3
	};
	checkCSE(input, {
		Instruction::DUP2,
		Instruction::DUP1,
		Instruction::MSTORE,
		u256(64),
		Instruction::DUP2,
		Instruction::SHA3,
		Instruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_sha3_twice_same_content)
{
	// sha3 twice from different dynamic location but with same content
	AssemblyItems input{
		Instruction::DUP1,
		u256(0x80),
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		u256(0x80),
		Instruction::SHA3, // sha3(m[128..(128+32)])
		Instruction::DUP2,
		u256(12),
		Instruction::MSTORE, // m[12] = DUP1
		u256(0x20),
		u256(12),
		Instruction::SHA3 // sha3(m[12..(12+32)])
	};
	checkCSE(input, {
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE,
		u256(0x20),
		Instruction::SWAP1,
		Instruction::SHA3,
		u256(12),
		Instruction::DUP3,
		Instruction::SWAP1,
		Instruction::MSTORE,
		Instruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_sha3_twice_same_content_dynamic_store_in_between)
{
	// sha3 twice from different dynamic location but with same content,
	// dynamic mstore in between, which forces us to re-calculate the sha3
	AssemblyItems input{
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		Instruction::DUP1,
		Instruction::DUP3,
		Instruction::SHA3, // sha3(m[128..(128+32)])
		u256(12),
		Instruction::DUP5,
		Instruction::DUP2,
		Instruction::MSTORE, // m[12] = DUP1
		Instruction::DUP12,
		Instruction::DUP14,
		Instruction::MSTORE, // destroys memory knowledge
		Instruction::SWAP2,
		Instruction::SWAP1,
		Instruction::SWAP2,
		Instruction::SHA3 // sha3(m[12..(12+32)])
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_sha3_twice_same_content_noninterfering_store_in_between)
{
	// sha3 twice from different dynamic location but with same content,
	// dynamic mstore in between, but does not force us to re-calculate the sha3
	AssemblyItems input{
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		Instruction::DUP1,
		Instruction::DUP3,
		Instruction::SHA3, // sha3(m[128..(128+32)])
		u256(12),
		Instruction::DUP5,
		Instruction::DUP2,
		Instruction::MSTORE, // m[12] = DUP1
		Instruction::DUP12,
		u256(12 + 32),
		Instruction::MSTORE, // does not destoy memory knowledge
		Instruction::DUP13,
		u256(128 - 32),
		Instruction::MSTORE, // does not destoy memory knowledge
		u256(0x20),
		u256(12),
		Instruction::SHA3 // sha3(m[12..(12+32)])
	};
	// if this changes too often, only count the number of SHA3 and MSTORE instructions
	AssemblyItems output = getCSE(input);
	BOOST_CHECK_EQUAL(4, count(output.begin(), output.end(), AssemblyItem(Instruction::MSTORE)));
	BOOST_CHECK_EQUAL(1, count(output.begin(), output.end(), AssemblyItem(Instruction::SHA3)));
}

BOOST_AUTO_TEST_CASE(control_flow_graph_remove_unused)
{
	// remove parts of the code that are unused
	AssemblyItems input{
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		u256(7),
		AssemblyItem(Tag, 1),
	};
	checkCFG(input, {});
}

BOOST_AUTO_TEST_CASE(control_flow_graph_remove_unused_loop)
{
	AssemblyItems input{
		AssemblyItem(PushTag, 3),
		Instruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(7),
		AssemblyItem(PushTag, 2),
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(8),
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		AssemblyItem(Tag, 3),
		u256(11)
	};
	checkCFG(input, {u256(11)});
}

BOOST_AUTO_TEST_CASE(control_flow_graph_reconnect_single_jump_source)
{
	// move code that has only one unconditional jump source
	AssemblyItems input{
		u256(1),
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(2),
		AssemblyItem(PushTag, 3),
		Instruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(3),
		AssemblyItem(PushTag, 2),
		Instruction::JUMP,
		AssemblyItem(Tag, 3),
		u256(4),
	};
	checkCFG(input, {u256(1), u256(3), u256(2), u256(4)});
}

BOOST_AUTO_TEST_CASE(control_flow_graph_do_not_remove_returned_to)
{
	// do not remove parts that are "returned to"
	AssemblyItems input{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		Instruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(2)
	};
	checkCFG(input, {u256(2)});
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
