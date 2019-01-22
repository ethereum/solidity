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
#include <libyul/optimiser/NameCollector.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace yul;

void Rematerialiser::run(Dialect const& _dialect, Block& _ast)
{
	Rematerialiser{_dialect, _ast}(_ast);
}

Rematerialiser::Rematerialiser(Dialect const& _dialect, Block& _ast):
	DataFlowAnalyzer(_dialect),
	m_referenceCounts(ReferencesCounter::countReferences(_ast))
{
}

void Rematerialiser::visit(Expression& _e)
{
	if (_e.type() == typeid(Identifier))
	{
		Identifier& identifier = boost::get<Identifier>(_e);
		if (m_value.count(identifier.name))
		{
			YulString name = identifier.name;
			assertThrow(m_value.at(name), OptimizerException, "");
			auto const& value = *m_value.at(name);
			size_t refs = m_referenceCounts[name];
			size_t cost = CodeCost::codeCost(value);
			if (refs <= 1 || cost == 0 || (refs <= 5 && cost <= 1))
			{
				assertThrow(m_referenceCounts[name] > 0, OptimizerException, "");
				for (auto const& ref: m_references[name])
					assertThrow(inScope(ref), OptimizerException, "");
				// update reference counts
				m_referenceCounts[name]--;
				for (auto const& ref: ReferencesCounter::countReferences(value))
					m_referenceCounts[ref.first] += ref.second;
				_e = (ASTCopier{}).translate(value);
			}
		}
	}
	DataFlowAnalyzer::visit(_e);
}
