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
/**
 * Module for applying replacement rules against Expressions.
 */

#include <libyul/optimiser/SimplificationRules.h>

#include <libyul/optimiser/Utilities.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/AsmData.h>

#include <libevmasm/RuleList.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;


SimplificationRule<Pattern> const* SimplificationRules::findFirstMatch(
	Expression const& _expr,
	Dialect const& _dialect,
	map<YulString, Expression const*> const& _ssaValues
)
{
	if (_expr.type() != typeid(FunctionalInstruction))
		return nullptr;

	static SimplificationRules rules;
	assertThrow(rules.isInitialized(), OptimizerException, "Rule list not properly initialized.");

	FunctionalInstruction const& instruction = boost::get<FunctionalInstruction>(_expr);
	for (auto const& rule: rules.m_rules[uint8_t(instruction.instruction)])
	{
		rules.resetMatchGroups();
		if (rule.pattern.matches(_expr, _dialect, _ssaValues))
			return &rule;
	}
	return nullptr;
}

bool SimplificationRules::isInitialized() const
{
	return !m_rules[uint8_t(solidity::Instruction::ADD)].empty();
}

void SimplificationRules::addRules(vector<SimplificationRule<Pattern>> const& _rules)
{
	for (auto const& r: _rules)
		addRule(r);
}

void SimplificationRules::addRule(SimplificationRule<Pattern> const& _rule)
{
	m_rules[uint8_t(_rule.pattern.instruction())].push_back(_rule);
}

SimplificationRules::SimplificationRules()
{
	// Multiple occurrences of one of these inside one rule must match the same equivalence class.
	// Constants.
	Pattern A(PatternKind::Constant);
	Pattern B(PatternKind::Constant);
	Pattern C(PatternKind::Constant);
	// Anything.
	Pattern X;
	Pattern Y;
	A.setMatchGroup(1, m_matchGroups);
	B.setMatchGroup(2, m_matchGroups);
	C.setMatchGroup(3, m_matchGroups);
	X.setMatchGroup(4, m_matchGroups);
	Y.setMatchGroup(5, m_matchGroups);

	addRules(simplificationRuleList(A, B, C, X, Y));
	assertThrow(isInitialized(), OptimizerException, "Rule list not properly initialized.");
}

Pattern::Pattern(solidity::Instruction _instruction, vector<Pattern> const& _arguments):
	m_kind(PatternKind::Operation),
	m_instruction(_instruction),
	m_arguments(_arguments)
{
}

void Pattern::setMatchGroup(unsigned _group, map<unsigned, Expression const*>& _matchGroups)
{
	m_matchGroup = _group;
	m_matchGroups = &_matchGroups;
}

bool Pattern::matches(
	Expression const& _expr,
	Dialect const& _dialect,
	map<YulString, Expression const*> const& _ssaValues
) const
{
	Expression const* expr = &_expr;

	// Resolve the variable if possible.
	// Do not do it for "Any" because we can check identity better for variables.
	if (m_kind != PatternKind::Any && _expr.type() == typeid(Identifier))
	{
		YulString varName = boost::get<Identifier>(_expr).name;
		if (_ssaValues.count(varName))
			if (Expression const* new_expr = _ssaValues.at(varName))
				expr = new_expr;
	}
	assertThrow(expr, OptimizerException, "");

	if (m_kind == PatternKind::Constant)
	{
		if (expr->type() != typeid(Literal))
			return false;
		Literal const& literal = boost::get<Literal>(*expr);
		if (literal.kind != LiteralKind::Number)
			return false;
		if (m_data && *m_data != u256(literal.value.str()))
			return false;
		assertThrow(m_arguments.empty(), OptimizerException, "");
	}
	else if (m_kind == PatternKind::Operation)
	{
		if (expr->type() != typeid(FunctionalInstruction))
			return false;
		FunctionalInstruction const& instr = boost::get<FunctionalInstruction>(*expr);
		if (m_instruction != instr.instruction)
			return false;
		assertThrow(m_arguments.size() == instr.arguments.size(), OptimizerException, "");
		for (size_t i = 0; i < m_arguments.size(); ++i)
			if (!m_arguments[i].matches(instr.arguments.at(i), _dialect, _ssaValues))
				return false;
	}
	else
	{
		assertThrow(m_arguments.empty(), OptimizerException, "\"Any\" should not have arguments.");
	}

	if (m_matchGroup)
	{
		// We support matching multiple expressions that require the same value
		// based on identical ASTs, which have to be movable.

		// TODO: add tests:
		// - { let x := mload(0) let y := and(x, x) }
		// - { let x := 4 let y := and(x, y) }

		// This code uses `_expr` again for "Any", because we want the comparison to be done
		// on the variables and not their values.
		// The assumption is that CSE or local value numbering has been done prior to this step.

		if (m_matchGroups->count(m_matchGroup))
		{
			assertThrow(m_kind == PatternKind::Any, OptimizerException, "Match group repetition for non-any.");
			Expression const* firstMatch = (*m_matchGroups)[m_matchGroup];
			assertThrow(firstMatch, OptimizerException, "Match set but to null.");
			return
				SyntacticalEqualityChecker::equal(*firstMatch, _expr) &&
				MovableChecker(_dialect, _expr).movable();
		}
		else if (m_kind == PatternKind::Any)
			(*m_matchGroups)[m_matchGroup] = &_expr;
		else
		{
			assertThrow(m_kind == PatternKind::Constant, OptimizerException, "Match group set for operation.");
			// We do not use _expr here, because we want the actual number.
			(*m_matchGroups)[m_matchGroup] = expr;
		}
	}
	return true;
}

solidity::Instruction Pattern::instruction() const
{
	assertThrow(m_kind == PatternKind::Operation, OptimizerException, "");
	return m_instruction;
}

Expression Pattern::toExpression(SourceLocation const& _location) const
{
	if (matchGroup())
		return ASTCopier().translate(matchGroupValue());
	if (m_kind == PatternKind::Constant)
	{
		assertThrow(m_data, OptimizerException, "No match group and no constant value given.");
		return Literal{_location, LiteralKind::Number, YulString{formatNumber(*m_data)}, {}};
	}
	else if (m_kind == PatternKind::Operation)
	{
		vector<Expression> arguments;
		for (auto const& arg: m_arguments)
			arguments.emplace_back(arg.toExpression(_location));
		return FunctionalInstruction{_location, m_instruction, std::move(arguments)};
	}
	assertThrow(false, OptimizerException, "Pattern of kind 'any', but no match group.");
}

u256 Pattern::d() const
{
	return valueOfNumberLiteral(boost::get<Literal>(matchGroupValue()));
}

Expression const& Pattern::matchGroupValue() const
{
	assertThrow(m_matchGroup > 0, OptimizerException, "");
	assertThrow(!!m_matchGroups, OptimizerException, "");
	assertThrow((*m_matchGroups)[m_matchGroup], OptimizerException, "");
	return *(*m_matchGroups)[m_matchGroup];
}
