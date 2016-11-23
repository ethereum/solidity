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
 * @file KnownState.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Contains knowledge about the state of the virtual machine at a specific instruction.
 */

#pragma once

#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <memory>
#include <ostream>
#pragma warning(push)
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wredeclared-class-member"
#include <boost/bimap.hpp>
#pragma warning(pop)
#pragma GCC diagnostic pop
#include <libdevcore/CommonIO.h>
#include <libdevcore/Exceptions.h>
#include <libevmasm/ExpressionClasses.h>
#include <libevmasm/SemanticInformation.h>

namespace dev
{
namespace eth
{

class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

/**
 * Class to infer and store knowledge about the state of the virtual machine at a specific
 * instruction.
 *
 * The general workings are that for each assembly item that is fed, an equivalence class is
 * derived from the operation and the equivalence class of its arguments. DUPi, SWAPi and some
 * arithmetic instructions are used to infer equivalences while these classes are determined.
 */
class KnownState
{
public:
	using Id = ExpressionClasses::Id;
	struct StoreOperation
	{
		enum Target { Invalid, Memory, Storage };
		StoreOperation(): target(Invalid), sequenceNumber(-1) {}
		StoreOperation(
			Target _target,
			Id _slot,
			unsigned _sequenceNumber,
			Id _expression
		): target(_target), slot(_slot), sequenceNumber(_sequenceNumber), expression(_expression) {}
		bool isValid() const { return target != Invalid; }
		Target target;
		Id slot;
		unsigned sequenceNumber;
		Id expression;
	};

	explicit KnownState(
		std::shared_ptr<ExpressionClasses> _expressionClasses = std::make_shared<ExpressionClasses>()
	): m_expressionClasses(_expressionClasses)
	{
	}

	/// Streams debugging information to @a _out.
	std::ostream& stream(std::ostream& _out) const;

	/// Feeds the item into the system for analysis.
	/// @returns a possible store operation
	StoreOperation feedItem(AssemblyItem const& _item, bool _copyItem = false);

	/// Resets any knowledge about storage.
	void resetStorage() { m_storageContent.clear(); }
	/// Resets any knowledge about storage.
	void resetMemory() { m_memoryContent.clear(); }
	/// Resets any knowledge about the current stack.
	void resetStack() { m_stackElements.clear(); m_stackHeight = 0; }
	/// Resets any knowledge.
	void reset() { resetStorage(); resetMemory(); resetStack(); }

	unsigned sequenceNumber() const { return m_sequenceNumber; }

	/// Replaces the state by the intersection with _other, i.e. only equal knowledge is retained.
	/// If the stack heighht is different, the smaller one is used and the stack is compared
	/// relatively.
	/// @param _combineSequenceNumbers if true, sets the sequence number to the maximum of both
	void reduceToCommonKnowledge(KnownState const& _other, bool _combineSequenceNumbers);

	/// @returns a shared pointer to a copy of this state.
	std::shared_ptr<KnownState> copy() const { return std::make_shared<KnownState>(*this); }

	/// @returns true if the knowledge about the state of both objects is (known to be) equal.
	bool operator==(KnownState const& _other) const;

	/// Retrieves the current equivalence class fo the given stack element (or generates a new
	/// one if it does not exist yet).
	Id stackElement(int _stackHeight, SourceLocation const& _location);
	/// @returns the stackElement relative to the current stack height.
	Id relativeStackElement(int _stackOffset, SourceLocation const& _location = SourceLocation());

	/// @returns its set of tags if the given expression class is a known tag union; returns a set
	/// containing the tag if it is a PushTag expression and the empty set otherwise.
	std::set<u256> tagsInExpression(Id _expressionId);
	/// During analysis, different tags on the stack are partially treated as the same class.
	/// This removes such classes not to confuse later analyzers.
	void clearTagUnions();

	int stackHeight() const { return m_stackHeight; }
	std::map<int, Id> const& stackElements() const { return m_stackElements; }
	ExpressionClasses& expressionClasses() const { return *m_expressionClasses; }

	std::map<Id, Id> const& storageContent() const { return m_storageContent; }

private:
	/// Assigns a new equivalence class to the next sequence number of the given stack element.
	void setStackElement(int _stackHeight, Id _class);
	/// Swaps the given stack elements in their next sequence number.
	void swapStackElements(int _stackHeightA, int _stackHeightB, SourceLocation const& _location);

	/// Increments the sequence number, deletes all storage information that might be overwritten
	/// and stores the new value at the given slot.
	/// @returns the store operation, which might be invalid if storage was not modified
	StoreOperation storeInStorage(Id _slot, Id _value, SourceLocation const& _location);
	/// Retrieves the current value at the given slot in storage or creates a new special sload class.
	Id loadFromStorage(Id _slot, SourceLocation const& _location);
	/// Increments the sequence number, deletes all memory information that might be overwritten
	/// and stores the new value at the given slot.
	/// @returns the store operation, which might be invalid if memory was not modified
	StoreOperation storeInMemory(Id _slot, Id _value, SourceLocation const& _location);
	/// Retrieves the current value at the given slot in memory or creates a new special mload class.
	Id loadFromMemory(Id _slot, SourceLocation const& _location);
	/// Finds or creates a new expression that applies the sha3 hash function to the contents in memory.
	Id applySha3(Id _start, Id _length, SourceLocation const& _location);

	/// @returns a new or already used Id representing the given set of tags.
	Id tagUnion(std::set<u256> _tags);

	/// Current stack height, can be negative.
	int m_stackHeight = 0;
	/// Current stack layout, mapping stack height -> equivalence class
	std::map<int, Id> m_stackElements;
	/// Current sequence number, this is incremented with each modification to storage or memory.
	unsigned m_sequenceNumber = 1;
	/// Knowledge about storage content.
	std::map<Id, Id> m_storageContent;
	/// Knowledge about memory content. Keys are memory addresses, note that the values overlap
	/// and are not contained here if they are not completely known.
	std::map<Id, Id> m_memoryContent;
	/// Keeps record of all sha3 hashes that are computed.
	std::map<std::vector<Id>, Id> m_knownSha3Hashes;
	/// Structure containing the classes of equivalent expressions.
	std::shared_ptr<ExpressionClasses> m_expressionClasses;
	/// Container for unions of tags stored on the stack.
	boost::bimap<Id, std::set<u256>> m_tagUnions;
};

}
}
