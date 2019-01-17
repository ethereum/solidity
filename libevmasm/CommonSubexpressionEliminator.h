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
 * @file CommonSubexpressionEliminator.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Optimizer step for common subexpression elimination and stack reorganisation.
 */

#pragma once

#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <ostream>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>
#include <libevmasm/ExpressionClasses.h>
#include <libevmasm/SemanticInformation.h>
#include <libevmasm/KnownState.h>

namespace langutil
{
struct SourceLocation;
}

namespace dev
{
namespace eth
{

class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

/**
 * Optimizer step that performs common subexpression elimination and stack reorganisation,
 * i.e. it tries to infer equality among expressions and compute the values of two expressions
 * known to be equal only once.
 *
 * The general workings are that for each assembly item that is fed into the eliminator, an
 * equivalence class is derived from the operation and the equivalence class of its arguments.
 * DUPi, SWAPi and some arithmetic instructions are used to infer equivalences while these
 * classes are determined.
 *
 * When the list of optimized items is requested, they are generated in a bottom-up fashion,
 * adding code for equivalence classes that were not yet computed.
 */
class CommonSubexpressionEliminator
{
public:
	using Id = ExpressionClasses::Id;
	using StoreOperation = KnownState::StoreOperation;

	explicit CommonSubexpressionEliminator(KnownState const& _state): m_initialState(_state), m_state(_state) {}

	/// Feeds AssemblyItems into the eliminator and @returns the iterator pointing at the first
	/// item that must be fed into a new instance of the eliminator.
	/// @param _msizeImportant if false, do not consider modification of MSIZE a side-effect
	template <class _AssemblyItemIterator>
	_AssemblyItemIterator feedItems(_AssemblyItemIterator _iterator, _AssemblyItemIterator _end, bool _msizeImportant);

	/// @returns the resulting items after optimization.
	AssemblyItems getOptimizedItems();

private:
	/// Feeds the item into the system for analysis.
	void feedItem(AssemblyItem const& _item, bool _copyItem = false);

	/// Tries to optimize the item that breaks the basic block at the end.
	void optimizeBreakingItem();

	KnownState m_initialState;
	KnownState m_state;
	/// Keeps information about which storage or memory slots were written to at which sequence
	/// number with what instruction.
	std::vector<StoreOperation> m_storeOperations;

	/// The item that breaks the basic block, can be nullptr.
	/// It is usually appended to the block but can be optimized in some cases.
	AssemblyItem const* m_breakingItem = nullptr;
};

/**
 * Unit that generates code from current stack layout, target stack layout and information about
 * the equivalence classes.
 */
class CSECodeGenerator
{
public:
	using StoreOperation = CommonSubexpressionEliminator::StoreOperation;
	using StoreOperations = std::vector<StoreOperation>;
	using Id = ExpressionClasses::Id;

	/// Initializes the code generator with the given classes and store operations.
	/// The store operations have to be sorted by sequence number in ascending order.
	CSECodeGenerator(ExpressionClasses& _expressionClasses, StoreOperations const& _storeOperations);

	/// @returns the assembly items generated from the given requirements
	/// @param _initialSequenceNumber starting sequence number, do not generate sequenced operations
	/// before this number.
	/// @param _initialStack current contents of the stack (up to stack height of zero)
	/// @param _targetStackContents final contents of the stack, by stack height relative to initial
	/// @note should only be called once on each object.
	AssemblyItems generateCode(
		unsigned _initialSequenceNumber,
		int _initialStackHeight,
		std::map<int, Id> const& _initialStack,
		std::map<int, Id> const& _targetStackContents
	);

private:
	/// Recursively discovers all dependencies to @a m_requests.
	void addDependencies(Id _c);

	/// Produce code that generates the given element if it is not yet present.
	/// @param _allowSequenced indicates that sequence-constrained operations are allowed
	void generateClassElement(Id _c, bool _allowSequenced = false);
	/// @returns the position of the representative of the given id on the stack.
	/// @note throws an exception if it is not on the stack.
	int classElementPosition(Id _id) const;

	/// @returns true if the copy of @a _element can be removed from stack position _fromPosition
	/// - in general or, if given, while computing @a _result.
	bool canBeRemoved(Id _element, Id _result = Id(-1), int _fromPosition = c_invalidPosition);

	/// Appends code to remove the topmost stack element if it can be removed.
	bool removeStackTopIfPossible();

	/// Appends a dup instruction to m_generatedItems to retrieve the element at the given stack position.
	void appendDup(int _fromPosition, langutil::SourceLocation const& _location);
	/// Appends a swap instruction to m_generatedItems to retrieve the element at the given stack position.
	/// @note this might also remove the last item if it exactly the same swap instruction.
	void appendOrRemoveSwap(int _fromPosition, langutil::SourceLocation const& _location);
	/// Appends the given assembly item.
	void appendItem(AssemblyItem const& _item);

	static const int c_invalidPosition = -0x7fffffff;

	AssemblyItems m_generatedItems;
	/// Current height of the stack relative to the start.
	int m_stackHeight = 0;
	/// If (b, a) is in m_requests then b is needed to compute a.
	std::multimap<Id, Id> m_neededBy;
	/// Current content of the stack.
	std::map<int, Id> m_stack;
	/// Current positions of equivalence classes, equal to the empty set if already deleted.
	std::map<Id, std::set<int>> m_classPositions;

	/// The actual eqivalence class items and how to compute them.
	ExpressionClasses& m_expressionClasses;
	/// Keeps information about which storage or memory slots were written to by which operations.
	/// The operations are sorted ascendingly by sequence number.
	std::map<std::pair<StoreOperation::Target, Id>, StoreOperations> m_storeOperations;
	/// The set of equivalence classes that should be present on the stack at the end.
	std::set<Id> m_finalClasses;
	std::map<int, Id> m_targetStack;
};

template <class _AssemblyItemIterator>
_AssemblyItemIterator CommonSubexpressionEliminator::feedItems(
	_AssemblyItemIterator _iterator,
	_AssemblyItemIterator _end,
	bool _msizeImportant
)
{
	assertThrow(!m_breakingItem, OptimizerException, "Invalid use of CommonSubexpressionEliminator.");
	for (; _iterator != _end && !SemanticInformation::breaksCSEAnalysisBlock(*_iterator, _msizeImportant); ++_iterator)
		feedItem(*_iterator);
	if (_iterator != _end)
		m_breakingItem = &(*_iterator++);
	return _iterator;
}

}
}
