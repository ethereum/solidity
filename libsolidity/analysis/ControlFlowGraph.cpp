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

#include <libsolidity/analysis/ControlFlowGraph.h>
#include <libsolidity/analysis/ControlFlowBuilder.h>

#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>

using namespace std;
using namespace dev::solidity;

bool CFG::constructFlow(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	applyModifiers();
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}


bool CFG::visit(ModifierDefinition const& _modifier)
{
	m_modifierControlFlow[&_modifier] = ControlFlowBuilder::createModifierFlow(m_nodeContainer, _modifier);
	return false;
}

bool CFG::visit(FunctionDefinition const& _function)
{
	m_functionControlFlow[&_function] = ControlFlowBuilder::createFunctionFlow(m_nodeContainer, _function);
	return false;
}

FunctionFlow const& CFG::functionFlow(FunctionDefinition const& _function) const
{
	solAssert(m_functionControlFlow.count(&_function), "");
	return *m_functionControlFlow.find(&_function)->second;
}

CFGNode* CFG::NodeContainer::newNode()
{
	m_nodes.emplace_back(new CFGNode());
	return m_nodes.back().get();
}

void CFG::applyModifiers()
{
	for (auto const& function: m_functionControlFlow)
	{
		for (auto const& modifierInvocation: boost::adaptors::reverse(function.first->modifiers()))
		{
			if (auto modifierDefinition = dynamic_cast<ModifierDefinition const*>(
				modifierInvocation->name()->annotation().referencedDeclaration
			))
			{
				solAssert(m_modifierControlFlow.count(modifierDefinition), "");
				applyModifierFlowToFunctionFlow(*m_modifierControlFlow[modifierDefinition], function.second.get());
			}
		}
	}
}

void CFG::applyModifierFlowToFunctionFlow(
	ModifierFlow const& _modifierFlow,
	FunctionFlow* _functionFlow
)
{
	solAssert(!!_functionFlow, "");

	map<CFGNode*, CFGNode*> copySrcToCopyDst;

	// inherit the revert node of the function
	copySrcToCopyDst[_modifierFlow.revert] = _functionFlow->revert;

	// replace the placeholder nodes by the function entry and exit
	copySrcToCopyDst[_modifierFlow.placeholderEntry] = _functionFlow->entry;
	copySrcToCopyDst[_modifierFlow.placeholderExit] = _functionFlow->exit;

	stack<CFGNode*> nodesToCopy;
	nodesToCopy.push(_modifierFlow.entry);

	// map the modifier entry to a new node that will become the new function entry
	copySrcToCopyDst[_modifierFlow.entry] = m_nodeContainer.newNode();

	while (!nodesToCopy.empty())
	{
		CFGNode* copySrcNode = nodesToCopy.top();
		nodesToCopy.pop();

		solAssert(copySrcToCopyDst.count(copySrcNode), "");

		CFGNode* copyDstNode = copySrcToCopyDst[copySrcNode];

		copyDstNode->block = copySrcNode->block;
		for (auto const& entry: copySrcNode->entries)
		{
			if (!copySrcToCopyDst.count(entry))
			{
				copySrcToCopyDst[entry] = m_nodeContainer.newNode();
				nodesToCopy.push(entry);
			}
			copyDstNode->entries.emplace_back(copySrcToCopyDst[entry]);
		}
		for (auto const& exit: copySrcNode->exits)
		{
			if (!copySrcToCopyDst.count(exit))
			{
				copySrcToCopyDst[exit] = m_nodeContainer.newNode();
				nodesToCopy.push(exit);
			}
			copyDstNode->exits.emplace_back(copySrcToCopyDst[exit]);
		}
	}

	// if the modifier control flow never reached its exit node,
	// we need to create a new (disconnected) exit node now
	if (!copySrcToCopyDst.count(_modifierFlow.exit))
		copySrcToCopyDst[_modifierFlow.exit] = m_nodeContainer.newNode();

	_functionFlow->entry = copySrcToCopyDst[_modifierFlow.entry];
	_functionFlow->exit = copySrcToCopyDst[_modifierFlow.exit];
}