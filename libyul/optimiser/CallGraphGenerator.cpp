// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST walker that generates the call graph.
 */

#include <libyul/AsmData.h>
#include <libyul/optimiser/CallGraphGenerator.h>

#include <libevmasm/Instruction.h>

#include <stack>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

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

