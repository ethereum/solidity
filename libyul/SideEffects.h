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

#pragma once

#include <set>

namespace solidity::yul
{

/**
 * Side effects of code.
 *
 * The default-constructed value applies to the "empty code".
 */
struct SideEffects
{
	/// If true, expressions in this code can be freely moved and copied without altering the
	/// semantics.
	/// At statement level, it means that functions containing this code can be
	/// called multiple times, their calls can be rearranged and calls can also be
	/// deleted without changing the semantics.
	/// This means it cannot depend on storage or memory, cannot have any side-effects,
	/// but it can depend on state that is constant across an EVM-call.
	bool movable = true;
	/// If true, the expressions in this code can be freely moved over other statements, expressions
	/// or function calls, as long as these do not invalidate the blockchain state.
	bool movableIfStateInvariant = true;
	/// If true, the expression in this code can be freely moved over other statements, expressions
	/// or function calls, as long as these do not invalidate the storage or the blockchain state.
	bool movableIfStorageInvariant = true;
	/// If true, the expressions in this code can be freely moved over other statements, expressions
	/// or function calls, as long as these do not invalidate memory or contain a MSIZE.
	bool movableIfMemoryInvariant = true;
	/// If true, the code can be removed without changing the semantics.
	bool sideEffectFree = true;
	/// If true, the code can be removed without changing the semantics as long as
	/// the whole program does not contain the msize instruction.
	bool sideEffectFreeIfNoMSize = true;
	/// If false, the blockchain state is guaranteed to be unchanged by the code under all
	/// circumstances.
	bool invalidatesState = false;
	/// If false, storage and the blockchain state is guaranteed to be unchanged by the code under
	/// all circumstances.
	bool invalidatesStorage = false;
	/// If false, memory is guaranteed to be unchanged by the code under all
	/// circumstances.
	bool invalidatesMemory = false;
	/// If true, the block will contain a MSIZE
	bool containsMSize = false;

	SideEffects() {}
	SideEffects(
		bool _movable,
		bool _movableIfStateInvariant,
		bool _movableIfStorageInvariant,
		bool _movableIfMemoryInvariant,
		bool _sideEffectFree,
		bool _sideEffectFreeIfNoMSize,
		bool _invalidatesState,
		bool _invalidatesStorage,
		bool _invalidatesMemory,
		bool _containsMSize
	):
		movable(_movable),
		movableIfStateInvariant(_movableIfStateInvariant),
		movableIfStorageInvariant(_movableIfStorageInvariant),
		movableIfMemoryInvariant(_movableIfMemoryInvariant),
		sideEffectFree(_sideEffectFree),
		sideEffectFreeIfNoMSize(_sideEffectFreeIfNoMSize),
		invalidatesState(_invalidatesState),
		invalidatesStorage(_invalidatesStorage),
		invalidatesMemory(_invalidatesMemory),
		containsMSize(_containsMSize)
	{}
	/// @returns the worst-case side effects.
	static SideEffects worst()
	{
		return SideEffects{
			false, // movable
			false, // movableIfStateInvariant
			false, // movableIfStorageInvariant
			false, // movableIfMemoryInvariant
			false, // sideEffectFree
			false, // sideEffectFreeIfNoMSize
			true,  // invalidatesState
			true,  // invalidatesStorage
			true,  // invalidatesMemory
			true   // containsMSize
		};
	}

	/// @returns the combined side effects of two pieces of code.
	SideEffects operator+(SideEffects const& _other)
	{
		return SideEffects{
			movable && _other.movable,
			movableIfStateInvariant && _other.movableIfStateInvariant,
			movableIfStorageInvariant && _other.movableIfStorageInvariant,
			movableIfMemoryInvariant && _other.movableIfMemoryInvariant,
			sideEffectFree && _other.sideEffectFree,
			sideEffectFreeIfNoMSize && _other.sideEffectFreeIfNoMSize,
			invalidatesState || _other.invalidatesState,
			invalidatesStorage || _other.invalidatesStorage,
			invalidatesMemory || _other.invalidatesMemory,
			containsMSize || _other.containsMSize,
		};
	}

	/// Adds the side effects of another piece of code to this side effect.
	SideEffects& operator+=(SideEffects const& _other)
	{
		*this = *this + _other;
		return *this;
	}

	bool operator==(SideEffects const& _other) const
	{
		return
			movable == _other.movable &&
			movableIfStateInvariant == _other.movableIfStateInvariant &&
			movableIfStorageInvariant == _other.movableIfStorageInvariant &&
			movableIfMemoryInvariant == _other.movableIfMemoryInvariant &&
			sideEffectFree == _other.sideEffectFree &&
			sideEffectFreeIfNoMSize == _other.sideEffectFreeIfNoMSize &&
			invalidatesState == _other.invalidatesState &&
			invalidatesStorage == _other.invalidatesStorage &&
			invalidatesMemory == _other.invalidatesMemory &&
			containsMSize == _other.containsMSize;
	}
};

}
