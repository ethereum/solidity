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
#include <libyul/optimiser/ConditionalBranchFlattener.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

void ConditionalBranchFlattener::run(OptimiserStepContext& _context, Block& _ast)
{
	ConditionalBranchFlattener{_context}(_ast);
}

void ConditionalBranchFlattener::operator()(Block& _block)
{
	iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> std::optional<std::vector<Statement>>
		{
			visit(_s);

			if (!std::holds_alternative<If>(_s))
				return {};

			If& ifStatement = std::get<If>(_s);
			std::vector<Statement>& bodyStatements = ifStatement.body.statements;

			if (bodyStatements.size() != 1)
				return {};

			Statement& bodyStatement = bodyStatements.front();
			if (!std::holds_alternative<Assignment>(bodyStatement))
				return {};

			Assignment& assignment = std::get<Assignment>(bodyStatement);
			if (assignment.variableNames.size() != 1)
				return {};

			SideEffectsCollector sideEffectsCollector(m_context.dialect);
			sideEffectsCollector.visit(*assignment.value);
			if (!sideEffectsCollector.movable())
				return {};

			langutil::DebugData::ConstPtr debugData = ifStatement.debugData;
			YulName conditionName = m_context.dispenser.newName({YulName("condition")});

			auto normalizedCondition = std::make_unique<Expression>(
				createBuiltinCall(
					debugData,
					"iszero",
					make_vector<Expression>(
						createBuiltinCall(
							debugData,
							"iszero",
							make_vector<Expression>(std::move(*ifStatement.condition))
						)
					)
				)
			);

			std::vector<Statement> transformed;
			transformed.reserve(2);

			transformed.push_back(VariableDeclaration{
				debugData,
				{NameWithDebugData{debugData, conditionName}},
				std::move(normalizedCondition)
			});

			YulName targetName = assignment.variableNames[0].name;
			std::unique_ptr<Expression> rhs = std::move(assignment.value);

			Expression mask = createBuiltinCall(
				debugData,
				"sub",
				make_vector<Expression>(
					m_context.dialect.zeroLiteral(),
					Identifier{debugData, conditionName}
				)
			);

			Expression diff = createBuiltinCall(
				debugData,
				"xor",
				make_vector<Expression>(
					Identifier{debugData, targetName},
					std::move(*rhs)
				)
			);

			Expression maskedDiff = createBuiltinCall(
				debugData,
				"and",
				make_vector<Expression>(
					std::move(mask),
					std::move(diff)
				)
			);

			Expression finalExpr = createBuiltinCall(
				debugData,
				"xor",
				make_vector<Expression>(
					Identifier{debugData, targetName},
					std::move(maskedDiff)
				)
			);

			transformed.push_back(Assignment{
				debugData,
				assignment.variableNames,
				std::make_unique<Expression>(std::move(finalExpr))
			});

			return transformed;
		}
	);
}

FunctionCall ConditionalBranchFlattener::createBuiltinCall(
	langutil::DebugData::ConstPtr _debugData,
	std::string const& _name,
	std::vector<Expression> _arguments
)
{
	auto handle = m_context.dialect.findBuiltin(_name);
	yulAssert(handle.has_value(), "Builtin not found: " + _name);
	return FunctionCall{
		std::move(_debugData),
		{BuiltinName{nullptr, *handle}},
		std::move(_arguments)
	};
}
