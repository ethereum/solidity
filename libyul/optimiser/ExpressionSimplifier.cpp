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
// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#include <libyul/optimiser/ExpressionSimplifier.h>

#include <libyul/optimiser/SimplificationRules.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AST.h>

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

	while (auto const* match = SimplificationRules::findFirstMatch(_expression, m_dialect, m_value))
		_expression = match->action().toExpression(locationOf(_expression));
}
