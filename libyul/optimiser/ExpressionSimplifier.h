// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/optimiser/DataFlowAnalyzer.h>

namespace solidity::yul
{
struct Dialect;
struct OptimiserStepContext;

/**
 * Applies simplification rules to all expressions.
 * The component will work best if the code is in SSA form, but
 * this is not required for correctness.
 *
 * It tracks the current values of variables using the DataFlowAnalyzer
 * and takes them into account for replacements.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class ExpressionSimplifier: public DataFlowAnalyzer
{
public:
	static constexpr char const* name{"ExpressionSimplifier"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void visit(Expression& _expression) override;

private:
	explicit ExpressionSimplifier(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}
};

}
