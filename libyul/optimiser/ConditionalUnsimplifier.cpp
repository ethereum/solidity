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
	if (_switch.expression->type() != typeid(Identifier))
	{
		ASTModifier::operator()(_switch);
		return;
	}
	YulString expr = boost::get<Identifier>(*_switch.expression).name;
	for (auto& _case: _switch.cases)
	{
		if (_case.value)
		{
			(*this)(*_case.value);
			if (
				!_case.body.statements.empty() &&
				_case.body.statements.front().type() == typeid(Assignment)
			)
			{
				Assignment const& assignment = boost::get<Assignment>(_case.body.statements.front());
				if (
					assignment.variableNames.size() == 1 &&
					assignment.variableNames.front().name == expr &&
					assignment.value->type() == typeid(Literal) &&
					valueOfLiteral(boost::get<Literal>(*assignment.value)) == valueOfLiteral(*_case.value)
				)
					_case.body.statements.erase(_case.body.statements.begin());
			}
		}
		(*this)(_case.body);
	}
}

void ConditionalUnsimplifier::operator()(Block& _block)
{
	iterateReplacingWindow<2>(
		_block.statements,
		[&](Statement& _stmt1, Statement& _stmt2) -> std::optional<vector<Statement>>
		{
			visit(_stmt1);
			if (_stmt1.type() == typeid(If))
			{
				If& _if = boost::get<If>(_stmt1);
				if (
					_if.condition->type() == typeid(Identifier) &&
					!_if.body.statements.empty()
				)
				{
					YulString condition = boost::get<Identifier>(*_if.condition).name;
					if (
						_stmt2.type() == typeid(Assignment) &&
						TerminationFinder(m_dialect).controlFlowKind(_if.body.statements.back()) !=
							TerminationFinder::ControlFlow::FlowOut
					)
					{
						Assignment const& assignment = boost::get<Assignment>(_stmt2);
						if (
							assignment.variableNames.size() == 1 &&
							assignment.variableNames.front().name == condition &&
							assignment.value->type() == typeid(Literal) &&
							valueOfLiteral(boost::get<Literal>(*assignment.value)) == 0
						)
							return {make_vector<Statement>(std::move(_stmt1))};
					}
				}
			}
			return {};
		}
	);
}
