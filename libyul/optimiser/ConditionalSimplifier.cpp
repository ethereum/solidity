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
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

void ConditionalSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	ConditionalSimplifier{
		_context.dialect,
		ControlFlowSideEffectsCollector{_context.dialect, _ast}.functionSideEffectsNamed()
	}(_ast);
}

void ConditionalSimplifier::operator()(Switch& _switch)
{
	visit(*_switch.expression);
	if (!std::holds_alternative<Identifier>(*_switch.expression))
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
			_case.body.statements.insert(_case.body.statements.begin(),
				Assignment{
					_case.body.debugData,
					{Identifier{_case.body.debugData, expr}},
					std::make_unique<Expression>(*_case.value)
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
		[&](Statement& _s) -> std::optional<std::vector<Statement>>
		{
			visit(_s);
			if (std::holds_alternative<If>(_s))
			{
				If& _if = std::get<If>(_s);
				if (
					std::holds_alternative<Identifier>(*_if.condition) &&
					!_if.body.statements.empty() &&
					TerminationFinder(m_dialect, &m_functionSideEffects).controlFlowKind(_if.body.statements.back()) !=
						TerminationFinder::ControlFlow::FlowOut
				)
				{
					YulString condition = std::get<Identifier>(*_if.condition).name;
					std::shared_ptr<DebugData const> debugData = _if.debugData;
					return make_vector<Statement>(
						std::move(_s),
						Assignment{
							debugData,
							{Identifier{debugData, condition}},
							std::make_unique<Expression>(m_dialect.zeroLiteralForType(m_dialect.boolType))
						}
					);
				}
			}
			return {};
		}
	);
}
