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

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmDataForward.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace yul
{
struct Dialect;
struct OptimiserStepContext;

/**
 * Optimiser component that modifies an AST in place, inlining functions that can be
 * inlined inside functional expressions, i.e. functions that
 *  - return a single value
 *  - have a body like r := <functional expression>
 *  - neither reference themselves nor r in the right hand side
 *
 * Furthermore, for all parameters, all of the following need to be true
 *  - the argument is movable
 *  - the parameter is either referenced less than twice in the function body, or the argument is rather cheap
 *    ("cost" of at most 1 like a constant up to 0xff)
 *
 * This component can only be used on sources with unique names.
 */
class ExpressionInliner: public ASTModifier
{
public:
	static constexpr char const* name{"ExpressionInliner"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(FunctionDefinition& _fun) override;

	void visit(Expression& _expression) override;

private:
	ExpressionInliner(
		Dialect const& _dialect,
		std::map<YulString, FunctionDefinition const*> const& _inlinableFunctions
	): m_dialect(_dialect), m_inlinableFunctions(_inlinableFunctions)
	{}

	Dialect const& m_dialect;
	std::map<YulString, FunctionDefinition const*> const& m_inlinableFunctions;

	std::map<YulString, YulString> m_varReplacements;
	/// Set of functions we are currently visiting inside.
	std::set<YulString> m_currentFunctions;
};

}
