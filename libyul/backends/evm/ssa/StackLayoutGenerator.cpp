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

#include <libyul/backends/evm/ssa/StackLayoutGenerator.h>

#include <libyul/backends/evm/ssa/stack/InstructionStackIn.h>
#include <libyul/backends/evm/ssa/stack/Shuffler.h>

#include <libyul/backends/evm/ssa/JunkAdmittingBlocksFinder.h>
#include <libyul/backends/evm/ssa/PhiInverse.h>
#include <libyul/backends/evm/ssa/ShuffleTrace.h>
#include <libyul/backends/evm/ssa/StackUtils.h>

#include <libsolutil/Visitor.h>

#include <range/v3/algorithm/count.hpp>
#include <range/v3/algorithm/replace.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/to_container.hpp>

#include <queue>

using namespace solidity::yul::ssa;

namespace
{
void handlePhiFunctions(StackData& _stackData, PhiInverse const& _phiInverse, LivenessAnalysis::LivenessData const& _liveness, SSACFG const& _cfg)
{
	// add any phi function values here that are not already contained in the stack
	for (auto const& [phi, preImage]: _phiInverse.data())
	{
		auto reversedStackData = _stackData | ranges::views::reverse;
		auto const phiSlot = StackSlot::makeValue(_cfg, phi);
		auto const preImageSlot = StackSlot::makeValue(_cfg, preImage);
		auto it = ranges::find(reversedStackData, preImageSlot);
		if (_liveness.contains(preImage))
		{
			// Both the phi function and the preimage are part of the live-in set.
			// If the preimage occurs more than once on the stack, one occurrence is
			// symbolically replaced by the phi function; otherwise, we push the phi value.
			if (ranges::count(_stackData, preImageSlot) > 1)
				*it = phiSlot;
			else
				_stackData.emplace_back(phiSlot);
		}
		else
		{
			// replace all occurrences of the preimage with the phi value
			ranges::replace(_stackData, preImageSlot, phiSlot);
			// if it's not contained, push it (could be derived from a literal)
			if (it == ranges::end(reversedStackData))
				_stackData.emplace_back(phiSlot);
		}
	}
}

void declareJunk(Stack& _stack, LivenessAnalysis::LivenessData const& _live)
{
	for (StackOffset offset{0}; offset < _stack.size(); ++offset.value)
	{
		auto const& slot = _stack[offset];
		if (slot.isValue() && !_live.contains(slot.value()))
			_stack.declareJunk(offset);
	}
}

}

StackLayoutGenerator::Result StackLayoutGenerator::generate(
	LivenessAnalysis const& _liveness,
	CallSites const& _callSites,
	ControlFlowGraphs::FunctionGraphID const _graphID,
	bool const _spillingAllowed
)
{
	spill::SpillSet spillSet;
	spill::SpillStoreTraces spillStoreTraces;
	while (true)
	{
		auto const spillCountBefore = spillSet.numSpilled();
		StackLayoutGenerator generator(_liveness, _callSites, _graphID, _spillingAllowed, std::move(spillSet));
		spillSet = std::move(generator.m_spillSet);
		spillSet.closeUnderReachabilityConstraints(_liveness.cfg(), generator.m_resultLayout, &spillStoreTraces);
		// only the traces of the final iteration survive; they are recorded against the stable spill set
		if (spillSet.numSpilled() == spillCountBefore)
			return Result{std::move(generator.m_resultLayout), std::move(spillSet), std::move(spillStoreTraces)};

		// there is an upper bound to how much can be spilled (number of variables). this assert ensures termination.
		yulAssert(spillSet.numSpilled() > spillCountBefore, "spill set cannot shrink");
	}
}

StackLayoutGenerator::StackLayoutGenerator(
	LivenessAnalysis const& _liveness,
	CallSites const& _callSites,
	ControlFlowGraphs::FunctionGraphID const _graphID,
	bool const _spillingAllowed,
	spill::SpillSet _initialSpillSet
):
	m_cfg(_liveness.cfg()),
	m_liveness(_liveness),
	m_callSites(_callSites),
	m_graphID(_graphID),
	m_hasFunctionReturnLabel(!_liveness.cfg().isMainGraph() && _liveness.cfg().canContinue),
	m_spillingAllowed(_spillingAllowed),
	m_junkAdmittingBlocksFinder(std::make_unique<JunkAdmittingBlocksFinder>(_liveness.cfg(), _liveness.topologicalSort())),
	m_inputStackProposalsPerBlock(m_cfg.numBlocks()),
	m_resultLayout(m_cfg.numBlocks()),
	m_spillSet(std::move(_initialSpillSet))
{
	// traverse the cfg layer-wise using Kahn's algorithm:
	// if a block is visited, the predecessors of that block have already their exit layouts defined
	// with minor exceptions when dealing with back-edges
	{
		// Future optimization: it might be beneficial to revisit the loop heads (back edge targets) after the first iteration
		std::vector<std::size_t> inDegreesIgnoringBackedges(m_cfg.numBlocks(), 0);

		for (SSACFG::BlockId const id: m_cfg.liveBlocks())
			for (auto const& entry: m_cfg.block(id).entries)
				if (!m_liveness.topologicalSort().backEdge(entry, id))
					inDegreesIgnoringBackedges[id.value] += 1;

		std::queue<SSACFG::BlockId> traversalQueue;
		traversalQueue.push(m_cfg.entry);

		std::size_t numVisited = 0;
		while (!traversalQueue.empty())
		{
			auto currentBlockId = traversalQueue.front();
			traversalQueue.pop();

			visitBlock(currentBlockId);

			m_cfg.block(currentBlockId).forEachExit([&](SSACFG::BlockId const& _exit){
				if (--inDegreesIgnoringBackedges[_exit.value] == 0)
					traversalQueue.push(_exit);
			});
			++numVisited;
		}
		yulAssert(numVisited == m_liveness.topologicalSort().preOrder().size());
	}
}

void StackLayoutGenerator::defineStackIn(SSACFG::BlockId const& _blockId)
{
	// we already have an input layout defined, return
	if (m_resultLayout[_blockId])
		return;
	BlockLayout blockLayout{};

	if (_blockId == m_cfg.entry)
	{
		if (!m_cfg.isMainGraph())
		{
			blockLayout.stackIn.reserve(m_cfg.arguments.size() + (m_hasFunctionReturnLabel ? 1u : 0u));
			if (m_hasFunctionReturnLabel)
				blockLayout.stackIn.push_back(Slot::makeFunctionReturnLabel(m_graphID));
			for (auto const& arg: m_cfg.arguments | ranges::views::reverse)
				blockLayout.stackIn.push_back(Slot::makeValue(m_cfg, arg));
		}
		m_resultLayout[_blockId] = std::move(blockLayout);
		return;
	}

	auto const& block = m_cfg.block(_blockId);
	auto const& liveIn = m_liveness.liveIn(_blockId);

	auto const& stackInProposals = m_inputStackProposalsPerBlock[_blockId.value];
	yulAssert(!stackInProposals.empty(), fmt::format("None of the parents of block {} were generated", _blockId));

	if (block.entries.size() == 1)
	{
		// pass through
		yulAssert(stackInProposals.size() == 1);
		blockLayout.stackIn = stackInProposals[0].second;
		handlePhiFunctions(blockLayout.stackIn, PhiInverse(m_cfg, stackInProposals[0].first, _blockId), liveIn, m_cfg);
		Stack stack(blockLayout.stackIn);
		declareJunk(stack, liveIn);
	}
	else
	{
		// Pre-compute each parent's proposal
		std::vector<StackData> proposals(stackInProposals.size());
		for (std::size_t i = 0; i < stackInProposals.size(); ++i)
		{
			proposals[i] = stackInProposals[i].second;
			handlePhiFunctions(proposals[i], PhiInverse(m_cfg, stackInProposals[i].first, _blockId), liveIn, m_cfg);
			{
				Stack stack(proposals[i]);
				declareJunk(stack, liveIn);
			}
		}
		// For each candidate stack-in layout (one parent's proposal), reconcile every parent to it,
		// discovering the spills needed to make each reconciliation realizable
		std::vector<std::size_t> cumulativeGas(stackInProposals.size(), 0);
		std::vector<spill::SpillSet> candidateSpillSets(stackInProposals.size());
		for (std::size_t i = 0; i < stackInProposals.size(); ++i)
		{
			spill::SpillSet candidateSpillSet = m_spillSet;
			for (std::size_t j = 0; j < stackInProposals.size(); ++j)
			{
				StackData edgeStack = stackInProposals[j].second;
				stack::ShuffleResult const result = stack::shuffle(
					edgeStack,
					stackPreImage(m_cfg, proposals[i], PhiInverse(m_cfg, stackInProposals[j].first, _blockId)),
					candidateSpillSet,
					m_spillingAllowed
				);
				yulAssert(result.status == stack::ShuffleResult::Status::Admissible);
				cumulativeGas[i] += stackOpsGas(m_cfg, result.trace);
			}
			candidateSpillSets[i] = std::move(candidateSpillSet);
		}
		// Pick the candidate that spills the least; break ties by gas, then by preferring smaller stacks.
		auto const candidateCost = [&](std::size_t const _i) {
			return std::make_tuple(candidateSpillSets[_i].numSpilled(), cumulativeGas[_i], proposals[_i].size());
		};
		std::size_t best = 0;
		for (std::size_t i = 1; i < stackInProposals.size(); ++i)
			if (candidateCost(i) < candidateCost(best))
				best = i;
		yulAssert(
			m_spillingAllowed || candidateSpillSets[best].numSpilled() == m_spillSet.numSpilled(),
			"Stack too deep, but spilling is disabled because the function is part of a recursive call chain."
		);
		m_spillSet = std::move(candidateSpillSets[best]);
		blockLayout.stackIn = std::move(proposals[best]);
	}

	// Validate every incoming forward edge
	for (auto const& [parentBlockId, parentExitStack]: stackInProposals)
	{
		StackData edgeStack = parentExitStack;
		auto shuffleResult = stack::shuffle(
			edgeStack,
			stackPreImage(m_cfg, blockLayout.stackIn, PhiInverse(m_cfg, parentBlockId, _blockId)),
			m_spillSet,
			m_spillingAllowed
		);
		yulAssert(
			shuffleResult.status == stack::ShuffleResult::Status::Admissible,
			"Stack too deep, but spilling is disabled because the function is part of a recursive call chain."
		);
		blockLayout.addTraceForStackIn(parentBlockId, std::move(shuffleResult.trace));
	}

	m_resultLayout[_blockId] = std::move(blockLayout);
}

void StackLayoutGenerator::visitBlock(SSACFG::BlockId const& _blockId)
{
	defineStackIn(_blockId);
	yulAssert(m_resultLayout[_blockId]);
	BlockLayout& blockLayout = *m_resultLayout[_blockId];

	SSACFG::BasicBlock const& block = m_cfg.block(_blockId);

	StackData currentStackData = blockLayout.stackIn;
	Stack stack(currentStackData);

	blockLayout.operationShuffles.reserve(block.instructions.size());
	m_cfg.forEachOperation(block, [&](InstId const _instId, SSACFG::Inst const& _inst) {
		auto opLiveOutWithoutOutputs = m_liveness.operationLiveOut(_instId);
		m_cfg.forEachOutput(_instId, [&](InstId const id) { opLiveOutWithoutOutputs.erase(id); });

		std::vector<Slot> requiredStackTop;
		if (_inst.opcode == InstOpcode::Call)
		{
			auto const& callPayload = m_cfg.callPayload(_instId);
			if (callPayload.canContinue)
			{
				auto const callSiteID = m_callSites.callSiteID(_instId);
				yulAssert(callSiteID.has_value());
				requiredStackTop.emplace_back(Slot::makeFunctionCallReturnLabel(*callSiteID));
			}
		}
		requiredStackTop +=
			_inst.inputs |
			ranges::views::reverse |
			ranges::views::transform([this](InstId const& _id) { return StackSlot::makeValue(m_cfg, _id); });

		{
			// Values that are dead before the operation are left on the stack; the shuffle to the
			// optimal target pops them as surplus as needed
			StackSlotLiveness const opLiveOutSlots = toStackSlotLiveness(m_cfg, opLiveOutWithoutOutputs);
			auto const target = stack::buildInstructionStackIn(stack.data(), requiredStackTop, opLiveOutSlots, m_spillSet);
			auto const spillCountBefore = m_spillSet.numSpilled();
			auto shuffleResult = stack::shuffle(currentStackData, target, m_spillSet, m_spillingAllowed);
			yulAssert(shuffleResult.status == stack::ShuffleResult::Status::Admissible, "Spilling not allowed, stack too deep.");
			yulAssert(m_spillingAllowed || m_spillSet.numSpilled() == spillCountBefore, "Spilling not allowed, stack too deep.");
			blockLayout.operationShuffles.push_back(std::move(shuffleResult.trace));
		}
		for (std::size_t i = 0; i < requiredStackTop.size(); ++i)
			stack.pop();
		m_cfg.forEachOutput(_instId, [&](InstId const id) {
			stack.push(Slot::makeValue(m_cfg, id));
		});
	});

	// we don't explicitly visit backedges and might have to spill here, too
	auto const validateBackEdge = [&](SSACFG::BlockId const& _target) {
		if (!m_liveness.topologicalSort().backEdge(_blockId, _target))
			return;
		yulAssert(m_resultLayout[_target], "Back-edge target must have its stackIn defined already.");
		StackData const target = stackPreImage(m_cfg, m_resultLayout[_target]->stackIn, PhiInverse(m_cfg, _blockId, _target));
		StackData exitStack = currentStackData;
		auto shuffleResult = stack::shuffle(exitStack, target, m_spillSet, m_spillingAllowed);
		yulAssert(
			shuffleResult.status == stack::ShuffleResult::Status::Admissible,
			"Stack too deep, but spilling is disabled because the function is part of a recursive call chain."
		);
		m_resultLayout[_target]->addTraceForStackIn(_blockId, std::move(shuffleResult.trace));
	};

	std::visit(
		solidity::util::GenericVisitor{
			[&](SSACFG::BasicBlock::ConditionalJump const& _cJump) {
				auto const& blockLiveOut = m_liveness.liveOut(_blockId);

				// check if we have to do anything (dup the condition, bring it to the top etc)
				bool const conditionSlotAlreadyFinal =
					!blockLiveOut.contains(_cJump.condition) &&  // if our live out does not contain the condition (ie we dont have to dup it)
					!stack.empty() &&   // our stack is not empty
					stack.top().isValue() && stack.top().value() == _cJump.condition;  // and the condition is already on top
				if (!conditionSlotAlreadyFinal)
				{
					auto const condition = Slot::makeValue(m_cfg, _cJump.condition);
					StackSlotLiveness const blockLiveOutSlots = toStackSlotLiveness(m_cfg, blockLiveOut);
					auto const target = stack::buildInstructionStackIn(stack.data(), {condition}, blockLiveOutSlots, m_spillSet);
					auto const spillCountBefore = m_spillSet.numSpilled();
					auto shuffleResult = stack::shuffle(currentStackData, target, m_spillSet, m_spillingAllowed);
					yulAssert(shuffleResult.status == stack::ShuffleResult::Status::Admissible, "Spilling not allowed, stack too deep.");
					yulAssert(m_spillingAllowed || m_spillSet.numSpilled() == spillCountBefore, "Spilling not allowed, stack too deep.");
					blockLayout.exitShuffle = std::move(shuffleResult.trace);
				}

				yulAssert(!stack.empty() && stack.top().isValue() && stack.top().value() == _cJump.condition);

				// Pop condition from symbolic stack (consumed by JUMPI).
				// After this, currentStackData reflects the post-JUMPI stack.
				stack.pop();

				// Define successor stack-in layouts
				m_inputStackProposalsPerBlock[_cJump.zero.value].emplace_back(_blockId, currentStackData);
				m_inputStackProposalsPerBlock[_cJump.nonZero.value].emplace_back(_blockId, currentStackData);

				validateBackEdge(_cJump.zero);
				validateBackEdge(_cJump.nonZero);
			},
			[&](SSACFG::BasicBlock::FunctionReturn const& _functionReturn) {
				yulAssert(m_hasFunctionReturnLabel, "When there is a proper function return, we need to have a label for it");
				// in case there are return values, let's bring the function return label to the top
				StackData returnStack = _functionReturn.returnValues | ranges::views::transform([this](InstId const _id) { return StackSlot::makeValue(m_cfg, _id); }) | ranges::to<std::vector>;
				returnStack.push_back(StackSlot::makeFunctionReturnLabel(m_graphID));
				auto shuffleResult = stack::shuffle(currentStackData, returnStack, m_spillSet, m_spillingAllowed);
				yulAssert(shuffleResult.status == stack::ShuffleResult::Status::Admissible, "Spilling not allowed, stack too deep.");
				blockLayout.exitShuffle = std::move(shuffleResult.trace);
			},
			[&](SSACFG::BasicBlock::Jump const& _jump) {
				m_inputStackProposalsPerBlock[_jump.target.value].emplace_back(_blockId, currentStackData);

				validateBackEdge(_jump.target);
			},
			[&](SSACFG::BasicBlock::MainExit const&) {},
			[&](SSACFG::BasicBlock::Terminated const&) {}
		},
		block.exit
	);
}
