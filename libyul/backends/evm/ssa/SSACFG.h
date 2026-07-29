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
 * Control flow graph and stack layout structures used during code generation.
 */

#pragma once

#include <libyul/backends/evm/ssa/InstructionStore.h>
#include <libyul/backends/evm/ssa/SSACFGDebugInfo.h>
#include <libyul/backends/evm/ssa/SSACFGTypes.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Numeric.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <concepts>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace solidity::yul::ssa
{
class LivenessAnalysis;
struct ControlFlowGraphs;

class SSACFG
{
public:
	using DebugInfo = SSACFGDebugInfo;

	explicit SSACFG(
		EVMDialect const& _evmVersion,
		std::unique_ptr<DebugInfo> _debugInfo = nullptr,
		std::size_t const _instructionCountHint = 0
	):
		m_instructions(_instructionCountHint),
		evmDialect(_evmVersion),
		debugInfo(std::move(_debugInfo))
	{}

	SSACFG(SSACFG const&) = delete;
	SSACFG(SSACFG&&) = delete;
	SSACFG& operator=(SSACFG const&) = delete;
	SSACFG& operator=(SSACFG&&) = delete;
	~SSACFG() = default;

	using BlockId = ssa::BlockId;

	using BuiltinCall = InstructionStore::BuiltinCall;
	using Call = InstructionStore::Call;
	using Inst = InstructionStore::Inst;

	struct BasicBlock
	{
		struct MainExit {};
		struct ConditionalJump
		{
			InstId condition;
			BlockId nonZero;
			BlockId zero;
		};
		struct Jump
		{
			BlockId target;
		};
		struct FunctionReturn
		{
			std::vector<InstId> returnValues;
		};
		struct Terminated {};
		std::vector<BlockId> entries;
		std::vector<InstId> instructions;
		std::variant<MainExit, Jump, ConditionalJump, FunctionReturn, Terminated> exit = MainExit{};

		template<std::invocable<BlockId> Callable>
		void forEachExit(Callable&& _callable) const
		{
			if (auto* jump = std::get_if<Jump>(&exit))
				_callable(jump->target);
			else if (auto* conditionalJump = std::get_if<ConditionalJump>(&exit))
			{
				_callable(conditionalJump->nonZero);
				_callable(conditionalJump->zero);
			}
		}

		bool isMainExitBlock() const { return std::holds_alternative<MainExit>(exit); }
		bool isTerminationBlock() const { return std::holds_alternative<Terminated>(exit); }
		bool isFunctionReturnBlock() const { return std::holds_alternative<FunctionReturn>(exit); }
		bool isJumpBlock() const { return std::holds_alternative<Jump>(exit); }
	};

	BlockId makeBlock(langutil::DebugData::ConstPtr _debugData)
	{
		BlockId blockId;
		if (!m_freeBlocks.empty())
		{
			blockId = m_freeBlocks.back();
			yulAssert(blockId.value < m_freeBlocks.size());
			std::optional<BasicBlock>& block = m_blocks[blockId.value];
			yulAssert(!block.has_value());
			m_freeBlocks.pop_back();
			block.emplace(BasicBlock{{}, {}, BasicBlock::Terminated{}});
		}
		else
		{
			yulAssert(m_blocks.size() < std::numeric_limits<BlockId::ValueType>::max());
			blockId = BlockId{static_cast<BlockId::ValueType>(m_blocks.size())};
			m_blocks.emplace_back(BasicBlock{{}, {}, BasicBlock::Terminated{}});
		}
		if (debugInfo)
			debugInfo->setBlockDebugData(blockId, std::move(_debugData));
		return blockId;
	}
	BasicBlock& block(BlockId _id)
	{
		auto& slot = m_blocks.at(_id.value);
		yulAssert(slot.has_value(), fmt::format("Access of dead block #{}", _id.value));
		return *slot;
	}
	BasicBlock const& block(BlockId _id) const
	{
		auto const& slot = m_blocks.at(_id.value);
		yulAssert(slot.has_value(), fmt::format("Access of dead block #{}", _id.value));
		return *slot;
	}
	size_t numBlocks() const { return m_blocks.size(); }
	bool hasBlock(BlockId const _id) const { return _id.value < m_blocks.size() && m_blocks[_id.value].has_value(); }
	void resetBlock(BlockId const _id)
	{
		yulAssert(_id.value < m_blocks.size());
		auto& block = m_blocks[_id.value];
		yulAssert(block.has_value(), "double reset");
		yulAssert(
			ranges::all_of(block->instructions, [this](InstId const _instId) { return isTombstone(_instId); }),
			"can only reset blocks that have no live instructions left"
		);
		block.reset();
		m_freeBlocks.push_back(_id);
	}

	InputRangeOf<BlockId> auto liveBlocks() const
	{
		return
			ranges::views::iota(BlockId::ValueType{0}, static_cast<BlockId::ValueType>(m_blocks.size())) |
			ranges::views::filter([this](BlockId::ValueType const _v) { return m_blocks[_v].has_value(); }) |
			ranges::views::transform([](BlockId::ValueType const _v) { return BlockId{_v}; });
	}

	InstructionStore& instructionStore() { return m_instructions; }
	InstructionStore const& instructionStore() const { return m_instructions; }

	Inst& inst(InstId _id) { return m_instructions.inst(_id); }
	Inst const& inst(InstId _id) const { return m_instructions.inst(_id); }
	size_t numInsts() const { return m_instructions.numInsts(); }
	InputRangeOf<InstId> auto instructionIds() const
	{
		return
			ranges::views::iota(static_cast<InstId::ValueType>(0), static_cast<InstId::ValueType>(numInsts())) |
			ranges::views::transform([](auto const _value) { return InstId{_value}; });
	}
	std::vector<Inst> const& instructions() const { return m_instructions.instructions(); }

	/// Returns the opcode category for a given InstId.
	InstOpcode kindOf(InstId const _id) const { return m_instructions.kindOf(_id); }

	bool isPhi(InstId const _id) const { return inst(_id).isPhi(); }
	bool isUpsilon(InstId const _id) const { return inst(_id).isUpsilon(); }
	bool isLiteral(InstId const _id) const { return inst(_id).isLiteral(); }
	bool isUnreachable(InstId const _id) const { return inst(_id).isUnreachable(); }
	bool isFunctionArg(InstId const _id) const { return inst(_id).isFunctionArg(); }
	bool isProjection(InstId const _id) const { return inst(_id).isProjection(); }
	bool isIdentity(InstId const _id) const { return inst(_id).isIdentity(); }
	bool isNop(InstId const _id) const { return inst(_id).isNop(); }
	bool isMemoryGuard(InstId const _id) const { return inst(_id).isMemoryGuard(); }
	bool isTombstone(InstId const _id) const { return inst(_id).isTombstone(); }
	bool isOperation(InstId const _id) const { return inst(_id).isOperation(); }

	/// Walks the Identity chain starting at `_id` and returns the terminal (non-Identity) target.
	InstId resolveIdentity(InstId _id) const
	{
		while (_id.hasValue() && kindOf(_id) == InstOpcode::Identity)
		{
			auto const& i = inst(_id);
			yulAssert(i.inputs.size() == 1);
			yulAssert(i.inputs[0] != _id, "Identity self-loop");
			_id = i.inputs[0];
		}
		return _id;
	}

	/// Returns the phi targeted by an Upsilon Inst.
	InstId upsilonPhi(InstId const _id) const { return m_instructions.upsilonPhi(_id); }

	/// Returns the u256 payload of a Const Inst.
	u256 const& literalPayload(InstId const _id) const { return m_instructions.literalPayload(_id); }

	BuiltinCall const& builtinPayload(InstId const _id) const { return m_instructions.builtinPayload(_id); }

	Call const& callPayload(InstId const _id) const { return m_instructions.callPayload(_id); }

	/// Returns the projection index of a Projection Inst.
	InstructionStore::NumReturnsSizeType projectionIndex(InstId const _id) const { return m_instructions.projectionIndex(_id); }

	/// Creates a Phi Inst in the given block and returns its InstId.
	InstId newPhi(BlockId const _definingBlock)
	{
		InstId const id = scheduleInBlock(m_instructions.appendPhi(_definingBlock), _definingBlock);
		if (debugInfo)
			debugInfo->setValueDebugData(id, debugInfo->blockDebugData(_definingBlock));
		return id;
	}

	InstId newFunctionArgument()
	{
		InstId const id = scheduleInBlock(m_instructions.appendFunctionArg(entry), entry);
		if (debugInfo)
			debugInfo->setValueDebugData(id, debugInfo->blockDebugData(entry));
		return id;
	}

	/// Allocates a MemoryGuard Inst at `_block`. The boundary value lives in `ControlFlowGraphs::memoryGuard`.
	InstId makeMemoryGuard(BlockId const _block, langutil::DebugData::ConstPtr _debugData = {})
	{
		InstId const id = scheduleInBlock(m_instructions.appendMemoryGuard(_block), _block);
		if (debugInfo && _debugData)
			debugInfo->setValueDebugData(id, std::move(_debugData));
		return id;
	}

	InstId unreachableValue()
	{
		return m_instructions.appendUnreachable();
	}

	/// Literal InstIds are deduplicated. Const Insts are pinned to the entry block.
	InstId newLiteral(langutil::DebugData::ConstPtr _debugData, u256 _value)
	{
		auto const [id, newlyAllocated] = m_instructions.appendLiteral(entry, std::move(_value));
		// Newly allocated (not deduplicated): schedule in entry and attach debug data.
		if (newlyAllocated)
		{
			scheduleInBlock(id, entry);
			if (debugInfo)
				debugInfo->setValueDebugData(id, std::move(_debugData));
		}
		return id;
	}

	/// Allocates a BuiltinCall at `_block` and, for `_numReturns >= 2`, the `_numReturns` matching
	/// Projections immediately after it in `m_insts` as a single contiguous cluster.
	InstId makeBuiltinCallWithProjections(
		BlockId const _block,
		BuiltinCall _payload,
		std::vector<InstId> _inputs,
		InstructionStore::NumReturnsSizeType const _numReturns,
		langutil::DebugData::ConstPtr _debugData = {}
	)
	{
		if (_numReturns < 2)
			return makeBuiltinCall(_block, std::move(_payload), std::move(_inputs), _debugData);
		InstId const producer = m_instructions.appendBuiltinCallWithProjections(
			_block, std::move(_payload), std::move(_inputs), _numReturns
		);
		scheduleInBlock(producer, _block);
		if (debugInfo && _debugData)
			debugInfo->setInstDebugData(producer, _debugData);
		for (InstructionStore::NumReturnsSizeType k = 0; k < _numReturns; ++k)
		{
			InstId const projId{producer.value + 1u + k};
			scheduleInBlock(projId, _block);
			if (debugInfo && _debugData)
				debugInfo->setValueDebugData(projId, _debugData);
		}
		return producer;
	}

	/// Allocates a user Call at `_block` and, for `_numReturns >= 2`, the `_numReturns` matching
	/// Projections immediately after it in `m_insts` as a single contiguous cluster.
	InstId makeCallWithProjections(
		BlockId const _block,
		Call _payload,
		std::vector<InstId> _inputs,
		InstructionStore::NumReturnsSizeType const _numReturns,
		langutil::DebugData::ConstPtr _debugData = {}
	)
	{
		if (_numReturns < 2)
			return makeCall(_block, std::move(_payload), std::move(_inputs), _debugData);
		yulAssert(_payload.numReturns == _numReturns, "Call payload's numReturns disagrees with caller's _numReturns");
		InstId const producer = m_instructions.appendCallWithProjections(
			_block, std::move(_payload), std::move(_inputs)
		);
		scheduleInBlock(producer, _block);
		if (debugInfo && _debugData)
			debugInfo->setInstDebugData(producer, _debugData);
		for (InstructionStore::NumReturnsSizeType k = 0; k < _numReturns; ++k)
		{
			InstId const projId{producer.value + 1u + k};
			scheduleInBlock(projId, _block);
			if (debugInfo && _debugData)
				debugInfo->setValueDebugData(projId, _debugData);
		}
		return producer;
	}

	InstId emitUpsilon(BlockId const _block, InstId const _value, InstId const _phi)
	{
		return scheduleInBlock(m_instructions.appendUpsilon(_block, _value, _phi), _block);
	}

	/// Flips _target's opcode to Identity forwarding _forward
	void replaceWithIdentity(InstId const _target, InstId const _forward)
	{
		m_instructions.replaceWithIdentity(_target, _forward);
	}

	/// Flips _target to Const carrying _value
	void replaceWithConst(InstId const _target, u256 _value)
	{
		m_instructions.replaceWithConst(_target, std::move(_value));
	}

	/// Flips _target to Nop. Caller must ensure _target produces no observable value
	void replaceWithNop(InstId const _target)
	{
		m_instructions.replaceWithNop(_target);
	}

	/// Tombstones an Inst, sweeping any trailing Projection cluster too
	void tombstone(InstId const _id)
	{
		m_instructions.tombstone(_id);
	}

	/// Returns the number of Projections trailing `_producer` in `m_insts` (0 for
	/// non-multi-return slots). Thin wrapper around `InstructionStore::numTrailingProjections`.
	InstructionStore::NumReturnsSizeType numTrailingProjections(InstId const _producer) const
	{
		return m_instructions.numTrailingProjections(_producer);
	}

	std::string toDot(
		bool _includeDiGraphDefinition=true,
		std::optional<size_t> _functionIndex=std::nullopt,
		LivenessAnalysis const* _liveness=nullptr,
		ControlFlowGraphs const* _controlFlow=nullptr
	) const;

private:
	InstId scheduleInBlock(InstId const _id, BlockId const _block)
	{
		block(_block).instructions.push_back(_id);
		return _id;
	}

	std::vector<std::optional<BasicBlock>> m_blocks;
	// free list of blocks
	std::vector<BlockId> m_freeBlocks;
	InstructionStore m_instructions;
public:
	EVMDialect const& evmDialect;
	std::unique_ptr<DebugInfo> debugInfo;
	BlockId entry = BlockId{0};
	std::set<BlockId> exits;
	std::string name{};
	bool canContinue = true;
	std::vector<InstId> arguments;
	std::size_t numReturns = 0;

	bool isMainGraph() const { return name.empty(); }

	/// Iterates `_block.instructions`, invoking `_fn(instId, inst)` for every Phi.
	template<typename Callable>
	void forEachPhi(BasicBlock const& _block, Callable&& _fn)
	{
		forEachInstWhere(*this, _block, &Inst::isPhi, std::forward<Callable>(_fn));
	}
	template<typename Callable>
	void forEachPhi(BasicBlock const& _block, Callable&& _fn) const
	{
		forEachInstWhere(*this, _block, &Inst::isPhi, std::forward<Callable>(_fn));
	}

	/// Iterates `_block.instructions`, invoking `_fn(instId, inst)` for every Upsilon.
	template<typename Callable>
	void forEachUpsilon(BasicBlock const& _block, Callable&& _fn)
	{
		forEachInstWhere(*this, _block, &Inst::isUpsilon, std::forward<Callable>(_fn));
	}
	template<typename Callable>
	void forEachUpsilon(BasicBlock const& _block, Callable&& _fn) const
	{
		forEachInstWhere(*this, _block, &Inst::isUpsilon, std::forward<Callable>(_fn));
	}

	/// Iterates `_block.instructions`, invoking `_fn(instId, inst)` for every operation. Trailing
	/// Projections are skipped here; access them via `projectionsOf(instId)` / `forEachOutput(instId, ...)`.
	template<std::invocable<InstId, Inst&> Callable>
	void forEachOperation(BasicBlock const& _block, Callable&& _fn)
	{
		forEachInstWhere(*this, _block, &Inst::isOperation, std::forward<Callable>(_fn));
	}
	template<std::invocable<InstId, Inst const&> Callable>
	void forEachOperation(BasicBlock const& _block, Callable&& _fn) const
	{
		forEachInstWhere(*this, _block, &Inst::isOperation, std::forward<Callable>(_fn));
	}

	std::size_t numReturnsOf(InstId _op) const
	{
		auto const& i = inst(_op);
		switch (i.opcode)
		{
		case InstOpcode::Call:
			return callPayload(_op).numReturns;
		case InstOpcode::BuiltinCall:
			return evmDialect.builtin(builtinPayload(_op).builtin).numReturns;
		case InstOpcode::MemoryGuard:
			return 1;
		default:
			yulAssert(false, fmt::format("numReturnsOf called on non-operation inst {}", _op));
		}
	}

	/// View over the contiguous trailing Projections of `_producer` in `m_insts`. Empty for ops with <= 1 returns.
	/// Note this relies on the m_insts-contiguity invariant established by `make(Builtin)CallWithProjections`:
	/// projections live at `m_insts[_producer.value+1..+numReturns]`.
	InputRangeOf<InstId> auto projectionsOf(InstId const _producer) const
	{
		std::size_t const n = numReturnsOf(_producer);
		std::size_t const count = n >= 2 ? n : 0;
		std::size_t const firstIdx = static_cast<std::size_t>(_producer.value) + 1;
		std::size_t const lastIdx = firstIdx + count;
		// `appendInst` already caps `numInsts()` below `max(InstId::ValueType)`, so this also guarantees
		// the casts to `InstId::ValueType` below cannot wrap.
		yulAssert(
			lastIdx <= numInsts(),
			fmt::format(
				"Producer {} declares {} returns but only {} trailing m_insts slots are available",
				_producer, n, numInsts() - firstIdx
			)
		);
		for (std::size_t v = firstIdx; v < lastIdx; ++v)
		{
			auto const& trailing = inst(InstId{static_cast<InstId::ValueType>(v)});
			yulAssert(
				trailing.isProjection() && trailing.inputs.size() == 1 && trailing.inputs.front() == _producer,
				fmt::format("Trailing slot {} is not a Projection of producer {}", InstId{static_cast<InstId::ValueType>(v)}, _producer)
			);
		}
		return
			ranges::views::iota(static_cast<InstId::ValueType>(firstIdx), static_cast<InstId::ValueType>(lastIdx)) |
			ranges::views::transform(&toInstId);
	}

	/// View over the logical outputs of `_producer` in stack order: the producer itself if it has a single return,
	/// the trailing Projections if it has multiple, and an empty range otherwise.
	InputRangeOf<InstId> auto outputsOf(InstId const _producer) const
	{
		std::size_t const n = numReturnsOf(_producer);
		if (n >= 2)
			return projectionsOf(_producer);
		auto const firstIdx = static_cast<InstId::ValueType>(_producer.value);
		return
			ranges::views::iota(firstIdx, static_cast<InstId::ValueType>(firstIdx + n)) |
			ranges::views::transform(&toInstId);
	}

	/// Invokes `_fn(InstId)` once per logical output of `_producer` in stack order.
	template<std::invocable<InstId> Fn>
	void forEachOutput(InstId const _producer, Fn&& _fn) const
	{
		for (InstId const id: outputsOf(_producer))
			_fn(id);
	}

private:
	static InstId toInstId(InstId::ValueType const _v) { return InstId{_v}; }

	InstId makeBuiltinCall(
		BlockId const _block,
		BuiltinCall _payload,
		std::vector<InstId> _inputs,
		langutil::DebugData::ConstPtr _debugData = {}
	)
	{
		InstId const id = scheduleInBlock(
			m_instructions.appendBuiltinCall(_block, std::move(_payload), std::move(_inputs)),
			_block
		);
		if (debugInfo && _debugData)
			debugInfo->setInstDebugData(id, std::move(_debugData));
		return id;
	}

	InstId makeCall(
		BlockId const _block,
		Call _payload,
		std::vector<InstId> _inputs,
		langutil::DebugData::ConstPtr _debugData = {}
	)
	{
		InstId const id = scheduleInBlock(
			m_instructions.appendCall(_block, std::move(_payload), std::move(_inputs)),
			_block
		);
		if (debugInfo && _debugData)
			debugInfo->setInstDebugData(id, std::move(_debugData));
		return id;
	}

	/// Shared implementation for the public `forEach*` overloads; works for both
	/// `SSACFG&` and `SSACFG const&` since `_self.inst(id)` propagates const-ness.
	template<typename Self, typename Pred, typename Callable>
	static void forEachInstWhere(Self& _self, BasicBlock const& _block, Pred _pred, Callable&& _fn)
	{
		for (InstId const id: _block.instructions)
		{
			auto& inst = _self.inst(id);
			if (std::invoke(_pred, inst))
				_fn(id, inst);
		}
	}

#ifdef SLOW_DEBUG
public:
	void checkInvariants() const;
private:
	class Dominance
	{
	public:
		explicit Dominance(SSACFG const& _cfg):
			m_reachableFromEntry(reachableAvoiding(_cfg, BlockId{}))
		{
			for (BlockId const candidate: _cfg.liveBlocks())
			{
				std::set<BlockId> const reachable = reachableAvoiding(_cfg, candidate);

				// Whatever the entry cannot reach while avoiding `candidate` is dominated by it
				for (BlockId const blockId: _cfg.liveBlocks())
					if (!reachable.count(blockId))
						m_dominators[blockId].insert(candidate);
			}
		}

		bool dominates(BlockId const _dominator, BlockId const _block) const
		{
			auto const it = m_dominators.find(_block);
			return it != m_dominators.end() && it->second.count(_dominator) > 0;
		}

		bool isReachableFromEntry(BlockId const _block) const
		{
			return m_reachableFromEntry.count(_block) > 0;
		}

	private:
		/// Blocks reachable from the entry if _avoid is deleted from the CFG.
		static std::set<BlockId> reachableAvoiding(SSACFG const& _cfg, BlockId const _avoid)
		{
			std::set<BlockId> reachable;
			std::vector<BlockId> worklist;
			if (_cfg.entry != _avoid)
			{
				reachable.insert(_cfg.entry);
				worklist.push_back(_cfg.entry);
			}
			while (!worklist.empty())
			{
				BlockId const blockId = worklist.back();
				worklist.pop_back();
				_cfg.block(blockId).forEachExit([&](BlockId const successor) {
					if (successor != _avoid && reachable.insert(successor).second)
						worklist.push_back(successor);
				});
			}
			return reachable;
		}

		std::map<BlockId, std::set<BlockId>> m_dominators; // map: block->its dominators
		std::set<BlockId> m_reachableFromEntry;
	};

	std::vector<std::uint32_t> checkScheduling() const;
	void checkEachInstScheduledOnce(std::vector<std::uint32_t> const& _scheduleCount) const;
	void checkBlockConstraints() const;
	void checkEdgeConsistency() const;
	void checkPhiOperands() const;
	void checkExitShapes() const;
	void checkEntry() const;
	void checkArguments() const;
	void checkBlockRef(BlockId _id, std::string const& _ctx) const;
	void checkDominance() const;
	void checkProjectionsFollowProducerInBlock() const;
	void checkBlockSuccPredSymmetry() const;
	void checkPhiAtTopOfBlocks() const;
	void checkExitConsistency() const;
#endif
};

}
