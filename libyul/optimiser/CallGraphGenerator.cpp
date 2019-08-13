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


void CallGraphGenerator::operator()(FunctionalInstruction const& _functionalInstruction)
{
	m_callGraph.insert(m_currentFunction, YulString{
		dev::eth::instructionInfo(_functionalInstruction.instruction).name
	});
	ASTWalker::operator()(_functionalInstruction);
}

void CallGraphGenerator::operator()(FunctionCall const& _functionCall)
{
	m_callGraph.insert(m_currentFunction, _functionCall.functionName.name);
	ASTWalker::operator()(_functionCall);
}

void CallGraphGenerator::operator()(FunctionDefinition const& _functionDefinition)
{
	YulString previousFunction = m_currentFunction;
	m_currentFunction = _functionDefinition.name;
	ASTWalker::operator()(_functionDefinition);
	m_currentFunction = previousFunction;
}

