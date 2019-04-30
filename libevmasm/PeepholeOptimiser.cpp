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
/**
 * @file PeepholeOptimiser.cpp
 * Performs local optimising code changes to assembly.
 */

#include <libevmasm/PeepholeOptimiser.h>

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace dev::eth;
using namespace dev;

// TODO: Extend this to use the tools from ExpressionClasses.cpp

namespace
{

struct OptimiserState
{
	AssemblyItems const& items;
	size_t i;
	std::back_insert_iterator<AssemblyItems> out;
};

template <class Method, size_t Arguments>
struct ApplyRule
{
};
template <class Method>
struct ApplyRule<Method, 4>
{
	static bool applyRule(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		return Method::applySimple(_in[0], _in[1], _in[2], _in[3], _out);
	}
};
template <class Method>
struct ApplyRule<Method, 3>
{
	static bool applyRule(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		return Method::applySimple(_in[0], _in[1], _in[2], _out);
	}
};
template <class Method>
struct ApplyRule<Method, 2>
{
	static bool applyRule(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		return Method::applySimple(_in[0], _in[1], _out);
	}
};
template <class Method>
struct ApplyRule<Method, 1>
{
	static bool applyRule(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		return Method::applySimple(_in[0], _out);
	}
};

template <class Method, size_t WindowSize>
struct SimplePeepholeOptimizerMethod
{
	static bool apply(OptimiserState& _state)
	{
		if (
			_state.i + WindowSize <= _state.items.size() &&
			ApplyRule<Method, WindowSize>::applyRule(_state.items.begin() + _state.i, _state.out)
		)
		{
			_state.i += WindowSize;
			return true;
		}
		else
			return false;
	}
};

struct Identity: SimplePeepholeOptimizerMethod<Identity, 1>
{
	static bool applySimple(AssemblyItem const& _item, std::back_insert_iterator<AssemblyItems> _out)
	{
		*_out = _item;
		return true;
	}
};

struct PushPop: SimplePeepholeOptimizerMethod<PushPop, 2>
{
	static bool applySimple(AssemblyItem const& _push, AssemblyItem const& _pop, std::back_insert_iterator<AssemblyItems>)
	{
		auto t = _push.type();
		return _pop == Instruction::POP && (
			SemanticInformation::isDupInstruction(_push) ||
			t == Push || t == PushString || t == PushTag || t == PushSub ||
			t == PushSubSize || t == PushProgramSize || t == PushData || t == PushLibraryAddress
		);
	}
};

struct OpPop: SimplePeepholeOptimizerMethod<OpPop, 2>
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

struct DoubleSwap: SimplePeepholeOptimizerMethod<DoubleSwap, 2>
{
	static size_t applySimple(AssemblyItem const& _s1, AssemblyItem const& _s2, std::back_insert_iterator<AssemblyItems>)
	{
		return _s1 == _s2 && SemanticInformation::isSwapInstruction(_s1);
	}
};

struct DoublePush: SimplePeepholeOptimizerMethod<DoublePush, 2>
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

struct CommutativeSwap: SimplePeepholeOptimizerMethod<CommutativeSwap, 2>
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

struct SwapComparison: SimplePeepholeOptimizerMethod<SwapComparison, 2>
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

struct IsZeroIsZeroJumpI: SimplePeepholeOptimizerMethod<IsZeroIsZeroJumpI, 4>
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

struct JumpToNext: SimplePeepholeOptimizerMethod<JumpToNext, 3>
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

struct TagConjunctions: SimplePeepholeOptimizerMethod<TagConjunctions, 3>
{
	static bool applySimple(
		AssemblyItem const& _pushTag,
		AssemblyItem const& _pushConstant,
		AssemblyItem const& _and,
		std::back_insert_iterator<AssemblyItems> _out
	)
	{
		if (
			_pushTag.type() == PushTag &&
			_and == Instruction::AND &&
			_pushConstant.type() == Push &&
			(_pushConstant.data() & u256(0xFFFFFFFF)) == u256(0xFFFFFFFF)
		)
		{
			*_out = _pushTag;
			return true;
		}
		else
			return false;
	}
};

struct TruthyAnd: SimplePeepholeOptimizerMethod<TruthyAnd, 3>
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
		auto it = _state.items.begin() + _state.i;
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

		size_t i = 1;
		while (it + i != end && it[i].type() != Tag)
			i++;
		if (i > 1)
		{
			*_state.out = it[0];
			_state.i += i;
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
	return std::count(_items.begin(), _items.end(), Instruction::POP);
}

}

bool PeepholeOptimiser::optimise()
{
	OptimiserState state {m_items, 0, std::back_inserter(m_optimisedItems)};
	while (state.i < m_items.size())
		applyMethods(
			state,
			PushPop(), OpPop(), DoublePush(), DoubleSwap(), CommutativeSwap(), SwapComparison(),
			IsZeroIsZeroJumpI(), JumpToNext(), UnreachableCode(),
			TagConjunctions(), TruthyAnd(), Identity()
		);
	if (m_optimisedItems.size() < m_items.size() || (
		m_optimisedItems.size() == m_items.size() && (
			eth::bytesRequired(m_optimisedItems, 3) < eth::bytesRequired(m_items, 3) ||
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
