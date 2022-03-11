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

#include <libsolidity/analysis/ControlFlowAnalyzer.h>

#include <liblangutil/SourceLocation.h>
#include <libsolutil/Algorithms.h>

#include <range/v3/algorithm/sort.hpp>

#include <functional>

using namespace std;
using namespace std::placeholders;
using namespace solidity::langutil;
using namespace solidity::frontend;


bool ControlFlowAnalyzer::run()
{
	for (auto& [pair, flow]: m_cfg.allFunctionFlows())
		analyze(*pair.function, pair.contract, *flow);

	return !Error::containsErrors(m_errorReporter.errors());
}

void ControlFlowAnalyzer::analyze(FunctionDefinition const& _function, ContractDefinition const* _contract, FunctionFlow const& _flow)
{
	if (!_function.isImplemented())
		return;

	optional<string> mostDerivedContractName;

	// The name of the most derived contract only required if it differs from
	// the functions contract
	if (_contract && _contract != _function.annotation().contract)
		mostDerivedContractName = _contract->name();

	checkUninitializedAccess(
		_flow.entry,
		_flow.exit,
		_function.body().statements().empty(),
		mostDerivedContractName
	);
	checkUnreachable(_flow.entry, _flow.exit, _flow.revert, _flow.transactionReturn);
}


void ControlFlowAnalyzer::checkUninitializedAccess(CFGNode const* _entry, CFGNode const* _exit, bool _emptyBody, optional<string> _contractName)
{
	struct NodeInfo
	{
		set<VariableDeclaration const*> unassignedVariablesAtEntry;
		set<VariableDeclaration const*> unassignedVariablesAtExit;
		set<VariableOccurrence const*> uninitializedVariableAccesses;
		/// Propagate the information from another node to this node.
		/// To be used to propagate information from a node to its exit nodes.
		/// Returns true, if new variables were added and thus the current node has
		/// to be traversed again.
		bool propagateFrom(NodeInfo const& _entryNode)
		{
			size_t previousUnassignedVariablesAtEntry = unassignedVariablesAtEntry.size();
			size_t previousUninitializedVariableAccessess = uninitializedVariableAccesses.size();
			unassignedVariablesAtEntry += _entryNode.unassignedVariablesAtExit;
			uninitializedVariableAccesses += _entryNode.uninitializedVariableAccesses;
			return
				unassignedVariablesAtEntry.size() > previousUnassignedVariablesAtEntry ||
				uninitializedVariableAccesses.size() > previousUninitializedVariableAccessess
			;
		}
	};
	map<CFGNode const*, NodeInfo> nodeInfos;
	set<CFGNode const*> nodesToTraverse;
	nodesToTraverse.insert(_entry);

	// Walk all paths starting from the nodes in ``nodesToTraverse`` until ``NodeInfo::propagateFrom``
	// returns false for all exits, i.e. until all paths have been walked with maximal sets of unassigned
	// variables and accesses.
	while (!nodesToTraverse.empty())
	{
		CFGNode const* currentNode = *nodesToTraverse.begin();
		nodesToTraverse.erase(nodesToTraverse.begin());

		auto& nodeInfo = nodeInfos[currentNode];
		auto unassignedVariables = nodeInfo.unassignedVariablesAtEntry;
		for (auto const& variableOccurrence: currentNode->variableOccurrences)
		{
			switch (variableOccurrence.kind())
			{
				case VariableOccurrence::Kind::Assignment:
					unassignedVariables.erase(&variableOccurrence.declaration());
					break;
				case VariableOccurrence::Kind::InlineAssembly:
					// We consider all variables referenced in inline assembly as accessed.
					// So far any reference is enough, but we might want to actually analyze
					// the control flow in the assembly at some point.
				case VariableOccurrence::Kind::Access:
				case VariableOccurrence::Kind::Return:
					if (unassignedVariables.count(&variableOccurrence.declaration()))
					{
						// Merely store the unassigned access. We do not generate an error right away, since this
						// path might still always revert. It is only an error if this is propagated to the exit
						// node of the function (i.e. there is a path with an uninitialized access).
						nodeInfo.uninitializedVariableAccesses.insert(&variableOccurrence);
					}
					break;
				case VariableOccurrence::Kind::Declaration:
					unassignedVariables.insert(&variableOccurrence.declaration());
					break;
			}
		}
		nodeInfo.unassignedVariablesAtExit = std::move(unassignedVariables);

		// Propagate changes to all exits and queue them for traversal, if needed.
		for (auto const& exit: currentNode->exits)
			if (
				auto exists = util::valueOrNullptr(nodeInfos, exit);
				nodeInfos[exit].propagateFrom(nodeInfo) || !exists
			)
				nodesToTraverse.insert(exit);
	}

	auto const& exitInfo = nodeInfos[_exit];
	if (!exitInfo.uninitializedVariableAccesses.empty())
	{
		vector<VariableOccurrence const*> uninitializedAccessesOrdered(
			exitInfo.uninitializedVariableAccesses.begin(),
			exitInfo.uninitializedVariableAccesses.end()
		);
		ranges::sort(
			uninitializedAccessesOrdered,
			[](VariableOccurrence const* lhs, VariableOccurrence const* rhs) -> bool
			{
				return *lhs < *rhs;
			}
		);

		for (auto const* variableOccurrence: uninitializedAccessesOrdered)
		{
			VariableDeclaration const& varDecl = variableOccurrence->declaration();

			SecondarySourceLocation ssl;
			if (variableOccurrence->occurrence())
				ssl.append("The variable was declared here.", varDecl.location());

			bool isStorage = varDecl.type()->dataStoredIn(DataLocation::Storage);
			bool isCalldata = varDecl.type()->dataStoredIn(DataLocation::CallData);
			if (isStorage || isCalldata)
				m_errorReporter.typeError(
					3464_error,
					variableOccurrence->occurrence() ?
						*variableOccurrence->occurrence() :
						varDecl.location(),
					ssl,
					"This variable is of " +
					string(isStorage ? "storage" : "calldata") +
					" pointer type and can be " +
					(variableOccurrence->kind() == VariableOccurrence::Kind::Return ? "returned" : "accessed") +
					" without prior assignment, which would lead to undefined behaviour."
				);
			else if (!_emptyBody && varDecl.name().empty())
			{
				if (!m_unassignedReturnVarsAlreadyWarnedFor.emplace(&varDecl).second)
					continue;

				m_errorReporter.warning(
					6321_error,
					varDecl.location(),
					"Unnamed return variable can remain unassigned" +
					(
						_contractName.has_value() ?
						" when the function is called when \"" + _contractName.value() + "\" is the most derived contract." :
						"."
					) +
					" Add an explicit return with value to all non-reverting code paths or name the variable."
				);
			}
		}
	}
}

void ControlFlowAnalyzer::checkUnreachable(CFGNode const* _entry, CFGNode const* _exit, CFGNode const* _revert, CFGNode const* _transactionReturn)
{
	// collect all nodes reachable from the entry point
	std::set<CFGNode const*> reachable = util::BreadthFirstSearch<CFGNode const*>{{_entry}}.run(
		[](CFGNode const* _node, auto&& _addChild) {
			for (CFGNode const* exit: _node->exits)
				_addChild(exit);
		}
	).visited;

	// traverse all paths backwards from exit, revert and transaction return
	// and extract (valid) source locations of unreachable nodes into sorted set
	std::set<SourceLocation> unreachable;
	util::BreadthFirstSearch<CFGNode const*>{{_exit, _revert, _transactionReturn}}.run(
		[&](CFGNode const* _node, auto&& _addChild) {
			if (!reachable.count(_node) && _node->location.isValid())
				unreachable.insert(_node->location);
			for (CFGNode const* entry: _node->entries)
				_addChild(entry);
		}
	);

	for (auto it = unreachable.begin(); it != unreachable.end();)
	{
		SourceLocation location = *it++;
		// Extend the location, as long as the next location overlaps (unreachable is sorted).
		for (; it != unreachable.end() && it->start <= location.end; ++it)
			location.end = std::max(location.end, it->end);

		if (m_unreachableLocationsAlreadyWarnedFor.emplace(location).second)
			m_errorReporter.warning(5740_error, location, "Unreachable code.");
	}
}
