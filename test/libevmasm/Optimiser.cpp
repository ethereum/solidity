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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Tests for the Solidity optimizer.
 */

#include <test/Common.h>

#include <libevmasm/CommonSubexpressionEliminator.h>
#include <libevmasm/PeepholeOptimiser.h>
#include <libevmasm/Inliner.h>
#include <libevmasm/JumpdestRemover.h>
#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/BlockDeduplicator.h>
#include <libevmasm/Assembly.h>

#include <boost/test/unit_test.hpp>

#include <range/v3/algorithm/any_of.hpp>

#include <string>
#include <tuple>
#include <memory>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::evmasm;

namespace solidity::frontend::test
{

namespace
{
	AssemblyItems addDummyLocations(AssemblyItems const& _input)
	{
		// add dummy locations to each item so that we can check that they are not deleted
		AssemblyItems input = _input;
		for (AssemblyItem& item: input)
			item.setLocation({1, 3, nullptr});
		return input;
	}

	evmasm::KnownState createInitialState(AssemblyItems const& _input)
	{
		evmasm::KnownState state;
		for (auto const& item: addDummyLocations(_input))
			state.feedItem(item, true);
		return state;
	}

	AssemblyItems CSE(AssemblyItems const& _input, evmasm::KnownState const& _state = evmasm::KnownState())
	{
		AssemblyItems input = addDummyLocations(_input);

		bool usesMsize = ranges::any_of(_input, [](AssemblyItem const& _i) {
			return _i == AssemblyItem{InternalInstruction::MSIZE} || _i.type() == VerbatimBytecode;
		});
		evmasm::CommonSubexpressionEliminator cse(_state);
		BOOST_REQUIRE(cse.feedItems(input.begin(), input.end(), usesMsize) == input.end());
		AssemblyItems output = cse.getOptimizedItems();

		for (AssemblyItem const& item: output)
		{
			BOOST_CHECK(item == InternalInstruction::POP || item.location().isValid());
		}
		return output;
	}

	void checkCSE(
		AssemblyItems const& _input,
		AssemblyItems const& _expectation,
		KnownState const& _state = evmasm::KnownState()
	)
	{
		AssemblyItems output = CSE(_input, _state);
		BOOST_CHECK_EQUAL_COLLECTIONS(_expectation.begin(), _expectation.end(), output.begin(), output.end());
	}

	/// In contrast to the function `CSE`, this function doesn't finish the CSE optimization on an
	/// instruction that breaks CSE Analysis block. Copied from Assembly.cpp
	AssemblyItems fullCSE(AssemblyItems const& _input)
	{
		AssemblyItems optimisedItems;

		bool usesMSize = ranges::any_of(_input, [](AssemblyItem const& _i) {
			return _i == AssemblyItem{InternalInstruction::MSIZE} || _i.type() == VerbatimBytecode;
		});

		auto iter = _input.begin();
		while (iter != _input.end())
		{
			KnownState emptyState;
			CommonSubexpressionEliminator eliminator{emptyState};
			auto orig = iter;
			iter = eliminator.feedItems(iter, _input.end(), usesMSize);
			bool shouldReplace = false;
			AssemblyItems optimisedChunk;
			optimisedChunk = eliminator.getOptimizedItems();
			shouldReplace = (optimisedChunk.size() < static_cast<size_t>(iter - orig));
			if (shouldReplace)
				optimisedItems += optimisedChunk;
			else
				copy(orig, iter, back_inserter(optimisedItems));
		}

		return optimisedItems;
	}

	void checkFullCSE(
		AssemblyItems const& _input,
		AssemblyItems const& _expectation
	)
	{
		AssemblyItems output = fullCSE(_input);
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
				copy(output.begin() + static_cast<int>(block.begin), output.begin() + static_cast<int>(block.end),
					 back_inserter(optItems));
			output = std::move(optItems);
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

BOOST_AUTO_TEST_CASE(cse_push_immutable_same)
{
	AssemblyItem pushImmutable{PushImmutable, 0x1234};
	checkCSE({pushImmutable, pushImmutable}, {pushImmutable, InternalInstruction::DUP1});
}

BOOST_AUTO_TEST_CASE(cse_push_immutable_different)
{
	AssemblyItems input{{PushImmutable, 0x1234},{PushImmutable, 0xABCD}};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_assign_immutable)
{
	{
		AssemblyItems input{u256(0x42), {AssignImmutable, 0x1234}};
		checkCSE(input, input);
	}
	{
		AssemblyItems input{{AssignImmutable, 0x1234}};
		checkCSE(input, input);
	}
}


BOOST_AUTO_TEST_CASE(cse_assign_immutable_breaks)
{
	AssemblyItems input = addDummyLocations(AssemblyItems{
		u256(0x42),
		{AssignImmutable, 0x1234},
		InternalInstruction::ORIGIN
	});

	evmasm::CommonSubexpressionEliminator cse{evmasm::KnownState()};
	// Make sure CSE breaks after AssignImmutable.
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end(), false) == input.begin() + 2);
}

BOOST_AUTO_TEST_CASE(cse_intermediate_swap)
{
	evmasm::KnownState state;
	evmasm::CommonSubexpressionEliminator cse(state);
	AssemblyItems input{
		InternalInstruction::SWAP1, InternalInstruction::POP, InternalInstruction::ADD, u256(0), InternalInstruction::SWAP1,
		InternalInstruction::SLOAD, InternalInstruction::SWAP1, u256(100), InternalInstruction::EXP, InternalInstruction::SWAP1,
		InternalInstruction::DIV, u256(0xff), InternalInstruction::AND
	};
	BOOST_REQUIRE(cse.feedItems(input.begin(), input.end(), false) == input.end());
	AssemblyItems output = cse.getOptimizedItems();
	BOOST_CHECK(!output.empty());
}

BOOST_AUTO_TEST_CASE(cse_negative_stack_access)
{
	AssemblyItems input{InternalInstruction::DUP2, u256(0)};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_negative_stack_end)
{
	AssemblyItems input{InternalInstruction::ADD};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_intermediate_negative_stack)
{
	AssemblyItems input{InternalInstruction::ADD, u256(1), InternalInstruction::DUP1};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_pop)
{
	checkCSE({InternalInstruction::POP}, {InternalInstruction::POP});
}

BOOST_AUTO_TEST_CASE(cse_unneeded_items)
{
	AssemblyItems input{
		InternalInstruction::ADD,
		InternalInstruction::SWAP1,
		InternalInstruction::POP,
		u256(7),
		u256(8),
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_constant_addition)
{
	AssemblyItems input{u256(7), u256(8), InternalInstruction::ADD};
	checkCSE(input, {u256(7 + 8)});
}

BOOST_AUTO_TEST_CASE(cse_invariants)
{
	AssemblyItems input{
		InternalInstruction::DUP1,
		InternalInstruction::DUP1,
		u256(0),
		InternalInstruction::OR,
		InternalInstruction::OR
	};
	checkCSE(input, {InternalInstruction::DUP1});
}

BOOST_AUTO_TEST_CASE(cse_subself)
{
	checkCSE({InternalInstruction::DUP1, InternalInstruction::SUB}, {InternalInstruction::POP, u256(0)});
}

BOOST_AUTO_TEST_CASE(cse_subother)
{
	checkCSE({InternalInstruction::SUB}, {InternalInstruction::SUB});
}

BOOST_AUTO_TEST_CASE(cse_double_negation)
{
	checkCSE({InternalInstruction::DUP5, InternalInstruction::NOT, InternalInstruction::NOT}, {InternalInstruction::DUP5});
}

BOOST_AUTO_TEST_CASE(cse_double_iszero)
{
	checkCSE({InternalInstruction::GT, InternalInstruction::ISZERO, InternalInstruction::ISZERO}, {InternalInstruction::GT});
	checkCSE({InternalInstruction::GT, InternalInstruction::ISZERO}, {InternalInstruction::GT, InternalInstruction::ISZERO});
	checkCSE(
		{InternalInstruction::ISZERO, InternalInstruction::ISZERO, InternalInstruction::ISZERO},
		{InternalInstruction::ISZERO}
	);
}

BOOST_AUTO_TEST_CASE(cse_associativity)
{
	AssemblyItems input{
		InternalInstruction::DUP1,
		InternalInstruction::DUP1,
		u256(0),
		InternalInstruction::OR,
		InternalInstruction::OR
	};
	checkCSE(input, {InternalInstruction::DUP1});
}

BOOST_AUTO_TEST_CASE(cse_associativity2)
{
	AssemblyItems input{
		u256(0),
		InternalInstruction::DUP2,
		u256(2),
		u256(1),
		InternalInstruction::DUP6,
		InternalInstruction::ADD,
		u256(2),
		InternalInstruction::ADD,
		InternalInstruction::ADD,
		InternalInstruction::ADD,
		InternalInstruction::ADD
	};
	checkCSE(input, {InternalInstruction::DUP2, InternalInstruction::DUP2, InternalInstruction::ADD, u256(5), InternalInstruction::ADD});
}

BOOST_AUTO_TEST_CASE(cse_double_shift_right_overflow)
{
	if (solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
	{
		AssemblyItems input{
			InternalInstruction::CALLVALUE,
			u256(2),
			InternalInstruction::SHR,
			u256(-1),
			InternalInstruction::SHR
		};
		checkCSE(input, {u256(0)});
	}
}

BOOST_AUTO_TEST_CASE(cse_double_shift_left_overflow)
{
	if (solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
	{
		AssemblyItems input{
			InternalInstruction::DUP1,
			u256(2),
			InternalInstruction::SHL,
			u256(-1),
			InternalInstruction::SHL
		};
		checkCSE(input, {u256(0)});
	}
}

BOOST_AUTO_TEST_CASE(cse_byte_ordering_bug)
{
	AssemblyItems input{
		u256(31),
		InternalInstruction::CALLVALUE,
		InternalInstruction::BYTE
	};
	checkCSE(input, {u256(31), InternalInstruction::CALLVALUE, InternalInstruction::BYTE});
}

BOOST_AUTO_TEST_CASE(cse_byte_ordering_fix)
{
	AssemblyItems input{
		InternalInstruction::CALLVALUE,
		u256(31),
		InternalInstruction::BYTE
	};
	checkCSE(input, {u256(0xff), InternalInstruction::CALLVALUE, InternalInstruction::AND});
}

BOOST_AUTO_TEST_CASE(cse_storage)
{
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
		u256(0),
		InternalInstruction::SLOAD,
		InternalInstruction::ADD,
		u256(0),
		InternalInstruction::SSTORE
	};
	checkCSE(input, {
		u256(0),
		InternalInstruction::DUP1,
		InternalInstruction::SLOAD,
		InternalInstruction::DUP1,
		InternalInstruction::ADD,
		InternalInstruction::SWAP1,
		InternalInstruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_noninterleaved_storage)
{
	// two stores to the same location should be replaced by only one store, even if we
	// read in the meantime
	AssemblyItems input{
		u256(7),
		InternalInstruction::DUP2,
		InternalInstruction::SSTORE,
		InternalInstruction::DUP1,
		InternalInstruction::SLOAD,
		u256(8),
		InternalInstruction::DUP3,
		InternalInstruction::SSTORE
	};
	checkCSE(input, {
		u256(8),
		InternalInstruction::DUP2,
		InternalInstruction::SSTORE,
		u256(7)
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage)
{
	// stores and reads to/from two unknown locations, should not optimize away the first store
	AssemblyItems input{
		u256(7),
		InternalInstruction::DUP2,
		InternalInstruction::SSTORE, // store to "DUP1"
		InternalInstruction::DUP2,
		InternalInstruction::SLOAD, // read from "DUP2", might be equal to "DUP1"
		u256(0),
		InternalInstruction::DUP3,
		InternalInstruction::SSTORE // store different value to "DUP1"
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_same_value)
{
	// stores and reads to/from two unknown locations, should not optimize away the first store
	// but it should optimize away the second, since we already know the value will be the same
	AssemblyItems input{
		u256(7),
		InternalInstruction::DUP2,
		InternalInstruction::SSTORE, // store to "DUP1"
		InternalInstruction::DUP2,
		InternalInstruction::SLOAD, // read from "DUP2", might be equal to "DUP1"
		u256(6),
		u256(1),
		InternalInstruction::ADD,
		InternalInstruction::DUP3,
		InternalInstruction::SSTORE // store same value to "DUP1"
	};
	checkCSE(input, {
		u256(7),
		InternalInstruction::DUP2,
		InternalInstruction::SSTORE,
		InternalInstruction::DUP2,
		InternalInstruction::SLOAD
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_at_known_location)
{
	// stores and reads to/from two known locations, should optimize away the first store,
	// because we know that the location is different
	AssemblyItems input{
		u256(0x70),
		u256(1),
		InternalInstruction::SSTORE, // store to 1
		u256(2),
		InternalInstruction::SLOAD, // read from 2, is different from 1
		u256(0x90),
		u256(1),
		InternalInstruction::SSTORE // store different value at 1
	};
	checkCSE(input, {
		u256(2),
		InternalInstruction::SLOAD,
		u256(0x90),
		u256(1),
		InternalInstruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_interleaved_storage_at_known_location_offset)
{
	// stores and reads to/from two locations which are known to be different,
	// should optimize away the first store, because we know that the location is different
	AssemblyItems input{
		u256(0x70),
		InternalInstruction::DUP2,
		u256(1),
		InternalInstruction::ADD,
		InternalInstruction::SSTORE, // store to "DUP1"+1
		InternalInstruction::DUP1,
		u256(2),
		InternalInstruction::ADD,
		InternalInstruction::SLOAD, // read from "DUP1"+2, is different from "DUP1"+1
		u256(0x90),
		InternalInstruction::DUP3,
		u256(1),
		InternalInstruction::ADD,
		InternalInstruction::SSTORE // store different value at "DUP1"+1
	};
	checkCSE(input, {
		u256(2),
		InternalInstruction::DUP2,
		InternalInstruction::ADD,
		InternalInstruction::SLOAD,
		u256(0x90),
		u256(1),
		InternalInstruction::DUP4,
		InternalInstruction::ADD,
		InternalInstruction::SSTORE
	});
}

BOOST_AUTO_TEST_CASE(cse_deep_stack)
{
	AssemblyItems input{
		InternalInstruction::ADD,
		InternalInstruction::SWAP1,
		InternalInstruction::POP,
		InternalInstruction::SWAP8,
		InternalInstruction::POP,
		InternalInstruction::SWAP8,
		InternalInstruction::POP,
		InternalInstruction::SWAP8,
		InternalInstruction::SWAP5,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
	};
	checkCSE(input, {
		InternalInstruction::SWAP4,
		InternalInstruction::SWAP12,
		InternalInstruction::SWAP3,
		InternalInstruction::SWAP11,
		InternalInstruction::POP,
		InternalInstruction::SWAP1,
		InternalInstruction::SWAP3,
		InternalInstruction::ADD,
		InternalInstruction::SWAP8,
		InternalInstruction::POP,
		InternalInstruction::SWAP6,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
		InternalInstruction::POP,
	});
}

BOOST_AUTO_TEST_CASE(cse_jumpi_no_jump)
{
	AssemblyItems input{
		u256(0),
		u256(1),
		InternalInstruction::DUP2,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI
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
		InternalInstruction::DUP2,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI
	};
	checkCSE(input, {
		u256(1),
		InternalInstruction::DUP1,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP
	});
}

BOOST_AUTO_TEST_CASE(cse_empty_keccak256)
{
	AssemblyItems input{
		u256(0),
		InternalInstruction::DUP2,
		InternalInstruction::KECCAK256
	};
	checkCSE(input, {
		u256(util::keccak256(bytesConstRef()))
	});
}

BOOST_AUTO_TEST_CASE(cse_partial_keccak256)
{
	AssemblyItems input{
		u256(0xabcd) << (256 - 16),
		u256(0),
		InternalInstruction::MSTORE,
		u256(2),
		u256(0),
		InternalInstruction::KECCAK256
	};
	checkCSE(input, {
		u256(0xabcd) << (256 - 16),
		u256(0),
		InternalInstruction::MSTORE,
		u256(util::keccak256(bytes{0xab, 0xcd}))
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_location)
{
	// Keccak-256 twice from same dynamic location
	AssemblyItems input{
		InternalInstruction::DUP2,
		InternalInstruction::DUP1,
		InternalInstruction::MSTORE,
		u256(64),
		InternalInstruction::DUP2,
		InternalInstruction::KECCAK256,
		u256(64),
		InternalInstruction::DUP3,
		InternalInstruction::KECCAK256
	};
	checkCSE(input, {
		InternalInstruction::DUP2,
		InternalInstruction::DUP1,
		InternalInstruction::MSTORE,
		u256(64),
		InternalInstruction::DUP2,
		InternalInstruction::KECCAK256,
		InternalInstruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content)
{
	// Keccak-256 twice from different dynamic location but with same content
	AssemblyItems input{
		InternalInstruction::DUP1,
		u256(0x80),
		InternalInstruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		u256(0x80),
		InternalInstruction::KECCAK256, // keccak256(m[128..(128+32)])
		InternalInstruction::DUP2,
		u256(12),
		InternalInstruction::MSTORE, // m[12] = DUP1
		u256(0x20),
		u256(12),
		InternalInstruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	checkCSE(input, {
		u256(0x80),
		InternalInstruction::DUP2,
		InternalInstruction::DUP2,
		InternalInstruction::MSTORE,
		u256(0x20),
		InternalInstruction::SWAP1,
		InternalInstruction::KECCAK256,
		u256(12),
		InternalInstruction::DUP3,
		InternalInstruction::SWAP1,
		InternalInstruction::MSTORE,
		InternalInstruction::DUP1
	});
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content_dynamic_store_in_between)
{
	// Keccak-256 twice from different dynamic location but with same content,
	// dynamic mstore in between, which forces us to re-calculate the hash
	AssemblyItems input{
		u256(0x80),
		InternalInstruction::DUP2,
		InternalInstruction::DUP2,
		InternalInstruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		InternalInstruction::DUP1,
		InternalInstruction::DUP3,
		InternalInstruction::KECCAK256, // keccak256(m[128..(128+32)])
		u256(12),
		InternalInstruction::DUP5,
		InternalInstruction::DUP2,
		InternalInstruction::MSTORE, // m[12] = DUP1
		InternalInstruction::DUP12,
		InternalInstruction::DUP14,
		InternalInstruction::MSTORE, // destroys memory knowledge
		InternalInstruction::SWAP2,
		InternalInstruction::SWAP1,
		InternalInstruction::SWAP2,
		InternalInstruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	checkCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_keccak256_twice_same_content_noninterfering_store_in_between)
{
	// Keccak-256 twice from different dynamic location but with same content,
	// dynamic mstore in between, but does not force us to re-calculate the hash
	AssemblyItems input{
		u256(0x80),
		InternalInstruction::DUP2,
		InternalInstruction::DUP2,
		InternalInstruction::MSTORE, // m[128] = DUP1
		u256(0x20),
		InternalInstruction::DUP1,
		InternalInstruction::DUP3,
		InternalInstruction::KECCAK256, // keccak256(m[128..(128+32)])
		u256(12),
		InternalInstruction::DUP5,
		InternalInstruction::DUP2,
		InternalInstruction::MSTORE, // m[12] = DUP1
		InternalInstruction::DUP12,
		u256(12 + 32),
		InternalInstruction::MSTORE, // does not destroy memory knowledge
		InternalInstruction::DUP13,
		u256(128 - 32),
		InternalInstruction::MSTORE, // does not destroy memory knowledge
		u256(0x20),
		u256(12),
		InternalInstruction::KECCAK256 // keccak256(m[12..(12+32)])
	};
	// if this changes too often, only count the number of SHA3 and MSTORE instructions
	AssemblyItems output = CSE(input);
	BOOST_CHECK_EQUAL(4, count(output.begin(), output.end(), AssemblyItem(InternalInstruction::MSTORE)));
	BOOST_CHECK_EQUAL(1, count(output.begin(), output.end(), AssemblyItem(InternalInstruction::KECCAK256)));
}

BOOST_AUTO_TEST_CASE(cse_with_initially_known_stack)
{
	evmasm::KnownState state = createInitialState(AssemblyItems{
		u256(0x12),
		u256(0x20),
		InternalInstruction::ADD
	});
	AssemblyItems input{
		u256(0x12 + 0x20)
	};
	checkCSE(input, AssemblyItems{InternalInstruction::DUP1}, state);
}

BOOST_AUTO_TEST_CASE(cse_equality_on_initially_known_stack)
{
	evmasm::KnownState state = createInitialState(AssemblyItems{InternalInstruction::DUP1});
	AssemblyItems input{
		InternalInstruction::EQ
	};
	AssemblyItems output = CSE(input, state);
	// check that it directly pushes 1 (true)
	BOOST_CHECK(find(output.begin(), output.end(), AssemblyItem(u256(1))) != output.end());
}

BOOST_AUTO_TEST_CASE(cse_access_previous_sequence)
{
	// Tests that the code generator detects whether it tries to access SLOAD instructions
	// from a sequenced expression which is not in its scope.
	evmasm::KnownState state = createInitialState(AssemblyItems{
		u256(0),
		InternalInstruction::SLOAD,
		u256(1),
		InternalInstruction::ADD,
		u256(0),
		InternalInstruction::SSTORE
	});
	// now stored: val_1 + 1 (value at sequence 1)
	// if in the following instructions, the SLOAD cresolves to "val_1 + 1",
	// this cannot be generated because we cannot load from sequence 1 anymore.
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
	};
	BOOST_CHECK_THROW(CSE(input, state), StackTooDeepException);
	// @todo for now, this throws an exception, but it should recover to the following
	// (or an even better version) at some point:
	// 0, SLOAD, 1, ADD, SSTORE, 0 SLOAD
}

BOOST_AUTO_TEST_CASE(cse_optimise_return)
{
	checkCSE(
		AssemblyItems{u256(0), u256(7), InternalInstruction::RETURN},
		AssemblyItems{InternalInstruction::STOP}
	);
}

BOOST_AUTO_TEST_CASE(control_flow_graph_remove_unused)
{
	// remove parts of the code that are unused
	AssemblyItems input{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		u256(7),
		AssemblyItem(Tag, 1),
	};
	checkCFG(input, {});
}

BOOST_AUTO_TEST_CASE(control_flow_graph_remove_unused_loop)
{
	AssemblyItems input{
		AssemblyItem(PushTag, 3),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(7),
		AssemblyItem(PushTag, 2),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(8),
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
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
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(2),
		AssemblyItem(PushTag, 3),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(3),
		AssemblyItem(PushTag, 2),
		InternalInstruction::JUMP,
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
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		InternalInstruction::JUMP,
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
		InternalInstruction::SWAP3,
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(6),
		InternalInstruction::SWAP3,
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(6),
		InternalInstruction::SWAP3,
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 3)
	};
	BlockDeduplicator deduplicator(input);
	deduplicator.deduplicate();

	set<u256> pushTags;
	for (AssemblyItem const& item: input)
		if (item.type() == PushTag)
			pushTags.insert(item.data());
	BOOST_CHECK_EQUAL(pushTags.size(), 2);
}

BOOST_AUTO_TEST_CASE(block_deduplicator_assign_immutable_same)
{
	AssemblyItems blocks{
		AssemblyItem(Tag, 1),
		u256(42),
		AssemblyItem{AssignImmutable, 0x1234},
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(42),
		AssemblyItem{AssignImmutable, 0x1234},
		InternalInstruction::JUMP
	};

	AssemblyItems input = AssemblyItems{
		AssemblyItem(PushTag, 2),
		AssemblyItem(PushTag, 1),
	} + blocks;
	AssemblyItems output = AssemblyItems{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 1),
	} + blocks;
	BlockDeduplicator deduplicator(input);
	deduplicator.deduplicate();
	BOOST_CHECK_EQUAL_COLLECTIONS(input.begin(), input.end(), output.begin(), output.end());
}

BOOST_AUTO_TEST_CASE(block_deduplicator_assign_immutable_different_value)
{
	AssemblyItems input{
		AssemblyItem(PushTag, 2),
		AssemblyItem(PushTag, 1),
		AssemblyItem(Tag, 1),
		u256(42),
		AssemblyItem{AssignImmutable, 0x1234},
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(23),
		AssemblyItem{AssignImmutable, 0x1234},
		InternalInstruction::JUMP
	};
	BlockDeduplicator deduplicator(input);
	BOOST_CHECK(!deduplicator.deduplicate());
}

BOOST_AUTO_TEST_CASE(block_deduplicator_assign_immutable_different_hash)
{
	AssemblyItems input{
		AssemblyItem(PushTag, 2),
		AssemblyItem(PushTag, 1),
		AssemblyItem(Tag, 1),
		u256(42),
		AssemblyItem{AssignImmutable, 0x1234},
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(42),
		AssemblyItem{AssignImmutable, 0xABCD},
		InternalInstruction::JUMP
	};
	BlockDeduplicator deduplicator(input);
	BOOST_CHECK(!deduplicator.deduplicate());
}

BOOST_AUTO_TEST_CASE(block_deduplicator_loops)
{
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		InternalInstruction::JUMPI,
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(5),
		u256(6),
		InternalInstruction::SSTORE,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		InternalInstruction::SSTORE,
		AssemblyItem(PushTag, 2),
		InternalInstruction::JUMP,
	};
	BlockDeduplicator deduplicator(input);
	deduplicator.deduplicate();

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
		InternalInstruction::JUMP,
		u256(0),
		InternalInstruction::SLOAD,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		InternalInstruction::SSTORE,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		u256(5),
		u256(6)
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 2),
		u256(5),
		u256(6),
		InternalInstruction::SSTORE,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP
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
		InternalInstruction::DUP1,
		u256(5),
		InternalInstruction::DUP1,
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
		InternalInstruction::CALLDATASIZE,
		InternalInstruction::LT,
		InternalInstruction::POP
	};
	PeepholeOptimiser peepOpt(items);
	for (size_t i = 0; i < 3; i++)
		BOOST_CHECK(peepOpt.optimise());
	BOOST_CHECK(items.empty());
}

BOOST_AUTO_TEST_CASE(peephole_commutative_swap1)
{
	vector<InternalInstruction> ops{
		InternalInstruction::ADD,
		InternalInstruction::MUL,
		InternalInstruction::EQ,
		InternalInstruction::AND,
		InternalInstruction::OR,
		InternalInstruction::XOR
	};
	for (InternalInstruction const op: ops)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			InternalInstruction::SWAP1,
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
	vector<InternalInstruction> ops{
		InternalInstruction::SUB,
		InternalInstruction::DIV,
		InternalInstruction::SDIV,
		InternalInstruction::MOD,
		InternalInstruction::SMOD,
		InternalInstruction::EXP
	};
	for (InternalInstruction const op: ops)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			InternalInstruction::SWAP1,
			op,
			u256(4),
			u256(5)
		};
		AssemblyItems expectation{
			u256(1),
			u256(2),
			InternalInstruction::SWAP1,
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
	map<InternalInstruction, InternalInstruction> swappableOps{
		{ InternalInstruction::LT, InternalInstruction::GT },
		{ InternalInstruction::GT, InternalInstruction::LT },
		{ InternalInstruction::SLT, InternalInstruction::SGT },
		{ InternalInstruction::SGT, InternalInstruction::SLT }
	};

	for (auto const& op: swappableOps)
	{
		AssemblyItems items{
			u256(1),
			u256(2),
			InternalInstruction::SWAP1,
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
		InternalInstruction::BALANCE,
		u256(0),
		InternalInstruction::NOT,
		InternalInstruction::AND,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI
	};
	AssemblyItems expectation{
		AssemblyItem(Tag, 1),
		InternalInstruction::BALANCE,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI
	};
	PeepholeOptimiser peepOpt(items);
	BOOST_REQUIRE(peepOpt.optimise());
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}


BOOST_AUTO_TEST_CASE(peephole_iszero_iszero_jumpi)
{
	AssemblyItems items{
		AssemblyItem(Tag, 1),
		u256(0),
		InternalInstruction::CALLDATALOAD,
		InternalInstruction::ISZERO,
		InternalInstruction::ISZERO,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI,
		u256(0),
		u256(0x20),
		InternalInstruction::RETURN
	};
	AssemblyItems expectation{
		AssemblyItem(Tag, 1),
		u256(0),
		InternalInstruction::CALLDATALOAD,
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI,
		u256(0),
		u256(0x20),
		InternalInstruction::RETURN
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
		InternalInstruction::JUMP,
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		u256(5),
		u256(6),
		AssemblyItem(Tag, 1),
		InternalInstruction::JUMP
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

	Assembly main{false, {}};
	AssemblyPointer sub = make_shared<Assembly>(true, string{});

	sub->append(u256(1));
	auto t1 = sub->newTag();
	sub->append(t1);
	sub->append(u256(2));
	sub->append(InternalInstruction::JUMP);
	auto t2 = sub->newTag();
	sub->append(t2); // Identical to T1, will be unified
	sub->append(u256(2));
	sub->append(InternalInstruction::JUMP);
	auto t3 = sub->newTag();
	sub->append(t3);
	auto t4 = sub->newTag();
	sub->append(t4);
	auto t5 = sub->newTag();
	sub->append(t5); // This will be removed
	sub->append(u256(7));
	sub->append(t4.pushTag());
	sub->append(InternalInstruction::JUMP);

	size_t subId = static_cast<size_t>(main.appendSubroutine(sub).data());
	main.append(t1.toSubAssemblyTag(subId));
	main.append(t1.toSubAssemblyTag(subId));
	main.append(u256(8));

	Assembly::OptimiserSettings settings;
	settings.runInliner = false;
	settings.runJumpdestRemover = true;
	settings.runPeephole = true;
	settings.runDeduplicate = true;
	settings.runCSE = true;
	settings.runConstantOptimiser = true;
	settings.evmVersion = solidity::test::CommonOptions::get().evmVersion();
	settings.expectedExecutionsPerDeployment = OptimiserSettings{}.expectedExecutionsPerDeployment;

	main.optimise(settings);

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
		u256(1), t1.tag(), u256(2), InternalInstruction::JUMP, t4.tag(), u256(7), t4.pushTag(), InternalInstruction::JUMP
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
		InternalInstruction::DUP2,
		InternalInstruction::SUB
	}, {
		InternalInstruction::DUP1
	});

	checkCSE({
		InternalInstruction::DUP1,
		u256(0),
		InternalInstruction::SUB
	}, {
		u256(0),
		InternalInstruction::DUP2,
		InternalInstruction::SWAP1,
		InternalInstruction::SUB
	});
}

BOOST_AUTO_TEST_CASE(cse_simple_verbatim)
{
	auto verbatim = AssemblyItem{bytes{1, 2, 3, 4, 5}, 0, 0};
	AssemblyItems input{verbatim};
	checkCSE(input, input);
	checkFullCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_mload_pop)
{
	AssemblyItems input{
		u256(1000),
		InternalInstruction::MLOAD,
		InternalInstruction::POP,
	};

	AssemblyItems output{
	};

	checkCSE(input, output);
	checkFullCSE(input, output);
}

BOOST_AUTO_TEST_CASE(cse_verbatim_mload)
{
	auto verbatim = AssemblyItem{bytes{1, 2, 3, 4, 5}, 0, 0};
	AssemblyItems input{
		u256(1000),
		InternalInstruction::MLOAD,	// Should not be removed
		InternalInstruction::POP,
		verbatim,
		u256(1000),
		InternalInstruction::MLOAD,	// Should not be removed
		InternalInstruction::POP,
	};

	checkFullCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_sload_verbatim_dup)
{
	auto verbatim = AssemblyItem{bytes{1, 2, 3, 4, 5}, 0, 0};
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
		u256(0),
		InternalInstruction::SLOAD,
		verbatim
	};

	AssemblyItems output{
		u256(0),
		InternalInstruction::SLOAD,
		InternalInstruction::DUP1,
		verbatim
	};

	checkCSE(input, output);
	checkFullCSE(input, output);
}

BOOST_AUTO_TEST_CASE(cse_verbatim_sload_sideeffect)
{
	auto verbatim = AssemblyItem{bytes{1, 2, 3, 4, 5}, 0, 0};
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
		verbatim,
		u256(0),
		InternalInstruction::SLOAD,
	};

	checkFullCSE(input, input);
}

BOOST_AUTO_TEST_CASE(cse_verbatim_eq)
{
	auto verbatim = AssemblyItem{bytes{1, 2, 3, 4, 5}, 0, 0};
	AssemblyItems input{
		u256(0),
		InternalInstruction::SLOAD,
		verbatim,
		InternalInstruction::DUP1,
		InternalInstruction::EQ
	};

	checkFullCSE(input, input);
}

BOOST_AUTO_TEST_CASE(verbatim_knownstate)
{
	KnownState state = createInitialState(AssemblyItems{
			InternalInstruction::DUP1,
			InternalInstruction::DUP2,
			InternalInstruction::DUP3,
			InternalInstruction::DUP4
		});
	map<int, unsigned> const& stackElements = state.stackElements();

	BOOST_CHECK(state.stackHeight() == 4);
	// One more than stack height because of the initial unknown element.
	BOOST_CHECK(stackElements.size() == 5);
	BOOST_CHECK(stackElements.count(0));
	unsigned initialElement = stackElements.at(0);
	// Check if all the DUPs were correctly matched to the same class.
	for (auto const& height: {1, 2, 3, 4})
		BOOST_CHECK(stackElements.at(height) == initialElement);

	auto verbatim2i5o = AssemblyItem{bytes{1, 2, 3, 4, 5}, 2, 5};
	state.feedItem(verbatim2i5o);

	BOOST_CHECK(state.stackHeight() == 7);
	// Stack elements
	// Before verbatim: {{0, x}, {1, x}, {2, x}, {3, x}, {4, x}}
	// After verbatim: {{0, x}, {1, x}, {2, x}, {3, a}, {4, b}, {5, c}, {6, d}, {7, e}}
	BOOST_CHECK(stackElements.size() == 8);

	for (auto const& height: {1, 2})
		BOOST_CHECK(stackElements.at(height) == initialElement);

	for (auto const& height: {3, 4, 5, 6, 7})
		BOOST_CHECK(stackElements.at(height) != initialElement);

	for (auto const& height1: {3, 4, 5, 6, 7})
		for (auto const& height2: {3, 4, 5, 6, 7})
			if (height1 < height2)
				BOOST_CHECK(stackElements.at(height1) != stackElements.at(height2));
}

BOOST_AUTO_TEST_CASE(cse_remove_redundant_shift_masking)
{
	if (!solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
		return;

	for (unsigned i = 1; i < 256; i++)
	{
		checkCSE({
			u256(boost::multiprecision::pow(u256(2), i) - 1),
			InternalInstruction::CALLVALUE,
			u256(256-i),
			InternalInstruction::SHR,
			InternalInstruction::AND
		}, {
			InternalInstruction::CALLVALUE,
			u256(256-i),
			InternalInstruction::SHR,
		});

		checkCSE({
			InternalInstruction::CALLVALUE,
			u256(256-i),
			InternalInstruction::SHR,
			u256(boost::multiprecision::pow(u256(2), i)-1),
			InternalInstruction::AND
		}, {
			InternalInstruction::CALLVALUE,
			u256(256-i),
			InternalInstruction::SHR,
		});
	}

	// Check that opt. does NOT trigger
	for (unsigned i = 1; i < 255; i++)
	{
		checkCSE({
			u256(boost::multiprecision::pow(u256(2), i) - 1),
			InternalInstruction::CALLVALUE,
			u256(255-i),
			InternalInstruction::SHR,
			InternalInstruction::AND
		}, { // Opt. did some reordering
			InternalInstruction::CALLVALUE,
			u256(255-i),
			InternalInstruction::SHR,
			u256(boost::multiprecision::pow(u256(2), i)-1),
			InternalInstruction::AND
		});

		checkCSE({
			InternalInstruction::CALLVALUE,
			u256(255-i),
			InternalInstruction::SHR,
			u256(boost::multiprecision::pow(u256(2), i)-1),
			InternalInstruction::AND
		}, { // Opt. did some reordering
			u256(boost::multiprecision::pow(u256(2), i)-1),
			InternalInstruction::CALLVALUE,
			u256(255-i),
			InternalInstruction::SHR,
			InternalInstruction::AND
		});
	}

	//(x >> (31*8)) & 0xffffffff
	checkCSE({
		InternalInstruction::CALLVALUE,
		u256(31*8),
		InternalInstruction::SHR,
		u256(0xffffffff),
		InternalInstruction::AND
	}, {
		InternalInstruction::CALLVALUE,
		u256(31*8),
		InternalInstruction::SHR
	});
}

BOOST_AUTO_TEST_CASE(cse_remove_unwanted_masking_of_address)
{
	vector<InternalInstruction> ops{
		InternalInstruction::ADDRESS,
		InternalInstruction::CALLER,
		InternalInstruction::ORIGIN,
		InternalInstruction::COINBASE
	};
	for (auto const& op: ops)
	{
		checkCSE({
			u256("0xffffffffffffffffffffffffffffffffffffffff"),
			op,
			InternalInstruction::AND
		}, {
			op
		});

		checkCSE({
			op,
			u256("0xffffffffffffffffffffffffffffffffffffffff"),
			InternalInstruction::AND
		}, {
			op
		});

		// do not remove mask for other masking
		checkCSE({
			u256(1234),
			op,
			InternalInstruction::AND
		}, {
			op,
			u256(1234),
			InternalInstruction::AND
		});

		checkCSE({
			op,
			u256(1234),
			InternalInstruction::AND
		}, {
			u256(1234),
			op,
			InternalInstruction::AND
		});
	}

	// leave other opcodes untouched
	checkCSE({
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		InternalInstruction::CALLVALUE,
		InternalInstruction::AND
	}, {
		InternalInstruction::CALLVALUE,
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		InternalInstruction::AND
	});

	checkCSE({
		InternalInstruction::CALLVALUE,
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		InternalInstruction::AND
	}, {
		u256("0xffffffffffffffffffffffffffffffffffffffff"),
		InternalInstruction::CALLVALUE,
		InternalInstruction::AND
	});
}

BOOST_AUTO_TEST_CASE(cse_replace_too_large_shift)
{
	if (!solidity::test::CommonOptions::get().evmVersion().hasBitwiseShifting())
		return;

	checkCSE({
		InternalInstruction::CALLVALUE,
		u256(299),
		InternalInstruction::SHL
	}, {
		u256(0)
	});

	checkCSE({
		InternalInstruction::CALLVALUE,
		u256(299),
		InternalInstruction::SHR
	}, {
		u256(0)
	});

	checkCSE({
		InternalInstruction::CALLVALUE,
		u256(255),
		InternalInstruction::SHL
	}, {
		InternalInstruction::CALLVALUE,
		u256(255),
		InternalInstruction::SHL
	});

	checkCSE({
		InternalInstruction::CALLVALUE,
		u256(255),
		InternalInstruction::SHR
	}, {
		InternalInstruction::CALLVALUE,
		u256(255),
		InternalInstruction::SHR
	});
}

BOOST_AUTO_TEST_CASE(inliner)
{
	AssemblyItem jumpInto{InternalInstruction::JUMP};
	jumpInto.setJumpType(AssemblyItem::JumpType::IntoFunction);
	AssemblyItem jumpOutOf{InternalInstruction::JUMP};
	jumpOutOf.setJumpType(AssemblyItem::JumpType::OutOfFunction);
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		jumpInto,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		InternalInstruction::CALLVALUE,
		InternalInstruction::SWAP1,
		jumpOutOf,
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		InternalInstruction::CALLVALUE,
		InternalInstruction::SWAP1,
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		InternalInstruction::CALLVALUE,
		InternalInstruction::SWAP1,
		jumpOutOf,
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}


BOOST_AUTO_TEST_CASE(inliner_no_inline_type)
{
	// Will not inline due to jump types.
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		InternalInstruction::CALLVALUE,
		InternalInstruction::SWAP1,
		InternalInstruction::JUMP,
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		items.begin(), items.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_no_inline)
{
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::CALLVALUE,
		InternalInstruction::JUMPI,
		InternalInstruction::JUMP,
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::CALLVALUE,
		InternalInstruction::JUMPI,
		InternalInstruction::JUMP,
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}


BOOST_AUTO_TEST_CASE(inliner_single_jump)
{
	AssemblyItem jumpInto{InternalInstruction::JUMP};
	jumpInto.setJumpType(AssemblyItem::JumpType::IntoFunction);
	AssemblyItem jumpOutOf{InternalInstruction::JUMP};
	jumpOutOf.setJumpType(AssemblyItem::JumpType::OutOfFunction);
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		jumpInto,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		jumpOutOf,
	};
	AssemblyItems expectation{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		jumpOutOf,
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_end_of_bytecode)
{
	AssemblyItem jumpInto{InternalInstruction::JUMP};
	jumpInto.setJumpType(AssemblyItem::JumpType::IntoFunction);
	// Cannot inline, since the block at Tag_2 does not end in a jump.
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		jumpInto,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		items.begin(), items.end()
	);
}


BOOST_AUTO_TEST_CASE(inliner_cse_break)
{
	AssemblyItem jumpInto{InternalInstruction::JUMP};
	jumpInto.setJumpType(AssemblyItem::JumpType::IntoFunction);
	AssemblyItem jumpOutOf{InternalInstruction::JUMP};
	jumpOutOf.setJumpType(AssemblyItem::JumpType::OutOfFunction);
	// Could be inlined, but we only consider non-CSE-breaking blocks ending in JUMP so far.
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		AssemblyItem(PushTag, 2),
		jumpInto,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP,
		AssemblyItem(Tag, 2),
		InternalInstruction::STOP, // CSE breaking instruction
		jumpOutOf
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		items.begin(), items.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_stop)
{
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP
	};
	AssemblyItems expectation{
		InternalInstruction::STOP,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_stop_jumpi)
{
	// Because of `jumpi`, will not be inlined.
	AssemblyItems items{
		u256(1),
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMPI,
		AssemblyItem(Tag, 1),
		InternalInstruction::STOP
	};
	AssemblyItems expectation = items;
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_revert)
{
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(0),
		InternalInstruction::DUP1,
		InternalInstruction::REVERT
	};
	AssemblyItems expectation{
		u256(0),
		InternalInstruction::DUP1,
		InternalInstruction::REVERT,
		AssemblyItem(Tag, 1),
		u256(0),
		InternalInstruction::DUP1,
		InternalInstruction::REVERT
	};

	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_revert_increased_datagas)
{
	// Inlining this would increase data gas (5 bytes v/s 4 bytes), therefore, skipped.
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		u256(0),
		u256(0),
		InternalInstruction::REVERT
	};

	AssemblyItems expectation = items;
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}

BOOST_AUTO_TEST_CASE(inliner_invalid)
{
	AssemblyItems items{
		AssemblyItem(PushTag, 1),
		InternalInstruction::JUMP,
		AssemblyItem(Tag, 1),
		InternalInstruction::INVALID
	};

	AssemblyItems expectation = {
		InternalInstruction::INVALID,
		AssemblyItem(Tag, 1),
		InternalInstruction::INVALID
	};
	Inliner{items, {}, Assembly::OptimiserSettings{}.expectedExecutionsPerDeployment, false, {}}.optimise();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		items.begin(), items.end(),
		expectation.begin(), expectation.end()
	);
}


BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
