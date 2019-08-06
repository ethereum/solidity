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
 * Specific AST walker that generates call graph
 */

#include <libyul/AsmData.h>
#include <libyul/optimiser/CallGraphGenerator.h>

#include <libevmasm/Instruction.h>

#include <stack>

using namespace std;
using namespace dev;
using namespace yul;

CallGraphGenerator::CallGraphGenerator(Block const& _ast)
{
	operator()(_ast);
}

map<YulString, set<YulString>> CallGraphGenerator::getCallGraph() const
{
	return m_callGraph.forward;
}

map<YulString, set<YulString>> CallGraphGenerator::getCallGraphRev() const
{
	return m_callGraph.backward;
}

set<YulString> CallGraphGenerator::getDirectAndIndirectCaller(YulString _fname) const
{
	set<YulString> callers;
	stack<YulString> s;
	s.push(_fname);
	while (!s.empty())
	{
		YulString cur = s.top();
		s.pop();
		set<YulString> curCallers = getCallGraphRev()[cur];
		for (auto f: curCallers)
		{
			if (!callers.count(f))
			{
				callers.insert(f);
				s.push(f);
			}
		}
	}
	return callers;
}

void CallGraphGenerator::operator()(FunctionalInstruction const& _instr)
{
	// assertThrow(false, OptimizerException, "");
	ASTWalker::operator()(_instr);
	if (m_currentFunction)
	{
		// TODO: instruction$$ is a work around to denote instruction
		m_callGraph.insert(*m_currentFunction, YulString("instruction$$" + dev::eth::instructionInfo(_instr.instruction).name));
	}
}

void CallGraphGenerator::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);
	if (m_currentFunction)
	{
		m_callGraph.insert(*m_currentFunction, _functionCall.functionName.name);
	}
}

void CallGraphGenerator::operator()(FunctionDefinition const& _functionDefinition)
{
	if (m_currentFunction)
		assertThrow(false, OptimizerException, "Function defined in function.");
	else
	{
		m_userDefinedFunctions.insert(_functionDefinition.name);
		m_currentFunction = _functionDefinition.name;
		ASTWalker::operator()(_functionDefinition);
		m_currentFunction = boost::none;
	}
}

