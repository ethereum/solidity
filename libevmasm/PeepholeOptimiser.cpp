/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file PeepholeOptimiser.h
 * Performs local optimising code changes to assembly.
 */

#include "PeepholeOptimiser.h"

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace dev::eth;
using namespace dev;

// TODO: Extend this to use the tools from ExpressionClasses.cpp

struct Identity
{
	static size_t windowSize() { return 1; }
	static bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		*_out = *_in;
		return true;
	}
};

struct PushPop
{
	static size_t windowSize() { return 2; }
	static bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems>)
	{
		auto t = _in[0].type();
		if (_in[1] == Instruction::POP && (
			SemanticInformation::isDupInstruction(_in[0]) ||
			t == Push || t == PushString || t == PushTag || t == PushSub ||
			t == PushSubSize || t == PushProgramSize || t == PushData || t == PushLibraryAddress
		))
			return true;
		else
			return false;
	}
};

struct DoubleSwap
{
	static size_t windowSize() { return 2; }
	static bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems>)
	{
		if (_in[0] == _in[1] && SemanticInformation::isSwapInstruction(_in[0]))
			return true;
		else
			return false;
	}
};

struct JumpToNext
{
	static size_t windowSize() { return 3; }
	static bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		if (
			_in[0].type() == PushTag &&
			(_in[1] == Instruction::JUMP || _in[1] == Instruction::JUMPI) &&
			_in[2].type() == Tag &&
			_in[0].data() == _in[2].data()
		)
		{
			if (_in[1] == Instruction::JUMPI)
				*_out = AssemblyItem(Instruction::POP, _in[1].location());
			*_out = _in[2];
			return true;
		}
		else
			return false;
	}
};

struct TagConjunctions
{
	static size_t windowSize() { return 3; }
	static bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out)
	{
		if (
			_in[0].type() == PushTag &&
			_in[2] == Instruction::AND &&
			_in[1].type() == Push &&
			(_in[1].data() & u256(0xFFFFFFFF)) == u256(0xFFFFFFFF)
		)
		{
			*_out = _in[0];
			return true;
		}
		else
			return false;
	}
};

struct OptimiserState
{
	AssemblyItems const& items;
	size_t i;
	std::back_insert_iterator<AssemblyItems> out;
};

void applyMethods(OptimiserState&)
{
	assertThrow(false, OptimizerException, "Peephole optimizer failed to apply identity.");
}

template <typename Method, typename... OtherMethods>
void applyMethods(OptimiserState& _state, Method, OtherMethods... _other)
{
	if (_state.i + Method::windowSize() <= _state.items.size() && Method::apply(_state.items.begin() + _state.i, _state.out))
		_state.i += Method::windowSize();
	else
		applyMethods(_state, _other...);
}

bool PeepholeOptimiser::optimise()
{
	OptimiserState state {m_items, 0, std::back_inserter(m_optimisedItems)};
	while (state.i < m_items.size())
		applyMethods(state, PushPop(), DoubleSwap(), JumpToNext(), TagConjunctions(), Identity());
	if (m_optimisedItems.size() < m_items.size())
	{
		m_items = std::move(m_optimisedItems);
		return true;
	}
	else
		return false;

}
