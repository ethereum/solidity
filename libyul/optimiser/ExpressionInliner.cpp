// SPDX-License-Identifier: GPL-3.0
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
using namespace solidity;
using namespace solidity::yul;

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
	if (holds_alternative<FunctionCall>(_expression))
	{
		FunctionCall& funCall = std::get<FunctionCall>(_expression);
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

		_expression = Substitution(substitutions).translate(*std::get<Assignment>(fun.body.statements.front()).value);
	}
}
