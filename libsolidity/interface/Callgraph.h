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

#pragma once

#include <libdevcore/CommonData.h>

#include <set>

namespace dev
{
namespace solidity
{

class Declaration;
class VariableDeclaration;
class ASTNode;

struct CallgraphNode
{
	/// Variable declarations this node writes to, unknown if it contains a nullptr.
	/// Note that due to aliasing, this might be incorrect for reference types.
	std::set<VariableDeclaration const*> writes;
	/// Variable declaration this node reads from, unknown if it contains a nullptr.
	/// Note that due to aliasing, this might be incorrect for reference types.
	std::set<VariableDeclaration const*> reads;
	/// Roughly, a node has side effects (at statement level) if it cannot be removed
	/// from the code without changing semantics. In particular, if a node
	/// performs any modifications to any state, it has side effects.
	/// Assignments to local variables count as side-effects,
	/// usage of temporary memory does not count as a side-effect (unless this memory
	/// is used elsewhere).
	/// If any modification to the state is captured only by the value of the
	/// expression, it does not have side effects.
	bool hasSideEffects = false;
	/// Reads from storage (of any account).
	bool readsStorage = false;
	/// Writes to storage (of any account).
	bool writesStorage = false;
	bool writesLogs = false;
	/// Reads anything from the environment inculding:
	///  - balances
	///  - coinbase (block.coinbase)
	///  - timestamp (block.timestamp / now)
	///  - difficulty (block.difficulty)
	///  - block number (block.number)
	///  - gas limit (block.gaslimit)
	///  - origin (tx.origin)
	///  - gas (tx.gas)
	///  - gas price (tx.gasprice)
	///  - sender (msg.sender)
	///
	/// but excluding:
	///  - input (msg.data)
	///  - function signature (msg.sig)
	///  - value transferred (msg.value)
	bool readsEnvironment = false;
	/// Whether it might performs an external call or create.
	bool calls = false;
	/// Whether it might send Ether somewhere.
	bool sendsValue = false;
	bool selfdestructs = false;

	bool canBeUsedInPureFunction() const
	{
		return canBeUsedInViewFunction() && !readsStorage && !readsEnvironment;
	}

	bool canBeUsedInViewFunction() const
	{
		return !writesStorage && !writesLogs && !sendsValue && !selfdestructs;
	}

	/// TODO this is not assigned yet
	/// The outgoing arches of the call graph node.
	/// Can contain a nullptr, which means "unknown".
	std::set<ASTNode const*> nextNodes;

	/// TODO this is not assigned yet
	/// Statements that are visited during execution but are not part of the
	/// syntactic area of the current node.
	/// Can contain a nullptr, which means "unknown".
	std::set<ASTNode const*> calledToNodes;

	CallgraphNode& operator+=(CallgraphNode const& _other)
	{
		writes += _other.writes;
		reads += _other.reads;
		if (_other.hasSideEffects)
			hasSideEffects = true;
		if (_other.readsStorage)
			readsStorage = true;
		if (_other.writesStorage)
			writesStorage = true;
		if (_other.writesLogs)
			writesLogs = true;
		if (_other.readsEnvironment)
			readsEnvironment = true;
		if (_other.calls)
			calls = true;
		if (_other.sendsValue)
			sendsValue = true;
		if (_other.selfdestructs)
			selfdestructs = true;
		calledToNodes += _other.calledToNodes;
		return *this;
	}

	/// @returns a call graph node which assumes anything could happen.
	static CallgraphNode anything()
	{
		CallgraphNode node;
		node.reads.insert(nullptr);
		node.writes.insert(nullptr);
		node.hasSideEffects = true;
		node.readsStorage = true;
		node.writesStorage = true;
		node.writesLogs = true;
		node.readsEnvironment = true;
		node.calls = true;
		node.sendsValue = true;
		node.selfdestructs = true;
		return node;
	}
};

}
}
