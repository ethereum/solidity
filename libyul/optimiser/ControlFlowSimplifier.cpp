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
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AST.h>
#include <libyul/Utilities.h>
#include <libyul/Dialect.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <range/v3/action/remove_if.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

using OptionalStatements = std::optional<std::vector<Statement>>;

namespace
{

ExpressionStatement makeDiscardCall(
	langutil::DebugData::ConstPtr const& _debugData,
	BuiltinFunction const& _discardFunction,
	Expression&& _expression
)
{
	return {_debugData, FunctionCall{
		_debugData,
		Identifier{_debugData, _discardFunction.name},
		{std::move(_expression)}
	}};
}

void removeEmptyDefaultFromSwitch(Switch& _switchStmt)
{
	ranges::actions::remove_if(
		_switchStmt.cases,
		[](Case const& _case) { return !_case.value && _case.body.statements.empty(); }
	);
}

void removeEmptyCasesFromSwitch(Switch& _switchStmt)
{
	if (hasDefaultCase(_switchStmt))
		return;

	ranges::actions::remove_if(
		_switchStmt.cases,
		[](Case const& _case) { return _case.body.statements.empty(); }
	);
}

}

void ControlFlowSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	ControlFlowSimplifier{_context.dialect}(_ast);
}

void ControlFlowSimplifier::operator()(Block& _block)
{
	simplify(_block.statements);
}

void ControlFlowSimplifier::operator()(FunctionDefinition& _funDef)
{
	ASTModifier::operator()(_funDef);
	if (!_funDef.body.statements.empty() && std::holds_alternative<Leave>(_funDef.body.statements.back()))
		_funDef.body.statements.pop_back();
}

void ControlFlowSimplifier::visit(Statement& _st)
{
	if (std::holds_alternative<ForLoop>(_st))
	{
		ForLoop& forLoop = std::get<ForLoop>(_st);
		yulAssert(forLoop.pre.statements.empty(), "");

		size_t outerBreak = m_numBreakStatements;
		size_t outerContinue = m_numContinueStatements;
		m_numBreakStatements = 0;
		m_numContinueStatements = 0;

		ASTModifier::visit(_st);

		if (!forLoop.body.statements.empty())
		{
			bool isTerminating = false;
			TerminationFinder::ControlFlow controlFlow = TerminationFinder{m_dialect}.controlFlowKind(forLoop.body.statements.back());
			if (controlFlow == TerminationFinder::ControlFlow::Break)
			{
				isTerminating = true;
				--m_numBreakStatements;
			}
			else if (
				controlFlow == TerminationFinder::ControlFlow::Terminate ||
				controlFlow == TerminationFinder::ControlFlow::Leave
			)
				isTerminating = true;

			if (isTerminating && m_numContinueStatements == 0 && m_numBreakStatements == 0)
			{
				If replacement{forLoop.debugData, std::move(forLoop.condition), std::move(forLoop.body)};
				if (controlFlow == TerminationFinder::ControlFlow::Break)
					replacement.body.statements.resize(replacement.body.statements.size() - 1);
				_st = std::move(replacement);
			}
		}

		m_numBreakStatements = outerBreak;
		m_numContinueStatements = outerContinue;
	}
	else
		ASTModifier::visit(_st);
}

void ControlFlowSimplifier::simplify(std::vector<yul::Statement>& _statements)
{
	GenericVisitor visitor{
		VisitorFallback<OptionalStatements>{},
		[&](If& _ifStmt) -> OptionalStatements {
			if (_ifStmt.body.statements.empty() && m_dialect.discardFunction())
			{
				OptionalStatements s = std::vector<Statement>{};
				s->emplace_back(makeDiscardCall(
					_ifStmt.debugData,
					*m_dialect.discardFunction(),
					std::move(*_ifStmt.condition)
				));
				return s;
			}
			return {};
		},
		[&](Switch& _switchStmt) -> OptionalStatements {
			removeEmptyDefaultFromSwitch(_switchStmt);
			removeEmptyCasesFromSwitch(_switchStmt);

			if (_switchStmt.cases.empty())
				return reduceNoCaseSwitch(_switchStmt);
			else if (_switchStmt.cases.size() == 1)
				return reduceSingleCaseSwitch(_switchStmt);

			return {};
		}
	};
	iterateReplacing(
		_statements,
		[&](Statement& _stmt) -> OptionalStatements
		{
			OptionalStatements result = std::visit(visitor, _stmt);
			if (result)
				simplify(*result);
			else
				visit(_stmt);
			return result;
		}
	);
}

OptionalStatements ControlFlowSimplifier::reduceNoCaseSwitch(Switch& _switchStmt) const
{
	yulAssert(_switchStmt.cases.empty(), "Expected no case!");
	BuiltinFunction const* discardFunction =
		m_dialect.discardFunction();
	if (!discardFunction)
		return {};

	return make_vector<Statement>(makeDiscardCall(
		debugDataOf(*_switchStmt.expression),
		*discardFunction,
		std::move(*_switchStmt.expression)
	));
}

OptionalStatements ControlFlowSimplifier::reduceSingleCaseSwitch(Switch& _switchStmt) const
{
	yulAssert(_switchStmt.cases.size() == 1, "Expected only one case!");

	auto& switchCase = _switchStmt.cases.front();
	langutil::DebugData::ConstPtr debugData = debugDataOf(*_switchStmt.expression);
	if (switchCase.value)
	{
		if (!m_dialect.equalityFunction())
			return {};
		return make_vector<Statement>(If{
			std::move(_switchStmt.debugData),
			std::make_unique<Expression>(FunctionCall{
				debugData,
				Identifier{debugData, m_dialect.equalityFunction()->name},
				{std::move(*switchCase.value), std::move(*_switchStmt.expression)}
			}),
			std::move(switchCase.body)
		});
	}
	else
	{
		if (!m_dialect.discardFunction())
			return {};

		return make_vector<Statement>(
			makeDiscardCall(
				debugData,
				*m_dialect.discardFunction(),
				std::move(*_switchStmt.expression)
			),
			std::move(switchCase.body)
		);
	}
}

