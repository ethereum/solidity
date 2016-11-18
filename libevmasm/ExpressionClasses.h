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
#include <libevmasm/AssemblyItem.h>

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
		AssemblyItem const* item = nullptr;
		Ids arguments;
		/// Storage modification sequence, only used for storage and memory operations.
		unsigned sequenceNumber = 0;
		/// Behaves as if this was a tuple of (item->type(), item->data(), arguments, sequenceNumber).
		bool operator<(Expression const& _other) const;
	};

	/// Retrieves the id of the expression equivalence class resulting from the given item applied to the
	/// given classes, might also create a new one.
	/// @param _copyItem if true, copies the assembly item to an internal storage instead of just
	/// keeping a pointer.
	/// The @a _sequenceNumber indicates the current storage or memory access sequence.
	Id find(
		AssemblyItem const& _item,
		Ids const& _arguments = {},
		bool _copyItem = true,
		unsigned _sequenceNumber = 0
	);
	/// @returns the canonical representative of an expression class.
	Expression const& representative(Id _id) const { return m_representatives.at(_id); }
	/// @returns the number of classes.
	Id size() const { return m_representatives.size(); }

	/// Forces the given @a _item with @a _arguments to the class @a _id. This can be used to
	/// add prior knowledge e.g. about CALLDATA, but has to be used with caution. Will not work as
	/// expected if @a _item applied to @a _arguments already exists.
	void forceEqual(Id _id, AssemblyItem const& _item, Ids const& _arguments, bool _copyItem = true);

	/// @returns the id of a new class which is different to all other classes.
	Id newClass(SourceLocation const& _location);

	/// @returns true if the values of the given classes are known to be different (on every input).
	/// @note that this function might still return false for some different inputs.
	bool knownToBeDifferent(Id _a, Id _b);
	/// Similar to @a knownToBeDifferent but require that abs(_a - b) >= 32.
	bool knownToBeDifferentBy32(Id _a, Id _b);
	/// @returns true if the value of the given class is known to be zero.
	/// @note that this is not the negation of knownNonZero
	bool knownZero(Id _c);
	/// @returns true if the value of the given class is known to be nonzero.
	/// @note that this is not the negation of knownZero
	bool knownNonZero(Id _c);
	/// @returns a pointer to the value if the given class is known to be a constant,
	/// and a nullptr otherwise.
	u256 const* knownConstant(Id _c);

	/// Stores a copy of the given AssemblyItem and returns a pointer to the copy that is valid for
	/// the lifetime of the ExpressionClasses object.
	AssemblyItem const* storeItem(AssemblyItem const& _item);

	std::string fullDAGToString(Id _id) const;

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
	/// All expression ever encountered.
	std::set<Expression> m_expressions;
	std::vector<std::shared_ptr<AssemblyItem>> m_spareAssemblyItems;
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

	AssemblyItem toAssemblyItem(SourceLocation const& _location) const;
	std::vector<Pattern> arguments() const { return m_arguments; }

	/// @returns the id of the matched expression if this pattern is part of a match group.
	Id id() const { return matchGroupValue().id; }
	/// @returns the data of the matched expression if this pattern is part of a match group.
	u256 const& d() const { return matchGroupValue().item->data(); }

	std::string toString() const;

private:
	bool matchesBaseItem(AssemblyItem const* _item) const;
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
