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
#include <libyul/AST.h>

#include <range/v3/algorithm/all_of.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void Rematerialiser::run(Dialect const& _dialect, Block& _ast, set<YulString> _varsToAlwaysRematerialize, bool _onlySelectedVariables)
{
	Rematerialiser{_dialect, _ast, std::move(_varsToAlwaysRematerialize), _onlySelectedVariables}(_ast);
}

void Rematerialiser::run(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	set<YulString> _varsToAlwaysRematerialize,
	bool _onlySelectedVariables
)
{
	Rematerialiser{_dialect, _function, std::move(_varsToAlwaysRematerialize), _onlySelectedVariables}(_function);
}

Rematerialiser::Rematerialiser(
	Dialect const& _dialect,
	Block& _ast,
	set<YulString> _varsToAlwaysRematerialize,
	bool _onlySelectedVariables
):
	DataFlowAnalyzer(_dialect),
	m_referenceCounts(ReferencesCounter::countReferences(_ast)),
	m_varsToAlwaysRematerialize(std::move(_varsToAlwaysRematerialize)),
	m_onlySelectedVariables(_onlySelectedVariables)
{
}

Rematerialiser::Rematerialiser(
	Dialect const& _dialect,
	FunctionDefinition& _function,
	set<YulString> _varsToAlwaysRematerialize,
	bool _onlySelectedVariables
):
	DataFlowAnalyzer(_dialect),
	m_referenceCounts(ReferencesCounter::countReferences(_function)),
	m_varsToAlwaysRematerialize(std::move(_varsToAlwaysRematerialize)),
	m_onlySelectedVariables(_onlySelectedVariables)
{
}

void Rematerialiser::visit(Expression& _e)
{
	if (holds_alternative<Identifier>(_e))
	{
		Identifier& identifier = std::get<Identifier>(_e);
		YulString name = identifier.name;
		if (m_value.count(name))
		{
			assertThrow(m_value.at(name).value, OptimizerException, "");
			AssignedValue const& value = m_value.at(name);
			size_t refs = m_referenceCounts[name];
			size_t cost = CodeCost::codeCost(m_dialect, *value.value);
			if (
				(
					!m_onlySelectedVariables && (
						(refs <= 1 && value.loopDepth == m_loopDepth) ||
						cost == 0 ||
						(refs <= 5 && cost <= 1 && m_loopDepth == 0)
					)
				) || m_varsToAlwaysRematerialize.count(name)
			)
			{
				assertThrow(m_referenceCounts[name] > 0, OptimizerException, "");
				if (ranges::all_of(m_references[name], [&](auto const& ref) { return inScope(ref); }))
				{
					// update reference counts
					m_referenceCounts[name]--;
					for (auto const& ref: ReferencesCounter::countReferences(*value.value))
						m_referenceCounts[ref.first] += ref.second;
					_e = (ASTCopier{}).translate(*value.value);
				}
			}
		}
	}
	DataFlowAnalyzer::visit(_e);
}

void LiteralRematerialiser::visit(Expression& _e)
{
	if (holds_alternative<Identifier>(_e))
	{
		Identifier& identifier = std::get<Identifier>(_e);
		YulString name = identifier.name;
		if (m_value.count(name))
		{
			Expression const* value = m_value.at(name).value;
			assertThrow(value, OptimizerException, "");
			if (holds_alternative<Literal>(*value))
				_e = *value;
		}
	}
	DataFlowAnalyzer::visit(_e);
}
