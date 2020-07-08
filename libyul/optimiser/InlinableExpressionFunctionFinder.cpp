// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that identifies functions to be inlined.
 */

#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>

#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void InlinableExpressionFunctionFinder::operator()(Identifier const& _identifier)
{
	checkAllowed(_identifier.name);
	ASTWalker::operator()(_identifier);
}

void InlinableExpressionFunctionFinder::operator()(FunctionCall const& _funCall)
{
	checkAllowed(_funCall.functionName.name);
	ASTWalker::operator()(_funCall);
}

void InlinableExpressionFunctionFinder::operator()(FunctionDefinition const& _function)
{
	if (_function.returnVariables.size() == 1 && _function.body.statements.size() == 1)
	{
		YulString retVariable = _function.returnVariables.front().name;
		Statement const& bodyStatement = _function.body.statements.front();
		if (holds_alternative<Assignment>(bodyStatement))
		{
			Assignment const& assignment = std::get<Assignment>(bodyStatement);
			if (assignment.variableNames.size() == 1 && assignment.variableNames.front().name == retVariable)
			{
				// TODO: use code size metric here

				// We cannot overwrite previous settings, because this function definition
				// would not be valid here if we were searching inside a functionally inlinable
				// function body.
				assertThrow(m_disallowedIdentifiers.empty() && !m_foundDisallowedIdentifier, OptimizerException, "");
				m_disallowedIdentifiers = set<YulString>{retVariable, _function.name};
				std::visit(*this, *assignment.value);
				if (!m_foundDisallowedIdentifier)
					m_inlinableFunctions[_function.name] = &_function;
				m_disallowedIdentifiers.clear();
				m_foundDisallowedIdentifier = false;
			}
		}
	}
	ASTWalker::operator()(_function.body);
}
