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
#include <libsolutil/Visitor.h>
#include <liblangutil/Exceptions.h>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>

namespace solidity::util
{

// TODO: This is currently only used for permuteDup as special case handling, which is not the best way to do things.
//       Not worth spending time reviewing this.
template<typename GetTargetPosition, typename Swap, typename Pop>
void permute(unsigned _n, GetTargetPosition _getTargetPosition, Swap _swap, Pop _pop)
{
	static_assert(
		std::is_same_v<std::invoke_result_t<GetTargetPosition, unsigned>, int>,
		"_getTargetPosition needs to have the signature int(unsigned)"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Swap, unsigned>, void>,
		"_swap needs to have the signature void(unsigned)"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Pop>, void>,
		"_pop needs to have the signature void()"
	);
	if (_n == 0) return;
	int targetPositionTop = _getTargetPosition(_n - 1);

	if (targetPositionTop < 0)
	{
		// The last element should not be kept.
		// Pop it and recurse.
		_pop();
		permute(_n - 1, _getTargetPosition, _swap, _pop);
		return;
	}
	// TODO: exception?
	//	assertThrow(static_cast<unsigned>(targetPositionTop) < _n, langutil::InternalCompilerError, "Invalid permutation.");
	if (static_cast<unsigned>(targetPositionTop) == _n - 1)
	{
		// The last element is in position.
		// Search for the deepest element that is not in position.
		// If there is none, we are done. Otherwise swap it up and recurse.
		for (int i = 0; i < static_cast<int>(_n - 1); ++i)
			if (_getTargetPosition(static_cast<unsigned>(i)) != i)
			{
				_swap(_n - static_cast<unsigned>(i) - 1);
				permute(_n, _getTargetPosition, _swap, _pop);
				return;
			}
	}
	else
	{
		// The last element is not in position.
		// Move it to its position and recurse.
		_swap(_n - static_cast<unsigned>(targetPositionTop) - 1);
		permute(_n, _getTargetPosition, _swap, _pop);
	}
}

// TODO: This is now only used in StackLayoutGenerator.cpp in ``createIdealLayout`` and that usage is actually abuse,
//       since it provides "invalid" target positions (it works, but it's not soundly specified).
//       Hence ``createIdealLayout`` should rather be rewritten properly and it does not make much sense to review
//       this in detail.
template<typename GetTargetPositions, typename Swap, typename Dup, typename Push, typename Pop>
void permuteDup(unsigned _n, GetTargetPositions _getTargetPositions, Swap _swap, Dup _dup, Push _push, Pop _pop, bool _debug = false)
{
	static_assert(
		std::is_same_v<std::invoke_result_t<GetTargetPositions, unsigned>, std::set<unsigned>>,
		"_getTargetPosition needs to have the signature std::vector<int>(unsigned)"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Swap, unsigned>, void>,
		"_swap needs to have the signature void(unsigned)"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Dup, unsigned>, void>,
		"_dup needs to have the signature void(unsigned)"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Push>, void>,
		"_push needs to have the signature void()"
	);
	static_assert(
		std::is_same_v<std::invoke_result_t<Pop>, void>,
		"_pop needs to have the signature void()"
	);
	if (_n == 0) return;

	if (_debug)
	{
		for (auto offset: ranges::views::iota(0u, _n))
		{
			auto targetPositions = _getTargetPositions(offset);
			std::cout << "{ ";
			for (auto pos: targetPositions)
				std::cout << pos << " ";
			std::cout << "} ";
		}
		std::cout << std::endl;
	}

	std::set<unsigned> targetPositionsTop = _getTargetPositions(_n - 1);

	if (targetPositionsTop.empty())
	{
		// The last element should not be kept.
		// Pop it and recurse.
		_pop();
		permuteDup(_n - 1, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
		return;
	}
	if (targetPositionsTop.count(_n - 1))
	{
		if (_debug)
			std::cout << "Top position should stay" << std::endl;
		// The last element should remain at the top (but potentially also be dupped).
		/*if (targetPositionsTop.size() > 1)
		{
			std::cout << "TOP targets: { ";
			for (auto i: targetPositionsTop)
				std::cout << i << " ";
			std::cout << "}" << std::endl;
			// The last element should remain at the top and be dupped. Dup it and recurse.
			_dup(1);
			permuteDup(_n + 1, _getTargetPositions, _swap, _dup, _push, _pop);
			return;
		}
		else*/
		{
			if (_debug)
				std::cout << "Look for deeper element to be dupped." << std::endl;
			// The last element should *only* exist at the current top.
			// Look for the deepest element that should still be dupped.
			for (auto offset: ranges::views::iota(0u, _n))
			{
				auto targetPositions = _getTargetPositions(offset);
				if (targetPositions.size() > 1)
				{
					if (_debug)
						std::cout << "DUP element " << offset << " (DUP" << (_n - offset) << ")" << std::endl;
					// Dup it, adjust the target positions and recurse.
					// The next recursion will move the duplicate in place.
					_dup(_n - offset);
					permuteDup(_n + 1, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
					return;
				}
			}
			// There is no more dupping requested, so we can switch to the non-dupping version.
			permute(_n, [&](unsigned _i) -> int {
				auto const& targetPositions = _getTargetPositions(_i);
				if (targetPositions.empty())
					return -1;
				else
				{
					assertThrow(targetPositions.size() == 1, langutil::InternalCompilerError, "");
					return static_cast<int>(*targetPositions.begin());
				}
			}, _swap, _pop);
			return;
		}
	}
	else
	{
		// The last element should end up at *some* position that isn't its current one.
		auto topTargetPos = *targetPositionsTop.begin();
		if (_debug)
			std::cout << "Top target pos: " << topTargetPos << std::endl;
		if (topTargetPos < _n - 1)
		{
			// If the element is supposed to exist anywhere deeper than the current top, swap it there and recurse.
			_swap(_n - static_cast<unsigned>(topTargetPos) - 1);
			permuteDup(_n, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
			return;
		}
		else
		{
			// If there is an element that is supposed to be dupped to the current top position. Find it, dup it and recurse.
			for (auto offset: ranges::views::iota(0u, _n))
			{
				auto targetPositions = _getTargetPositions(offset);
				if (targetPositions.size() > 1 && targetPositions.count(static_cast<unsigned>(_n)))
				{
					_dup(static_cast<unsigned>(targetPositions.size() - offset));
					permuteDup(_n + 1, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
					return;
				}
			}
			// If there is any other element that is supposed to be dupped. Find it, dup it and recurse.
			for (auto offset: ranges::views::iota(0u, _n))
			{
				auto targetPositions = _getTargetPositions(offset);
				if (targetPositions.size() > 1)
				{
					_dup(static_cast<unsigned>(targetPositions.size() - offset));
					permuteDup(_n + 1, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
					return;
				}
			}
			// There must be a new element requested. Request it to be pushed and recurse.
			_push();
			permuteDup(_n + 1, _getTargetPositions, _swap, _dup, _push, _pop, _debug);
			return;

			assertThrow(false, langutil::InternalCompilerError, "Invalid permutation.");
		}
	}
}

}
