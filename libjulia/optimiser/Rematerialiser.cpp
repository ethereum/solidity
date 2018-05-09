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
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 */

#include <libjulia/optimiser/Rematerialiser.h>

#include <libjulia/optimiser/Metrics.h>
#include <libjulia/optimiser/ASTCopier.h>
#include <libjulia/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::julia;

void Rematerialiser::visit(Expression& _e)
{
	if (_e.type() == typeid(Identifier))
	{
		Identifier& identifier = boost::get<Identifier>(_e);
		if (m_value.count(identifier.name))
		{
			string name = identifier.name;
			bool expressionValid = true;
			for (auto const& ref: m_references[name])
				if (!inScope(ref))
				{
					expressionValid = false;
					break;
				}
			assertThrow(m_value.at(name), OptimizerException, "");
			auto const& value = *m_value.at(name);
			if (expressionValid && CodeSize::codeSize(value) <= 7)
				_e = (ASTCopier{}).translate(value);
		}
	}
	DataFlowAnalyzer::visit(_e);
}
