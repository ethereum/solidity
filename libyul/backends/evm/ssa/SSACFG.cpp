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

#include <libyul/Exceptions.h>
#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/JunkAdmittingBlocksFinder.h>
#include <libyul/backends/evm/ssa/LivenessAnalysis.h>
#include <libyul/backends/evm/ssa/io/DotExporterBase.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <fmt/ranges.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#ifdef SLOW_DEBUG
#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#endif

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::ssa;

namespace
{

/// Build a human-readable Phi/Upsilon annotation for a phi value.
/// Shows which upsilons feed it, listed per predecessor block.
std::string formatPhi(SSACFG const& _cfg, InstId _phiId)
{
	// Collect all upsilons targeting _phiId from the whole CFG.
	std::vector<std::string> formattedUpsilons;
	for (BlockId const blockId: _cfg.liveBlocks())
		_cfg.forEachUpsilon(_cfg.block(blockId), [&](InstId const instId, SSACFG::Inst const& inst) {
			if (_cfg.upsilonPhi(instId) == _phiId)
				formattedUpsilons.push_back(
					fmt::format("Block {} => {}", blockId.value, inst.inputs.at(0).str(_cfg))
				);
		});
	if (!formattedUpsilons.empty())
		return fmt::format("φ(\\l\\\n\t{}\\l\\\n)", fmt::join(formattedUpsilons, ",\\l\\\n\t"));
	return "φ()";
}

class SSACFGDotExporter: public io::DotExporterBase
{
public:
	SSACFGDotExporter(SSACFG const& _cfg, size_t _functionIndex, LivenessAnalysis const* _liveness, ControlFlowGraphs const* _controlFlow):
		DotExporterBase(_cfg, _functionIndex),
		m_liveness(_liveness),
		m_controlFlow(_controlFlow)
	{
		if (_liveness)
			m_junkAdmittingBlocks = std::make_unique<JunkAdmittingBlocksFinder>(_cfg, _liveness->topologicalSort());
	}

protected:
	void writeBlockLabel(std::ostream& _out, BlockId _blockId) override
	{
		auto const& block = m_cfg.block(_blockId);
		auto const valueToString = [&](InstId const& valueId) { return valueId.str(m_cfg); };

		if (m_liveness)
		{
			_out << fmt::format(
				"\\\nBlock {}; ({}, max {})\\n",
				_blockId.value,
				m_liveness->topologicalSort().preOrderIndexOf(_blockId.value),
				m_liveness->topologicalSort().maxSubtreePreOrderIndexOf(_blockId.value)
			);
			_out << fmt::format(
				"LiveIn: {}\\l\\\n",
				fmt::join(m_liveness->liveIn(_blockId) | ranges::views::transform([&](auto const& liveIn) { return valueToString(liveIn.first) + fmt::format("[{}]", liveIn.second); }), ", ")
			);
			_out << fmt::format(
				"LiveOut: {}\\l\\n",
				fmt::join(m_liveness->liveOut(_blockId) | ranges::views::transform([&](auto const& liveOut) { return valueToString(liveOut.first) + fmt::format("[{}]", liveOut.second); }), ", ")
			);
			auto const usedVariables = m_liveness->used(_blockId);
			_out << fmt::format(
				"Used: {}\\l\\n",
				fmt::join(usedVariables | ranges::views::transform([&](auto const& used) { return valueToString(used.first) + fmt::format("[{}]", used.second); }), ", ")
			);
		}
		else
			_out << fmt::format("\\\nBlock {}\\n", _blockId.value);

		// Phis first, then BuiltinCall / Call in program order. Upsilons are rendered
		// under successor phis (via formatPhi) and skipped here. Trailing Projections of
		// multi-return calls are rendered as standalone lines following their producer.
		m_cfg.forEachPhi(block, [&](InstId const instId, SSACFG::Inst const&) {
			_out << fmt::format("phi{} := {}\\l\\\n", instId.value, formatPhi(m_cfg, instId));
		});
		m_cfg.forEachOperation(block, [&](InstId const instId, SSACFG::Inst const& inst) {
			std::string label;
			switch (inst.opcode)
			{
			case InstOpcode::Call:
			{
				auto const graphID = m_cfg.callPayload(instId).graphID;
				label = m_controlFlow ? m_controlFlow->functionGraph(graphID)->name : fmt::format("func{}", graphID);
				break;
			}
			case InstOpcode::BuiltinCall:
				label = m_cfg.evmDialect.builtin(m_cfg.builtinPayload(instId).builtin).name;
				break;
			case InstOpcode::MemoryGuard:
				if (m_controlFlow && m_controlFlow->memoryGuard)
					label = fmt::format("memoryguard<{}>", toCompactHexWithPrefix(*m_controlFlow->memoryGuard));
				else
					label = "memoryguard";
				break;
			default:
				yulAssert(false);
			}
			if (m_cfg.numReturnsOf(instId) >= 1)
				_out << fmt::format("{} := ", valueToString(instId));
			_out << fmt::format(
				"{}({})\\l\\\n",
				escapeLabel(label),
				fmt::join(inst.inputs | ranges::views::transform(valueToString), ", ")
			);
			for (InstId const projId: m_cfg.projectionsOf(instId))
				_out << fmt::format(
					"{} := {}.proj({})\\l\\\n",
					valueToString(projId),
					valueToString(instId),
					static_cast<unsigned>(m_cfg.projectionIndex(projId))
				);
		});
	}

	std::vector<std::pair<std::string, std::string>> blockNodeAttributes(BlockId _blockId) override
	{
		if (m_junkAdmittingBlocks && m_junkAdmittingBlocks->allowsAdditionOfJunk(_blockId))
			return {{"fillcolor", "\"#FF746C\""}, {"style", "filled"}};
		return {};
	}

	EdgeStyle edgeStyle(BlockId _source, BlockId _target) override
	{
		if (m_liveness && m_liveness->topologicalSort().backEdge(_source, _target))
			return EdgeStyle::Dashed;
		return EdgeStyle::Solid;
	}

private:
	LivenessAnalysis const* m_liveness;
	ControlFlowGraphs const* m_controlFlow;
	std::unique_ptr<JunkAdmittingBlocksFinder> m_junkAdmittingBlocks;
};

}

std::string SSACFG::toDot(
	bool _includeDiGraphDefinition,
	std::optional<size_t> _functionIndex,
	LivenessAnalysis const* _liveness,
	ControlFlowGraphs const* _controlFlow
) const
{
	SSACFGDotExporter exporter(*this, _functionIndex.value_or(isMainGraph() ? 0 : 1), _liveness, _controlFlow);
	if (!isMainGraph())
		return exporter.exportFunction(*this, _includeDiGraphDefinition);
	else
		return exporter.exportBlocks(entry, _includeDiGraphDefinition);
}

#ifdef SLOW_DEBUG
std::string SSACFG::graphName() const
{
	return isMainGraph() ? "<main>" : name;
}

void SSACFG::checkInvariants() const
{
	m_instructions.checkInvariants();
	auto const scheduleCount = checkScheduling();
	checkEachInstScheduledOnce(scheduleCount);
	checkBlockConstraints();
	checkEdgeConsistency();
	checkPhiOperands();
	checkExitShapes();
	checkEntry();
	checkArguments();
	checkDominance();
	checkProducerProjectionsInBlock();
	checkProjectionsFollowProducerInBlock();
	checkNonContinuingOperations();
}

// Every multi-return operation must be directly followed by exactly one projection per return value
void SSACFG::checkProducerProjectionsInBlock() const
{
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		for (std::size_t i = 0; i < bb.instructions.size(); ++i)
		{
			InstId const producerId = bb.instructions[i];
			if (!inst(producerId).canHaveProjections())
				continue;
			std::size_t const numReturns = numReturnsOf(producerId);
			if (numReturns < 2)
				continue;

			std::size_t const following = bb.instructions.size() - i - 1;
			yulAssert(
				following >= numReturns,
				fmt::format(
					"Operation {} returns {} values but only {} instructions follow it in block {} [graph {}]",
					producerId, numReturns, following, blockId, graphName()
				)
			);
			for (std::size_t k = 1; k <= numReturns; ++k)
			{
				InstId const projectionId = bb.instructions[i + k];
				Inst const& projection = inst(projectionId);
				yulAssert(
					projection.isProjection() &&
					projection.inputs.size() == 1 &&
					projection.inputs.front() == producerId,
					fmt::format(
						"{} follows operation {} in block {} but is not one of its {} projections [graph {}]",
						projectionId, producerId, blockId, numReturns, graphName()
					)
				);
			}
		}
	}
}

// Check if a projection is a direct successor of a producer, i.e.
// there are no other instructions in between. Does not check order.
void SSACFG::checkProjectionsFollowProducerInBlock() const {
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		for (std::size_t i = 0; i < bb.instructions.size(); i++) {
			auto const& projId = bb.instructions[i];
			auto const& proj = m_instructions.inst(projId);
			if (!proj.isProjection())
				continue;

			// Find producer
			InstId producerId = InstId{};
			yulAssert(i > 0, fmt::format("Projection {} is the first instruction of block {} [graph {}]", projId, blockId, graphName()));
			std::size_t producerPos = 0;
			for (std::size_t j = i; j > 0; j--)
			{
				InstId const instId = bb.instructions[j - 1];
				if (!m_instructions.inst(instId).isProjection())
				{
					producerId = instId;
					producerPos = j - 1;
					break;
				}
			}
			yulAssert(producerId.hasValue(), fmt::format("Projection {} in block {} has no producer before it [graph {}]", projId, blockId, graphName()));

			// Check producer is an operation
			auto const& producer = m_instructions.inst(producerId);
			yulAssert(producer.canHaveProjections(), fmt::format("Producer {} of projection {} is not an operation [graph {}]", producerId, projId, graphName()));

			// Check producer-projection relationship
			auto const trailingProjs = numTrailingProjections(producerId);
			std::size_t const distanceFromProducer = i - producerPos;
			yulAssert(distanceFromProducer <= trailingProjs, fmt::format("Projection {} is beyond the projection cluster of {} [graph {}]", projId, producerId, graphName()));
			yulAssert(proj.inputs.size() == 1, fmt::format("Projection {} needs exactly one input [graph {}]", projId, graphName()));
			yulAssert(proj.inputs[0] == producerId, fmt::format("Projection {} does not point at its producer {} [graph {}]", projId, producerId, graphName()));
		}
	}
}

void SSACFG::checkBlockRef(BlockId const _id, std::string const& _ctx) const
{
	yulAssert(hasBlock(_id), fmt::format("BlockId {} referenced by {} is not a live block [graph {}]", _id, _ctx, graphName()));
}

std::vector<std::uint32_t> SSACFG::checkScheduling() const
{
	std::vector<std::uint32_t> scheduleCount(numInsts(), 0);
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		for (InstId const instId: bb.instructions)
		{
			m_instructions.checkValidOperand(instId, fmt::format("instructions of block {}", blockId));
			yulAssert(inst(instId).block == blockId,
				fmt::format("{} scheduled in block {} but inst.block is {} [graph {}]", instId, blockId, inst(instId).block, graphName())
			);
			++scheduleCount[instId.value];
		}
		if (auto const* cj = std::get_if<BasicBlock::ConditionalJump>(&bb.exit))
			m_instructions.checkValidOperand(cj->condition, fmt::format("condition of block {}", blockId));
		else if (auto const* ret = std::get_if<BasicBlock::FunctionReturn>(&bb.exit))
			for (InstId const rv: ret->returnValues)
				m_instructions.checkValidOperand(rv, fmt::format("return value of block {}", blockId));
	}
	return scheduleCount;
}

// Single definition: every InstId appears as the result of exactly one instruction
// i.e. no InstId appears >= 2 times
void SSACFG::checkEachInstScheduledOnce(std::vector<std::uint32_t> const& _scheduleCount) const
{
	for (InstId const instId: instructionIds())
	{
		auto const count = _scheduleCount[instId.value];
		if (isTombstone(instId))
			yulAssert(count == 0, fmt::format("Tombstone {} appears in a block [graph {}]", instId, graphName()));
		else if (isUnreachable(instId))
			yulAssert(count == 0, fmt::format("Unreachable {} should not be scheduled [graph {}]", instId, graphName()));
		else
			yulAssert(count == 1, fmt::format("{} scheduled {} times (expected once) [graph {}]", instId, count, graphName()));
	}
}

// SSA dominance: every use is dominated by its definition
void SSACFG::checkDominance() const
{
	Dominance const dominance(*this);
	for (BlockId const curBlock: liveBlocks())
	{
		BasicBlock const& bb = block(curBlock);
		std::map<InstId, std::size_t> positionInBlock;
		for (auto const& [index, instId]: bb.instructions | ranges::views::enumerate)
			positionInBlock[instId] = index;

		// _usePos is the index of the using instruction within this block, or nullopt for uses in the
		// block's exit, which happen after every instruction scheduled in the block.
		auto checkUse = [&](InstId const _use, std::optional<std::size_t> const _usePos, std::string const& _ctx)
		{
			yulAssert(!isTombstone(_use), fmt::format("{} uses tombstoned {} [graph {}]", _ctx, _use, graphName()));
			// Unreachable is a pseudo-value that is deliberately not scheduled in any block
			if (isUnreachable(_use))
				return;

			BlockId const defBlock = inst(_use).block;
			checkBlockRef(defBlock, fmt::format("{}, defining {}", _ctx, _use));

			if (defBlock == curBlock)
			{
				// Defined & used in the same block
				auto const it = positionInBlock.find(_use);
				yulAssert(it != positionInBlock.end(), fmt::format("{} uses {}, not scheduled in its own block [graph {}]", _ctx, _use, graphName()));
				if (_usePos)
					yulAssert(
						it->second < *_usePos,
						fmt::format("{} uses {}, which is defined later in the same block [graph {}]", _ctx, _use, graphName())
					);
			}
			else
				// Not defined here, so the block it's defined in must dominate this block
				yulAssert(
					dominance.dominates(defBlock, curBlock),
					fmt::format("{} uses {}: its defining block {} does not dominate {} [graph {}]", _ctx, _use, defBlock, curBlock, graphName())
				);
		};

		for (auto const& [idx, instId]: bb.instructions | ranges::views::enumerate)
		{
			if (isTombstone(instId))
				continue;
			for (InstId const input: inst(instId).inputs)
				checkUse(input, idx, fmt::format("{} in block {}", instId, curBlock));
		}

		std::string const exitCtx = fmt::format("Exit of block {}", curBlock);
		if (auto const* condJump = std::get_if<BasicBlock::ConditionalJump>(&bb.exit))
			checkUse(condJump->condition, std::nullopt, exitCtx);
		else if (auto const* ret = std::get_if<BasicBlock::FunctionReturn>(&bb.exit))
			for (InstId const returnValue: ret->returnValues)
				checkUse(returnValue, std::nullopt, exitCtx);
	}
}

void SSACFG::checkAllBlocksReachable() const
{
	Dominance const dominance(*this);
	for (BlockId const blockId: liveBlocks())
		yulAssert(
			dominance.isReachableFromEntry(blockId),
			fmt::format("Block {} is live but unreachable from the entry block [graph {}]", blockId, graphName())
		);
}

// A non-continuing operation ends control flow, so nothing may follow it && its block must terminate
void SSACFG::checkNonContinuingOperations() const
{
	auto const isNonContinuing = [&](InstId const _instId) {
		switch (inst(_instId).opcode)
		{
		case InstOpcode::BuiltinCall:
			return evmDialect.builtin(builtinPayload(_instId).builtin).controlFlowSideEffects.terminatesOrReverts();
		case InstOpcode::Call:
			return !callPayload(_instId).canContinue;
		default:
			return false;
		}
	};

	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& b = block(blockId);
		InstId lastOperation{};
		for (InstId const instId: b.instructions)
		{
			if (isTombstone(instId) || !isOperation(instId))
				continue;
			yulAssert(
				!lastOperation.hasValue() || !isNonContinuing(lastOperation),
				fmt::format("Operation {} in block {} follows non-continuing operation {} [graph {}]", instId, blockId, lastOperation, graphName())
			);
			lastOperation = instId;
		}
		if (lastOperation.hasValue() && isNonContinuing(lastOperation))
			yulAssert(
				b.isTerminationBlock(),
				fmt::format("Block {} ends in non-continuing operation {} but does not terminate [graph {}]", blockId, lastOperation, graphName())
			);
	}
}

void SSACFG::checkBlockConstraints() const
{
#if 0
	// Phis live at the top of the block instructions
	for (BlockId const blockId: liveBlocks())
	{
		InstId nonPhi{};
		bool seenNonPhi = false;
		for (InstId const instId: block(blockId).instructions)
		{
			if (isTombstone(instId))
				continue;
			if (isPhi(instId))
				yulAssert(!seenNonPhi, fmt::format(
					"Phi {} is after a non-Phi ({}) in block {}. [graph {}]",
					instId, nonPhi, blockId, graphName()));
			else {
				seenNonPhi = true;
				nonPhi = instId;
			}
		}
	}
#endif

	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId))
			continue;
		Inst const& i = inst(instId);

		// Const is pinned to entry
		if (i.opcode == InstOpcode::Const)
			yulAssert(i.block == entry, fmt::format("Const {} not pinned to entry block [graph {}]", instId, graphName()));

		// Num returns of an operation matches trailing projections
		if (i.isOperation())
		{
			std::size_t const n = numReturnsOf(instId);
			if (n >= 2)
				yulAssert(numTrailingProjections(instId) == n, fmt::format("Operation {} has a broken projection cluster [graph {}]", instId, graphName()));
		}
	}
}

void SSACFG::checkEdgeConsistency() const
{
	std::map<std::pair<BlockId, BlockId>, int> succSide, predSide;
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		bb.forEachExit([&](BlockId const succ) {
			checkBlockRef(succ, fmt::format("successor of block {}", blockId));
			++succSide[{blockId, succ}];
		});
		for (BlockId const pred: bb.entries)
		{
			checkBlockRef(pred, fmt::format("predecessor of block {}", blockId));
			++predSide[{pred, blockId}];
		}
	}
	yulAssert(succSide == predSide, fmt::format("CFG predecessor/successor edges are inconsistent [graph {}]", graphName()));
}

void SSACFG::checkEntry() const
{
	yulAssert(hasBlock(entry), fmt::format("Entry block is not live [graph {}]", graphName()));
	yulAssert(block(entry).entries.empty(), fmt::format("Entry block {} has predecessors [graph {}]", entry, graphName()));

	// A Phi merges one value per predecessor edge; the entry block has no predecessors, so it
	// cannot contain any Phi
	for (InstId const instId: block(entry).instructions)
	{
		if (isTombstone(instId))
			continue;
		yulAssert(!isPhi(instId), fmt::format(
			"Phi {} is in the entry block {}, but the entry block has no predecessor edges to merge values from [graph {}]",
			instId, entry, graphName()));
	}
}

void SSACFG::checkArguments() const
{
	std::size_t functionArgCount = 0;
	for (InstId const instId: instructionIds())
		if (!isTombstone(instId) && isFunctionArg(instId))
		{
			++functionArgCount;
			// Arguments must live in the entry block as per SSA dominance rule
			yulAssert(inst(instId).block == entry, fmt::format("FunctionArg {} not in entry block [graph {}]", instId, graphName()));
		}
	yulAssert(functionArgCount == arguments.size(),
		fmt::format("{} FunctionArg insts but {} arguments [graph {}]", functionArgCount, arguments.size(), graphName()));

	for (InstId const arg: arguments)
	{
		m_instructions.checkValidOperand(arg, "cfg.arguments");
		yulAssert(isFunctionArg(arg), fmt::format("arguments entry {} is not a FunctionArg [graph {}]", arg, graphName()));
	}
}

void SSACFG::checkPhiOperands() const
{
	// Collect, per phi, the source blocks of the Upsilons feeding it.
	std::map<InstId, std::vector<BlockId>> upsilonSources; // phi -> [block of each upsilon feeding it]
	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId) || !isUpsilon(instId))
			continue;
		InstId const phi = upsilonPhi(instId);
		checkBlockRef(inst(instId).block, fmt::format("block of upsilon {}", instId));
		upsilonSources[phi].push_back(inst(instId).block);
	}

	// A phi must be fed by exactly one Upsilon per predecessor edge of its block, i.e. the
	// multiset of Upsilon source blocks equals the multiset of the block's predecessors.
	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId) || !isPhi(instId))
			continue;
		std::vector<BlockId> predecessors = block(inst(instId).block).entries;
		std::vector<BlockId>& sources = upsilonSources[instId];
		std::sort(predecessors.begin(), predecessors.end());
		std::sort(sources.begin(), sources.end());
		yulAssert(
			sources == predecessors,
			fmt::format("Phi {} in block {} is not fed by exactly one Upsilon per predecessor [graph {}]", instId, inst(instId).block, graphName())
		);
	}
}

void SSACFG::checkExitShapes() const
{
	[[maybe_unused]] std::size_t mainExitCount = 0;
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		if (bb.isMainExitBlock())
		{
			yulAssert(isMainGraph(), fmt::format("MainExit block {} in a function graph [graph {}]", blockId, graphName()));
			++mainExitCount;
		}
		else if (auto const* ret = std::get_if<BasicBlock::FunctionReturn>(&bb.exit))
		{
			yulAssert(!isMainGraph(), fmt::format("FunctionReturn block {} in the main graph [graph {}]", blockId, graphName()));
			yulAssert(ret->returnValues.size() == numReturns,
				fmt::format("FunctionReturn block {} yields {} values but graph declares {} [graph {}]", blockId, ret->returnValues.size(), numReturns, graphName())
			);
		}
	}
#if 0
	yulAssert(mainExitCount == (isMainGraph() ? 1u : 0u),
		fmt::format("Graph has {} MainExit block(s), expected {} [graph {}]", mainExitCount, isMainGraph() ? 1u : 0u, graphName())
	);
#endif
}
#endif
