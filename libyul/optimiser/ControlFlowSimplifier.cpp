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

	OptionalStatements s = vector<Statement>{};

	s->emplace_back(makePopExpressionStatement(loc, std::move(*_switchStmt.expression)));

	return s;
}

OptionalStatements reduceSingleCaseSwitch(Switch& _switchStmt)
{
	yulAssert(_switchStmt.cases.size() == 1, "Expected only one case!");

	auto& switchCase = _switchStmt.cases.front();
	auto loc = locationOf(*_switchStmt.expression);
	if (switchCase.value)
	{
		OptionalStatements s = vector<Statement>{};
		s->emplace_back(If{
				std::move(_switchStmt.location),
				make_unique<Expression>(FunctionalInstruction{
					std::move(loc),
					dev::eth::Instruction::EQ,
					{std::move(*switchCase.value), std::move(*_switchStmt.expression)}
				}),
				std::move(switchCase.body)
		});
		return s;
	}
	else
	{
		OptionalStatements s = vector<Statement>{};
		s->emplace_back(makePopExpressionStatement(loc, std::move(*_switchStmt.expression)));
		s->emplace_back(std::move(switchCase.body));
		return s;
	}
}

}

void ControlFlowSimplifier::operator()(Block& _block)
{
	simplify(_block.statements);
}

void ControlFlowSimplifier::simplify(std::vector<yul::Statement>& _statements)
{
	GenericFallbackReturnsVisitor<OptionalStatements, If, Switch, ForLoop> const visitor(
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
		},
		[&](ForLoop&) -> OptionalStatements {
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
