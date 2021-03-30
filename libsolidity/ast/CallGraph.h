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

/// Data structure representing a function call graph.

#pragma once

#include <libsolidity/ast/AST.h>

#include <map>
#include <set>
#include <variant>

namespace solidity::frontend
{

/**
 * Function call graph for a contract at the granularity of Solidity functions and modifiers.
 * The graph can represent the situation either at contract creation or after deployment.
 * The graph does not preserve temporal relations between calls - edges coming out of the same node
 * show which calls were performed but not in what order.
 *
 * Stores also extra information about contracts that can be created and events that can be emitted
 * from any of the functions in it.
 */
struct CallGraph
{
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

	/// Graph edges. Edges are directed and lead from the caller to the callee.
	/// The map contains a key for every possible caller, even if does not actually perform
	/// any calls.
	std::map<Node, std::set<Node, CompareByID>, CompareByID> edges;

	/// Contracts that need to be compiled before this one can be compiled.
	/// The value is the ast node that created the dependency.
	std::map<ContractDefinition const*, ASTNode const*, ASTCompareByID<ContractDefinition>> bytecodeDependency;

	/// Events that may get emitted by functions present in the graph.
	std::set<EventDefinition const*, ASTNode::CompareByID> emittedEvents;

	/// Errors that are used by functions present in the graph.
	std::set<ErrorDefinition const*, ASTNode::CompareByID> usedErrors;
};

}
