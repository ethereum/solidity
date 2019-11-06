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
/**
 * Specific AST walker that generates the call graph.
 */

#include <libyul/AsmData.h>
#include <libyul/optimiser/CallGraphGenerator.h>

#include <libevmasm/Instruction.h>

#include <stack>

using namespace std;
using namespace dev;
using namespace yul;

map<YulString, set<YulString>> CallGraphGenerator::callGraph(Block const& _ast)
{
	CallGraphGenerator gen;
	gen(_ast);
	return std::move(gen.m_callGraph);
}

void CallGraphGenerator::operator()(FunctionCall const& _functionCall)
{
	m_callGraph[m_currentFunction].insert(_functionCall.functionName.name);
	ASTWalker::operator()(_functionCall);
}

void CallGraphGenerator::operator()(FunctionDefinition const& _functionDefinition)
{
	YulString previousFunction = m_currentFunction;
	m_currentFunction = _functionDefinition.name;
	yulAssert(m_callGraph.count(m_currentFunction) == 0, "");
	m_callGraph[m_currentFunction] = {};
	ASTWalker::operator()(_functionDefinition);
	m_currentFunction = previousFunction;
}

CallGraphGenerator::CallGraphGenerator()
{
	m_callGraph[YulString{}] = {};
}

