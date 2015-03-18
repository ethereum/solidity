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
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace eth
{

class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

using EquivalenceClassId = unsigned;
using EquivalenceClassIds = std::vector<EquivalenceClassId>;

/**
 * Optimizer step that performs common subexpression elimination and stack reorginasation,
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

private:
	/// @returns true if the given items starts a new basic block
	bool breaksBasicBlock(AssemblyItem const& _item);
	/// Feeds the item into the system for analysis.
	void feedItem(AssemblyItem const& _item);

	/// Assigns a new equivalence class to the next sequence number of the given stack element.
	void setStackElement(int _stackHeight, EquivalenceClassId _class);
	/// Swaps the given stack elements in their next sequence number.
	void swapStackElements(int _stackHeightA, int _stackHeightB);
	/// Retrieves the current equivalence class fo the given stack element (or generates a new
	/// one if it does not exist yet).
	EquivalenceClassId getStackElement(int _stackHeight);
	/// Retrieves the equivalence class resulting from the given item applied to the given classes,
	/// might also create a new one.
	EquivalenceClassId getClass(AssemblyItem const& _item, EquivalenceClassIds const& _arguments = {});

	/// @returns the next sequence number of the given stack element.
	unsigned getNextStackElementSequence(int _stackHeight);

	/// Current stack height, can be negative.
	int m_stackHeight = 0;
	/// Mapping (stack height, sequence number) -> equivalence class
	std::map<std::pair<int, unsigned>, EquivalenceClassId> m_stackElements;
	/// Vector of equivalence class representatives - we only store one item of an equivalence
	/// class and the index is used as identifier.
	std::vector<std::pair<AssemblyItem const*, EquivalenceClassIds>> m_equivalenceClasses;
	/// List of items generated during analysis.
	std::vector<std::shared_ptr<AssemblyItem>> m_spareAssemblyItem;

};

class CSECodeGenerator
{
public:
	/// @returns the assembly items generated from the given requirements
	/// @param _currentStack current contents of the stack (up to stack height of zero)
	/// @param _targetStackContents final contents of the stack, by stack height relative to initial
	/// @param _equivalenceClasses equivalence classes as expressions of how to compute them
	AssemblyItems generateCode(
		std::map<int, EquivalenceClassId> const& _currentStack,
		std::map<int, EquivalenceClassId> const& _targetStackContents,
		std::vector<std::pair<AssemblyItem const*, EquivalenceClassIds>> const& _equivalenceClasses
	);

private:
	/// Recursively discovers all dependencies to @a m_requests.
	void addDependencies(EquivalenceClassId _c);

	/// Produce code that generates the given element if it is not yet present.
	/// @returns the stack position of the element.
	int generateClassElement(EquivalenceClassId _c);

	/// @returns true if @a _element can be removed while computing @a _result.
	bool canBeRemoved(EquivalenceClassId _element, EquivalenceClassId _result);

	/// Appends a dup instruction to m_generatedItems to retrieve the element at the given stack position.
	void appendDup(int _fromPosition);
	/// Appends a swap instruction to m_generatedItems to retrieve the element at the given stack position.
	void appendSwap(int _fromPosition);
	/// Appends the given assembly item.
	void appendItem(AssemblyItem const& _item);

	static const int c_invalidPosition = std::numeric_limits<int>::min();

	AssemblyItems m_generatedItems;
	/// Current height of the stack relative to the start.
	int m_stackHeight = 0;
	/// If (b, a) is in m_requests then b is needed to compute a.
	std::multimap<EquivalenceClassId, EquivalenceClassId> m_neededBy;
	/// Current content of the stack.
	std::map<int, EquivalenceClassId> m_stack;
	/// Current positions of equivalence classes, equal to c_invalidPosition if already deleted.
	std::map<EquivalenceClassId, int> m_classPositions;

	/// The actual eqivalence class items and how to compute them.
	std::vector<std::pair<AssemblyItem const*, EquivalenceClassIds>> m_equivalenceClasses;
	/// The set of equivalence classes that should be present on the stack at the end.
	std::set<EquivalenceClassId> m_finalClasses;
};

template <class _AssemblyItemIterator>
_AssemblyItemIterator CommonSubexpressionEliminator::feedItems(
	_AssemblyItemIterator _iterator,
	_AssemblyItemIterator _end
)
{
	std::cout << "---------------Feeding items to the CSE engine:--------------" << std::endl;
	for (; _iterator != _end && !breaksBasicBlock(*_iterator); ++_iterator)
		feedItem(*_iterator);
	return _iterator;
}

}
}
