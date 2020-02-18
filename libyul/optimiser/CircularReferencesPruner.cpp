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
#include <libyul/optimiser/CircularReferencesPruner.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/AsmData.h>

#include <libsolutil/Algorithms.h>

using namespace std;
using namespace solidity::yul;

void CircularReferencesPruner::run(OptimiserStepContext& _context, Block& _ast)
{
	CircularReferencesPruner{_context.reservedIdentifiers}(_ast);
}

void CircularReferencesPruner::operator()(Block& _block)
{
	set<YulString> functionsToKeep =
		functionsCalledFromOutermostContext(CallGraphGenerator::callGraph(_block));

	for (auto&& statement: _block.statements)
		if (holds_alternative<FunctionDefinition>(statement))
		{
			FunctionDefinition const& funDef = std::get<FunctionDefinition>(statement);
			if (!functionsToKeep.count(funDef.name))
				statement = Block{};
		}

	removeEmptyBlocks(_block);
}

set<YulString> CircularReferencesPruner::functionsCalledFromOutermostContext(CallGraph const& _callGraph)
{
	set<YulString> verticesToTraverse = m_reservedIdentifiers;
	verticesToTraverse.insert(YulString(""));

	return util::BreadthFirstSearch<YulString>{{verticesToTraverse.begin(), verticesToTraverse.end()}}.run(
		[&_callGraph](YulString _function, auto&& _addChild) {
			if (_callGraph.functionCalls.count(_function))
				for (auto const& callee: _callGraph.functionCalls.at(_function))
					if (_callGraph.functionCalls.count(callee))
						_addChild(callee);
		}).visited;
}
