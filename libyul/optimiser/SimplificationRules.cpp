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
/**
 * Module for applying replacement rules against Expressions.
 */

#include <libyul/optimiser/SimplificationRules.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AST.h>
#include <libyul/Utilities.h>

#include <libevmasm/RuleList.h>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;
using namespace solidity::yul;

SimplificationRules::Rule const* SimplificationRules::findFirstMatch(
	Expression const& _expr,
	Dialect const& _dialect,
	map<YulString, AssignedValue> const& _ssaValues
)
{
	auto instruction = instructionAndArguments(_dialect, _expr);
	if (!instruction)
		return nullptr;

	static std::map<std::optional<EVMVersion>, std::unique_ptr<SimplificationRules>> evmRules;

	std::optional<EVMVersion> version;
	if (yul::EVMDialect const* evmDialect = dynamic_cast<yul::EVMDialect const*>(&_dialect))
		version = evmDialect->evmVersion();

	if (!evmRules[version])
		evmRules[version] = std::make_unique<SimplificationRules>(version);

	SimplificationRules& rules = *evmRules[version];
	assertThrow(rules.isInitialized(), OptimizerException, "Rule list not properly initialized.");

	for (auto const& rule: rules.m_rules[uint8_t(instruction->first)])
	{
		rules.resetMatchGroups();
		if (rule.pattern.matches(_expr, _dialect, _ssaValues))
			if (!rule.feasible || rule.feasible())
				return &rule;
	}
	return nullptr;
}

bool SimplificationRules::isInitialized() const
{
	return !m_rules[uint8_t(evmasm::Instruction::ADD)].empty();
}

std::optional<std::pair<evmasm::Instruction, vector<Expression> const*>>
	SimplificationRules::instructionAndArguments(Dialect const& _dialect, Expression const& _expr)
{
	if (holds_alternative<FunctionCall>(_expr))
		if (auto const* dialect = dynamic_cast<EVMDialect const*>(&_dialect))
			if (auto const* builtin = dialect->builtin(std::get<FunctionCall>(_expr).functionName.name))
				if (builtin->instruction)
					return make_pair(*builtin->instruction, &std::get<FunctionCall>(_expr).arguments);

	return {};
}

void SimplificationRules::addRules(std::vector<Rule> const& _rules)
{
	for (auto const& r: _rules)
		addRule(r);
}

void SimplificationRules::addRule(Rule const& _rule)
{
	m_rules[uint8_t(_rule.pattern.instruction())].push_back(_rule);
}

SimplificationRules::SimplificationRules(std::optional<langutil::EVMVersion> _evmVersion)
{
	// Multiple occurrences of one of these inside one rule must match the same equivalence class.
	// Constants.
	Pattern A(PatternKind::Constant);
	Pattern B(PatternKind::Constant);
	Pattern C(PatternKind::Constant);
	// Anything.
	Pattern W;
	Pattern X;
	Pattern Y;
	Pattern Z;
	A.setMatchGroup(1, m_matchGroups);
	B.setMatchGroup(2, m_matchGroups);
	C.setMatchGroup(3, m_matchGroups);
	W.setMatchGroup(4, m_matchGroups);
	X.setMatchGroup(5, m_matchGroups);
	Y.setMatchGroup(6, m_matchGroups);
	Z.setMatchGroup(7, m_matchGroups);

	addRules(simplificationRuleList(_evmVersion, A, B, C, W, X, Y, Z));
	assertThrow(isInitialized(), OptimizerException, "Rule list not properly initialized.");
}

yul::Pattern::Pattern(evmasm::Instruction _instruction, initializer_list<Pattern> _arguments):
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
	map<YulString, AssignedValue> const& _ssaValues
) const
{
	Expression const* expr = &_expr;

	// Resolve the variable if possible.
	// Do not do it for "Any" because we can check identity better for variables.
	if (m_kind != PatternKind::Any && holds_alternative<Identifier>(_expr))
	{
		YulString varName = std::get<Identifier>(_expr).name;
		if (_ssaValues.count(varName))
			if (Expression const* new_expr = _ssaValues.at(varName).value)
				expr = new_expr;
	}
	assertThrow(expr, OptimizerException, "");

	if (m_kind == PatternKind::Constant)
	{
		if (!holds_alternative<Literal>(*expr))
			return false;
		Literal const& literal = std::get<Literal>(*expr);
		if (literal.kind != LiteralKind::Number)
			return false;
		if (m_data && *m_data != u256(literal.value.str()))
			return false;
		assertThrow(m_arguments.empty(), OptimizerException, "");
	}
	else if (m_kind == PatternKind::Operation)
	{
		auto instrAndArgs = SimplificationRules::instructionAndArguments(_dialect, *expr);
		if (!instrAndArgs || m_instruction != instrAndArgs->first)
			return false;
		assertThrow(m_arguments.size() == instrAndArgs->second->size(), OptimizerException, "");
		for (size_t i = 0; i < m_arguments.size(); ++i)
		{
			Expression const& arg = instrAndArgs->second->at(i);
			// If this is a direct function call instead of a variable or literal,
			// we reject the match because side-effects could prevent us from
			// arbitrarily modifying the code.
			if (
				holds_alternative<FunctionCall>(arg) ||
				!m_arguments[i].matches(arg, _dialect, _ssaValues)
			)
				return false;
		}
	}
	else
	{
		assertThrow(m_arguments.empty(), OptimizerException, "\"Any\" should not have arguments.");
		assertThrow(!holds_alternative<FunctionCall>(*expr), OptimizerException, "\"Any\" at top-level.");
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
			assertThrow(
				!holds_alternative<FunctionCall>(_expr) &&
				!holds_alternative<FunctionCall>(*firstMatch),
				OptimizerException,
				"Group matches have to be literals or variables."
			);

			return SyntacticallyEqual{}(*firstMatch, _expr);
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

evmasm::Instruction Pattern::instruction() const
{
	assertThrow(m_kind == PatternKind::Operation, OptimizerException, "");
	return m_instruction;
}

Expression Pattern::toExpression(shared_ptr<DebugData const> const& _debugData) const
{
	if (matchGroup())
		return ASTCopier().translate(matchGroupValue());
	if (m_kind == PatternKind::Constant)
	{
		assertThrow(m_data, OptimizerException, "No match group and no constant value given.");
		return Literal{_debugData, LiteralKind::Number, YulString{formatNumber(*m_data)}, {}};
	}
	else if (m_kind == PatternKind::Operation)
	{
		vector<Expression> arguments;
		for (auto const& arg: m_arguments)
			arguments.emplace_back(arg.toExpression(_debugData));

		string name = instructionInfo(m_instruction).name;
		transform(begin(name), end(name), begin(name), [](auto _c) { return tolower(_c); });

		return FunctionCall{_debugData,
			Identifier{_debugData, YulString{name}},
			std::move(arguments)
		};
	}
	assertThrow(false, OptimizerException, "Pattern of kind 'any', but no match group.");
}

u256 Pattern::d() const
{
	return valueOfNumberLiteral(std::get<Literal>(matchGroupValue()));
}

Expression const& Pattern::matchGroupValue() const
{
	assertThrow(m_matchGroup > 0, OptimizerException, "");
	assertThrow(!!m_matchGroups, OptimizerException, "");
	assertThrow((*m_matchGroups)[m_matchGroup], OptimizerException, "");
	return *(*m_matchGroups)[m_matchGroup];
}
