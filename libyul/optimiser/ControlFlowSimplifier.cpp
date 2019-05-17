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
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>

using namespace std;
using namespace dev;
using namespace yul;

using OptionalStatements = boost::optional<vector<Statement>>;

namespace
{

ExpressionStatement makePopExpressionStatement(langutil::SourceLocation const& _location, Expression&& _expression)
{
	return {_location, FunctionalInstruction{
		_location,
		dev::eth::Instruction::POP,
		{std::move(_expression)}
	}};
}

void removeEmptyDefaultFromSwitch(Switch& _switchStmt)
{
	boost::remove_erase_if(
		_switchStmt.cases,
		[](Case const& _case) { return !_case.value && _case.body.statements.empty(); }
	);
}

void removeEmptyCasesFromSwitch(Switch& _switchStmt)
{
	bool hasDefault = boost::algorithm::any_of(
		_switchStmt.cases,
		[](Case const& _case) { return !_case.value; }
	);

	if (hasDefault)
		return;

	boost::remove_erase_if(
		_switchStmt.cases,
		[](Case const& _case) { return _case.body.statements.empty(); }
	);
}

OptionalStatements reduceNoCaseSwitch(Switch& _switchStmt)
{
	yulAssert(_switchStmt.cases.empty(), "Expected no case!");

	auto loc = locationOf(*_switchStmt.expression);

	return make_vector<Statement>(makePopExpressionStatement(loc, std::move(*_switchStmt.expression)));
}

OptionalStatements reduceSingleCaseSwitch(Switch& _switchStmt)
{
	yulAssert(_switchStmt.cases.size() == 1, "Expected only one case!");

	auto& switchCase = _switchStmt.cases.front();
	auto loc = locationOf(*_switchStmt.expression);
	if (switchCase.value)
		return make_vector<Statement>(If{
			std::move(_switchStmt.location),
			make_unique<Expression>(FunctionalInstruction{
				std::move(loc),
				dev::eth::Instruction::EQ,
				{std::move(*switchCase.value), std::move(*_switchStmt.expression)}
			}),
			std::move(switchCase.body)
		});
	else
		return make_vector<Statement>(
			makePopExpressionStatement(loc, std::move(*_switchStmt.expression)),
			std::move(switchCase.body)
		);
}

}

void ControlFlowSimplifier::operator()(Block& _block)
{
	simplify(_block.statements);
}

void ControlFlowSimplifier::visit(Statement& _st)
{
	if (_st.type() == typeid(ForLoop))
	{
		ForLoop& forLoop = boost::get<ForLoop>(_st);
		yulAssert(forLoop.pre.statements.empty(), "");

		size_t outerBreak = m_numBreakStatements;
		size_t outerContinue = m_numContinueStatements;
		m_numBreakStatements = 0;
		m_numContinueStatements = 0;

		ASTModifier::visit(_st);

		if (!forLoop.body.statements.empty())
		{
			bool isTerminating = false;
			TerminationFinder::ControlFlow controlFlow = TerminationFinder::controlFlowKind(forLoop.body.statements.back());
			if (controlFlow == TerminationFinder::ControlFlow::Break)
			{
				isTerminating = true;
				--m_numBreakStatements;
			}
			else if (controlFlow == TerminationFinder::ControlFlow::Terminate)
				isTerminating = true;

			if (isTerminating && m_numContinueStatements == 0 && m_numBreakStatements == 0)
			{
				If replacement{forLoop.location, std::move(forLoop.condition), std::move(forLoop.body)};
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
	GenericFallbackReturnsVisitor<OptionalStatements, If, Switch> const visitor(
		[&](If& _ifStmt) -> OptionalStatements {
			if (_ifStmt.body.statements.empty())
			{
				OptionalStatements s = vector<Statement>{};
				s->emplace_back(makePopExpressionStatement(_ifStmt.location, std::move(*_ifStmt.condition)));
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
	);

	iterateReplacing(
		_statements,
		[&](Statement& _stmt) -> OptionalStatements
		{
			OptionalStatements result = boost::apply_visitor(visitor, _stmt);
			if (result)
				simplify(*result);
			else
				visit(_stmt);
			return result;
		}
	);
}
