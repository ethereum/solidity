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

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <ostream>
#include <vector>
#include <variant>

namespace solidity::frontend
{

/**
 * Creates a Function call graph for a contract at the granularity of Solidity
 * functions and modifiers
 *
 * Includes the following special nodes:
 *  - EntryCreation: All calls made at contract creation originate from this node.
 *    Constructors are modelled to be all called by this node instead of calling each other
 *    due to implicit constructors that don't exist at this stage.
 *  - InternalCreationDispatch: Represents the internal dispatch function at creation time
 *  - InternalDispatch: Represents the runtime dispatch for internal function pointers
 *    and complex expressions
 *  - Entry: Represents the runtime dispatch for all external functions
 *
 *  Nodes are a variant of either the enum SpecialNode or an ASTNode pointer.
 *  ASTNodes are usually inherited from CallableDeclarations
 *  (FunctionDefinition, ModifierDefinition, EventDefinition) but for functions
 *  without declaration it is directly the FunctionCall AST node.
 *
 *  Functions that are not called right away as well as functions without
 *  declarations have an edge to the internal dispatch node.
 *
 *  Auto-generated getter functions for public state variables are ignored.
 */
class FunctionCallGraphBuilder: private ASTConstVisitor
{
public:
	enum class SpecialNode { EntryCreation, InternalCreationDispatch, InternalDispatch, Entry };

	using Node = std::variant<ASTNode const*, SpecialNode>;

	struct CompareByID
	{
		using is_transparent = void;
		bool operator()(Node const& _lhs, Node const& _rhs) const;
		bool operator()(Node const& _lhs, int64_t _rhs) const;
		bool operator()(int64_t _lhs, Node const& _rhs) const;
	};

	struct ContractCallGraph
	{
		ContractCallGraph(ContractDefinition const& _contract): contract(_contract) {}

		/// Contract for which this is the graph
		ContractDefinition const& contract;

		std::map<Node, std::set<Node, CompareByID>, CompareByID> edges;

		/// Set of contracts created
		std::set<ContractDefinition const*, ASTNode::CompareByID> createdContracts;
	};

	static std::unique_ptr<ContractCallGraph> create(ContractDefinition const& _contract);

private:
	FunctionCallGraphBuilder(ContractDefinition const& _contract);

	bool visit(FunctionCall const& _functionCall) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(NewExpression const& _newExpression) override;
	void endVisit(MemberAccess const& _memberAccess) override;
	void endVisit(ModifierInvocation const& _modifierInvocation) override;

	void visitCallable(CallableDeclaration const* _callable);

	bool add(Node _caller, Node _callee);
	void processFunction(CallableDeclaration const& _callable, bool _calledDirectly = true);

	ContractDefinition const* m_contract = nullptr;
	std::optional<Node> m_currentNode = SpecialNode::EntryCreation;
	std::unique_ptr<ContractCallGraph> m_graph = nullptr;
	Node m_currentDispatch = SpecialNode::InternalCreationDispatch;
};

std::ostream& operator<<(std::ostream& _out, FunctionCallGraphBuilder::Node const& _node);

}
