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

#include <libsolidity/analysis/ControlFlowAnalyzer.h>

#include <liblangutil/SourceLocation.h>
#include <boost/range/algorithm/sort.hpp>

using namespace std;
using namespace langutil;
using namespace dev::solidity;

bool ControlFlowAnalyzer::analyze(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

bool ControlFlowAnalyzer::visit(FunctionDefinition const& _function)
{
	if (_function.isImplemented())
	{
		auto const& functionFlow = m_cfg.functionFlow(_function);
		checkUninitializedAccess(functionFlow.entry, functionFlow.exit);
	}
	return false;
}

void ControlFlowAnalyzer::checkUninitializedAccess(CFGNode const* _entry, CFGNode const* _exit) const
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
						if (variableOccurrence.declaration().type()->dataStoredIn(DataLocation::Storage))
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
			if (nodeInfos[exit].propagateFrom(nodeInfo))
				nodesToTraverse.insert(exit);
	}

	auto const& exitInfo = nodeInfos[_exit];
	if (!exitInfo.uninitializedVariableAccesses.empty())
	{
		vector<VariableOccurrence const*> uninitializedAccessesOrdered(
			exitInfo.uninitializedVariableAccesses.begin(),
			exitInfo.uninitializedVariableAccesses.end()
		 );
		boost::range::sort(
			uninitializedAccessesOrdered,
			[](VariableOccurrence const* lhs, VariableOccurrence const* rhs) -> bool
			{
				return *lhs < *rhs;
			}
		);

		for (auto const* variableOccurrence: uninitializedAccessesOrdered)
		{
			SecondarySourceLocation ssl;
			if (variableOccurrence->occurrence())
				ssl.append("The variable was declared here.", variableOccurrence->declaration().location());

			m_errorReporter.typeError(
				variableOccurrence->occurrence() ?
					variableOccurrence->occurrence()->location() :
					variableOccurrence->declaration().location(),
				ssl,
				string("This variable is of storage pointer type and can be ") +
				(variableOccurrence->kind() == VariableOccurrence::Kind::Return ? "returned" : "accessed") +
				" without prior assignment."
			);
		}
	}
}
