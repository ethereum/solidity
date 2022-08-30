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

#include <libsolidity/analysis/ControlFlowRevertPruner.h>

#include <libsolutil/Algorithms.h>

#include <range/v3/algorithm/remove.hpp>


namespace solidity::frontend
{

namespace
{

/// Find the right scope for the called function: When calling a base function,
/// we keep the most derived, but we use the called contract in case it is a
/// library function or nullptr for a free function.
ContractDefinition const* findScopeContract(FunctionDefinition const& _function, ContractDefinition const* _callingContract)
{
	if (auto const* functionContract = _function.annotation().contract)
	{
		if (_callingContract && _callingContract->derivesFrom(*functionContract))
			return _callingContract;
		else
			return functionContract;
	}

	return nullptr;
}
}

void ControlFlowRevertPruner::run()
{
	for (auto& [pair, flow]: m_cfg.allFunctionFlows())
		m_functions[pair] = RevertState::Unknown;

	findRevertStates();
	modifyFunctionFlows();
}

void ControlFlowRevertPruner::findRevertStates()
{
	std::set<CFG::FunctionContractTuple> pendingFunctions = util::keys(m_functions);
	// We interrupt the search whenever we encounter a call to a function with (yet) unknown
	// revert behaviour. The ``wakeUp`` data structure contains information about which
	// searches to restart once we know about the behaviour.
	std::map<CFG::FunctionContractTuple, std::set<CFG::FunctionContractTuple>> wakeUp;

	while (!pendingFunctions.empty())
	{
		CFG::FunctionContractTuple item = *pendingFunctions.begin();
		pendingFunctions.erase(pendingFunctions.begin());

		if (m_functions[item] != RevertState::Unknown)
			continue;

		bool foundExit = false;
		bool foundUnknown = false;

		FunctionFlow const& functionFlow = m_cfg.functionFlow(*item.function, item.contract);

		solidity::util::BreadthFirstSearch<CFGNode*>{{functionFlow.entry}}.run(
			[&](CFGNode* _node, auto&& _addChild) {
				if (_node == functionFlow.exit)
					foundExit = true;

				auto const* resolvedFunction = _node->functionDefinition;;
				if (resolvedFunction && resolvedFunction->isImplemented())
				{
					CFG::FunctionContractTuple calledFunctionTuple{
						findScopeContract(*resolvedFunction, item.contract),
						resolvedFunction
					};
					switch (m_functions.at(calledFunctionTuple))
					{
						case RevertState::Unknown:
							wakeUp[calledFunctionTuple].insert(item);
							foundUnknown = true;
							return;
						case RevertState::AllPathsRevert:
							return;
						case RevertState::HasNonRevertingPath:
							break;
					}
				}

				for (CFGNode* exit: _node->exits)
					_addChild(exit);
			}
		);

		auto& revertState = m_functions[item];

		if (foundExit)
			revertState = RevertState::HasNonRevertingPath;
		else if (!foundUnknown)
			revertState = RevertState::AllPathsRevert;

		if (revertState != RevertState::Unknown && wakeUp.count(item))
		{
			// Restart all searches blocked by this function.
			for (CFG::FunctionContractTuple const& nextItem: wakeUp[item])
				if (m_functions.at(nextItem) == RevertState::Unknown)
					pendingFunctions.insert(nextItem);
			wakeUp.erase(item);
		}
	}
}

void ControlFlowRevertPruner::modifyFunctionFlows()
{
	for (auto& item: m_functions)
	{
		FunctionFlow const& functionFlow = m_cfg.functionFlow(*item.first.function, item.first.contract);
		solidity::util::BreadthFirstSearch<CFGNode*>{{functionFlow.entry}}.run(
			[&](CFGNode* _node, auto&& _addChild) {
				auto const* resolvedFunction = _node->functionDefinition;
				if (resolvedFunction && resolvedFunction->isImplemented())
					switch (m_functions.at({findScopeContract(*resolvedFunction, item.first.contract), resolvedFunction}))
					{
						case RevertState::Unknown:
							[[fallthrough]];
						case RevertState::AllPathsRevert:
							// If the revert states of the functions do not
							// change anymore, we treat all "unknown" states as
							// "reverting", since they can only be caused by
							// recursion.
							for (CFGNode * node: _node->exits)
								ranges::remove(node->entries, _node);

							_node->exits = {functionFlow.revert};
							functionFlow.revert->entries.push_back(_node);
							return;
						default:
							break;
					}

				for (CFGNode* exit: _node->exits)
					_addChild(exit);
		});
	}
}

}
