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

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>

#include <fmt/format.h>

#include <set>

using namespace solidity::yul::ssa;

ControlFlowGraphsLiveness::ControlFlowGraphsLiveness(ControlFlowGraphs const& _controlFlow):
	controlFlowGraphs(_controlFlow),
	cfgLiveness(_controlFlow.functionGraphs | ranges::views::transform([](auto const& _cfg) { return std::make_unique<LivenessAnalysis>(*_cfg); }) | ranges::to<std::vector>)
{ }

std::string ControlFlowGraphsLiveness::toDot() const
{
	return controlFlowGraphs.get().toDot(this);
}

#ifdef SLOW_DEBUG
// Checks: (1) there is at least one graph, (2) the graph at mainGraphID() is the main one,
//         (3) all CFG names are unique, then (4) calls the invariant on the CFGs themselves
void ControlFlowGraphs::checkInvariants() const
{
	yulAssert(!functionGraphs.empty(), "No control flow graphs.");
	std::set<std::string> names;
	for (auto const& [index, cfg]: functionGraphs | ranges::views::enumerate)
	{
		if (index == mainGraphID())
			yulAssert(cfg->isMainGraph(), fmt::format("Graph in the main slot is named '{}'", cfg->name));
		else
			yulAssert(!cfg->isMainGraph(), fmt::format("Graph {} is unnamed, i.e. a second main graph", index));
		yulAssert(names.insert(cfg->name).second, fmt::format("Duplicate CFG name '{}'", cfg->name));
		cfg->checkInvariants();
	}
}
#endif
