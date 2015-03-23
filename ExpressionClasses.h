/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file ExpressionClasses.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#pragma once

#include <vector>
#include <map>
#include <memory>
#include <libdevcore/Common.h>
#include <libevmcore/AssemblyItem.h>

namespace dev
{
namespace eth
{

class Pattern;
struct ExpressionTemplate;

/**
 * Collection of classes of equivalent expressions that can also determine the class of an expression.
 * Identifiers are contiguously assigned to new classes starting from zero.
 */
class ExpressionClasses
{
public:
	using Id = unsigned;
	using Ids = std::vector<Id>;

	struct Expression
	{
		Id id;
		AssemblyItem const* item;
		Ids arguments;
		bool operator<(Expression const& _other) const;
	};

	/// Retrieves the id of the expression equivalence class resulting from the given item applied to the
	/// given classes, might also create a new one.
	Id find(AssemblyItem const& _item, Ids const& _arguments = {});
	/// @returns the canonical representative of an expression class.
	Expression const& representative(Id _id) const { return m_representatives.at(_id); }
	/// @returns the number of classes.
	Id size() const { return m_representatives.size(); }

	std::string fullDAGToString(Id _id);

private:
	/// Tries to simplify the given expression.
	/// @returns its class if it possible or Id(-1) otherwise.
	/// @param _secondRun is set to true for the second run where arguments of commutative expressions are reversed
	Id tryToSimplify(Expression const& _expr, bool _secondRun = false);

	/// Rebuilds an expression from a (matched) pattern.
	Id rebuildExpression(ExpressionTemplate const& _template);

	std::vector<std::pair<Pattern, std::function<Pattern()>>> createRules() const;

	/// Expression equivalence class representatives - we only store one item of an equivalence.
	std::vector<Expression> m_representatives;
	std::vector<std::shared_ptr<AssemblyItem>> m_spareAssemblyItem;
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
	Pattern(u256 const& _value): m_type(Push), m_requireDataMatch(true), m_data(_value) {}
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

	AssemblyItem toAssemblyItem() const { return AssemblyItem(m_type, m_data); }
	std::vector<Pattern> arguments() const { return m_arguments; }

	/// @returns the id of the matched expression if this pattern is part of a match group.
	Id id() const { return matchGroupValue().id; }
	/// @returns the data of the matched expression if this pattern is part of a match group.
	u256 d() const { return matchGroupValue().item->data(); }

	std::string toString() const;

private:
	bool matchesBaseItem(AssemblyItem const& _item) const;
	Expression const& matchGroupValue() const;

	AssemblyItemType m_type;
	bool m_requireDataMatch = false;
	u256 m_data = 0;
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
	explicit ExpressionTemplate(Pattern const& _pattern);
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
