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
/**
 * Specific AST walker that generates the call graph.
 */

#include <libyul/AST.h>
#include <libyul/optimiser/CallGraphGenerator.h>

#include <stack>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

namespace
{
// TODO: This algorithm is non-optimal.
struct CallGraphCycleFinder
{
	CallGraph const& callGraph;
	set<YulString> containedInCycle{};
	set<YulString> visited{};
	vector<YulString> currentPath{};

	void visit(YulString _function)
	{
		if (visited.count(_function))
			return;
		if (
			auto it = find(currentPath.begin(), currentPath.end(), _function);
			it != currentPath.end()
		)
			containedInCycle.insert(it, currentPath.end());
		else
		{
			currentPath.emplace_back(_function);
			if (callGraph.functionCalls.count(_function))
				for (auto const& child: callGraph.functionCalls.at(_function))
					visit(child);
			currentPath.pop_back();
			visited.insert(_function);
		}
	}
};
}

set<YulString> CallGraph::recursiveFunctions() const
{
	CallGraphCycleFinder cycleFinder{*this};
	// Visiting the root only is not enough, since there may be disconnected recursive functions.
	for (auto const& call: functionCalls)
		cycleFinder.visit(call.first);
	return cycleFinder.containedInCycle;
}

CallGraph CallGraphGenerator::callGraph(Block const& _ast)
{
	CallGraphGenerator gen;
	gen(_ast);
	return std::move(gen.m_callGraph);
}

void CallGraphGenerator::operator()(FunctionCall const& _functionCall)
{
	m_callGraph.functionCalls[m_currentFunction].insert(_functionCall.functionName.name);
	ASTWalker::operator()(_functionCall);
}

void CallGraphGenerator::operator()(ForLoop const& _forLoop)
{
	m_callGraph.functionsWithLoops.insert(m_currentFunction);
	ASTWalker::operator()(_forLoop);
}

void CallGraphGenerator::operator()(FunctionDefinition const& _functionDefinition)
{
	YulString previousFunction = m_currentFunction;
	m_currentFunction = _functionDefinition.name;
	yulAssert(m_callGraph.functionCalls.count(m_currentFunction) == 0, "");
	m_callGraph.functionCalls[m_currentFunction] = {};
	ASTWalker::operator()(_functionDefinition);
	m_currentFunction = previousFunction;
}

CallGraphGenerator::CallGraphGenerator()
{
	m_callGraph.functionCalls[YulString{}] = {};
}

