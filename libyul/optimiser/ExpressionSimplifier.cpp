// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#include <libyul/optimiser/ExpressionSimplifier.h>

#include <libyul/optimiser/SimplificationRules.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AsmData.h>

#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void ExpressionSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	ExpressionSimplifier{_context.dialect}(_ast);
}

void ExpressionSimplifier::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);
	while (auto match = SimplificationRules::findFirstMatch(_expression, m_dialect, m_value))
	{
		// Do not apply the rule if it removes non-constant parts of the expression.
		// TODO: The check could actually be less strict than "movable".
		// We only require "Does not cause side-effects".
		// Note: References to variables that are only assigned once are always movable,
		// so if the value of the variable is not movable, the expression that references
		// the variable still is.

		if (match->removesNonConstants && !SideEffectsCollector(m_dialect, _expression).movable())
			return;
		_expression = match->action().toExpression(locationOf(_expression));
	}
}
