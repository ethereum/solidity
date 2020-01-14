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
