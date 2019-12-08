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
 * Class that can answer questions about values of variables and their relations.
 */

#include <libyul/optimiser/KnowledgeBase.h>

#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/SimplificationRules.h>
#include <libyul/optimiser/Semantics.h>

#include <libdevcore/CommonData.h>

#include <variant>

using namespace std;
using namespace yul;
using namespace dev;

bool KnowledgeBase::knownToBeDifferent(YulString _a, YulString _b)
{
	// Try to use the simplification rules together with the
	// current values to turn `sub(_a, _b)` into a nonzero constant.
	// If that fails, try `eq(_a, _b)`.

	Expression expr1 = simplify(FunctionCall{{}, {{}, "sub"_yulstring}, make_vector<Expression>(Identifier{{}, _a}, Identifier{{}, _b})});
	if (holds_alternative<Literal>(expr1))
		return valueOfLiteral(std::get<Literal>(expr1)) != 0;

	Expression expr2 = simplify(FunctionCall{{}, {{}, "eq"_yulstring}, make_vector<Expression>(Identifier{{}, _a}, Identifier{{}, _b})});
	if (holds_alternative<Literal>(expr2))
		return valueOfLiteral(std::get<Literal>(expr2)) == 0;

	return false;
}

bool KnowledgeBase::knownToBeDifferentByAtLeast32(YulString _a, YulString _b)
{
	// Try to use the simplification rules together with the
	// current values to turn `sub(_a, _b)` into a constant whose absolute value is at least 32.

	Expression expr1 = simplify(FunctionCall{{}, {{}, "sub"_yulstring}, make_vector<Expression>(Identifier{{}, _a}, Identifier{{}, _b})});
	if (holds_alternative<Literal>(expr1))
	{
		u256 val = valueOfLiteral(std::get<Literal>(expr1));
		return val >= 32 && val <= u256(0) - 32;
	}

	return false;
}

Expression KnowledgeBase::simplify(Expression _expression)
{
	bool startedRecursion = (m_recursionCounter == 0);
	dev::ScopeGuard{[&] { if (startedRecursion) m_recursionCounter = 0; }};

	if (startedRecursion)
		m_recursionCounter = 100;
	else if (m_recursionCounter == 1)
		return _expression;
	else
		--m_recursionCounter;

	if (holds_alternative<FunctionCall>(_expression))
		for (Expression& arg: std::get<FunctionCall>(_expression).arguments)
			arg = simplify(arg);
	else if (holds_alternative<FunctionalInstruction>(_expression))
		for (Expression& arg: std::get<FunctionalInstruction>(_expression).arguments)
			arg = simplify(arg);

	if (auto match = SimplificationRules::findFirstMatch(_expression, m_dialect, m_variableValues))
		return simplify(match->action().toExpression(locationOf(_expression)));

	return _expression;
}
