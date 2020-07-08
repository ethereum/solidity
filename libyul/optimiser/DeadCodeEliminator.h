// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that removes unused variables and functions.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/YulString.h>

#include <map>
#include <set>

namespace solidity::yul
{
struct Dialect;
struct OptimiserStepContext;

/**
 * Optimisation stage that removes unreachable code
 *
 * Unreachable code is any code within a block which is preceded by a
 * leave, return, invalid, break, continue, selfdestruct or revert.
 *
 * Function definitions are retained as they might be called by earlier
 * code and thus are considered reachable.
 *
 * Because variables declared in a for loop's init block have their scope extended to the loop body,
 * we require ForLoopInitRewriter to run before this step.
 *
 * Prerequisite: ForLoopInitRewriter, Function Hoister, Function Grouper
 */
class DeadCodeEliminator: public ASTModifier
{
public:
	static constexpr char const* name{"DeadCodeEliminator"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(ForLoop& _for) override;
	void operator()(Block& _block) override;

private:
	DeadCodeEliminator(Dialect const& _dialect): m_dialect(_dialect) {}

	Dialect const& m_dialect;
};

}
