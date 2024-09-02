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

#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;

void ForLoopConditionIntoBody::run(OptimiserStepContext& _context, Block& _ast)
{
	ForLoopConditionIntoBody{_context.dialect}(_ast);
}

void ForLoopConditionIntoBody::operator()(ForLoop& _forLoop)
{
	if (
		m_dialect.booleanNegationFunction() &&
		!std::holds_alternative<Literal>(*_forLoop.condition) &&
		!std::holds_alternative<Identifier>(*_forLoop.condition)
	)
	{
		langutil::DebugData::ConstPtr debugData = debugDataOf(*_forLoop.condition);

		_forLoop.body.statements.emplace(
			begin(_forLoop.body.statements),
			If {
				debugData,
				std::make_unique<Expression>(
					FunctionCall {
						debugData,
						{debugData, m_dialect.booleanNegationFunction()->name},
						util::make_vector<Expression>(std::move(*_forLoop.condition))
					}
				),
				Block {debugData, util::make_vector<Statement>(Break{{}})}
			}
		);
		_forLoop.condition = std::make_unique<Expression>(
			Literal {
				debugData,
				LiteralKind::Boolean,
				LiteralValue{true}
			}
		);
	}
	ASTModifier::operator()(_forLoop);
}

