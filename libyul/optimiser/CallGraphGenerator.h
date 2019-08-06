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

#include <libdevcore/InvertibleMap.h>

#include <boost/optional.hpp>

#include <set>
#include <map>

namespace yul
{

/**
 * Specific AST walker that generates the call graph.
 *
 * The outermost (non-function) context is denoted by the empty string.
 */
class CallGraphGenerator: public ASTWalker
{
public:
	/// @returns the call graph of the visited AST.
	InvertibleRelation<YulString> const& callGraph() const { return m_callGraph; }

	using ASTWalker::operator();
	void operator()(FunctionalInstruction const& _functionalInstruction) override;
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;

private:
	InvertibleRelation<YulString> m_callGraph;
	/// The name of the function we are currently visiting during traversal.
	YulString m_currentFunction;
};

}
