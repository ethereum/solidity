// SPDX-License-Identifier: GPL-3.0

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
	/// If true, the code can be removed without changing the semantics.
	bool sideEffectFree = true;
	/// If true, the code can be removed without changing the semantics as long as
	/// the whole program does not contain the msize instruction.
	bool sideEffectFreeIfNoMSize = true;
	/// If false, storage is guaranteed to be unchanged by the code under all
	/// circumstances.
	bool invalidatesStorage = false;
	/// If false, memory is guaranteed to be unchanged by the code under all
	/// circumstances.
	bool invalidatesMemory = false;

	/// @returns the worst-case side effects.
	static SideEffects worst()
	{
		return SideEffects{false, false, false, true, true};
	}

	/// @returns the combined side effects of two pieces of code.
	SideEffects operator+(SideEffects const& _other)
	{
		return SideEffects{
			movable && _other.movable,
			sideEffectFree && _other.sideEffectFree,
			sideEffectFreeIfNoMSize && _other.sideEffectFreeIfNoMSize,
			invalidatesStorage || _other.invalidatesStorage,
			invalidatesMemory || _other.invalidatesMemory
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
			sideEffectFree == _other.sideEffectFree &&
			sideEffectFreeIfNoMSize == _other.sideEffectFreeIfNoMSize &&
			invalidatesStorage == _other.invalidatesStorage &&
			invalidatesMemory == _other.invalidatesMemory;
	}
};

}
