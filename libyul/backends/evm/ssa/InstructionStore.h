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
 * Owns the instruction table of an SSACFG: hands out InstIds, stores opcode
 * payloads, and provides lookups. Has no knowledge of basic blocks or debug
 * information; SSACFG composes those concerns on top.
 *
 * Each Inst owns its payload directly via `std::unique_ptr<Payload>`. Tombstoning
 * an Inst (or flipping it via `replaceWith*`) resets the unique_ptr, releasing
 * the variant and any heap memory it transitively held. Slot storage and the free-list bookkeeping
 * live together in `m_insts`. Literal payloads are deduplicated through `m_literalDedup`.
 */

#pragma once

#include <libyul/backends/evm/ssa/util/TLSFFreeList.h>
#include <libyul/backends/evm/ssa/SSACFGTypes.h>

#include <libyul/AST.h>
#include <libyul/Builtins.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Numeric.h>

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace solidity::yul::ssa
{

class InstructionStore
{
public:
	using NumReturnsSizeType = std::uint8_t;

	struct LiteralPayload { u256 value; };
	struct UpsilonPayload { InstId targetPhi; };
	struct BuiltinCall
	{
		BuiltinHandle builtin;
		/// Literal-kind arguments
		std::vector<Literal> literalArguments;
	};
	struct Call
	{
		FunctionGraphID graphID;
		bool canContinue;
		std::size_t numReturns;
	};

	using Payload = std::variant<
		LiteralPayload,
		UpsilonPayload,
		BuiltinCall,
		Call
	>;

	struct Inst
	{
		InstOpcode opcode;
		BlockId block{};
		std::vector<InstId> inputs{};
		std::unique_ptr<Payload> payload{};

		constexpr bool isPhi() const noexcept { return opcode == InstOpcode::Phi; }
		constexpr bool isUpsilon() const noexcept { return opcode == InstOpcode::Upsilon; }
		constexpr bool isLiteral() const noexcept { return opcode == InstOpcode::Const; }
		constexpr bool isUnreachable() const noexcept { return opcode == InstOpcode::Unreachable; }
		constexpr bool isFunctionArg() const noexcept { return opcode == InstOpcode::FunctionArg; }
		constexpr bool isProjection() const noexcept { return opcode == InstOpcode::Projection; }
		constexpr bool isIdentity() const noexcept { return opcode == InstOpcode::Identity; }
		constexpr bool isNop() const noexcept { return opcode == InstOpcode::Nop; }
		constexpr bool isMemoryGuard() const noexcept { return opcode == InstOpcode::MemoryGuard; }
		constexpr bool isTombstone() const noexcept { return opcode == InstOpcode::Tombstone; }
		/// Operation = a Call, BuiltinCall, or MemoryGuard
		constexpr bool isOperation() const noexcept
		{
			return
				opcode == InstOpcode::Call ||
				opcode == InstOpcode::BuiltinCall ||
				opcode == InstOpcode::MemoryGuard;
		}
		constexpr bool canHaveProjections() const noexcept
		{
			return opcode == InstOpcode::Call || opcode == InstOpcode::BuiltinCall;
		}
	};

	explicit InstructionStore(std::size_t const _instructionCountHint = 0): m_insts(_instructionCountHint) {}
	InstructionStore(InstructionStore const&) = delete;
	InstructionStore(InstructionStore&&) = default;
	InstructionStore& operator=(InstructionStore const&) = delete;
	InstructionStore& operator=(InstructionStore&&) = default;
	~InstructionStore() = default;

	Inst& inst(InstId _id) { return m_insts[_id.value]; }
	Inst const& inst(InstId _id) const { return m_insts[_id.value]; }
	std::size_t numInsts() const { return m_insts.size(); }
	std::vector<Inst> const& instructions() const { return m_insts.data(); }

	/// Returns the opcode category for a given InstId.
	InstOpcode kindOf(InstId const _id) const { return inst(_id).opcode; }

	/// Returns the phi targeted by an Upsilon Inst.
	InstId upsilonPhi(InstId const _id) const { return payloadAs<InstOpcode::Upsilon, UpsilonPayload>(_id).targetPhi; }

	/// Returns the u256 payload of a Const Inst.
	u256 const& literalPayload(InstId const _id) const { return payloadAs<InstOpcode::Const, LiteralPayload>(_id).value; }

	BuiltinCall const& builtinPayload(InstId const _id) const { return payloadAs<InstOpcode::BuiltinCall, BuiltinCall>(_id); }

	Call const& callPayload(InstId const _id) const { return payloadAs<InstOpcode::Call, Call>(_id); }

	/// Returns the projection index of a Projection Inst
	NumReturnsSizeType projectionIndex(InstId const _id) const
	{
		Inst const& projectionInst = inst(_id);
		yulAssert(projectionInst.opcode == InstOpcode::Projection);
		yulAssert(projectionInst.inputs.size() == 1);
		InstId const producer = projectionInst.inputs.front();
		yulAssert(_id.value > producer.value);
		std::size_t const offset = static_cast<std::size_t>(_id.value) - static_cast<std::size_t>(producer.value) - 1;
		yulAssert(offset <= std::numeric_limits<NumReturnsSizeType>::max());
		return static_cast<NumReturnsSizeType>(offset);
	}

	/// Counts the trailing Projections immediately following `_producer` in `m_insts`
	NumReturnsSizeType numTrailingProjections(InstId const _producer) const
	{
		auto const& producerInst = inst(_producer);
		if (!producerInst.isOperation())
			return 0;
		std::size_t count = 0;
		while (true)
		{
			std::size_t const idx = static_cast<std::size_t>(_producer.value) + 1u + count;
			if (idx >= m_insts.size())
				break;
			using IndexType = util::TLSFFreeList<Inst, InstTombstone>::Index;
			yulAssert(idx < std::numeric_limits<IndexType>::max());
			auto const& trailing = m_insts[static_cast<IndexType>(idx)];
			if (!trailing.isProjection())
				break;
			yulAssert(trailing.inputs.size() == 1);
			yulAssert(trailing.inputs.front() == _producer);
			++count;
			yulAssert(count <= std::numeric_limits<NumReturnsSizeType>::max());
		}
		return static_cast<NumReturnsSizeType>(count);
	}

#ifdef SLOW_DEBUG
	void checkInvariants() const;
	void checkValidOperand(InstId _id, std::string const& _ctx) const;
#endif

	/// Allocates a new Phi Inst defined in `_definingBlock`. Returns the new InstId.
	InstId appendPhi(BlockId const _definingBlock)
	{
		return appendInst(Inst{InstOpcode::Phi, _definingBlock, {}, nullptr});
	}

	/// Allocates a new FunctionArg Inst defined in `_entryBlock`. Returns the new InstId.
	InstId appendFunctionArg(BlockId const _entryBlock)
	{
		return appendInst(Inst{InstOpcode::FunctionArg, _entryBlock, {}, nullptr});
	}

	/// Allocates a new MemoryGuard Inst defined in `_block`. The boundary value lives in `ControlFlowGraphs::memoryGuard`.
	InstId appendMemoryGuard(BlockId const _block)
	{
		yulAssert(_block.hasValue());
		return appendInst(Inst{InstOpcode::MemoryGuard, _block, {}, nullptr});
	}

	/// Allocates a new Unreachable Inst. Not pinned to any block (see class comment).
	InstId appendUnreachable()
	{
		return appendInst(Inst{InstOpcode::Unreachable, BlockId{}, {}, nullptr});
	}

	/// Allocates a new Const Inst with payload `_value`, pinned to `_entryBlock`.
	/// Literals are deduplicated by value (see class comment).
	/// Returns id of the corresponding Const instruction and a bool indicating
	/// whether new instruction has been appended (`true`) or existing instruction reused (`false`).
	std::pair<InstId, bool> appendLiteral(BlockId const _entryBlock, u256 _value)
	{
		yulAssert(_entryBlock.hasValue());
		if (auto const it = m_literalDedup.find(_value); it != m_literalDedup.end())
			return {it->second, false};
		InstId const id = appendInst(Inst{
			InstOpcode::Const,
			_entryBlock,
			{},
			std::make_unique<Payload>(LiteralPayload{_value})
		});
		m_literalDedup.emplace(std::move(_value), id);
		return {id, true};
	}

	/// Allocates a new single-output BuiltinCall Inst. Returns the new InstId.
	/// For multi-output BuiltinCalls, use `appendBuiltinCallWithProjections` so the
	/// producer + projections land in a contiguous cluster.
	InstId appendBuiltinCall(
		BlockId const _block,
		BuiltinCall _payload,
		std::vector<InstId> _inputs
	)
	{
		yulAssert(_block.hasValue());
		return appendInst(Inst{
			InstOpcode::BuiltinCall,
			_block,
			std::move(_inputs),
			std::make_unique<Payload>(std::move(_payload))
		});
	}

	/// Allocates a new single-output Call Inst. Returns the new InstId.
	/// For multi-output Calls, use `appendCallWithProjections` so the producer +
	/// projections land in a contiguous cluster.
	InstId appendCall(
		BlockId const _block,
		Call _payload,
		std::vector<InstId> _inputs
	)
	{
		yulAssert(_block.hasValue());
		yulAssert(_payload.numReturns < 2, "Multi-return Call must use appendCallWithProjections");
		return appendInst(Inst{
			InstOpcode::Call,
			_block,
			std::move(_inputs),
			std::make_unique<Payload>(std::move(_payload))
		});
	}

	/// Allocates a multi-return BuiltinCall along with its `_numReturns` trailing
	/// Projections in a contiguous cluster (length `1 + _numReturns`). Returns the
	/// producer's InstId; projections live at `producer.value + 1 .. + _numReturns`
	/// and the caller can read them via `SSACFG::projectionsOf`.
	InstId appendBuiltinCallWithProjections(
		BlockId const _block,
		BuiltinCall _payload,
		std::vector<InstId> _inputs,
		NumReturnsSizeType const _numReturns
	)
	{
		yulAssert(_block.hasValue());
		yulAssert(_numReturns >= 2, "appendBuiltinCallWithProjections requires _numReturns >= 2");
		std::uint32_t const length = static_cast<std::uint32_t>(_numReturns) + 1u;
		std::uint32_t const start = reserveRun(length);
		InstId const producerId{start};
		m_insts[start] = Inst{
			InstOpcode::BuiltinCall,
			_block,
			std::move(_inputs),
			std::make_unique<Payload>(std::move(_payload))
		};
		for (NumReturnsSizeType k = 0; k < _numReturns; ++k)
			m_insts[start + 1u + k] = Inst{InstOpcode::Projection, _block, {producerId}, nullptr};
		return producerId;
	}

	/// Allocates a multi-return Call along with its `_payload.numReturns` trailing
	/// Projections in a contiguous cluster. See `appendBuiltinCallWithProjections`.
	InstId appendCallWithProjections(
		BlockId const _block,
		Call _payload,
		std::vector<InstId> _inputs
	)
	{
		yulAssert(_block.hasValue());
		yulAssert(_payload.numReturns >= 2, "appendCallWithProjections requires _payload.numReturns >= 2");
		yulAssert(_payload.numReturns <= std::numeric_limits<NumReturnsSizeType>::max());
		auto const numReturns = static_cast<NumReturnsSizeType>(_payload.numReturns);
		std::uint32_t const length = static_cast<std::uint32_t>(numReturns) + 1u;
		std::uint32_t const start = reserveRun(length);
		InstId const producerId{start};
		m_insts[start] = Inst{
			InstOpcode::Call,
			_block,
			std::move(_inputs),
			std::make_unique<Payload>(std::move(_payload))
		};
		for (NumReturnsSizeType k = 0; k < numReturns; ++k)
			m_insts[start + 1u + k] = Inst{InstOpcode::Projection, _block, {producerId}, nullptr};
		return producerId;
	}

	/// Allocates a new Upsilon Inst targeting `_phi`, feeding `_value` from `_block`.
	InstId appendUpsilon(BlockId const _block, InstId const _value, InstId const _phi)
	{
		yulAssert(inst(_phi).isPhi());
		return appendInst(Inst{
			InstOpcode::Upsilon,
			_block,
			{_value},
			std::make_unique<Payload>(UpsilonPayload{_phi})
		});
	}

	/// Flips _target to Identity forwarding _forward.
	void replaceWithIdentity(InstId const _target, InstId const _forward)
	{
		yulAssert(_target.hasValue());
		yulAssert(_forward.hasValue());
		yulAssert(_target != _forward, "Cannot replace value with itself");
		auto& instruction = inst(_target);
		// every Const InstId is the unique handle for its value, so flipping it to Identity would silently rebind
		// every other user of that literal to _forward; almost certainly not what was intended
		yulAssert(!instruction.isLiteral(), "replaceWithIdentity must not morph a deduped Const slot");
		yulAssert(!instruction.isMemoryGuard(), "replaceWithIdentity must not morph a MemoryGuard slot");
		yulAssert(
			numTrailingProjections(_target) == 0,
			"replaceWithIdentity must not morph a multi-return producer slot"
		);
		instruction.opcode = InstOpcode::Identity;
		instruction.inputs.assign(1, _forward);
		instruction.payload.reset();
	}

	/// Flips _target to Const carrying _value. If _value is already deduped to another slot,
	/// flips _target to Identity forwarding to that slot instead.
	void replaceWithConst(InstId const _target, u256 const& _value)
	{
		yulAssert(_target.hasValue());
		auto& instruction = inst(_target);
		yulAssert(!instruction.isMemoryGuard(), "replaceWithConst must not morph a MemoryGuard slot");
		yulAssert(
			numTrailingProjections(_target) == 0,
			"replaceWithConst must not morph a multi-return producer slot"
		);
		// No-op if already this value.
		if (instruction.opcode == InstOpcode::Const && std::get<LiteralPayload>(*instruction.payload).value == _value)
			return;
		dropDedupIfConst(instruction);
		if (auto const it = m_literalDedup.find(_value); it != m_literalDedup.end())
		{
			instruction.opcode = InstOpcode::Identity;
			instruction.inputs.assign(1, it->second);
			instruction.payload.reset();
			return;
		}
		instruction.opcode = InstOpcode::Const;
		instruction.inputs.clear();
		instruction.payload = std::make_unique<Payload>(LiteralPayload{_value});
		m_literalDedup.emplace(std::move(_value), _target);
	}

	/// Flips _target to Nop. Caller must ensure _target produces no observable value (e.g., an Upsilon, or a void Call).
	void replaceWithNop(InstId const _target)
	{
		yulAssert(_target.hasValue());
		auto& instruction = inst(_target);
		yulAssert(!instruction.isMemoryGuard(), "replaceWithNop must not morph a MemoryGuard slot");
		yulAssert(
			numTrailingProjections(_target) == 0,
			"replaceWithNop must not morph a multi-return producer slot"
		);
		dropDedupIfConst(instruction);
		instruction.opcode = InstOpcode::Nop;
		instruction.inputs.clear();
		instruction.payload.reset();
	}

	/// Tombstones an Inst, releasing its slot(s) to the free pool. If `_id` is a
	/// multi-return producer (i.e. has trailing Projections), the entire cluster
	/// is swept and released as one contiguous run.
	void tombstone(InstId const _id)
	{
		yulAssert(_id.hasValue());
		auto const& instruction = inst(_id);
		if (instruction.isTombstone())
			return;

		yulAssert(
			!instruction.isProjection(),
			"illegal call of tombstone(_id) on a Projection, tombstone the producer instead"
		);
		std::uint32_t const trailingCount = numTrailingProjections(_id);
		dropDedupIfConst(instruction);
		// tombstone this inst and all trailing projections
		m_insts.deallocate(_id.value, trailingCount + 1u);
	}

private:
	/// Allocates a new Projection Inst projecting output of `_producer`.
	InstId appendProjection(BlockId const _block, InstId const _producer)
	{
		yulAssert(_block.hasValue());
		yulAssert(_producer.hasValue());
		return appendInst(Inst{InstOpcode::Projection, _block, {_producer}, nullptr});
	}

	/// Single-slot append: takes a length-1 run from the free pool if available, else
	/// tail-extends.
	InstId appendInst(Inst _inst)
	{
		yulAssert(m_insts.size() < std::numeric_limits<InstId::ValueType>::max());
		std::uint32_t const idx = m_insts.allocate(1);
		m_insts[idx] = std::move(_inst);
		return InstId{idx};
	}

	/// Reserves a contiguous run of `_length` slots in `m_insts`, returning the start
	/// index. The façade applies a smallest-fit policy with split-on-take and
	/// tail-extends with Tombstone placeholders on miss; reused cluster slots
	/// arrive already in tombstoned state.
	std::uint32_t reserveRun(std::uint32_t const _length)
	{
		yulAssert(_length >= 2, "reserveRun is for clusters; use appendInst for single slots");
		yulAssert(m_insts.size() < std::numeric_limits<InstId::ValueType>::max() - _length);
		return m_insts.allocate(_length);
	}

	/// Asserts that the Inst at `_id` has opcode `Expected`, then returns its payload as `T`.
	template<InstOpcode Expected, typename T>
	T const& payloadAs(InstId const _id) const
	{
		auto const& i = inst(_id);
		yulAssert(i.opcode == Expected);
		yulAssert(i.payload);
		return std::get<T>(*i.payload);
	}

	/// If `_inst` is a Const, removes its entry from the literal dedup table. Used by
	/// the replaceWith* primitives and tombstone() before overwriting the slot.
	void dropDedupIfConst(Inst const& _inst)
	{
		if (_inst.opcode != InstOpcode::Const)
			return;
		yulAssert(_inst.payload);
		u256 const& value = std::get<LiteralPayload>(*_inst.payload).value;
		m_literalDedup.erase(value);
	}

	struct InstTombstone
	{
		static Inst make() noexcept
		{
			return Inst{InstOpcode::Tombstone, BlockId{}, {}, nullptr};
		}
		static bool isTombstone(Inst const& _i) noexcept
		{
			return _i.opcode == InstOpcode::Tombstone;
		}
	};

#ifdef SLOW_DEBUG
	void checkOpcodeShapes() const;
	void checkOpcodeShape(InstId _id) const;
	void checkLiteralDedup() const;
	void checkValueDependencyAcyclic() const;
#endif

	util::TLSFFreeList<Inst, InstTombstone> m_insts;
	std::map<u256, InstId> m_literalDedup;
};

}
