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
 * Types used throughout the SSA CFG.
 */

#pragma once

#include <fmt/format.h>

#include <range/v3/range/concepts.hpp>
#include <range/v3/range/traits.hpp>

#include <cstdint>
#include <limits>
#include <string>

namespace solidity::yul::ssa
{

/// Maximum stack depth reachable by the EVM
std::size_t constexpr reachableStackDepth = 16;

template<typename R, typename T>
concept InputRangeOf = ranges::input_range<R> && std::same_as<ranges::range_value_t<R>, T>;

class SSACFG;
class SSACFGStackLayout;
class StackSlot;
using StackData = std::vector<StackSlot>;
using FunctionGraphID = std::uint32_t;

struct BlockId
{
	using ValueType = std::uint32_t;
	ValueType value = std::numeric_limits<ValueType>::max();
	constexpr bool hasValue() const noexcept { return value != std::numeric_limits<ValueType>::max(); }
	auto operator<=>(BlockId const&) const = default;
};

struct InstId
{
	using ValueType = std::uint32_t;
	ValueType value = std::numeric_limits<ValueType>::max();
	constexpr bool hasValue() const noexcept { return value != std::numeric_limits<ValueType>::max(); }
	auto operator<=>(InstId const&) const = default;

	/// Returns a human-readable string representation
	std::string str(SSACFG const& _cfg) const;
};

enum class InstOpcode : std::uint8_t
{
	Const,  // literal u256
	Phi,  // no inputs, retrieves value by mapping
	Upsilon,  // records phi pre-image at a phi predecessor block
	BuiltinCall,  // dialect builtin
	Call,  // user-defined function call
	Unreachable,  // dead code
	FunctionArg, // function param without inputs and a single output valueid
	Projection, // projection: single InstId input (multi-output producer) plus immediate index
	Identity,  // forwards its single input, resolved away by transform::removeIdentities,
	Nop,  // used to turn a void-typed Inst (eg an upsilon whose phi was eliminated) into a deletable placeholder
	MemoryGuard,  // emits the subobject's memoryguard boundary; value is held on the enclosing ControlFlowGraphs
	Tombstone,  // marks a free slot in InstructionStore::m_insts, must never appear in block instructions
};

}

template<>
struct fmt::formatter<solidity::yul::ssa::BlockId>
{
	static auto constexpr parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(solidity::yul::ssa::BlockId const& _blockId, FormatContext& _ctx) const -> decltype(_ctx.out())
	{
		if (!_blockId.hasValue())
			return fmt::format_to(_ctx.out(), "empty");
		return fmt::format_to(_ctx.out(), "{}", _blockId.value);
	}
};

template<>
struct fmt::formatter<solidity::yul::ssa::InstId>
{
	static auto constexpr parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

	template<typename FormatContext>
	auto format(solidity::yul::ssa::InstId const& _instId, FormatContext& _ctx) const -> decltype(_ctx.out())
	{
		if (!_instId.hasValue())
			return fmt::format_to(_ctx.out(), "empty");
		return fmt::format_to(_ctx.out(), "v{}", _instId.value);
	}
};
