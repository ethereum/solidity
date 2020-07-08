// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Dialect.h>

namespace solidity::yul
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
