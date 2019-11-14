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
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/NameCollector.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

using namespace std;
using namespace dev;
using namespace yul;

void ConditionalSimplifier::operator()(Switch& _switch)
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
			_case.body.statements.insert(_case.body.statements.begin(),
				Assignment{
					_case.body.location,
					{Identifier{_case.body.location, expr}},
					make_unique<Expression>(*_case.value)
				}
			);
		}
		(*this)(_case.body);
	}
}

void ConditionalSimplifier::operator()(Block& _block)
{
	iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> std::optional<vector<Statement>>
		{
			visit(_s);
			if (_s.type() == typeid(If))
			{
				If& _if = boost::get<If>(_s);
				if (
					_if.condition->type() == typeid(Identifier) &&
					!_if.body.statements.empty() &&
					TerminationFinder(m_dialect).controlFlowKind(_if.body.statements.back()) !=
						TerminationFinder::ControlFlow::FlowOut
				)
				{
					YulString condition = boost::get<Identifier>(*_if.condition).name;
					langutil::SourceLocation location = _if.location;
					return make_vector<Statement>(
						std::move(_s),
						Assignment{
							location,
							{Identifier{location, condition}},
							make_unique<Expression>(Literal{
								location,
								LiteralKind::Number,
								"0"_yulstring,
								{}
							})
						}
					);
				}
			}
			return {};
		}
	);
}
