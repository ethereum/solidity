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

boost::optional<dev::u256> StructuralSimplifier::hasLiteralValue(Expression const& _expression) const
{
	Expression const* expr = &_expression;

	if (expr->type() == typeid(Identifier))
	{
		Identifier const& ident = boost::get<Identifier>(*expr);
		if (m_value.count(ident.name))
			expr = m_value.at(ident.name);
	}

	if (expr && expr->type() == typeid(Literal))
	{
		Literal const& literal = boost::get<Literal>(*expr);
		return valueOfLiteral(literal);
	}

	return boost::optional<u256>();
}

void StructuralSimplifier::simplify(std::vector<yul::Statement>& _statements)
{
	using OptionalStatements = boost::optional<vector<Statement>>;
	GenericFallbackReturnsVisitor<OptionalStatements, If, Switch, ForLoop> const visitor(
		[&](If& _ifStmt) -> OptionalStatements {
			if (_ifStmt.body.statements.empty())
			{
				OptionalStatements s = vector<Statement>{};
				s->emplace_back(makePopExpressionStatement(_ifStmt.location, std::move(*_ifStmt.condition)));
				return s;
			}
			if (expressionAlwaysTrue(*_ifStmt.condition))
				return {std::move(_ifStmt.body.statements)};
			else if (expressionAlwaysFalse(*_ifStmt.condition))
				return {vector<Statement>{}};
			return {};
		},
		[&](Switch& _switchStmt) -> OptionalStatements {
			auto& cases = _switchStmt.cases;

			if (cases.size() == 1)
			{
				auto& switchCase = cases.front();
				auto loc = locationOf(*_switchStmt.expression);
				if (switchCase.value)
				{
					OptionalStatements s = vector<Statement>{};
					s->emplace_back(If{
						std::move(_switchStmt.location),
						make_unique<Expression>(FunctionalInstruction{
								std::move(loc),
								solidity::Instruction::EQ,
								{std::move(*switchCase.value), std::move(*_switchStmt.expression)}
						}), std::move(switchCase.body)});
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
			else if (boost::optional<u256> const constExprVal = hasLiteralValue(*_switchStmt.expression))
			{
				Block* matchingCaseBlock = nullptr;
				Case* defaultCase = nullptr;

				for (auto& _case: cases)
				{
					if (_case.value && valueOfLiteral(*_case.value) == constExprVal)
					{
						matchingCaseBlock = &_case.body;
						break;
					}
					else if (!_case.value)
						defaultCase = &_case;
				}

				if (!matchingCaseBlock && defaultCase)
					matchingCaseBlock = &defaultCase->body;

				OptionalStatements s = vector<Statement>{};

				if (matchingCaseBlock)
					s->emplace_back(std::move(*matchingCaseBlock));

				return s;
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
