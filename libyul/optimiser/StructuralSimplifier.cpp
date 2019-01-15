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
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace {

ExpressionStatement makePopExpressionStatement(langutil::SourceLocation const& _location, Expression&& _expression)
{
	return {_location, FunctionalInstruction{
		_location,
		solidity::Instruction::POP,
		{std::move(_expression)}
	}};
}

}

void StructuralSimplifier::operator()(Block& _block)
{
	pushScope(false);
	simplify(_block.statements);
	popScope();
}

void StructuralSimplifier::simplify(std::vector<yul::Statement>& _statements)
{
	using OptionalStatements = boost::optional<vector<Statement>>;
	GenericFallbackReturnsVisitor<OptionalStatements, If, Switch, ForLoop> const visitor(
		[&](If& _ifStmt) -> OptionalStatements {
			if (_ifStmt.body.statements.empty())
				return {{makePopExpressionStatement(_ifStmt.location, std::move(*_ifStmt.condition))}};
			if (expressionAlwaysTrue(*_ifStmt.condition))
				return {std::move(_ifStmt.body.statements)};
			else if (expressionAlwaysFalse(*_ifStmt.condition))
				return {vector<Statement>{}};
			return {};
		},
		[](Switch& _switchStmt) -> OptionalStatements {
			if (_switchStmt.cases.size() == 1)
			{
				auto& switchCase = _switchStmt.cases.front();
				auto loc = locationOf(*_switchStmt.expression);
				if (switchCase.value)
					return {{If{
						std::move(_switchStmt.location),
						make_shared<Expression>(FunctionalInstruction{
								std::move(loc),
								solidity::Instruction::EQ,
								{std::move(*switchCase.value), std::move(*_switchStmt.expression)}
						}), std::move(switchCase.body)}}};
				else
					return {{
						makePopExpressionStatement(loc, std::move(*_switchStmt.expression)),
						std::move(switchCase.body)
					}};
			}
			else
				return {};
		},
		[&](ForLoop& _forLoop) -> OptionalStatements {
			if (expressionAlwaysFalse(*_forLoop.condition))
				return {std::move(_forLoop.pre.statements)};
			else
				return {};
		}
	);

	iterateReplacing(
		_statements,
		[&](Statement& _stmt) -> OptionalStatements
		{
			visit(_stmt);
			OptionalStatements result = boost::apply_visitor(visitor, _stmt);
			if (result)
				simplify(*result);
			return result;
		}
	);
}

bool StructuralSimplifier::expressionAlwaysTrue(Expression const& _expression)
{
	return boost::apply_visitor(GenericFallbackReturnsVisitor<bool, Identifier const, Literal const>(
		[&](Identifier const& _identifier) -> bool {
			if (auto expr = m_value[_identifier.name])
				return expressionAlwaysTrue(*expr);
			return false;
		},
		[](Literal const& _literal) -> bool {
			static YulString const trueString("true");
			return
				(_literal.kind == LiteralKind::Boolean && _literal.value == trueString) ||
				(_literal.kind == LiteralKind::Number && valueOfNumberLiteral(_literal) != u256(0))
			;
		}
	), _expression);
}

bool StructuralSimplifier::expressionAlwaysFalse(Expression const& _expression)
{
	return boost::apply_visitor(GenericFallbackReturnsVisitor<bool, Identifier const, Literal const>(
		[&](Identifier const& _identifier) -> bool {
			if (auto expr = m_value[_identifier.name])
				return expressionAlwaysFalse(*expr);
			return false;
		},
		[](Literal const& _literal) -> bool {
			static YulString const falseString("false");
			return
				(_literal.kind == LiteralKind::Boolean && _literal.value == falseString) ||
				(_literal.kind == LiteralKind::Number && valueOfNumberLiteral(_literal) == u256(0))
			;
		}
	), _expression);
}
