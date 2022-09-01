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
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceLocation.h>

#include <map>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

namespace solidity::frontend
{

/**
 * Occurrence of a variable in a block of control flow.
 * Stores the declaration of the referenced variable, the
 * kind of the occurrence and possibly the source location
 * at which it occurred.
 */
class VariableOccurrence
{
public:
	enum class Kind
	{
		Declaration,
		Access,
		Return,
		Assignment,
		InlineAssembly
	};
	VariableOccurrence(VariableDeclaration const& _declaration, Kind _kind, std::optional<langutil::SourceLocation>  _occurrence = {}):
		m_declaration(_declaration), m_occurrenceKind(_kind), m_occurrence(std::move(_occurrence))
	{
	}

	/// Defines a deterministic order on variable occurrences.
	bool operator<(VariableOccurrence const& _rhs) const
	{
		if (m_occurrence && _rhs.m_occurrence)
		{
			if (*m_occurrence < *_rhs.m_occurrence) return true;
			if (*_rhs.m_occurrence < *m_occurrence) return false;
		}
		else if (_rhs.m_occurrence)
			return true;
		else if (m_occurrence)
			return false;

		using KindCompareType = std::underlying_type<VariableOccurrence::Kind>::type;
		return
			std::make_pair(m_declaration.id(), static_cast<KindCompareType>(m_occurrenceKind)) <
			std::make_pair(_rhs.m_declaration.id(), static_cast<KindCompareType>(_rhs.m_occurrenceKind));
	}

	VariableDeclaration const& declaration() const { return m_declaration; }
	Kind kind() const { return m_occurrenceKind; }
	std::optional<langutil::SourceLocation> const& occurrence() const { return m_occurrence; }
private:
	/// Declaration of the occurring variable.
	VariableDeclaration const& m_declaration;
	/// Kind of occurrence.
	Kind m_occurrenceKind = Kind::Access;
	/// Source location at which the variable occurred, if available (may be nullptr).
	std::optional<langutil::SourceLocation> m_occurrence;
};

/**
 * Node of the Control Flow Graph.
 * The control flow is a directed graph connecting control flow blocks.
 * An arc between two nodes indicates that the control flow can possibly
 * move from its start node to its end node during execution.
 */
struct CFGNode
{
	/// Entry nodes. All CFG nodes from which control flow may move into this node.
	std::vector<CFGNode*> entries;
	/// Exit nodes. All CFG nodes to which control flow may continue after this node.
	std::vector<CFGNode*> exits;
	/// Function call done by this node
	FunctionDefinition const* functionDefinition;
	/// Variable occurrences in the node.
	std::vector<VariableOccurrence> variableOccurrences;
	// Source location of this control flow block.
	langutil::SourceLocation location;
};

/** Describes the control flow of a function. */
struct FunctionFlow
{
	virtual ~FunctionFlow() = default;

	/// Entry node. Control flow of the function starts here.
	/// This node is empty and does not have any entries.
	CFGNode* entry = nullptr;
	/// Exit node. All non-reverting control flow of the function ends here.
	/// This node is empty and does not have any exits, but may have multiple entries
	/// (e.g. all return statements of the function).
	CFGNode* exit = nullptr;
	/// Revert node. Control flow of the function in case of revert.
	/// This node is empty and does not have any exits, but may have multiple entries
	/// (e.g. all assert, require, revert and throw statements).
	CFGNode* revert = nullptr;
	/// Transaction return node. Destination node for inline assembly "return" calls.
	/// This node is empty and does not have any exits, but may have multiple entries
	/// (e.g. all inline assembly return calls).
	CFGNode* transactionReturn = nullptr;
};

class CFG: private ASTConstVisitor
{
public:
	struct FunctionContractTuple
	{
		ContractDefinition const* contract = nullptr;
		FunctionDefinition const* function = nullptr;

		// Use AST ids for comparison to keep a deterministic order in the
		// containers using this struct
		bool operator<(FunctionContractTuple const& _other) const
		{
			return
				std::make_pair(contract ? contract->id() : -1, function->id()) <
				std::make_pair(_other.contract ? _other.contract->id() : -1, _other.function->id());
		}
	};
	explicit CFG(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool constructFlow(ASTNode const& _astRoot);

	bool visit(FunctionDefinition const& _function) override;
	bool visit(ContractDefinition const& _contract) override;

	/// Get the function flow for the given function, using `_contract` as the
	/// most derived contract
	/// @param _function function to find the function flow for
	/// @param _contract most derived contract or nullptr for free functions
	FunctionFlow const& functionFlow(FunctionDefinition const& _function, ContractDefinition const* _contract = nullptr) const;

	std::map<FunctionContractTuple, std::unique_ptr<FunctionFlow>> const& allFunctionFlows() const
	{
		return m_functionControlFlow;
	}

	class NodeContainer
	{
	public:
		CFGNode* newNode();
	private:
		std::vector<std::unique_ptr<CFGNode>> m_nodes;
	};
private:
	langutil::ErrorReporter& m_errorReporter;

	/// Node container.
	/// All nodes allocated during the construction of the control flow graph
	/// are owned by the CFG class and stored in this container.
	NodeContainer m_nodeContainer;

	std::map<FunctionContractTuple, std::unique_ptr<FunctionFlow>> m_functionControlFlow;
};

}
