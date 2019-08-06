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

#pragma once

#include <libyul/optimiser/ASTWalker.h>

#include <libdevcore/InvertibleMap.h>

#include <boost/optional.hpp>

#include <set>
#include <map>

namespace yul
{

class CallGraphGenerator: public ASTWalker
{
public:
	explicit CallGraphGenerator(Block const& _ast);
	// TODO: return const reference instead?
	std::map<YulString, std::set<YulString>> getCallGraph() const;
	/// call graph, but edges are reversed
	std::map<YulString, std::set<YulString>> getCallGraphRev() const;
	/// returns the set of functions that directly or indirectly calls _fname
	std::set<YulString> getDirectAndIndirectCaller(YulString _fname) const;
	std::set<YulString> getUserDefinedFunctions() const { return m_userDefinedFunctions; }

	using ASTWalker::operator();
	void operator()(FunctionalInstruction const& _functionalInstruction) override;
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;

private:
	InvertibleRelation<YulString> m_callGraph;
	/// the name of function we are current in during traversal
	boost::optional<YulString> m_currentFunction;
	std::set<YulString> m_userDefinedFunctions;
};

}
