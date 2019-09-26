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
 * Optimiser component that performs function inlining inside expressions.
 */

#include <libyul/optimiser/ExpressionInliner.h>

#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Substitution.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace yul;

void ExpressionInliner::run(OptimiserStepContext& _context, Block& _ast)
{
	InlinableExpressionFunctionFinder funFinder;
	funFinder(_ast);
	ExpressionInliner inliner{_context.dialect, funFinder.inlinableFunctions()};
	inliner(_ast);
}

void ExpressionInliner::operator()(FunctionDefinition& _fun)
{
	ASTModifier::operator()(_fun);
}

void ExpressionInliner::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);
	if (_expression.type() == typeid(FunctionCall))
	{
		FunctionCall& funCall = boost::get<FunctionCall>(_expression);
		if (!m_inlinableFunctions.count(funCall.functionName.name))
			return;
		FunctionDefinition const& fun = *m_inlinableFunctions.at(funCall.functionName.name);

		map<YulString, Expression const*> substitutions;
		for (size_t i = 0; i < funCall.arguments.size(); i++)
		{
			Expression const& arg = funCall.arguments[i];
			YulString paraName = fun.parameters[i].name;

			if (!SideEffectsCollector(m_dialect, arg).movable())
				return;

			size_t refs = ReferencesCounter::countReferences(fun.body)[paraName];
			size_t cost = CodeCost::codeCost(m_dialect, arg);

			if (refs > 1 && cost > 1)
				return;

			substitutions[paraName] = &arg;
		}

		_expression = Substitution(substitutions).translate(*boost::get<Assignment>(fun.body.statements.front()).value);
	}
}
