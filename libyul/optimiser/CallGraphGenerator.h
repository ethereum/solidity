// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST walker that generates the call graph.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <libsolutil/InvertibleMap.h>

#include <map>
#include <optional>
#include <set>

namespace solidity::yul
{

struct CallGraph
{
	std::map<YulString, std::set<YulString>> functionCalls;
	std::set<YulString> functionsWithLoops;
};

/**
 * Specific AST walker that generates the call graph.
 *
 * It also generates information about which functions contain for loops.
 *
 * The outermost (non-function) context is denoted by the empty string.
 */
class CallGraphGenerator: public ASTWalker
{
public:
	static CallGraph callGraph(Block const& _ast);

	using ASTWalker::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(ForLoop const& _forLoop) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;

private:
	CallGraphGenerator();

	CallGraph m_callGraph;
	/// The name of the function we are currently visiting during traversal.
	YulString m_currentFunction;
};

}
