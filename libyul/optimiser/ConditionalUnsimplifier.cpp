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
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/NameCollector.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

using namespace std;
using namespace dev;
using namespace yul;

void ConditionalUnsimplifier::operator()(Switch& _switch)
{
	visit(*_switch.expression);
	if (!holds_alternative<Identifier>(*_switch.expression))
	{
		ASTModifier::operator()(_switch);
		return;
	}
	YulString expr = std::get<Identifier>(*_switch.expression).name;
	for (auto& _case: _switch.cases)
	{
		if (_case.value)
		{
			(*this)(*_case.value);
			if (
				!_case.body.statements.empty() &&
				holds_alternative<Assignment>(_case.body.statements.front())
			)
			{
				Assignment const& assignment = std::get<Assignment>(_case.body.statements.front());
				if (
					assignment.variableNames.size() == 1 &&
					assignment.variableNames.front().name == expr &&
					holds_alternative<Literal>(*assignment.value) &&
					valueOfLiteral(std::get<Literal>(*assignment.value)) == valueOfLiteral(*_case.value)
				)
					_case.body.statements.erase(_case.body.statements.begin());
			}
		}
		(*this)(_case.body);
	}
}

void ConditionalUnsimplifier::operator()(Block& _block)
{
	walkVector(_block.statements);
	iterateReplacingWindow<2>(
		_block.statements,
		[&](Statement& _stmt1, Statement& _stmt2) -> std::optional<vector<Statement>>
		{
			if (holds_alternative<If>(_stmt1))
			{
				If& _if = std::get<If>(_stmt1);
				if (
					holds_alternative<Identifier>(*_if.condition) &&
					!_if.body.statements.empty()
				)
				{
					YulString condition = std::get<Identifier>(*_if.condition).name;
					if (
						holds_alternative<Assignment>(_stmt2) &&
						TerminationFinder(m_dialect).controlFlowKind(_if.body.statements.back()) !=
							TerminationFinder::ControlFlow::FlowOut
					)
					{
						Assignment const& assignment = std::get<Assignment>(_stmt2);
						if (
							assignment.variableNames.size() == 1 &&
							assignment.variableNames.front().name == condition &&
							holds_alternative<Literal>(*assignment.value) &&
							valueOfLiteral(std::get<Literal>(*assignment.value)) == 0
						)
							return {make_vector<Statement>(std::move(_stmt1))};
					}
				}
			}
			return {};
		}
	);
}
