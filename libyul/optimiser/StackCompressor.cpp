/*(
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
/**
 * Optimisation stage that aggressively rematerializes certain variables ina a function to free
 * space on the stack until it is compilable.
 */

#include <libyul/optimiser/StackCompressor.h>

#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/Semantics.h>

#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{

template <typename ASTNode>
void eliminateVariables(shared_ptr<Dialect> const& _dialect, ASTNode& _node, size_t _numVariables)
{
	SSAValueTracker ssaValues;
	ssaValues(_node);

	map<YulString, size_t> references = ReferencesCounter::countReferences(_node);

	set<pair<size_t, YulString>> rematCosts;
	for (auto const& ssa: ssaValues.values())
	{
		if (!MovableChecker{*_dialect, *ssa.second}.movable())
			continue;
		size_t numRef = references[ssa.first];
		size_t cost = 0;
		if (numRef > 1)
			cost = CodeCost::codeCost(*ssa.second) * (numRef - 1);
		rematCosts.insert(make_pair(cost, ssa.first));
	}

	// Select at most _numVariables
	set<YulString> varsToEliminate;
	for (auto const& costs: rematCosts)
	{
		if (varsToEliminate.size() >= _numVariables)
			break;
		varsToEliminate.insert(costs.second);
	}

	Rematerialiser::run(*_dialect, _node, std::move(varsToEliminate));
	UnusedPruner::runUntilStabilised(*_dialect, _node);
}

}

bool StackCompressor::run(shared_ptr<Dialect> const& _dialect, Block& _ast)
{
	yulAssert(
		_ast.statements.size() > 0 && _ast.statements.at(0).type() == typeid(Block),
		"Need to run the function grouper before the stack compressor."
	);
	for (size_t iterations = 0; iterations < 4; iterations++)
	{
		map<YulString, int> stackSurplus = CompilabilityChecker::run(_dialect, _ast);
		if (stackSurplus.empty())
			return true;

		if (stackSurplus.count(YulString{}))
		{
			yulAssert(stackSurplus.at({}) > 0, "Invalid surplus value.");
			eliminateVariables(_dialect, boost::get<Block>(_ast.statements.at(0)), stackSurplus.at({}));
		}

		for (size_t i = 1; i < _ast.statements.size(); ++i)
		{
			FunctionDefinition& fun = boost::get<FunctionDefinition>(_ast.statements[i]);
			if (!stackSurplus.count(fun.name))
				continue;

			yulAssert(stackSurplus.at(fun.name) > 0, "Invalid surplus value.");
			eliminateVariables(_dialect, fun, stackSurplus.at(fun.name));
		}
	}
	return false;
}

