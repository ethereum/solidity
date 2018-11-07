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
 * Optimiser component that identifies functions to be inlined.
 */

#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>

#include <libyul/optimiser/Utilities.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

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
		if (bodyStatement.type() == typeid(Assignment))
		{
			Assignment const& assignment = boost::get<Assignment>(bodyStatement);
			if (assignment.variableNames.size() == 1 && assignment.variableNames.front().name == retVariable)
			{
				// FIXME: use code size metric here

				// We cannot overwrite previous settings, because this function definition
				// would not be valid here if we were searching inside a functionally inlinable
				// function body.
				assertThrow(m_disallowedIdentifiers.empty() && !m_foundDisallowedIdentifier, OptimizerException, "");
				m_disallowedIdentifiers = set<YulString>{retVariable, _function.name};
				boost::apply_visitor(*this, *assignment.value);
				if (!m_foundDisallowedIdentifier)
					m_inlinableFunctions[_function.name] = &_function;
				m_disallowedIdentifiers.clear();
				m_foundDisallowedIdentifier = false;
			}
		}
	}
	ASTWalker::operator()(_function.body);
}
