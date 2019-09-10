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

#pragma once

#include <libevmasm/SimplificationRule.h>

#include <libyul/AsmDataForward.h>
#include <libyul/AsmData.h>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <functional>
#include <vector>

namespace yul
{
struct Dialect;
class Pattern;

/**
 * Container for all simplification rules.
 */
class SimplificationRules: public boost::noncopyable
{
public:
	SimplificationRules();

	/// @returns a pointer to the first matching pattern and sets the match
	/// groups accordingly.
	/// @param _ssaValues values of variables that are assigned exactly once.
	static dev::eth::SimplificationRule<Pattern> const* findFirstMatch(
		Expression const& _expr,
		Dialect const& _dialect,
		std::map<YulString, Expression const*> const& _ssaValues
	);

	/// Checks whether the rulelist is non-empty. This is usually enforced
	/// by the constructor, but we had some issues with static initialization.
	bool isInitialized() const;

	static boost::optional<std::pair<dev::eth::Instruction, std::vector<Expression> const*>>
	instructionAndArguments(Dialect const& _dialect, Expression const& _expr);

private:
	void addRules(std::vector<dev::eth::SimplificationRule<Pattern>> const& _rules);
	void addRule(dev::eth::SimplificationRule<Pattern> const& _rule);

	void resetMatchGroups() { m_matchGroups.clear(); }

	std::map<unsigned, Expression const*> m_matchGroups;
	std::vector<dev::eth::SimplificationRule<Pattern>> m_rules[256];
};

enum class PatternKind
{
	Operation,
	Constant,
	Any
};

/**
 * Pattern to match against an expression.
 * Also stores matched expressions to retrieve them later, for constructing new expressions using
 * ExpressionTemplate.
 */
class Pattern
{
public:
	/// Matches any expression.
	Pattern(PatternKind _kind = PatternKind::Any): m_kind(_kind) {}
	// Matches a specific constant value.
	Pattern(unsigned _value): Pattern(dev::u256(_value)) {}
	// Matches a specific constant value.
	Pattern(dev::u256 const& _value): m_kind(PatternKind::Constant), m_data(std::make_shared<dev::u256>(_value)) {}
	// Matches a given instruction with given arguments
	Pattern(dev::eth::Instruction _instruction, std::vector<Pattern> const& _arguments = {});
	/// Sets this pattern to be part of the match group with the identifier @a _group.
	/// Inside one rule, all patterns in the same match group have to match expressions from the
	/// same expression equivalence class.
	void setMatchGroup(unsigned _group, std::map<unsigned, Expression const*>& _matchGroups);
	unsigned matchGroup() const { return m_matchGroup; }
	bool matches(
		Expression const& _expr,
		Dialect const& _dialect,
		std::map<YulString, Expression const*> const& _ssaValues
	) const;

	std::vector<Pattern> arguments() const { return m_arguments; }

	/// @returns the data of the matched expression if this pattern is part of a match group.
	dev::u256 d() const;

	dev::eth::Instruction instruction() const;

	/// Turns this pattern into an actual expression. Should only be called
	/// for patterns resulting from an action, i.e. with match groups assigned.
	Expression toExpression(langutil::SourceLocation const& _location) const;

private:
	Expression const& matchGroupValue() const;

	PatternKind m_kind = PatternKind::Any;
	dev::eth::Instruction m_instruction; ///< Only valid if m_kind is Operation
	std::shared_ptr<dev::u256> m_data; ///< Only valid if m_kind is Constant
	std::vector<Pattern> m_arguments;
	unsigned m_matchGroup = 0;
	std::map<unsigned, Expression const*>* m_matchGroups = nullptr;
};

}
