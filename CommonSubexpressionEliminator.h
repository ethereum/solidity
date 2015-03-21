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
 * @file CommonSubexpressionEliminator.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Optimizer step for common subexpression elimination and stack reorganisation.
 */

#pragma once

#include <vector>
#include <map>
#include <ostream>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>
#include <libevmcore/ExpressionClasses.h>

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
 * equivalence class is derived from the operation and the equivalence class of its arguments and
 * it is assigned to the next sequence number of a stack item. DUPi, SWAPi and some arithmetic
 * instructions are used to infer equivalences while these classes are determined.
 *
 * When the list of optimized items is requested, they are generated in a bottom-up fashion,
 * adding code for equivalence classes that were not yet computed.
 */
class CommonSubexpressionEliminator
{
public:
	/// Feeds AssemblyItems into the eliminator and @returns the iterator pointing at the first
	/// item that must be fed into a new instance of the eliminator.
	template <class _AssemblyItemIterator>
	_AssemblyItemIterator feedItems(_AssemblyItemIterator _iterator, _AssemblyItemIterator _end);

	/// @returns the resulting items after optimization.
	AssemblyItems getOptimizedItems();

	/// Streams debugging information to @a _out.
	std::ostream& stream(
		std::ostream& _out,
		std::map<int, ExpressionClasses::Id> _currentStack = std::map<int, ExpressionClasses::Id>(),
		std::map<int, ExpressionClasses::Id> _targetStack = std::map<int, ExpressionClasses::Id>()
	) const;

private:
	/// Feeds the item into the system for analysis.
	void feedItem(AssemblyItem const& _item);

	/// Simplifies the given item using
	/// Assigns a new equivalence class to the next sequence number of the given stack element.
	void setStackElement(int _stackHeight, ExpressionClasses::Id _class);
	/// Swaps the given stack elements in their next sequence number.
	void swapStackElements(int _stackHeightA, int _stackHeightB);
	/// Retrieves the current equivalence class fo the given stack element (or generates a new
	/// one if it does not exist yet).
	ExpressionClasses::Id stackElement(int _stackHeight);
	/// @returns the equivalence class id of the special initial stack element at the given height
	/// (must not be positive).
	ExpressionClasses::Id initialStackElement(int _stackHeight);

	/// Current stack height, can be negative.
	int m_stackHeight = 0;
	/// Current stack layout, mapping stack height -> equivalence class
	std::map<int, ExpressionClasses::Id> m_stackElements;
	/// Structure containing the classes of equivalent expressions.
	ExpressionClasses m_expressionClasses;
};

/**
 * Helper functions to provide context-independent information about assembly items.
 */
struct SemanticInformation
{
	/// @returns true if the given items starts a new basic block
	static bool breaksBasicBlock(AssemblyItem const& _item);
	/// @returns true if the item is a two-argument operation whose value does not depend on the
	/// order of its arguments.
	static bool isCommutativeOperation(AssemblyItem const& _item);
	static bool isDupInstruction(AssemblyItem const& _item);
	static bool isSwapInstruction(AssemblyItem const& _item);
};

/**
 * Unit that generates code from current stack layout, target stack layout and information about
 * the equivalence classes.
 */
class CSECodeGenerator
{
public:
	CSECodeGenerator(ExpressionClasses const& _expressionClasses):
		m_expressionClasses(_expressionClasses)
	{}

	/// @returns the assembly items generated from the given requirements
	/// @param _initialStack current contents of the stack (up to stack height of zero)
	/// @param _targetStackContents final contents of the stack, by stack height relative to initial
	/// @param _equivalenceClasses equivalence classes as expressions of how to compute them
	/// @note should only be called once on each object.
	AssemblyItems generateCode(
		std::map<int, ExpressionClasses::Id> const& _initialStack,
		std::map<int, ExpressionClasses::Id> const& _targetStackContents
	);

private:
	/// Recursively discovers all dependencies to @a m_requests.
	void addDependencies(ExpressionClasses::Id _c);

	/// Produce code that generates the given element if it is not yet present.
	/// @returns the stack position of the element.
	int generateClassElement(ExpressionClasses::Id _c);

	/// @returns true if @a _element can be removed - in general or, if given, while computing @a _result.
	bool canBeRemoved(ExpressionClasses::Id _element, ExpressionClasses::Id _result = ExpressionClasses::Id(-1));

	/// Appends code to remove the topmost stack element if it can be removed.
	bool removeStackTopIfPossible();

	/// Appends a dup instruction to m_generatedItems to retrieve the element at the given stack position.
	void appendDup(int _fromPosition);
	/// Appends a swap instruction to m_generatedItems to retrieve the element at the given stack position.
	/// @note this might also remove the last item if it exactly the same swap instruction.
	void appendSwapOrRemove(int _fromPosition);
	/// Appends the given assembly item.
	void appendItem(AssemblyItem const& _item);

	static const int c_invalidPosition = -0x7fffffff;

	AssemblyItems m_generatedItems;
	/// Current height of the stack relative to the start.
	int m_stackHeight = 0;
	/// If (b, a) is in m_requests then b is needed to compute a.
	std::multimap<ExpressionClasses::Id, ExpressionClasses::Id> m_neededBy;
	/// Current content of the stack.
	std::map<int, ExpressionClasses::Id> m_stack;
	/// Current positions of equivalence classes, equal to c_invalidPosition if already deleted.
	std::map<ExpressionClasses::Id, int> m_classPositions;

	/// The actual eqivalence class items and how to compute them.
	ExpressionClasses const& m_expressionClasses;
	/// The set of equivalence classes that should be present on the stack at the end.
	std::set<ExpressionClasses::Id> m_finalClasses;
};

template <class _AssemblyItemIterator>
_AssemblyItemIterator CommonSubexpressionEliminator::feedItems(
	_AssemblyItemIterator _iterator,
	_AssemblyItemIterator _end
)
{
	for (; _iterator != _end && !SemanticInformation::breaksBasicBlock(*_iterator); ++_iterator)
		feedItem(*_iterator);
	return _iterator;
}

}
}
