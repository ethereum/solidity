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


namespace solidity::frontend
{

namespace
{

/// Find the right scope for the called function: When calling a base function, we keep the most derived, but we use the called contract in case it is a library function or nullptr for a free function
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
	// build a lookup table for function calls / callers
	for (auto& [pair, flow]: m_cfg.allFunctionFlows())
		collectCalls(*pair.function, pair.contract);

	findRevertStates();
	modifyFunctionFlows();
}

FunctionDefinition const* ControlFlowRevertPruner::resolveCall(FunctionCall const& _functionCall, ContractDefinition const* _contract)
{
	auto result = m_resolveCache.find({&_functionCall, _contract});
	if (result != m_resolveCache.end())
		return result->second;

	auto const& functionType = dynamic_cast<FunctionType const&>(
		*_functionCall.expression().annotation().type
	);

	if (!functionType.hasDeclaration())
		return nullptr;

	auto const& unresolvedFunctionDefinition =
		dynamic_cast<FunctionDefinition const&>(functionType.declaration());

	FunctionDefinition const* returnFunctionDef = &unresolvedFunctionDefinition;

	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_functionCall.expression()))
	{
		if (*memberAccess->annotation().requiredLookup == VirtualLookup::Super)
		{
			if (auto const typeType = dynamic_cast<TypeType const*>(memberAccess->expression().annotation().type))
				if (auto const contractType = dynamic_cast<ContractType const*>(typeType->actualType()))
				{
					solAssert(contractType->isSuper(), "");
					ContractDefinition const* superContract = contractType->contractDefinition().superContract(*_contract);

					returnFunctionDef = &unresolvedFunctionDefinition.resolveVirtual(
						*_contract,
						superContract
					);
				}
		}
		else
		{
			solAssert(*memberAccess->annotation().requiredLookup == VirtualLookup::Static, "");
			returnFunctionDef = &unresolvedFunctionDefinition;
		}
	}
	else if (auto const* identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
	{
		solAssert(*identifier->annotation().requiredLookup == VirtualLookup::Virtual, "");
		returnFunctionDef = &unresolvedFunctionDefinition.resolveVirtual(*_contract);
	}

	if (returnFunctionDef && !returnFunctionDef->isImplemented())
		returnFunctionDef = nullptr;

	return m_resolveCache[{&_functionCall, _contract}] = returnFunctionDef;
}

void ControlFlowRevertPruner::findRevertStates()
{
	std::set<CFG::FunctionContractTuple> pendingFunctions = keys(m_functions);

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

				for (auto const* functionCall: _node->functionCalls)
				{
					auto const* resolvedFunction = resolveCall(*functionCall, item.contract);

					if (resolvedFunction == nullptr)
						continue;

					switch (m_functions.at({findScopeContract(*resolvedFunction, item.contract), resolvedFunction}))
					{
						case RevertState::Unknown:
							foundUnknown = true;
							return;
						case RevertState::AllPathsRevert:
							return;
						default:
							break;
					}
				}

				for (CFGNode* exit: _node->exits)
					_addChild(exit);
		});

		auto& revertState = m_functions[item];

		if (foundExit)
			revertState = RevertState::HasNonRevertingPath;
		else if (!foundUnknown)
			revertState = RevertState::AllPathsRevert;

		// Mark all functions depending on this one as modified again
		if (revertState != RevertState::Unknown)
			for (auto& nextItem: m_calledBy[item.function])
				// Ignore different most derived contracts in dependent callees
				if (
					item.contract == nullptr ||
					nextItem.contract == nullptr ||
					nextItem.contract == item.contract
				)
					pendingFunctions.insert(nextItem);
	}
}

void ControlFlowRevertPruner::modifyFunctionFlows()
{
	for (auto& item: m_functions)
	{
		FunctionFlow const& functionFlow = m_cfg.functionFlow(*item.first.function, item.first.contract);
		solidity::util::BreadthFirstSearch<CFGNode*>{{functionFlow.entry}}.run(
			[&](CFGNode* _node, auto&& _addChild) {
				for (auto const* functionCall: _node->functionCalls)
				{
					auto const* resolvedFunction = resolveCall(*functionCall, item.first.contract);

					if (resolvedFunction == nullptr)
						continue;

					switch (m_functions.at({findScopeContract(*resolvedFunction, item.first.contract), resolvedFunction}))
					{
						case RevertState::Unknown:
							[[fallthrough]];
						case RevertState::AllPathsRevert:
							// If the revert states of the functions do not
							// change anymore, we treat all "unknown" states as
							// "reverting", since they can only be caused by
							// recursion.
							_node->exits = {functionFlow.revert};
							return;
						default:
							break;
					}
				}

				for (CFGNode* exit: _node->exits)
					_addChild(exit);
		});
	}
}

void ControlFlowRevertPruner::collectCalls(FunctionDefinition const& _function, ContractDefinition const* _mostDerivedContract)
{
	FunctionFlow const& functionFlow = m_cfg.functionFlow(_function, _mostDerivedContract);

	CFG::FunctionContractTuple pair{_mostDerivedContract, &_function};

	solAssert(m_functions.count(pair) == 0, "");
	m_functions[pair] = RevertState::Unknown;

	solidity::util::BreadthFirstSearch<CFGNode*>{{functionFlow.entry}}.run(
		[&](CFGNode* _node, auto&& _addChild) {
			for (auto const* functionCall: _node->functionCalls)
				m_calledBy[resolveCall(*functionCall, _mostDerivedContract)].insert(pair);

			for (CFGNode* exit: _node->exits)
				_addChild(exit);
	});
}

}
