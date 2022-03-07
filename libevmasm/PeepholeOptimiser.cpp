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
 * @file PeepholeOptimiser.cpp
 * Performs local optimising code changes to assembly.
 */

#include <libevmasm/PeepholeOptimiser.h>

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;

// TODO: Extend this to use the tools from ExpressionClasses.cpp

namespace
{

struct OptimiserState
{
	AssemblyItems const& items;
	size_t i;
	std::back_insert_iterator<AssemblyItems> out;
};

template<typename FunctionType>
struct FunctionParameterCount;
template<typename R, typename... Args>
struct FunctionParameterCount<R(Args...)>
{
	static constexpr auto value = sizeof...(Args);
};

template <class Method>
struct SimplePeepholeOptimizerMethod
{
	template <size_t... Indices>
	static bool applyRule(AssemblyItems::const_iterator _in, back_insert_iterator<AssemblyItems> _out, index_sequence<Indices...>)
	{
		return Method::applySimple(_in[Indices]..., _out);
	}
	static bool apply(OptimiserState& _state)
	{
		static constexpr size_t WindowSize = FunctionParameterCount<decltype(Method::applySimple)>::value - 1;
		if (
			_state.i + WindowSize <= _state.items.size() &&
			applyRule(_state.items.begin() + static_cast<ptrdiff_t>(_state.i), _state.out, make_index_sequence<WindowSize>{})
		)
		{
			_state.i += WindowSize;
			return true;
		}
		else
			return false;
	}
};

struct Identity: SimplePeepholeOptimizerMethod<Identity>
{
	static bool applySimple(AssemblyItem const& _item, std::back_insert_iterator<AssemblyItems> _out)
	{
		*_out = _item;
		return true;
	}
};

struct PushPop: SimplePeepholeOptimizerMethod<PushPop>
{
	static bool applySimple(AssemblyItem const& _push, AssemblyItem const& _pop, std::back_insert_iterator<AssemblyItems>)
	{
		auto t = _push.type();
		return _pop == Instruction::POP && (
			SemanticInformation::isDupInstruction(_push) ||
			t == Push || t == PushTag || t == PushSub ||
			t == PushSubSize || t == PushProgramSize || t == PushData || t == PushLibraryAddress
		);
	}
};

struct OpPop: SimplePeepholeOptimizerMethod<OpPop>
{
	static bool applySimple(
		AssemblyItem const& _op,
		AssemblyItem const& _pop,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (_pop == Instruction::POP && _op.type() == Operation)
		{
			Instruction instr = _op.instruction();
			if (instructionInfo(instr).ret == 1 && !instructionInfo(instr).sideEffects)
			{
				for (int j = 0; j < instructionInfo(instr).args; j++)
					*_out = {Instruction::POP, _op.location()};
				return true;
			}
		}
		return false;
	}
};

struct DoubleSwap: SimplePeepholeOptimizerMethod<DoubleSwap>
{
	static size_t applySimple(AssemblyItem const& _s1, AssemblyItem const& _s2, std::back_insert_iterator<AssemblyItems>)
	{
		return _s1 == _s2 && SemanticInformation::isSwapInstruction(_s1);
	}
};

struct DoublePush: SimplePeepholeOptimizerMethod<DoublePush>
{
	static bool applySimple(AssemblyItem const& _push1, AssemblyItem const& _push2, std::back_insert_iterator<AssemblyItems> _out)
	{
		if (_push1.type() == Push && _push2.type() == Push && _push1.data() == _push2.data())
		{
			*_out = _push1;
			*_out = {Instruction::DUP1, _push2.location()};
			return true;
		}
		else
			return false;
	}
};

struct CommutativeSwap: SimplePeepholeOptimizerMethod<CommutativeSwap>
{
	static bool applySimple(AssemblyItem const& _swap, AssemblyItem const& _op, std::back_insert_iterator<AssemblyItems> _out)
	{
		// Remove SWAP1 if following instruction is commutative
		if (
			_swap == Instruction::SWAP1 &&
			SemanticInformation::isCommutativeOperation(_op)
		)
		{
			*_out = _op;
			return true;
		}
		else
			return false;
	}
};

struct SwapComparison: SimplePeepholeOptimizerMethod<SwapComparison>
{
	static bool applySimple(AssemblyItem const& _swap, AssemblyItem const& _op, std::back_insert_iterator<AssemblyItems> _out)
	{
		static map<Instruction, Instruction> const swappableOps{
			{ Instruction::LT, Instruction::GT },
			{ Instruction::GT, Instruction::LT },
			{ Instruction::SLT, Instruction::SGT },
			{ Instruction::SGT, Instruction::SLT }
		};

		if (
			_swap == Instruction::SWAP1 &&
			_op.type() == Operation &&
			swappableOps.count(_op.instruction())
		)
		{
			*_out = swappableOps.at(_op.instruction());
			return true;
		}
		else
			return false;
	}
};

/// Remove swapN after dupN
struct DupSwap: SimplePeepholeOptimizerMethod<DupSwap>
{
	static size_t applySimple(
		AssemblyItem const& _dupN,
		AssemblyItem const& _swapN,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			SemanticInformation::isDupInstruction(_dupN) &&
			SemanticInformation::isSwapInstruction(_swapN) &&
			getDupNumber(_dupN.instruction()) == getSwapNumber(_swapN.instruction())
		)
		{
			*_out = _dupN;
			return true;
		}
		else
			return false;
	}
};


struct IsZeroIsZeroJumpI: SimplePeepholeOptimizerMethod<IsZeroIsZeroJumpI>
{
	static size_t applySimple(
		AssemblyItem const& _iszero1,
		AssemblyItem const& _iszero2,
		AssemblyItem const& _pushTag,
		AssemblyItem const& _jumpi,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			_iszero1 == Instruction::ISZERO &&
			_iszero2 == Instruction::ISZERO &&
			_pushTag.type() == PushTag &&
			_jumpi == Instruction::JUMPI
		)
		{
			*_out = _pushTag;
			*_out = _jumpi;
			return true;
		}
		else
			return false;
	}
};

struct EqIsZeroJumpI: SimplePeepholeOptimizerMethod<EqIsZeroJumpI>
{
	static size_t applySimple(
		AssemblyItem const& _eq,
		AssemblyItem const& _iszero,
		AssemblyItem const& _pushTag,
		AssemblyItem const& _jumpi,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			_eq == Instruction::EQ &&
			_iszero == Instruction::ISZERO &&
			_pushTag.type() == PushTag &&
			_jumpi == Instruction::JUMPI
		)
		{
			*_out = AssemblyItem(Instruction::SUB, _eq.location());
			*_out = _pushTag;
			*_out = _jumpi;
			return true;
		}
		else
			return false;
	}
};

// push_tag_1 jumpi push_tag_2 jump tag_1: -> iszero push_tag_2 jumpi tag_1:
struct DoubleJump: SimplePeepholeOptimizerMethod<DoubleJump>
{
	static size_t applySimple(
		AssemblyItem const& _pushTag1,
		AssemblyItem const& _jumpi,
		AssemblyItem const& _pushTag2,
		AssemblyItem const& _jump,
		AssemblyItem const& _tag1,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			_pushTag1.type() == PushTag &&
			_jumpi == Instruction::JUMPI &&
			_pushTag2.type() == PushTag &&
			_jump == Instruction::JUMP &&
			_tag1.type() == Tag &&
			_pushTag1.data() == _tag1.data()
		)
		{
			*_out = AssemblyItem(Instruction::ISZERO, _jumpi.location());
			*_out = _pushTag2;
			*_out = _jumpi;
			*_out = _tag1;
			return true;
		}
		else
			return false;
	}
};

struct JumpToNext: SimplePeepholeOptimizerMethod<JumpToNext>
{
	static size_t applySimple(
		AssemblyItem const& _pushTag,
		AssemblyItem const& _jump,
		AssemblyItem const& _tag,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			_pushTag.type() == PushTag &&
			(_jump == Instruction::JUMP || _jump == Instruction::JUMPI) &&
			_tag.type() == Tag &&
			_pushTag.data() == _tag.data()
		)
		{
			if (_jump == Instruction::JUMPI)
				*_out = AssemblyItem(Instruction::POP, _jump.location());
			*_out = _tag;
			return true;
		}
		else
			return false;
	}
};

struct TagConjunctions: SimplePeepholeOptimizerMethod<TagConjunctions>
{
	static bool applySimple(
		AssemblyItem const& _pushTag,
		AssemblyItem const& _pushConstant,
		AssemblyItem const& _and,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (_and != Instruction::AND)
			return false;
		if (
			_pushTag.type() == PushTag &&
			_pushConstant.type() == Push &&
			(_pushConstant.data() & u256(0xFFFFFFFF)) == u256(0xFFFFFFFF)
		)
		{
			*_out = _pushTag;
			return true;
		}
		else if (
			// tag and constant are swapped
			_pushConstant.type() == PushTag &&
			_pushTag.type() == Push &&
			(_pushTag.data() & u256(0xFFFFFFFF)) == u256(0xFFFFFFFF)
		)
		{
			*_out = _pushConstant;
			return true;
		}
		else
			return false;
	}
};

struct TruthyAnd: SimplePeepholeOptimizerMethod<TruthyAnd>
{
	static bool applySimple(
		AssemblyItem const& _push,
		AssemblyItem const& _not,
		AssemblyItem const& _and,
		std::back_insert_iterator<AssemblyItems>
	)
	{
		return (
			_push.type() == Push && _push.data() == 0 &&
			_not == Instruction::NOT &&
			_and == Instruction::AND
		);
	}
};

/// Removes everything after a JUMP (or similar) until the next JUMPDEST.
struct UnreachableCode
{
	static bool apply(OptimiserState& _state)
	{
		auto it = _state.items.begin() + static_cast<ptrdiff_t>(_state.i);
		auto end = _state.items.end();
		if (it == end)
			return false;
		if (
			it[0] != Instruction::JUMP &&
			it[0] != Instruction::RETURN &&
			it[0] != Instruction::STOP &&
			it[0] != Instruction::INVALID &&
			it[0] != Instruction::SELFDESTRUCT &&
			it[0] != Instruction::REVERT
		)
			return false;

		ptrdiff_t i = 1;
		while (it + i != end && it[i].type() != Tag)
			i++;
		if (i > 1)
		{
			*_state.out = it[0];
			_state.i += static_cast<size_t>(i);
			return true;
		}
		else
			return false;
	}
};

void applyMethods(OptimiserState&)
{
	assertThrow(false, OptimizerException, "Peephole optimizer failed to apply identity.");
}

template <typename Method, typename... OtherMethods>
void applyMethods(OptimiserState& _state, Method, OtherMethods... _other)
{
	if (!Method::apply(_state))
		applyMethods(_state, _other...);
}

size_t numberOfPops(AssemblyItems const& _items)
{
	return static_cast<size_t>(std::count(_items.begin(), _items.end(), Instruction::POP));
}

}

bool PeepholeOptimiser::optimise()
{
	// Avoid referencing immutables too early by using approx. counting in bytesRequired()
	auto const approx = evmasm::Precision::Approximate;
	OptimiserState state {m_items, 0, std::back_inserter(m_optimisedItems)};
	while (state.i < m_items.size())
		applyMethods(
			state,
			PushPop(), OpPop(), DoublePush(), DoubleSwap(), CommutativeSwap(), SwapComparison(),
			DupSwap(), IsZeroIsZeroJumpI(), EqIsZeroJumpI(), DoubleJump(), JumpToNext(), UnreachableCode(),
			TagConjunctions(), TruthyAnd(), Identity()
		);
	if (m_optimisedItems.size() < m_items.size() || (
		m_optimisedItems.size() == m_items.size() && (
			evmasm::bytesRequired(m_optimisedItems, 3, approx) < evmasm::bytesRequired(m_items, 3, approx) ||
			numberOfPops(m_optimisedItems) > numberOfPops(m_items)
		)
	))
	{
		m_items = std::move(m_optimisedItems);
		return true;
	}
	else
		return false;
}
