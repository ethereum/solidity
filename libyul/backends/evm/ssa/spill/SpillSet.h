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
#include <libyul/backends/evm/ssa/StackSlot.h>

#include <libyul/Exceptions.h>

#include <cstdint>
#include <deque>
#include <map>
#include <set>

namespace solidity::yul::ssa::spill
{

/// Per def site of a spilled variable the recorded shuffle realizing its store: brings the variable to the
/// stack top and concludes with the `Store` op consuming it. Keyed by the defining Inst.
using SpillStoreTraces = std::map<InstId, ShuffleTrace>;

/// Per-CFG set of variables spilled to memory
class SpillSet
{
public:
	void add(SpillKey const _key)
	{
		yulAssert(_key.isVariable(), fmt::format("only variables can be spilled, not {}", _key));
		bool const inserted = m_values.insert(_key).second;
		yulAssert(inserted, fmt::format("can't spill a variable ({}) twice", _key));
	}

	bool isSpilled(SpillKey const _key) const { return m_values.contains(_key); }

	std::size_t numSpilled() const { return m_values.size(); }

	std::set<SpillKey> const& spilledValues() const { return m_values; }

	/// Finalizes the spill set by making every spilled value's def-site `mstore` reachable.
	/// If `_storeTraces` is provided, it is rebuilt to hold each spilled value's recorded def-site store trace.
	void closeUnderReachabilityConstraints(SSACFG const& _cfg, SSACFGStackLayout const& _layout, SpillStoreTraces* _storeTraces = nullptr);

	/// Yields a copy of this spill set minus `_key`.
	[[nodiscard]] SpillSet without(SpillKey _key) const;

private:
	/// Ensure that the variable `_key` can be spilled, i.e.,
	/// brought up to the top and `mstore`d. Might populate the spill set with more entries if not possible right away.
	void ensureDefSiteFeasible(SpillKey _key, StackData const& _defStack, std::deque<SpillKey>& _workQueue, SpillStoreTraces* _storeTraces);

	std::set<SpillKey> m_values;
};

}
