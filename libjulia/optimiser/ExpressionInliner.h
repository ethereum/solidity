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
 * Optimiser component that performs function inlining.
 */
#pragma once

#include <libjulia/optimiser/ASTWalker.h>

#include <libjulia/ASTDataForward.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace dev
{
namespace julia
{

/**
 * Optimiser component that modifies an AST in place, inlining functions that can be
 * inlined inside functional expressions, i.e. functions that
 *  - return a single value
 *  - have a body like r := <functional expression>
 *  - neither reference themselves nor r in the right hand side
 *
 * Furthermore, the arguments of the function call cannot have any side-effects.
 *
 * This component can only be used on sources with unique names.
 */
class ExpressionInliner: public ASTModifier
{
public:
	ExpressionInliner(Block& _block):
		m_block(_block)
	{}

	void run();

	using ASTModifier::operator();
	virtual void operator()(FunctionDefinition& _fun) override;

	virtual void visit(Expression& _expression) override;

private:
	std::map<std::string, FunctionDefinition const*> m_inlinableFunctions;
	std::map<std::string, std::string> m_varReplacements;
	/// Set of functions we are currently visiting inside.
	std::set<std::string> m_currentFunctions;

	Block& m_block;
};


}
}
