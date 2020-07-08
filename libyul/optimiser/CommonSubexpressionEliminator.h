// SPDX-License-Identifier: GPL-3.0
/**
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

struct Dialect;
struct SideEffects;

/**
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class CommonSubexpressionEliminator: public DataFlowAnalyzer
{
public:
	static constexpr char const* name{"CommonSubexpressionEliminator"};
	static void run(OptimiserStepContext&, Block& _ast);

private:
	CommonSubexpressionEliminator(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects
	);

protected:
	using ASTModifier::visit;
	void visit(Expression& _e) override;
};

}
