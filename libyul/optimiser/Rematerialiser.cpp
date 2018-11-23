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

#include <libyul/optimiser/Rematerialiser.h>

#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

void Rematerialiser::visit(Expression& _e)
{
	if (_e.type() == typeid(Identifier))
	{
		Identifier& identifier = boost::get<Identifier>(_e);
		if (m_value.count(identifier.name))
		{
			YulString name = identifier.name;
			for (auto const& ref: m_references[name])
				assertThrow(inScope(ref), OptimizerException, "");
			assertThrow(m_value.at(name), OptimizerException, "");
			auto const& value = *m_value.at(name);
			if (CodeSize::codeSize(value) <= 7)
				_e = (ASTCopier{}).translate(value);
		}
	}
	DataFlowAnalyzer::visit(_e);
}
