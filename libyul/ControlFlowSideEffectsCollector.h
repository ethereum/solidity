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
	/// Function call AST node, if present.
	FunctionCall const* functionCall = nullptr;
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
	std::map<FunctionDefinition const*, FunctionFlow> const& functionFlows() const { return m_functionFlows; }

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

	std::map<FunctionDefinition const*, FunctionFlow> m_functionFlows;
};


/**
 * Computes control-flow side-effects for user-defined functions.
 * Source does not have to be disambiguated, unless you want the side-effects
 * based on function names.
 */
class ControlFlowSideEffectsCollector
{
public:
	explicit ControlFlowSideEffectsCollector(
		Dialect const& _dialect,
		Block const& _ast
	);

	std::map<FunctionDefinition const*, ControlFlowSideEffects> const& functionSideEffects() const
	{
		return m_functionSideEffects;
	}
	/// Returns the side effects by function name, requires unique function names.
	std::map<YulString, ControlFlowSideEffects> functionSideEffectsNamed() const;
private:

	/// @returns false if nothing could be processed.
	bool processFunction(FunctionDefinition const& _function);

	/// @returns the next pending node of the function that is not
	/// a function call to a function that might not continue.
	/// De-queues the node or returns nullptr if no such node is found.
	ControlFlowNode const* nextProcessableNode(FunctionDefinition const& _function);

	/// @returns the side-effects of either a builtin call or a user defined function
	/// call (as far as already computed).
	ControlFlowSideEffects const& sideEffects(FunctionCall const& _call) const;

	/// Queues the given node to be processed (if not already visited)
	/// and if it is a function call, records that `_functionName` calls
	/// `*_node->functionCall`.
	void recordReachabilityAndQueue(FunctionDefinition const& _function, ControlFlowNode const* _node);

	Dialect const& m_dialect;
	ControlFlowBuilder m_cfgBuilder;
	/// Function references, but only for calls to user-defined functions.
	std::map<FunctionCall const*, FunctionDefinition const*> m_functionReferences;
	/// Side effects of user-defined functions, is being constructod.
	std::map<FunctionDefinition const*, ControlFlowSideEffects> m_functionSideEffects;
	/// Control flow nodes still to process, per function.
	std::map<FunctionDefinition const*, std::list<ControlFlowNode const*>> m_pendingNodes;
	/// Control flow nodes already processed, per function.
	std::map<FunctionDefinition const*, std::set<ControlFlowNode const*>> m_processedNodes;
	/// Set of reachable function calls nodes in each function (including calls to builtins).
	std::map<FunctionDefinition const*, std::set<FunctionCall const*>> m_functionCalls;
};


}
