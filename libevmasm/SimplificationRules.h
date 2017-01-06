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
 * @file SimplificationRules
 * @author Christian <chris@ethereum.org>
 * @date 2017
 * Module for applying replacement rules against Expressions.
 */

#pragma once

#include <libevmasm/ExpressionClasses.h>

#include <functional>
#include <vector>

namespace dev
{
namespace eth
{

class Pattern;

/**
 * Container for all simplification rules.
 */
class Rules: public boost::noncopyable
{
public:
	using Expression = ExpressionClasses::Expression;

	Rules();

	/// @returns a pointer to the first matching pattern and sets the match
	/// groups accordingly.
	std::pair<Pattern, std::function<Pattern()>> const* findFirstMatch(
		Expression const& _expr,
		ExpressionClasses const& _classes
	);

private:
	void addRules(std::vector<std::pair<Pattern, std::function<Pattern()>>> const& _rules);
	void addRule(std::pair<Pattern, std::function<Pattern()>> const& _rule);

	void resetMatchGroups() { m_matchGroups.clear(); }

	std::map<unsigned, Expression const*> m_matchGroups;
	std::vector<std::pair<Pattern, std::function<Pattern()>>> m_rules[256];
};

/**
 * Pattern to match against an expression.
 * Also stores matched expressions to retrieve them later, for constructing new expressions using
 * ExpressionTemplate.
 */
class Pattern
{
public:
	using Expression = ExpressionClasses::Expression;
	using Id = ExpressionClasses::Id;

	// Matches a specific constant value.
	Pattern(unsigned _value): Pattern(u256(_value)) {}
	// Matches a specific constant value.
	Pattern(u256 const& _value): m_type(Push), m_requireDataMatch(true), m_data(std::make_shared<u256>(_value)) {}
	// Matches a specific assembly item type or anything if not given.
	Pattern(AssemblyItemType _type = UndefinedItem): m_type(_type) {}
	// Matches a given instruction with given arguments
	Pattern(Instruction _instruction, std::vector<Pattern> const& _arguments = {});
	/// Sets this pattern to be part of the match group with the identifier @a _group.
	/// Inside one rule, all patterns in the same match group have to match expressions from the
	/// same expression equivalence class.
	void setMatchGroup(unsigned _group, std::map<unsigned, Expression const*>& _matchGroups);
	unsigned matchGroup() const { return m_matchGroup; }
	bool matches(Expression const& _expr, ExpressionClasses const& _classes) const;

	AssemblyItem toAssemblyItem(SourceLocation const& _location) const;
	std::vector<Pattern> arguments() const { return m_arguments; }

	/// @returns the id of the matched expression if this pattern is part of a match group.
	Id id() const { return matchGroupValue().id; }
	/// @returns the data of the matched expression if this pattern is part of a match group.
	u256 const& d() const { return matchGroupValue().item->data(); }

	std::string toString() const;

	AssemblyItemType type() const { return m_type; }
	Instruction instruction() const
	{
		assertThrow(type() == Operation, OptimizerException, "");
		return m_instruction;
	}

private:
	bool matchesBaseItem(AssemblyItem const* _item) const;
	Expression const& matchGroupValue() const;
	u256 const& data() const;

	AssemblyItemType m_type;
	bool m_requireDataMatch = false;
	Instruction m_instruction; ///< Only valid if m_type is Operation
	std::shared_ptr<u256> m_data; ///< Only valid if m_type is not Operation
	std::vector<Pattern> m_arguments;
	unsigned m_matchGroup = 0;
	std::map<unsigned, Expression const*>* m_matchGroups = nullptr;
};

/**
 * Template for a new expression that can be built from matched patterns.
 */
struct ExpressionTemplate
{
	using Expression = ExpressionClasses::Expression;
	using Id = ExpressionClasses::Id;
	explicit ExpressionTemplate(Pattern const& _pattern, SourceLocation const& _location);
	std::string toString() const;
	bool hasId = false;
	/// Id of the matched expression, if available.
	Id id = Id(-1);
	// Otherwise, assembly item.
	AssemblyItem item = UndefinedItem;
	std::vector<ExpressionTemplate> arguments;
};

}
}
