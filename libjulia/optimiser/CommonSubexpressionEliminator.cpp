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
	// We visit the inner expression first to first simplify inner expressions,
	// which hopefully allows more matches.
	// Note that the DataFlowAnalyzer itself only has code for visiting Statements,
	// so this basically invokes the AST walker directly and thus post-visiting
	// is also fine with regards to data flow analysis.
	DataFlowAnalyzer::visit(_e);

	if (_e.type() == typeid(Identifier))
	{
		Identifier& identifier = boost::get<Identifier>(_e);
		string const& name = identifier.name;
		if (m_value.count(name))
		{
			assertThrow(m_value.at(name), OptimizerException, "");
			if (m_value.at(name)->type() == typeid(Identifier))
			{
				string const& value = boost::get<Identifier>(*m_value.at(name)).name;
				if (inScope(value))
					_e = Identifier{locationOf(_e), value};
			}
		}
	}
	else
	{
		// TODO this search is rather inefficient.
		for (auto const& var: m_value)
		{
			assertThrow(var.second, OptimizerException, "");
			if (SyntacticalEqualityChecker::equal(_e, *var.second))
			{
				_e = Identifier{locationOf(_e), var.first};
				break;
			}
		}
	}
}
