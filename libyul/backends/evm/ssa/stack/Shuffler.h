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

#include <libyul/backends/evm/ssa/SSACFGTypes.h>
#include <libyul/backends/evm/ssa/ShuffleTrace.h>

#include <cstddef>
#include <optional>
#include <vector>

namespace solidity::yul::ssa::spill
{
class SpillSet;
}

namespace solidity::yul::ssa::stack
{

struct ShuffleResult
{
	enum class Status { Admissible, StackTooDeep };
	Status status = Status::Admissible;
	ShuffleTrace trace{};
	/// The plan: for every target offset the source offset that serves it; generated offsets are empty
	std::vector<std::optional<std::size_t>> sourceOf{};
};

/// Shuffles `_source` positionally to `_target` (junk slots in the target are wildcards) and returns the trace of
/// stack operations realizing it
[[nodiscard]] ShuffleResult shuffle(
	StackData& _source,
	StackData const& _target,
	spill::SpillSet& _spills,
	bool _spillingAllowed = true,
	std::size_t _reachableStackDepth = reachableStackDepth
);

}
