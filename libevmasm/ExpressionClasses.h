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

}
}
