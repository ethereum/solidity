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

#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/JunkAdmittingBlocksFinder.h>
#include <libyul/backends/evm/ssa/LivenessAnalysis.h>
#include <libyul/backends/evm/ssa/io/DotExporterBase.h>

#include <libsolutil/StringUtils.h>

#include <fmt/ranges.h>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#ifdef SLOW_DEBUG
#include <algorithm>
#include <map>
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
void SSACFG::checkInvariants() const
{
	m_instructions.checkInvariants();
	auto const scheduleCount = checkScheduling();
	checkEachInstScheduledOnce(scheduleCount);
	checkBlockConstraints();
	checkEdgeConsistency();
	checkPhiOperands();
	checkExitShapes();
	checkEntryExitAndArguments();
}

void SSACFG::checkBlockRef(BlockId const _id, std::string const& _ctx) const
{
	yulAssert(hasBlock(_id), fmt::format("BlockId {} referenced by {} is not a live block", _id, _ctx));
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
				fmt::format("{} scheduled in block {} but inst.block is {}", instId, blockId, inst(instId).block)
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

void SSACFG::checkEachInstScheduledOnce(std::vector<std::uint32_t> const& _scheduleCount) const
{
	for (InstId const instId: instructionIds())
	{
		auto const count = _scheduleCount[instId.value];
		if (isTombstone(instId))
			yulAssert(count == 0, fmt::format("Tombstone {} appears in a block", instId));
		else if (isUnreachable(instId))
			yulAssert(count == 0, fmt::format("Unreachable {} should not be scheduled", instId));
		else
			yulAssert(count == 1, fmt::format("{} scheduled {} times (expected once)", instId, count));
	}
}

void SSACFG::checkBlockConstraints() const
{
	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId))
			continue;
		Inst const& i = inst(instId);

		if (i.opcode == InstOpcode::Const)
			yulAssert(i.block == entry, fmt::format("Const {} not pinned to entry", instId));

		if (i.isOperation())
		{
			std::size_t const n = numReturnsOf(instId);
			if (n >= 2)
				yulAssert(numTrailingProjections(instId) == n, fmt::format("Operation {} has a broken projection cluster", instId));
		}
	}
}

void SSACFG::checkEdgeConsistency() const
{
	std::map<std::pair<BlockId::ValueType, BlockId::ValueType>, int> succSide, predSide;
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		bb.forEachExit([&](BlockId const succ) {
			checkBlockRef(succ, fmt::format("successor of block {}", blockId));
			++succSide[{blockId.value, succ.value}];
		});
		for (BlockId const pred: bb.entries)
		{
			checkBlockRef(pred, fmt::format("predecessor of block {}", blockId));
			++predSide[{pred.value, blockId.value}];
		}
	}
	yulAssert(succSide == predSide, "CFG predecessor/successor edges are inconsistent");
}

void SSACFG::checkEntryExitAndArguments() const
{
	yulAssert(hasBlock(entry), "Entry block is not live");
	yulAssert(block(entry).entries.empty(), fmt::format("Entry block {} has predecessors", entry));

	std::size_t functionArgCount = 0;
	for (InstId const instId: instructionIds())
		if (!isTombstone(instId) && isFunctionArg(instId))
		{
			++functionArgCount;
			yulAssert(inst(instId).block == entry, fmt::format("FunctionArg {} not in entry block", instId));
		}
	yulAssert(functionArgCount == arguments.size(),
		fmt::format("{} FunctionArg insts but {} arguments", functionArgCount, arguments.size()));
	for (InstId const arg: arguments)
	{
		m_instructions.checkValidOperand(arg, "cfg.arguments");
		yulAssert(isFunctionArg(arg), fmt::format("arguments entry {} is not a FunctionArg", arg));
	}
}

void SSACFG::checkPhiOperands() const
{
	// Collect, per phi, the source blocks of the Upsilons feeding it.
	std::map<InstId::ValueType, std::vector<BlockId::ValueType>> upsilonSources;
	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId) || !isUpsilon(instId))
			continue;
		InstId const phi = upsilonPhi(instId);
		checkBlockRef(inst(instId).block, fmt::format("block of upsilon {}", instId));
		upsilonSources[phi.value].push_back(inst(instId).block.value);
	}

	// A phi must be fed by exactly one Upsilon per predecessor edge of its block, i.e. the
	// multiset of Upsilon source blocks equals the multiset of the block's predecessors.
	for (InstId const instId: instructionIds())
	{
		if (isTombstone(instId) || !isPhi(instId))
			continue;
		std::vector<BlockId::ValueType> predecessors;
		for (BlockId const pred: block(inst(instId).block).entries)
			predecessors.push_back(pred.value);
		std::vector<BlockId::ValueType>& sources = upsilonSources[instId.value];
		std::sort(predecessors.begin(), predecessors.end());
		std::sort(sources.begin(), sources.end());
		yulAssert(
			sources == predecessors,
			fmt::format("Phi {} in block {} is not fed by exactly one Upsilon per predecessor", instId, inst(instId).block)
		);
	}
}

void SSACFG::checkExitShapes() const
{
	std::size_t mainExitCount = 0;
	for (BlockId const blockId: liveBlocks())
	{
		BasicBlock const& bb = block(blockId);
		if (bb.isMainExitBlock())
		{
			yulAssert(isMainGraph(), fmt::format("MainExit block {} in a function graph", blockId));
			++mainExitCount;
		}
		else if (auto const* ret = std::get_if<BasicBlock::FunctionReturn>(&bb.exit))
		{
			yulAssert(!isMainGraph(), fmt::format("FunctionReturn block {} in the main graph", blockId));
			yulAssert(ret->returnValues.size() == numReturns,
				fmt::format("FunctionReturn block {} yields {} values but graph declares {}", blockId, ret->returnValues.size(), numReturns)
			);
		}
	}
	yulAssert(mainExitCount == (isMainGraph() ? 1u : 0u),
		fmt::format("Graph has {} MainExit block(s), expected {}", mainExitCount, isMainGraph() ? 1u : 0u)
	);
}
#endif
