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
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#include <libyul/optimiser/ExpressionSimplifier.h>

#include <libyul/optimiser/SimplificationRules.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;


void ExpressionSimplifier::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);
	while (auto match = SimplificationRules::findFirstMatch(_expression, m_dialect, m_ssaValues))
	{
		// Do not apply the rule if it removes non-constant parts of the expression.
		// TODO: The check could actually be less strict than "movable".
		// We only require "Does not cause side-effects".
		// Note: References to variables that are only assigned once are always movable,
		// so if the value of the variable is not movable, the expression that references
		// the variable still is.

		if (match->removesNonConstants && !MovableChecker(m_dialect, _expression).movable())
			return;
		_expression = match->action().toExpression(locationOf(_expression));
	}
}

void ExpressionSimplifier::run(Dialect const& _dialect, Block& _ast)
{
	SSAValueTracker ssaValues;
	ssaValues(_ast);
	ExpressionSimplifier{_dialect, ssaValues.values()}(_ast);
}
