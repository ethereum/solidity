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
 * Unit tests for the SSA CFG invariant checks. Each test builds a valid graph, breaks exactly one
 * invariant and asserts that the corresponding check reports it.
 *
 * The whole file only exists in SLOW_DEBUG builds, where the checks themselves are compiled in.
 */

#include <test/Common.h>

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/SSACFG.h>

#include <boost/test/unit_test.hpp>

#ifdef SLOW_DEBUG

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace solidity::yul::ssa;

namespace solidity::yul::test
{

namespace
{

using BasicBlock = SSACFG::BasicBlock;
using Call = InstructionStore::Call;
using BuiltinCall = InstructionStore::BuiltinCall;
using LiteralPayload = InstructionStore::LiteralPayload;
using UpsilonPayload = InstructionStore::UpsilonPayload;
using Payload = InstructionStore::Payload;

void expectAssertion(
	std::function<void()> const& _call,
	std::vector<std::string> const& _expectedFragments,
	std::vector<std::string> const& _forbiddenFragments = {}
)
{
	try
	{
		_call();
	}
	catch (YulAssertion const& _exception)
	{
		std::string const* comment = _exception.comment();
		BOOST_REQUIRE_MESSAGE(comment, "Assertion thrown without a message.");
		for (std::string const& fragment: _expectedFragments)
			BOOST_CHECK_MESSAGE(
				comment->find(fragment) != std::string::npos,
				"Expected an assertion containing '" + fragment + "', got: " + *comment
			);
		for (std::string const& fragment: _forbiddenFragments)
			BOOST_CHECK_MESSAGE(
				comment->find(fragment) == std::string::npos,
				"Expected an assertion not containing '" + fragment + "', got: " + *comment
			);
		return;
	}
	BOOST_FAIL("Expected an assertion, but nothing was thrown.");
}

/// Owns the graphs under test and hands out pre-built, invariant-clean graph shapes.
struct Fixture
{
	EVMDialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(solidity::test::CommonOptions::get().evmVersion());
	ControlFlowGraphs cfgs;

	/// An empty graph name means "main graph".
	SSACFG& addGraph(std::string _name = {})
	{
		cfgs.functionGraphs.push_back(std::make_unique<SSACFG>(dialect));
		SSACFG& cfg = *cfgs.functionGraphs.back();
		cfg.name = std::move(_name);
		cfg.entry = cfg.makeBlock(nullptr);
		return cfg;
	}

	BuiltinHandle builtin(std::string_view const _name) const
	{
		auto const handle = dialect.findBuiltin(_name);
		BOOST_REQUIRE_MESSAGE(handle.has_value(), "Builtin not found in the dialect.");
		return *handle;
	}

	/// entry --(cond)--> left/right --> merge, with a phi in merge fed from both sides.
	struct Diamond
	{
		BlockId entry;
		BlockId left;
		BlockId right;
		BlockId merge;
		InstId condition;
		InstId leftValue;
		InstId rightValue;
		InstId phi;
		InstId leftUpsilon;
		InstId rightUpsilon;
	};

	Diamond buildDiamond(SSACFG& _cfg) const
	{
		Diamond diamond;
		diamond.entry = _cfg.entry;
		diamond.left = _cfg.makeBlock(nullptr);
		diamond.right = _cfg.makeBlock(nullptr);
		diamond.merge = _cfg.makeBlock(nullptr);

		diamond.condition = _cfg.newLiteral(nullptr, 1);
		diamond.leftValue = _cfg.newLiteral(nullptr, 10);
		diamond.rightValue = _cfg.newLiteral(nullptr, 20);

		_cfg.block(diamond.entry).exit = BasicBlock::ConditionalJump{diamond.condition, diamond.left, diamond.right};
		_cfg.block(diamond.left).entries = {diamond.entry};
		_cfg.block(diamond.left).exit = BasicBlock::Jump{diamond.merge};
		_cfg.block(diamond.right).entries = {diamond.entry};
		_cfg.block(diamond.right).exit = BasicBlock::Jump{diamond.merge};
		_cfg.block(diamond.merge).entries = {diamond.left, diamond.right};
		_cfg.block(diamond.merge).exit = BasicBlock::Terminated{};

		diamond.phi = _cfg.newPhi(diamond.merge);
		diamond.leftUpsilon = _cfg.emitUpsilon(diamond.left, diamond.leftValue, diamond.phi);
		diamond.rightUpsilon = _cfg.emitUpsilon(diamond.right, diamond.rightValue, diamond.phi);
		return diamond;
	}

	struct CallCluster
	{
		BlockId block;
		InstId call;
		InstId firstProjection;
		InstId secondProjection;
	};

	CallCluster addTwoReturnCall(SSACFG& _cfg, BlockId const _block) const
	{
		CallCluster cluster;
		cluster.block = _block;
		cluster.call = _cfg.makeCallWithProjections(_block, Call{1, true, 2}, /* inputs= */ {} , /*_numReturns=*/ 2);
		cluster.firstProjection = InstId{cluster.call.value + 1};
		cluster.secondProjection = InstId{cluster.call.value + 2};
		return cluster;
	}

	/// An ordinary input-less instruction, for tests that just need a slot to corrupt.
	InstId addScratchInstruction(SSACFG& _cfg, BlockId const _block) const
	{
		return _cfg.makeBuiltinCallWithProjections(_block, BuiltinCall{builtin("caller"), {}}, {}, 1);
	}

	/// entry --> second, both live and reachable.
	static BlockId addSecondBlock(SSACFG& _cfg)
	{
		BlockId const second = _cfg.makeBlock(nullptr);
		_cfg.block(_cfg.entry).exit = BasicBlock::Jump{second};
		_cfg.block(second).entries = {_cfg.entry};
		return second;
	}

	/// Turns an instruction into a Nop in place, without going through the store's bookkeeping.
	static void makeNop(SSACFG& _cfg, InstId const _id)
	{
		auto& instruction = _cfg.inst(_id);
		instruction.opcode = InstOpcode::Nop;
		instruction.inputs.clear();
		instruction.payload.reset();
	}

	static void unschedule(SSACFG& _cfg, InstId const _id)
	{
		auto& instructions = _cfg.block(_cfg.inst(_id).block).instructions;
		auto const it = std::find(instructions.begin(), instructions.end(), _id);
		BOOST_REQUIRE(it != instructions.end());
		instructions.erase(it);
	}

	/// Reschedules an instruction at the end of another block, keeping Inst::block in sync.
	static void moveTo(SSACFG& _cfg, InstId const _id, BlockId const _target)
	{
		unschedule(_cfg, _id);
		_cfg.inst(_id).block = _target;
		_cfg.block(_target).instructions.push_back(_id);
	}
};

}

BOOST_AUTO_TEST_SUITE(YulSSACFGInvariants)

// Sanity checks: every shape the tests below start from has to be invariant-clean, otherwise a
// broken-invariant test could pass for the wrong reason.

BOOST_AUTO_TEST_CASE(sanity_diamond_and_function_graph)
{
	Fixture fixture;
	SSACFG& main = fixture.addGraph();
	fixture.buildDiamond(main);

	SSACFG& function = fixture.addGraph("f");
	function.numReturns = 2;
	InstId const a = function.newLiteral(nullptr, 1);
	InstId const b = function.newLiteral(nullptr, 2);
	function.block(function.entry).exit = BasicBlock::FunctionReturn{{a, b}};

	BOOST_CHECK_NO_THROW(fixture.cfgs.checkInvariants());
	BOOST_CHECK_NO_THROW(main.checkAllBlocksReachable());
	BOOST_CHECK_NO_THROW(function.checkAllBlocksReachable());
}

BOOST_AUTO_TEST_CASE(sanity_two_return_call)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	fixture.addTwoReturnCall(cfg, cfg.entry);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_two_call_clusters)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	fixture.addTwoReturnCall(cfg, cfg.entry);
	fixture.addTwoReturnCall(cfg, cfg.entry);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_memory_guard_and_unreachable)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.makeMemoryGuard(cfg.entry);
	cfg.unreachableValue();
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_scratch_instruction)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	fixture.addScratchInstruction(cfg, cfg.entry);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_identity_and_nop)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const negation = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {literal}, 1);
	InstId const discard = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("pop"), {}}, {literal}, 0);
	cfg.replaceWithIdentity(negation, literal);
	cfg.replaceWithNop(discard);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_function_arguments)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.arguments.push_back(cfg.newFunctionArgument());
	cfg.arguments.push_back(cfg.newFunctionArgument());
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_terminating_builtin)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("stop"), {}}, {}, 0);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
}

BOOST_AUTO_TEST_CASE(sanity_two_blocks)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	Fixture::addSecondBlock(cfg);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
	BOOST_CHECK_NO_THROW(cfg.checkAllBlocksReachable());
}

// ControlFlowGraphs::checkInvariants

BOOST_AUTO_TEST_CASE(no_graphs_at_all)
{
	Fixture fixture;
	expectAssertion([&] { fixture.cfgs.checkInvariants(); }, {"No control flow graphs"});
}

BOOST_AUTO_TEST_CASE(main_slot_holds_a_named_graph)
{
	Fixture fixture;
	fixture.addGraph("f");
	expectAssertion([&] { fixture.cfgs.checkInvariants(); }, {"Graph in the main slot is named"});
}

BOOST_AUTO_TEST_CASE(two_main_graphs)
{
	Fixture fixture;
	fixture.addGraph();
	fixture.addGraph();
	expectAssertion([&] { fixture.cfgs.checkInvariants(); }, {"second main graph"});
}

BOOST_AUTO_TEST_CASE(duplicate_graph_names)
{
	Fixture fixture;
	fixture.addGraph();
	fixture.addGraph("f");
	fixture.addGraph("f");
	expectAssertion([&] { fixture.cfgs.checkInvariants(); }, {"Duplicate CFG name"});
}

BOOST_AUTO_TEST_CASE(operand_is_the_empty_id)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.leftUpsilon).inputs[0] = InstId{};
	expectAssertion([&] { cfg.checkInvariants(); }, {"Empty InstId"});
}

BOOST_AUTO_TEST_CASE(operand_is_out_of_bounds)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.leftUpsilon).inputs[0] = InstId{1000};
	expectAssertion([&] { cfg.checkInvariants(); }, {"out of bounds", "inputs of"});
}

BOOST_AUTO_TEST_CASE(operand_is_a_tombstone)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	// Flipping the opcode directly keeps the slot addressable, unlike InstructionStore::tombstone
	cfg.inst(diamond.leftValue).opcode = InstOpcode::Tombstone;
	expectAssertion([&] { cfg.checkInvariants(); }, {"Tombstone", "referenced"});
}

BOOST_AUTO_TEST_CASE(const_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.condition).inputs.push_back(diamond.leftValue);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Const", "has inputs"});
}

BOOST_AUTO_TEST_CASE(const_without_literal_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.condition).payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Const", "lacks"});
}

BOOST_AUTO_TEST_CASE(phi_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.phi).inputs.push_back(diamond.leftValue);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Phi", "has inputs"});
}

BOOST_AUTO_TEST_CASE(phi_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.phi).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Phi", "payload"});
}

BOOST_AUTO_TEST_CASE(upsilon_without_exactly_one_input)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.leftUpsilon).inputs.clear();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Upsilon", "one", "input"});
}

BOOST_AUTO_TEST_CASE(upsilon_without_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.leftUpsilon).payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Upsilon", "lacks", "payload"});
}

BOOST_AUTO_TEST_CASE(upsilon_targeting_a_non_phi)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	std::get<UpsilonPayload>(*cfg.inst(diamond.leftUpsilon).payload).targetPhi = diamond.leftValue;
	expectAssertion([&] { cfg.checkInvariants(); }, {"Upsilon", "targets non-phi"});
}

BOOST_AUTO_TEST_CASE(builtin_call_without_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("stop"), {}}, {}, 0);
	cfg.inst(call).payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"BuiltinCall", "lacks", "payload"});
}

BOOST_AUTO_TEST_CASE(call_without_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	cfg.inst(cluster.call).payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Call", "lacks", "payload"});
}

BOOST_AUTO_TEST_CASE(unreachable_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const unreachable = cfg.unreachableValue();
	cfg.inst(unreachable).inputs.push_back(literal);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Unreachable", "has inputs"});
}

BOOST_AUTO_TEST_CASE(unreachable_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const unreachable = cfg.unreachableValue();
	cfg.inst(unreachable).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Unreachable", "payload"});
}

BOOST_AUTO_TEST_CASE(function_arg_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	InstId const argument = cfg.newFunctionArgument();
	cfg.arguments.push_back(argument);
	cfg.inst(argument).inputs.push_back(argument);
	expectAssertion([&] { cfg.checkInvariants(); }, {"FunctionArg", "has", "inputs"});
}

BOOST_AUTO_TEST_CASE(function_arg_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	InstId const argument = cfg.newFunctionArgument();
	cfg.arguments.push_back(argument);
	cfg.inst(argument).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"FunctionArg", "has a payload"});
}

BOOST_AUTO_TEST_CASE(projection_without_exactly_one_input)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	cfg.inst(cluster.firstProjection).inputs.clear();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Projection", "exactly one input"});
}

BOOST_AUTO_TEST_CASE(projection_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	cfg.inst(cluster.firstProjection).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Projection", "has a payload"});
}

BOOST_AUTO_TEST_CASE(projection_preceding_its_producer)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	InstId const later = cfg.newLiteral(nullptr, 1);
	cfg.inst(cluster.firstProjection).inputs[0] = later;
	expectAssertion([&] { cfg.checkInvariants(); }, {"precedes its producer"});
}

BOOST_AUTO_TEST_CASE(projection_of_a_non_operation)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	// MemoryGuard is an operation, but not one that can produce projections
	cfg.inst(cluster.call).opcode = InstOpcode::MemoryGuard;
	cfg.inst(cluster.call).payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"of projection", "is not an operation"});
}

BOOST_AUTO_TEST_CASE(projection_of_a_single_return_producer)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	Fixture::makeNop(cfg, cluster.secondProjection);
	expectAssertion([&] { cfg.checkInvariants(); }, {"on single-return producer"});
}

BOOST_AUTO_TEST_CASE(projection_index_out_of_range)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	// A gap in the cluster caps numTrailingProjections, so the stray projection behind the gap
	// ends up with an index the producer does not have.
	InstId const gap = cfg.newLiteral(nullptr, 1);
	InstId const stray = cfg.newLiteral(nullptr, 2);
	BOOST_REQUIRE_EQUAL(gap.value, cluster.secondProjection.value + 1);
	BOOST_REQUIRE_EQUAL(stray.value, cluster.secondProjection.value + 2);
	auto& strayInst = cfg.inst(stray);
	strayInst.opcode = InstOpcode::Projection;
	strayInst.inputs = {cluster.call};
	strayInst.payload.reset();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Projection index of", "out of range"});
}

BOOST_AUTO_TEST_CASE(identity_without_exactly_one_input)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {literal}, 1);
	cfg.replaceWithIdentity(call, literal);
	cfg.inst(call).inputs.clear();
	expectAssertion([&] { cfg.checkInvariants(); }, {"Identity", "needs exactly one input"});
}

BOOST_AUTO_TEST_CASE(identity_self_loop)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {literal}, 1);
	cfg.replaceWithIdentity(call, literal);
	cfg.inst(call).inputs[0] = call;
	expectAssertion([&] { cfg.checkInvariants(); }, {"Identity", "is a self-loop"});
}

BOOST_AUTO_TEST_CASE(identity_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {literal}, 1);
	cfg.replaceWithIdentity(call, literal);
	cfg.inst(call).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Identity", "has a payload"});
}

BOOST_AUTO_TEST_CASE(nop_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("pop"), {}}, {literal}, 0);
	cfg.replaceWithNop(call);
	cfg.inst(call).inputs.push_back(literal);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Nop", "has inputs"});
}

BOOST_AUTO_TEST_CASE(nop_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const call = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("pop"), {}}, {literal}, 0);
	cfg.replaceWithNop(call);
	cfg.inst(call).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Nop", "has a payload"});
}

BOOST_AUTO_TEST_CASE(memory_guard_with_inputs)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const memoryGuard = cfg.makeMemoryGuard(cfg.entry);
	cfg.inst(memoryGuard).inputs.push_back(literal);
	expectAssertion([&] { cfg.checkInvariants(); }, {"MemoryGuard", "has inputs"});
}

BOOST_AUTO_TEST_CASE(memory_guard_with_payload)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const memoryGuard = cfg.makeMemoryGuard(cfg.entry);
	cfg.inst(memoryGuard).payload = std::make_unique<Payload>(LiteralPayload{0});
	expectAssertion([&] { cfg.checkInvariants(); }, {"MemoryGuard", "has a payload"});
}

// InstructionStore::checkLiteralDedup

BOOST_AUTO_TEST_CASE(dedup_entry_is_not_a_const)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	Fixture::makeNop(cfg, literal);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Dedup entry", "is not a Const"});
}

BOOST_AUTO_TEST_CASE(dedup_entry_value_differs_from_key)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	std::get<LiteralPayload>(*cfg.inst(literal).payload).value = 999;
	expectAssertion([&] { cfg.checkInvariants(); }, {"Dedup entry", "differing from its key"});
}

BOOST_AUTO_TEST_CASE(const_missing_from_dedup_table)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.newLiteral(nullptr, 1);
	InstId const scratch = fixture.addScratchInstruction(cfg, cfg.entry);
	auto& instruction = cfg.inst(scratch);
	instruction.opcode = InstOpcode::Const;
	instruction.payload = std::make_unique<Payload>(LiteralPayload{12345});
	expectAssertion([&] { cfg.checkInvariants(); }, {"missing from dedup table"});
}

BOOST_AUTO_TEST_CASE(const_is_not_the_canonical_dedup_slot)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.newLiteral(nullptr, 1);
	InstId const scratch = fixture.addScratchInstruction(cfg, cfg.entry);
	auto& instruction = cfg.inst(scratch);
	instruction.opcode = InstOpcode::Const;
	instruction.payload = std::make_unique<Payload>(LiteralPayload{1});
	expectAssertion([&] { cfg.checkInvariants(); }, {"dedup"});
}

BOOST_AUTO_TEST_CASE(value_dependency_cycle)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const literal = cfg.newLiteral(nullptr, 1);
	InstId const first = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {literal}, 1);
	InstId const second = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("not"), {}}, {first}, 1);
	cfg.replaceWithIdentity(first, second);
	cfg.replaceWithIdentity(second, first);
	expectAssertion([&] { cfg.checkInvariants(); }, {"cycle"});
}

BOOST_AUTO_TEST_CASE(instruction_block_disagrees_with_schedule)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.inst(diamond.leftUpsilon).block = diamond.right;
	expectAssertion([&] { cfg.checkInvariants(); }, {"but inst.block is"});
}

BOOST_AUTO_TEST_CASE(block_schedules_a_dangling_id)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.block(diamond.left).instructions.push_back(InstId{1000});
	expectAssertion([&] { cfg.checkInvariants(); }, {"out of bounds", "instructions of block"});
}

BOOST_AUTO_TEST_CASE(conditional_jump_condition_is_invalid)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	std::get<BasicBlock::ConditionalJump>(cfg.block(diamond.entry).exit).condition = InstId{};
	expectAssertion([&] { cfg.checkInvariants(); }, {"Empty InstId referenced by", "condition of block"});
}

BOOST_AUTO_TEST_CASE(function_return_value_is_invalid)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.numReturns = 1;
	cfg.block(cfg.entry).exit = BasicBlock::FunctionReturn{{InstId{}}};
	expectAssertion([&] { cfg.checkInvariants(); }, {"Empty InstId referenced by", "return value of block"});
}

BOOST_AUTO_TEST_CASE(instruction_scheduled_twice)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.block(diamond.left).instructions.push_back(diamond.leftUpsilon);
	expectAssertion([&] { cfg.checkInvariants(); }, {"scheduled 2 times"});
}

BOOST_AUTO_TEST_CASE(instruction_never_scheduled)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	Fixture::unschedule(cfg, diamond.leftUpsilon);
	expectAssertion([&] { cfg.checkInvariants(); }, {"scheduled 0 times"});
}

BOOST_AUTO_TEST_CASE(unreachable_value_is_scheduled)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const unreachable = cfg.unreachableValue();
	cfg.inst(unreachable).block = cfg.entry;
	cfg.block(cfg.entry).instructions.push_back(unreachable);
	expectAssertion([&] { cfg.checkInvariants(); }, {"should not be scheduled"});
}

BOOST_AUTO_TEST_CASE(const_not_pinned_to_entry)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	BlockId const second = Fixture::addSecondBlock(cfg);
	Fixture::moveTo(cfg, cfg.newLiteral(nullptr, 1), second);
	expectAssertion([&] { cfg.checkInvariants(); }, {"not pinned to entry block"});
}

BOOST_AUTO_TEST_CASE(broken_projection_cluster)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	std::get<Call>(*cfg.inst(cluster.call).payload).numReturns = 3;
	expectAssertion([&] { cfg.checkInvariants(); }, {"broken projection cluster"});
}

BOOST_AUTO_TEST_CASE(successor_is_not_live)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	std::get<BasicBlock::Jump>(cfg.block(diamond.left).exit).target = BlockId{99};
	expectAssertion([&] { cfg.checkInvariants(); }, {"not", "live", "successor of block"});
}

BOOST_AUTO_TEST_CASE(predecessor_is_not_live)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.block(diamond.merge).entries.push_back(BlockId{99});
	expectAssertion([&] { cfg.checkInvariants(); }, {"not",  "live", "predecessor of block"});
}

BOOST_AUTO_TEST_CASE(edges_disagree_between_the_two_sides)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.block(diamond.merge).entries.push_back(diamond.entry);
	expectAssertion([&] { cfg.checkInvariants(); }, {"edges are inconsistent"});
}

BOOST_AUTO_TEST_CASE(phi_missing_an_upsilon)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	Fixture::makeNop(cfg, diamond.rightUpsilon);
	expectAssertion([&] { cfg.checkInvariants(); }, {"not",  "exactly one Upsilon per predecessor"});
}

BOOST_AUTO_TEST_CASE(phi_fed_twice_from_the_same_predecessor)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	cfg.emitUpsilon(diamond.left, diamond.leftValue, diamond.phi);
	expectAssertion([&] { cfg.checkInvariants(); }, {"not" , "exactly one Upsilon per predecessor"});
}

BOOST_AUTO_TEST_CASE(main_exit_in_a_function_graph)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.block(cfg.entry).exit = BasicBlock::MainExit{};
	expectAssertion([&] { cfg.checkInvariants(); }, {"MainExit", "in a function graph"});
}

BOOST_AUTO_TEST_CASE(function_return_in_the_main_graph)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.block(cfg.entry).exit = BasicBlock::FunctionReturn{{}};
	expectAssertion([&] { cfg.checkInvariants(); }, {"FunctionReturn block", "in the main graph"});
}

BOOST_AUTO_TEST_CASE(function_return_arity_mismatch)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.numReturns = 2;
	InstId const literal = cfg.newLiteral(nullptr, 1);
	cfg.block(cfg.entry).exit = BasicBlock::FunctionReturn{{literal}};
	expectAssertion([&] { cfg.checkInvariants(); }, {"yields 1 values but graph declares 2"});
}

BOOST_AUTO_TEST_CASE(entry_block_is_not_live)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	Fixture::addSecondBlock(cfg);
	cfg.entry = BlockId{99};
	expectAssertion([&] { cfg.checkInvariants(); }, {"Entry block", "not live"});
}

BOOST_AUTO_TEST_CASE(entry_block_has_predecessors)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	BlockId const second = Fixture::addSecondBlock(cfg);
	cfg.block(second).exit = BasicBlock::Jump{cfg.entry};
	cfg.block(cfg.entry).entries = {second};
	expectAssertion([&] { cfg.checkInvariants(); }, {"Entry block", "has predecessors"});
}

BOOST_AUTO_TEST_CASE(phi_in_the_entry_block)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.newPhi(cfg.entry);
	expectAssertion([&] { cfg.checkInvariants(); }, {"Phi", "in the entry block"});
}

BOOST_AUTO_TEST_CASE(function_arg_outside_the_entry_block)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	BlockId const second = Fixture::addSecondBlock(cfg);
	InstId const argument = cfg.newFunctionArgument();
	cfg.arguments.push_back(argument);
	Fixture::moveTo(cfg, argument, second);
	expectAssertion([&] { cfg.checkInvariants(); }, {"FunctionArg", "not in entry block"});
}

BOOST_AUTO_TEST_CASE(function_arg_count_mismatch)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.newFunctionArgument();
	expectAssertion([&] { cfg.checkInvariants(); }, {"1 FunctionArg insts but 0 arguments"});
}

BOOST_AUTO_TEST_CASE(arguments_entry_is_invalid)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.newFunctionArgument();
	cfg.arguments.push_back(InstId{});
	expectAssertion([&] { cfg.checkInvariants(); }, {"Empty InstId referenced by cfg.arguments"});
}

BOOST_AUTO_TEST_CASE(arguments_entry_is_not_a_function_arg)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph("f");
	cfg.newFunctionArgument();
	cfg.arguments.push_back(cfg.newLiteral(nullptr, 1));
	expectAssertion([&] { cfg.checkInvariants(); }, {"arguments entry", "is not a FunctionArg"});
}

BOOST_AUTO_TEST_CASE(use_before_definition_in_the_same_block)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	InstId const a = cfg.newLiteral(nullptr, 1);
	InstId const b = cfg.newLiteral(nullptr, 2);
	InstId const sum = cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("add"), {}}, {a, b}, 1);
	Fixture::unschedule(cfg, sum);
	auto& instructions = cfg.block(cfg.entry).instructions;
	instructions.insert(instructions.begin(), sum);
	expectAssertion([&] { cfg.checkInvariants(); }, {"defined later in the same block"});
}

BOOST_AUTO_TEST_CASE(definition_does_not_dominate_use)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	auto const diamond = fixture.buildDiamond(cfg);
	InstId const definedInLeft = cfg.makeBuiltinCallWithProjections(
		diamond.left,
		BuiltinCall{fixture.builtin("add"), {}},
		{diamond.leftValue, diamond.rightValue}, // inputs
		1 // num returns
	);
	cfg.inst(diamond.rightUpsilon).inputs[0] = definedInLeft;
	expectAssertion([&] { cfg.checkInvariants(); }, {"does not dominate"});
}

BOOST_AUTO_TEST_CASE(producer_projections_missing_from_block)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	BlockId const second = Fixture::addSecondBlock(cfg);
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);

	// The producer's block still dominates the projection's new block, so SSA dominance holds and
	// only the "one projection per return value, right after the producer" rule is broken.
	Fixture::moveTo(cfg, cluster.secondProjection, second);
	expectAssertion([&] { cfg.checkInvariants(); }, {"returns 2 values but only 1 instructions follow it"});
}

BOOST_AUTO_TEST_CASE(producer_followed_by_a_non_projection)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	BlockId const second = Fixture::addSecondBlock(cfg);
	auto const cluster = fixture.addTwoReturnCall(cfg, cfg.entry);
	fixture.addScratchInstruction(cfg, cfg.entry);

	Fixture::moveTo(cfg, cluster.secondProjection, second);
	expectAssertion([&] { cfg.checkInvariants(); }, {"but is not one of its 2 projections"});
}

BOOST_AUTO_TEST_CASE(operation_after_a_non_continuing_builtin)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("stop"), {}}, {}, 0); // stop, so non-continuing
	fixture.addScratchInstruction(cfg, cfg.entry);
	expectAssertion([&] { cfg.checkInvariants(); }, {"non-continuing operation"});
}

BOOST_AUTO_TEST_CASE(operation_after_a_non_continuing_call)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.makeCallWithProjections(cfg.entry, Call{1, false, 0}, {}, 0); // false here means non-continuing
	fixture.addScratchInstruction(cfg, cfg.entry);
	expectAssertion([&] { cfg.checkInvariants(); }, {"non-continuing operation"});
}

BOOST_AUTO_TEST_CASE(non_continuing_operation_in_a_non_terminating_block)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.block(cfg.entry).exit = BasicBlock::MainExit{};
	cfg.makeBuiltinCallWithProjections(cfg.entry, BuiltinCall{fixture.builtin("stop"), {}}, {}, 0);
	expectAssertion([&] { cfg.checkInvariants(); }, {"does not terminate"});
}

BOOST_AUTO_TEST_CASE(live_block_unreachable_from_entry)
{
	Fixture fixture;
	SSACFG& cfg = fixture.addGraph();
	cfg.makeBlock(nullptr);
	BOOST_CHECK_NO_THROW(cfg.checkInvariants());
	expectAssertion([&] { cfg.checkAllBlocksReachable(); }, {"live but unreachable"});
}

BOOST_AUTO_TEST_SUITE_END()

}

#endif
