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
 * Optimiser component that identifies functions to be inlined.
 */

#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>

#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/AST.h>

using namespace solidity;
using namespace solidity::yul;

void InlinableExpressionFunctionFinder::operator()(Identifier const& _identifier)
{
	checkAllowed(_identifier);
	ASTWalker::operator()(_identifier);
}

void InlinableExpressionFunctionFinder::operator()(FunctionCall const& _funCall)
{
	checkAllowed(_funCall.functionName);
	ASTWalker::operator()(_funCall);
}

void InlinableExpressionFunctionFinder::operator()(FunctionDefinition const& _function)
{
	if (_function.returnVariables.size() == 1 && _function.body.statements.size() == 1)
	{
		YulName retVariable = _function.returnVariables.front().name;
		Statement const& bodyStatement = _function.body.statements.front();
		if (std::holds_alternative<Assignment>(bodyStatement))
		{
			Assignment const& assignment = std::get<Assignment>(bodyStatement);
			if (assignment.variableNames.size() == 1 && assignment.variableNames.front().name == retVariable)
			{
				// TODO: use code size metric here

				// We cannot overwrite previous settings, because this function definition
				// would not be valid here if we were searching inside a functionally inlinable
				// function body.
				assertThrow(m_disallowedIdentifiers.empty() && !m_foundDisallowedIdentifier, OptimizerException, "");
				m_disallowedIdentifiers = std::set<YulName>{retVariable, _function.name};
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
void InlinableExpressionFunctionFinder::checkAllowed(FunctionName _name)
{
	// disallowed function names can only ever be user-defined `yul::Identifier`s, not builtins
	if (std::holds_alternative<Identifier>(_name) && m_disallowedIdentifiers.count(std::get<Identifier>(_name).name))
		m_foundDisallowedIdentifier = true;
}
