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

#include <test/Options.h>

#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/PeepholeOptimiser.h>
#include <libevmasm/JumpdestRemover.h>
#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/BlockDeduplicator.h>
#include <libevmasm/Assembly.h>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <tuple>
#include <memory>

using namespace std;
using namespace dev::eth;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{
	AssemblyItems addDummyLocations(AssemblyItems const& _input)
	{
		// add dummy locations to each item so that we can check that they are not deleted
		AssemblyItems input = _input;
		for (AssemblyItem& item: input)
			item.setLocation(SourceLocation(1, 3, make_shared<string>("")));
		return input;
	}

	eth::KnownState createInitialState(AssemblyItems const& _input)
	{
		eth::KnownState state;
		for (auto const& item: addDummyLocations(_input))
			state.feedItem(item, true);
		return state;
	}

	AssemblyItems CSE(AssemblyItems const& _input, eth::KnownState const& _state = eth::KnownState())
	{
		AssemblyItems input = addDummyLocations(_input);

		bool usesMsize = (find(_input.begin(), _input.end(), AssemblyItem{Instruction::MSIZE}) != _input.end());
		eth::CommonSubexpressionEliminator cse(_state);
		BOOST_REQUIRE(cse.feedItems(input.begin(), input.end(), usesMsize) == input.end());
		AssemblyItems output = cse.getOptimizedItems();

		for (AssemblyItem const& item: output)
		{
			BOOST_CHECK(item == Instruction::POP || !item.location().isEmpty());
		}
		return output;
	}

	void checkCSE(
		AssemblyItems const& _input,
		AssemblyItems const& _expectation,
		KnownState const& _state = eth::KnownState()
	)
	{
		AssemblyItems output = CSE(_input, _state);
		BOOST_CHECK_EQUAL_COLLECTIONS(_expectation.begin(), _expectation.end(), output.begin(), output.end());
	}

	AssemblyItems CFG(AssemblyItems const& _input)
	{
		AssemblyItems output = _input;
		// Running it four times should be enough for these tests.
		for (unsigned i = 0; i < 4; ++i)
		{
			ControlFlowGraph cfg(output);
			AssemblyItems optItems;
			for (BasicBlock const& block: cfg.optimisedBlocks())
				copy(output.begin() + block.begin, output.begin() + block.end,
					 back_inserter(optItems));
			output = move(optItems);
		}
		return output;
	}

	void checkCFG(AssemblyItems const& _input, AssemblyItems const& _expectation)
	{
		AssemblyItems output = CFG(_input);
		BOOST_CHECK_EQUAL_COLLECTIONS(_expectation.begin(), _expectation.end(), output.begin(), output.end());
	}
}

BOOST_AUTO_TEST_SUITE(Optimiser)

BOOST_AUTO_TEST_CASE(cse_intermediate_swap)
{
	eth::KnownState state;
	eth::CommonSubexpressionEliminator cse(state);
	AssemblyItems input{
		Instruction::SWAP1, Instruction::POP, Instruction::ADD, u256(0), Instruction::SWAP1,
		Instruction::SLOAD, Instruction::SWAP1, u256(100), Instruction::EXP, Instruction::SWAP1,
		Instruction::DIV, u256(0xff), Instruction::AND
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end(), false) == input.end());
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

BOOST_AUTO_TEST_CASE(cse_double_iszero)
{
	checkCSE({Instruction::GT, Instruction::ISZERO, Instruction::ISZERO}, {Instruction::GT});
	checkCSE({Instruction::GT, Instruction::ISZERO}, {Instruction::GT, Instruction::ISZERO});
	checkCSE(
		{Instruction::ISZERO, Instruction::ISZERO, Instruction::ISZERO},
		{Instruction::ISZERO}
	);
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

BOOST_AUTO_TEST_CASE(cse_empty_keccak256)
{
	AssemblyItems input{
		u256(0),
		Instruction::DUP2,
		Instruction::KECCAK256
	};
	checkCSE(input, {
		u256(dev::keccak256(bytesConstRef()))
	});
}

BOOST_AUTO_TEST_CASE(cse_partial_keccak256)
{
	AssemblyItems input{
		u256(0xabcd) << (256 - 16),
		u256(0),
		Instruction::MSTORE,
		u256(2),
		u256(0),
		Instruction::KECCAK256
	};
	checkCSE(input, {
		u256(0xabcd) << (256 - 16),
		u256(0),
		Instruction::MSTORE,
		u256(dev::keccak256(bytes{0xab, 0xcd}))
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_location)
{
	// Keccak-256 twice from same dynamic location
	AssemblyItems input{
		Instruction::DUP2,
		Instruction::DUP1,
		Instruction::MSTORE,
		u256(64),
		Instruction::DUP2,
		Instruction::KECCAK256,
		u256(64),
		Instruction::DUP3,
		Instruction::KECCAK256
	};
	checkCSE(input, {
		Instruction::DUP2,
		Instruction::DUP1,
		Instruction::MSTORE,
		u256(64),
		Instruction::DUP2,
		Instruction::KECCAK256,
		Instruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content)
{
	// Keccak-256 twice from different dynamic location but with same content
	AssemblyItems input{
		Instruction::DUP1,
		u256(0x80),
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		u256(0x80),
		Instruction::KECCAK256, // keccak256(m[128..(128+32)])
		Instruction::DUP2,
		u256(12),
		Instruction::MSTORE, // m[12] = DUP1
		u256(0x20),
		u256(12),
		Instruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	checkCSE(input, {
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE,
		u256(0x20),
		Instruction::SWAP1,
		Instruction::KECCAK256,
		u256(12),
		Instruction::DUP3,
		Instruction::SWAP1,
		Instruction::MSTORE,
		Instruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content_dynamic_store_in_between)
{
	// Keccak-256 twice from different dynamic location but with same content,
	// dynamic mstore in between, which forces us to re-calculate the hash
	AssemblyItems input{
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		Instruction::DUP1,
		Instruction::DUP3,
		Instruction::KECCAK256, // keccak256(m[128..(128+32)])
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
		Instruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content_noninterfering_store_in_between)
{
	// Keccak-256 twice from different dynamic location but with same content,
	// dynamic mstore in between, but does not force us to re-calculate the hash
	AssemblyItems input{
		u256(0x80),
		Instruction::DUP2,
		Instruction::DUP2,
		Instruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		Instruction::DUP1,
		Instruction::DUP3,
		Instruction::KECCAK256, // keccak256(m[128..(128+32)])
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
		Instruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	// if this changes too often, only count the number of SHA3 and MSTORE instructions
	AssemblyItems output = CSE(input);
	BOOST_CHECK_EQUAL(4, count(output.begin(), output.end(), AssemblyItem(Instruction::MSTORE)));
	BOOST_CHECK_EQUAL(1, count(output.begin(), output.end(), AssemblyItem(Instruction::KECCAK256)));
}

BOOST_AUTO_TEST_CASE(cse_with_initially_known_stack)
{
	eth::KnownState state = createInitialState(AssemblyItems{
		u256(0x12),
		u256(0x20),
		Instruction::ADD
	});
	AssemblyItems input{
		u256(0x12 + 0x20)
	};
	checkCSE(input, AssemblyItems{Instruction::DUP1}, state);
}

BOOST_AUTO_TEST_CASE(cse_equality_on_initially_known_stack)
{
	eth::KnownState state = createInitialState(AssemblyItems{Instruction::DUP1});
	AssemblyItems input{
		Instruction::EQ
	};
	AssemblyItems output = CSE(input, state);
	// check that it directly pushes 1 (true)
	BOOST_CHECK(find(output.begin(), output.end(), AssemblyItem(u256(1))) != output.end());
}

BOOST_AUTO_TEST_CASE(cse_access_previous_sequence)
{
	// Tests that the code generator detects whether it tries to access SLOAD instructions
	// from a sequenced expression which is not in its scope.
	eth::KnownState state = createInitialState(AssemblyItems{
		u256(0),
		Instruction::SLOAD,
		u256(1),
		Instruction::ADD,
		u256(0),
		Instruction::SSTORE
	});
	// now stored: val_1 + 1 (value at sequence 1)
	// if in the following instructions, the SLOAD cresolves to "val_1 + 1",
	// this cannot be generated because we cannot load from sequence 1 anymore.
	AssemblyItems input{
		u256(0),
		Instruction::SLOAD,
	};
	BOOST_CHECK_THROW(CSE(input, state), StackTooDeepException);
	// @todo for now, this throws an exception, but it should recover to the following
	// (or an even better version) at some point:
	// 0, SLOAD, 1, ADD, SSTORE, 0 SLOAD
}

BOOST_AUTO_TEST_CASE(cse_optimise_return)
{
	checkCSE(
		AssemblyItems{u256(0), u256(7), Instruction::RETURN},
		AssemblyItems{Instruction::STOP}
	);
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

BOOST_AUTO_TEST_CASE(block_deduplicator)
{
	AssemblyItems input{
		AssemblyItem(PushTag, 2),
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 3),
		u256(6),
		Instruction::SWAP3,
		Instruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(6),
		Instruction::SWAP3,
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(6),
		Instruction::SWAP3,
		Instruction::JUMP,
		AssemblyItem(Tag, 3)
	};
	BlockDeduplicator dedup(input);
	dedup.deduplicate();

	set<u256> pushTags;
	for (AssemblyItem const& item: input)
		if (item.type() == PushTag)
			pushTags.insert(item.data());
	BOOST_CHECK_EQUAL(pushTags.size(), 2);
}

BOOST_AUTO_TEST_CASE(block_deduplicator_loops)
{
	AssemblyItems input{
		u256(0),
		Instruction::SLOAD,
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		Instruction::JUMPI,
		Instruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(5),
		u256(6),
		Instruction::SSTORE,
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		Instruction::SSTORE,
		AssemblyItem(PushTag, 2),
		Instruction::JUMP,
	};
	BlockDeduplicator dedup(input);
	dedup.deduplicate();

	set<u256> pushTags;
	for (AssemblyItem const& item: input)
		if (item.type() == PushTag)
			pushTags.insert(item.data());
	BOOST_CHECK_EQUAL(pushTags.size(), 1);
}

BOOST_AUTO_TEST_CASE(clear_unreachable_code)
{
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		u256(0),
		Instruction::SLOAD,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		Instruction::SSTORE,
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		u256(5),
		u256(6)
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		Instruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		Instruction::SSTORE,
		AssemblyItem(PushTag, 1),
		Instruction::JUMP
	};
	PeepholeOptimiser peepOpt(items);
	BOOST_REQUIRE(peepOpt.optimise());
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(peephole_double_push)
{
	AssemblyItems items{
		u256(0),
		u256(0),
		u256(5),
		u256(5),
		u256(4),
		u256(5)
	};
	AssemblyItems expectation{
		u256(0),
		Instruction::DUP1,
		u256(5),
		Instruction::DUP1,
		u256(4),
		u256(5)
	};
	PeepholeOptimiser peepOpt(items);
	BOOST_REQUIRE(peepOpt.optimise());
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(peephole_pop_calldatasize)
{
	AssemblyItems items{
		u256(4),
		Instruction::CALLDATASIZE,
		Instruction::LT,
		Instruction::POP
	};
	PeepholeOptimiser peepOpt(items);
	for (size_t i = 0; i < 3; i++)
		BOOST_CHECK(peepOpt.optimise());
	BOOST_CHECK(items.empty());
}

BOOST_AUTO_TEST_CASE(peephole_commutative_swap1)
{
	vector<Instruction> ops{
		Instruction::ADD,
		Instruction::MUL,
		Instruction::EQ,
		Instruction::AND,
		Instruction::OR,
		Instruction::XOR
	};
	for (Instruction const op: ops)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			Instruction::SWAP1,
			op,
			u256(4),
			u256(5)
		};
		AssemblyItems expectation{
			u256(1),
			u256(2),
			op,
			u256(4),
			u256(5)
		};
		PeepholeOptimiser peepOpt(items);
		BOOST_REQUIRE(peepOpt.optimise());
		BOOST_CHECK_EQUAL_COLLECTIONS(
			items.begin(), items.end(),
			expectation.begin(), expectation.end()
		);
	}
}

BOOST_AUTO_TEST_CASE(peephole_noncommutative_swap1)
{
	// NOTE: not comprehensive
	vector<Instruction> ops{
		Instruction::SUB,
		Instruction::DIV,
		Instruction::SDIV,
		Instruction::MOD,
		Instruction::SMOD,
		Instruction::EXP
	};
	for (Instruction const op: ops)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			Instruction::SWAP1,
			op,
			u256(4),
			u256(5)
		};
		AssemblyItems expectation{
			u256(1),
			u256(2),
			Instruction::SWAP1,
			op,
			u256(4),
			u256(5)
		};
		PeepholeOptimiser peepOpt(items);
		BOOST_REQUIRE(!peepOpt.optimise());
		BOOST_CHECK_EQUAL_COLLECTIONS(
			items.begin(), items.end(),
			expectation.begin(), expectation.end()
		);
	}
}

BOOST_AUTO_TEST_CASE(peephole_swap_comparison)
{
	map<Instruction, Instruction> swappableOps{
		{ Instruction::LT, Instruction::GT },
		{ Instruction::GT, Instruction::LT },
		{ Instruction::SLT, Instruction::SGT },
		{ Instruction::SGT, Instruction::SLT }
	};

	for (auto const& op: swappableOps)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			Instruction::SWAP1,
			op.first,
			u256(4),
			u256(5)
		};
		AssemblyItems expectation{
			u256(1),
			u256(2),
			op.second,
			u256(4),
			u256(5)
		};
		PeepholeOptimiser peepOpt(items);
		BOOST_REQUIRE(peepOpt.optimise());
		BOOST_CHECK_EQUAL_COLLECTIONS(
			items.begin(), items.end(),
			expectation.begin(), expectation.end()
		);
	}
}

BOOST_AUTO_TEST_CASE(peephole_truthy_and)
{
  AssemblyItems items{
    AssemblyItem(Tag, 1),
    Instruction::BALANCE,
    u256(0),
    Instruction::NOT,
    Instruction::AND,
    AssemblyItem(PushTag, 1),
    Instruction::JUMPI
  };
  AssemblyItems expectation{
    AssemblyItem(Tag, 1),
    Instruction::BALANCE,
    AssemblyItem(PushTag, 1),
    Instruction::JUMPI
  };
  PeepholeOptimiser peepOpt(items);
  BOOST_REQUIRE(peepOpt.optimise());
  BOOST_CHECK_EQUAL_COLLECTIONS(
    items.begin(), items.end(),
    expectation.begin(), expectation.end()
  );
}

BOOST_AUTO_TEST_CASE(jumpdest_removal)
{
	AssemblyItems items{
		AssemblyItem(Tag, 2),
		AssemblyItem(PushTag, 1),
		u256(5),
		AssemblyItem(Tag, 10),
		AssemblyItem(Tag, 3),
		u256(6),
		AssemblyItem(Tag, 1),
		Instruction::JUMP,
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		u256(5),
		u256(6),
		AssemblyItem(Tag, 1),
		Instruction::JUMP
	};
	JumpdestRemover jdr(items);
	BOOST_REQUIRE(jdr.optimise({}));
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(jumpdest_removal_subassemblies)
{
	// This tests that tags from subassemblies are not removed
	// if they are referenced by a super-assembly. Furthermore,
	// tag unifications (due to block deduplication) is also
	// visible at the super-assembly.

	Assembly main;
	AssemblyPointer sub = make_shared<Assembly>();

	sub->append(u256(1));
	auto t1 = sub->newTag();
	sub->append(t1);
	sub->append(u256(2));
	sub->append(Instruction::JUMP);
	auto t2 = sub->newTag();
	sub->append(t2); // Identical to T1, will be unified
	sub->append(u256(2));
	sub->append(Instruction::JUMP);
	auto t3 = sub->newTag();
	sub->append(t3);
	auto t4 = sub->newTag();
	sub->append(t4);
	auto t5 = sub->newTag();
	sub->append(t5); // This will be removed
	sub->append(u256(7));
	sub->append(t4.pushTag());
	sub->append(Instruction::JUMP);

	size_t subId = size_t(main.appendSubroutine(sub).data());
	main.append(t1.toSubAssemblyTag(subId));
	main.append(t1.toSubAssemblyTag(subId));
	main.append(u256(8));

	main.optimise(true, dev::test::Options::get().evmVersion());

	AssemblyItems expectationMain{
		AssemblyItem(PushSubSize, 0),
		t1.toSubAssemblyTag(subId).pushTag(),
		t1.toSubAssemblyTag(subId).pushTag(),
		u256(8)
	};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		main.items().begin(), main.items().end(),
		expectationMain.begin(), expectationMain.end()
	);

	AssemblyItems expectationSub{
		u256(1), t1.tag(), u256(2), Instruction::JUMP, t4.tag(), u256(7), t4.pushTag(), Instruction::JUMP
	};
	BOOST_CHECK_EQUAL_COLLECTIONS(
		sub->items().begin(), sub->items().end(),
		expectationSub.begin(), expectationSub.end()
	);
}

BOOST_AUTO_TEST_CASE(cse_sub_zero)
{
	checkCSE({
		u256(0),
		Instruction::DUP2,
		Instruction::SUB
	}, {
		Instruction::DUP1
	});

	checkCSE({
		Instruction::DUP1,
		u256(0),
		Instruction::SUB
	}, {
		u256(0),
		Instruction::DUP2,
		Instruction::SWAP1,
		Instruction::SUB
	});
}

BOOST_AUTO_TEST_CASE(cse_remove_unwanted_masking_of_address)
{
	vector<Instruction> ops{
		Instruction::ADDRESS,
		Instruction::CALLER,
		Instruction::ORIGIN,
		Instruction::COINBASE
	};
	for (auto const& op: ops)
	{
		checkCSE({
			u256("0xffffffffffffffffffffffffffffffffffffffff"),
			op,
			Instruction::AND
		}, {
			op
		});

		checkCSE({
			op,
			u256("0xffffffffffffffffffffffffffffffffffffffff"),
			Instruction::AND
		}, {
			op
		});

		// do not remove mask for other masking
		checkCSE({
			u256(1234),
			op,
			Instruction::AND
		}, {
			op,
			u256(1234),
			Instruction::AND
		});

		checkCSE({
			op,
			u256(1234),
			Instruction::AND
		}, {
			u256(1234),
			op,
			Instruction::AND
		});
	}

	// leave other opcodes untouched
	checkCSE({
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		Instruction::CALLVALUE,
		Instruction::AND
	}, {
		Instruction::CALLVALUE,
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		Instruction::AND
	});

	checkCSE({
		Instruction::CALLVALUE,
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		Instruction::AND
	}, {
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		Instruction::CALLVALUE,
		Instruction::AND
	});
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
