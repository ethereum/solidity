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
/** @file PathGasMeter.cpp
 * @author Christian <c@ethdev.com>
 * @date 2015
 */

#pragma once

#include <libevmasm/GasMeter.h>

#include <liblangutil/EVMVersion.h>

#include <set>
#include <vector>
#include <memory>

namespace dev
{
namespace eth
{

class KnownState;

struct GasPath
{
	size_t index = 0;
	std::shared_ptr<KnownState> state;
	u256 largestMemoryAccess;
	GasMeter::GasConsumption gas;
	std::set<size_t> visitedJumpdests;
};

/**
 * Computes an upper bound on the gas usage of a computation starting at a certain position in
 * a list of AssemblyItems in a given state until the computation stops.
 * Can be used to estimate the gas usage of functions on any given input.
 */
class PathGasMeter
{
public:
	explicit PathGasMeter(AssemblyItems const& _items, langutil::EVMVersion _evmVersion);

	GasMeter::GasConsumption estimateMax(size_t _startIndex, std::shared_ptr<KnownState> const& _state);

	static GasMeter::GasConsumption estimateMax(
		AssemblyItems const& _items,
		langutil::EVMVersion _evmVersion,
		size_t _startIndex,
		std::shared_ptr<KnownState> const& _state
	)
	{
		return PathGasMeter(_items, _evmVersion).estimateMax(_startIndex, _state);
	}

private:
	/// Adds a new path item to the queue, but only if we do not already have
	/// a higher gas usage at that point.
	/// This is not exact as different state might influence higher gas costs at a later
	/// point in time, but it greatly reduces computational overhead.
	void queue(std::unique_ptr<GasPath>&& _newPath);
	GasMeter::GasConsumption handleQueueItem();

	/// Map of jumpdest -> gas path, so not really a queue. We only have one queued up
	/// item per jumpdest, because of the behaviour of `queue` above.
	std::map<size_t, std::unique_ptr<GasPath>> m_queue;
	std::map<size_t, GasMeter::GasConsumption> m_highestGasUsagePerJumpdest;
	std::map<u256, size_t> m_tagPositions;
	AssemblyItems const& m_items;
	langutil::EVMVersion m_evmVersion;
};

}
}
