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
 *
 * Prerequisites: Disambiguator
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <libyul/Builtins.h>

#include <cstddef>
#include <map>
#include <set>
#include <vector>

namespace solidity::yul
{

/**
 * Cycle structure of a call graph, derived from it by CallGraph::analyzeCallCycles
 */
struct CallGraphCycles
{
	/// @returns true if @a _function is part of a (mutual) recursion. Always false for builtins.
	bool isRecursive(FunctionHandle const& _function) const { return recursiveFunctions.contains(_function); }
	/// @returns the index of the strongly-connected component that contains @a _function.
	std::size_t componentIndexOf(FunctionHandle const& _function) const { return componentOfFunction.at(_function); }
	/// @returns the members of the strongly-connected component with index @a _component.
	std::vector<FunctionHandle> const& component(std::size_t const _component) const { return stronglyConnectedComponents.at(_component); }

	/// The strongly-connected components of the call graph.
	std::vector<std::vector<FunctionHandle>> stronglyConnectedComponents;
	/// Maps each function to the index of the component in @a stronglyConnectedComponents containing it.
	std::map<FunctionHandle, std::size_t> componentOfFunction;
	/// The set of functions contained in cycles in the call graph, i.e., functions that are part of a
	/// (mutual) recursion. This does not include functions that merely call recursive functions.
	/// Never contains a builtin: at Yul level builtins cannot call other functions, so they have no
	/// outgoing call-graph edges and can be neither mutually nor directly recursive.
	std::set<FunctionHandle> recursiveFunctions;
};

struct CallGraph
{
	CallGraphCycles analyzeCallCycles() const;

	/// Map function definition name -> function name
	std::map<FunctionHandle, std::vector<FunctionHandle>> functionCalls;
	std::set<YulName> functionsWithLoops;
};

/**
 * Specific AST walker that generates the call graph.
 *
 * It also generates information about which functions contain for loops.
 *
 * The outermost (non-function) context is denoted by the empty string.
 *
 * @throws InputNotDisambiguatedException if input is not disambiguated
 */
class CallGraphGenerator: public ASTWalker
{
public:
	struct InputNotDisambiguatedException: virtual YulException {};

	static CallGraph callGraph(Block const& _ast);

	using ASTWalker::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(ForLoop const& _forLoop) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;

private:
	CallGraphGenerator();

	CallGraph m_callGraph;
	/// The name of the function we are currently visiting during traversal.
	YulName m_currentFunction;
};

}
