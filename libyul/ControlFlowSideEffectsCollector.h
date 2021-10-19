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
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/ControlFlowSideEffects.h>

#include <set>
#include <stack>
#include <optional>
#include <list>

namespace solidity::yul
{

struct Dialect;

struct ControlFlowNode
{
	std::vector<ControlFlowNode const*> successors;
	/// Name of the called function if the node calls a function.
	std::optional<YulString> functionCall;
};

/**
 * The control flow of a function with entry and exit nodes.
 */
struct FunctionFlow
{
	ControlFlowNode const* entry;
	ControlFlowNode const* exit;
};

/**
 * Requires: Disambiguator, Function Hoister.
 */
class ControlFlowBuilder: private ASTWalker
{
public:
	/// Computes the control-flows of all function defined in the block.
	/// Assumes the functions are hoisted to the topmost block.
	explicit ControlFlowBuilder(Block const& _ast);
	std::map<YulString, FunctionFlow> const& functionFlows() const { return m_functionFlows; }

private:
	using ASTWalker::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;
	void operator()(ForLoop const& _forLoop) override;
	void operator()(Break const& _break) override;
	void operator()(Continue const& _continue) override;
	void operator()(Leave const& _leaveStatement) override;

	void newConnectedNode();
	ControlFlowNode* newNode();

	std::vector<std::shared_ptr<ControlFlowNode>> m_nodes;

	ControlFlowNode* m_currentNode = nullptr;
	ControlFlowNode const* m_leave = nullptr;
	ControlFlowNode const* m_break = nullptr;
	ControlFlowNode const* m_continue = nullptr;

	std::map<YulString, FunctionFlow> m_functionFlows;
};


/**
 * Requires: Disambiguator, Function Hoister.
 */
class ControlFlowSideEffectsCollector
{
public:
	explicit ControlFlowSideEffectsCollector(
		Dialect const& _dialect,
		Block const& _ast
	);

	std::map<YulString, ControlFlowSideEffects> const& functionSideEffects() const
	{
		return m_functionSideEffects;
	}
private:

	/// @returns false if nothing could be processed.
	bool processFunction(YulString _name);

	/// @returns the next pending node of the function that is not
	/// a function call to a function that might not continue.
	/// De-queues the node or returns nullptr if no such node is found.
	ControlFlowNode const* nextProcessableNode(YulString _functionName);

	/// @returns the side-effects of either a builtin call or a user defined function
	/// call (as far as already computed).
	ControlFlowSideEffects const& sideEffects(YulString _functionName) const;

	/// Queues the given node to be processed (if not already visited)
	/// and if it is a function call, records that `_functionName` calls
	/// `*_node->functionCall`.
	void recordReachabilityAndQueue(YulString _functionName, ControlFlowNode const* _node);

	Dialect const& m_dialect;
	ControlFlowBuilder m_cfgBuilder;
	std::map<YulString, ControlFlowSideEffects> m_functionSideEffects;
	std::map<YulString, std::list<ControlFlowNode const*>> m_pendingNodes;
	std::map<YulString, std::set<ControlFlowNode const*>> m_processedNodes;
	/// `x` is in `m_functionCalls[y]` if a direct call to `x` is reachable inside `y`
	std::map<YulString, std::set<YulString>> m_functionCalls;
};


}
