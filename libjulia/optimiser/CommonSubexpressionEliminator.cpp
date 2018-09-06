/*(
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
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 */

#include <libjulia/optimiser/CommonSubexpressionEliminator.h>

#include <libjulia/optimiser/Metrics.h>
#include <libjulia/optimiser/SyntacticalEquality.h>
#include <libjulia/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::julia;

void CommonSubexpressionEliminator::visit(Expression& _e)
{
	// Single exception for substitution: We do not substitute one variable for another.
	if (_e.type() != typeid(Identifier))
		// TODO this search rather inefficient.
		for (auto const& var: m_value)
		{
			assertThrow(var.second, OptimizerException, "");
			if (SyntacticalEqualityChecker::equal(_e, *var.second))
			{
				_e = Identifier{locationOf(_e), var.first};
				break;
			}
		}
	DataFlowAnalyzer::visit(_e);
}
