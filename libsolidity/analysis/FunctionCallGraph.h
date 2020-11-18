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

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <deque>
#include <variant>
#include <vector>

namespace solidity::frontend
{

/**
 * Creates a Function call graph for a contract at the granularity of Solidity
 * functions and modifiers. The graph can represent the situation either at contract creation
 * or after deployment. The graph does not preserve temporal relations between calls - edges
 * coming out of the same node show which calls were performed but not in what order.
 *
 * Includes the following special nodes:
 *  - Entry: represents a call from the outside of the contract.
 *    After deployment this is the node that connects to all the functions exposed through the
 *    external interface. At contract creation it connects to the constructors and variable
 *    initializers, which are not explicitly called from within another function.
 *  - InternalDispatch: Represents the internal dispatch function, which calls internal functions
 *    determined at runtime by values of variables and expressions. Functions that are not called
 *    right away get an edge from this node.
 *
 *  Nodes are a variant of either the enum SpecialNode or a CallableDeclaration which currently
 *  can be a function or a modifier. There are no nodes representing event calls. Instead all
 *  emitted events and created contracts are gathered in separate sets included in the graph just
 *  for that purpose.
 *
 *  Auto-generated getter functions for public state variables are ignored, but function calls
 *  inside initial assignments are included in the creation graph.
 *
 *  Only calls reachable from an Entry node are included in the graph. The map representing edges
 *  is also guaranteed to contain keys representing all the reachable functions and modifiers, even
 *  if they have no outgoing edges.
 */
class FunctionCallGraphBuilder: private ASTConstVisitor
{
public:
	enum class SpecialNode
	{
		InternalDispatch,
		Entry,
	};

	using Node = std::variant<CallableDeclaration const*, SpecialNode>;

	struct CompareByID
	{
		using is_transparent = void;
		bool operator()(Node const& _lhs, Node const& _rhs) const;
		bool operator()(Node const& _lhs, int64_t _rhs) const;
		bool operator()(int64_t _lhs, Node const& _rhs) const;
	};

	struct ContractCallGraph
	{
		/// Contract for which this is the graph
		ContractDefinition const& contract;

		/// Graph edges. Edges are directed and lead from the caller to the callee.
		/// The map contains a key for every possible caller, even if does not actually perform
		/// any calls.
		std::map<Node, std::set<Node, CompareByID>, CompareByID> edges;

		/// Contracts that may get created with `new` by functions present in the graph.
		std::set<ContractDefinition const*, ASTNode::CompareByID> createdContracts;

		/// Events that may get emitted by functions present in the graph.
		std::set<EventDefinition const*, ASTNode::CompareByID> emittedEvents;
	};

	static ContractCallGraph buildCreationGraph(ContractDefinition const& _contract);
	static ContractCallGraph buildDeployedGraph(
		ContractDefinition const& _contract,
		FunctionCallGraphBuilder::ContractCallGraph const& _creationGraph
	);

private:
	FunctionCallGraphBuilder(ContractDefinition const& _contract):
		m_graph{_contract, {}, {}, {}} {}

	bool visit(FunctionCall const& _functionCall) override;
	bool visit(EmitStatement const& _emitStatement) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(MemberAccess const& _memberAccess) override;
	bool visit(ModifierInvocation const& _modifierInvocation) override;
	bool visit(NewExpression const& _newExpression) override;

	void enqueueCallable(CallableDeclaration const& _callable);
	void processQueue();

	void add(Node _caller, Node _callee);
	void functionReferenced(CallableDeclaration const& _callable, bool _calledDirectly = true);

	Node m_currentNode = SpecialNode::Entry;
	ContractCallGraph m_graph;
	std::deque<CallableDeclaration const*> m_visitQueue;
};

}
