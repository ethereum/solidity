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

#include "PathGasMeter.h"
#include <libevmasm/KnownState.h>
#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

PathGasMeter::PathGasMeter(AssemblyItems const& _items, langutil::EVMVersion _evmVersion):
	m_items(_items), m_evmVersion(_evmVersion)
{
	for (size_t i = 0; i < m_items.size(); ++i)
		if (m_items[i].type() == Tag)
			m_tagPositions[m_items[i].data()] = i;
}

GasMeter::GasConsumption PathGasMeter::estimateMax(
	size_t _startIndex,
	shared_ptr<KnownState> const& _state
)
{
	auto path = unique_ptr<GasPath>(new GasPath());
	path->index = _startIndex;
	path->state = _state->copy();
	queue(move(path));

	GasMeter::GasConsumption gas;
	while (!m_queue.empty() && !gas.isInfinite)
		gas = max(gas, handleQueueItem());
	return gas;
}

void PathGasMeter::queue(std::unique_ptr<GasPath>&& _newPath)
{
	if (
		m_highestGasUsagePerJumpdest.count(_newPath->index) &&
		_newPath->gas < m_highestGasUsagePerJumpdest.at(_newPath->index)
	)
		return;
	m_highestGasUsagePerJumpdest[_newPath->index] = _newPath->gas;
	m_queue[_newPath->index] = move(_newPath);
}

GasMeter::GasConsumption PathGasMeter::handleQueueItem()
{
	assertThrow(!m_queue.empty(), OptimizerException, "");

	unique_ptr<GasPath> path = move(m_queue.rbegin()->second);
	m_queue.erase(--m_queue.end());

	shared_ptr<KnownState> state = path->state;
	GasMeter meter(state, m_evmVersion, path->largestMemoryAccess);
	ExpressionClasses& classes = state->expressionClasses();
	GasMeter::GasConsumption gas = path->gas;
	size_t index = path->index;

	if (index >= m_items.size() || (index > 0 && m_items.at(index).type() != Tag))
		// Invalid jump usually provokes an out-of-gas exception, but we want to give an upper
		// bound on the gas that is needed without changing the behaviour, so it is fine to
		// return the current gas value.
		return gas;

	set<u256> jumpTags;
	for (; index < m_items.size() && !gas.isInfinite; ++index)
	{
		bool branchStops = false;
		jumpTags.clear();
		AssemblyItem const& item = m_items.at(index);
		if (item.type() == Tag || item == AssemblyItem(Instruction::JUMPDEST))
		{
			// Do not allow any backwards jump. This is quite restrictive but should work for
			// the simplest things.
			if (path->visitedJumpdests.count(index))
				return GasMeter::GasConsumption::infinite();
			path->visitedJumpdests.insert(index);
		}
		else if (item == AssemblyItem(Instruction::JUMP))
		{
			branchStops = true;
			jumpTags = state->tagsInExpression(state->relativeStackElement(0));
			if (jumpTags.empty()) // unknown jump destination
				return GasMeter::GasConsumption::infinite();
		}
		else if (item == AssemblyItem(Instruction::JUMPI))
		{
			ExpressionClasses::Id condition = state->relativeStackElement(-1);
			if (classes.knownNonZero(condition) || !classes.knownZero(condition))
			{
				jumpTags = state->tagsInExpression(state->relativeStackElement(0));
				if (jumpTags.empty()) // unknown jump destination
					return GasMeter::GasConsumption::infinite();
			}
			branchStops = classes.knownNonZero(condition);
		}
		else if (SemanticInformation::altersControlFlow(item))
			branchStops = true;

		gas += meter.estimateMax(item);

		for (u256 const& tag: jumpTags)
		{
			auto newPath = unique_ptr<GasPath>(new GasPath());
			newPath->index = m_items.size();
			if (m_tagPositions.count(tag))
				newPath->index = m_tagPositions.at(tag);
			newPath->gas = gas;
			newPath->largestMemoryAccess = meter.largestMemoryAccess();
			newPath->state = state->copy();
			newPath->visitedJumpdests = path->visitedJumpdests;
			queue(move(newPath));
		}

		if (branchStops)
			break;
	}

	return gas;
}
