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

#include <set>
#include <vector>
#include <memory>
#include <libevmasm/GasMeter.h>

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
	PathGasMeter(AssemblyItems const& _items);

	GasMeter::GasConsumption estimateMax(size_t _startIndex, std::shared_ptr<KnownState> const& _state);

private:
	GasMeter::GasConsumption handleQueueItem();

	std::vector<std::unique_ptr<GasPath>> m_queue;
	std::map<u256, size_t> m_tagPositions;
	AssemblyItems const& m_items;
};

}
}
