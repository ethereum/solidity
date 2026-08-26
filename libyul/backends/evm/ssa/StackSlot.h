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

#pragma once

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/SSACFG.h>

#include <range/v3/algorithm/find.hpp>

#include <cstdint>
#include <type_traits>

namespace solidity::yul::ssa
{

/// Registry for tracking function call sites.
///
/// Maps user-function-call operations to unique numeric IDs. These IDs are used
/// to generate return labels for function calls in the EVM bytecode.
class CallSites
{
public:
	using CallSiteID = std::uint32_t;

	std::optional<CallSiteID> callSiteID(InstId _instId) const
	{
		if (auto const it = ranges::find(m_data, _instId); it != m_data.end())
			return static_cast<CallSiteID>(std::distance(m_data.begin(), it));
		return std::nullopt;
	}

	InstId instId(CallSiteID _callSite) const
	{
		yulAssert(_callSite < m_data.size());
		return m_data[_callSite];
	}

	CallSiteID addCallSite(InstId _instId)
	{
		if (auto const id = callSiteID(_instId))
			return *id;
		yulAssert(_instId.hasValue());
		m_data.emplace_back(_instId);
		return static_cast<CallSiteID>(m_data.size() - 1);
	}
private:
	std::vector<InstId> m_data;
};

/// A discriminated union corresponding to a single EVM stack slot.
/// Can represent:
///		- ValueID: SSA values (including literals)
///		- Junk: Placeholder/unused values
///     - FunctionCallReturnLabel: Return addresses for function calls
///     - FunctionReturnLabel: Identifies the calling function's graph
///
/// Memory layout is optimized: 8 bytes size for cache efficiency, trivially copyable, standard layout, trivial
class StackSlot
{
public:
	enum struct Kind: std::uint8_t
	{
		Value, // u32 InstId
		Junk, // empty
		FunctionCallReturnLabel, // index into corresponding stack layout's call sites
		FunctionReturnLabel // identifying the function graph via ControlFlowGraphs
	};

	constexpr StackSlot() = default;
	constexpr StackSlot(StackSlot const&) = default;
	constexpr StackSlot(StackSlot&&) = default;
	constexpr StackSlot& operator=(StackSlot const&) = default;
	constexpr StackSlot& operator=(StackSlot&&) = default;

	constexpr bool isValue() const noexcept { return kind() == Kind::Value; }
	constexpr bool isLiteralValue() const noexcept { return m_valueOpcode == InstOpcode::Const; }
	constexpr bool isPhiValue() const noexcept { return m_valueOpcode == InstOpcode::Phi; }
	constexpr bool isFunctionReturnLabel() const noexcept { return kind() == Kind::FunctionReturnLabel; }
	constexpr bool isFunctionCallReturnLabel() const noexcept { return kind() == Kind::FunctionCallReturnLabel; }
	constexpr bool isJunk() const noexcept { return kind() == Kind::Junk; }
	constexpr Kind kind() const noexcept { return m_kind; }

	ControlFlowGraphs::FunctionGraphID functionReturnLabel() const { yulAssert(isFunctionReturnLabel()); return m_payload; }
	CallSites::CallSiteID functionCallReturnLabel() const { yulAssert(isFunctionCallReturnLabel()); return m_payload; }
	InstId value() const
	{
		yulAssert(isValue());
		return InstId{m_payload};
	}

	static constexpr StackSlot makeJunk() { return {0, Kind::Junk}; }
	static StackSlot makeValue(SSACFG const& _cfg, InstId _value)
	{
		return {_value.value, Kind::Value, _cfg.kindOf(_value)};
	}
	static StackSlot makeValue(InstructionStore const& _store, InstId _value)
	{
		return {_value.value, Kind::Value, _store.kindOf(_value)};
	}
	static constexpr StackSlot makeFunctionReturnLabel(ControlFlowGraphs::FunctionGraphID const _graphID) { return {_graphID, Kind::FunctionReturnLabel}; }
	static constexpr StackSlot makeFunctionCallReturnLabel(CallSites::CallSiteID const _callSiteID) { return {_callSiteID, Kind::FunctionCallReturnLabel};	}

	auto operator<=>(StackSlot const&) const = default;
private:
	constexpr StackSlot(std::uint32_t const _payload, Kind const _kind, InstOpcode const _valueOpcode = InstOpcode::Unreachable):
		m_payload(_payload),
		m_kind(_kind),
		m_valueOpcode(_valueOpcode)
	{}

	/// interpretation depends on kind
	std::uint32_t m_payload;
	Kind m_kind;
	/// for Kind::Value: cached Opcode of the defining Inst
	InstOpcode m_valueOpcode;
};
static_assert(sizeof(StackSlot) == 8, "Want cache efficiency, benchmark this if you go beyond 8 bytes");
static_assert(std::is_trivially_copyable_v<StackSlot>, "Should be able to use memcpy semantics");
static_assert(std::is_standard_layout_v<StackSlot>, "Want to have a predictable layout");
static_assert(std::is_trivial_v<StackSlot>, "Want to have no init/cpy overhead");

/// Whether a slot can be materialized on the stack top out of thin air, without a copy of it on the stack.
constexpr bool canBeFreelyGenerated(StackSlot const& _slot)
{
	return _slot.isLiteralValue() || _slot.isJunk() || _slot.isFunctionCallReturnLabel();
}

using StackData = std::vector<StackSlot>;
std::string slotToString(StackSlot const& _slot);
std::string stackToString(StackData const& _stackData);

/// Array index into stack from the bottom (offset 0 = bottom).
/// Natural for array-like access and iteration; used when treating the stack as a data structure.
struct StackOffset
{
	explicit constexpr StackOffset(size_t _value) : value(_value) {}
	size_t value;
	auto operator<=>(StackOffset const&) const = default;
};
// comparison operations with size_t
constexpr auto operator<=>(StackOffset const lhs, size_t const rhs) noexcept { return lhs.value <=> rhs; }
constexpr auto operator<=>(size_t const lhs, StackOffset const rhs) noexcept { return lhs <=> rhs.value; }

/// Distance from the stack top (depth 0 = top).
/// Natural for stack operations (SWAP1 = swap with depth 1); used for operations that
/// conceptually work "from the top".
struct StackDepth
{
	explicit constexpr StackDepth(size_t _value) : value(_value) {}
	size_t value;
	auto operator<=>(StackDepth const&) const = default;
};
// comparison operations with size_t
constexpr auto operator<=>(StackDepth const lhs, size_t const rhs) noexcept { return lhs.value <=> rhs; }
constexpr auto operator<=>(size_t const lhs, StackDepth const rhs) noexcept { return lhs <=> rhs.value; }

}
