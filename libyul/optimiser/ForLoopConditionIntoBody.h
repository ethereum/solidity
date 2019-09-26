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
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Dialect.h>

namespace yul
{

struct OptimiserStepContext;

/**
 * Rewrites ForLoop by moving iteration condition into the ForLoop body.
 * For example, `for {} lt(a, b) {} { mstore(1, 2) }` will become
 * `for {} 1 {} { if iszero(lt(a, b)) { break } mstore(1, 2) }`
 *
 * By moving the iteration check part into the ForLoop body, we can apply expression splitter
 * to the condition expression.
 *
 * This rewriter will skip loops that already have literal constant as iteration condition.
 *
 * Requirements:
 * - The Disambiguator must be run upfront.
 * - To avoid unnecessary rewrite, it is recommended to run this rewriter after StructuralSimplifier.
 * - Only works for dialects with a builtin boolean negation function.
 */
class ForLoopConditionIntoBody: public ASTModifier
{
public:
	static constexpr char const* name{"ForLoopConditionIntoBody"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(ForLoop& _forLoop) override;

private:
	ForLoopConditionIntoBody(Dialect const& _dialect): m_dialect(_dialect) {}

	Dialect const& m_dialect;
};

}
