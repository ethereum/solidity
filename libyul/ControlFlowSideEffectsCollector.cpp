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

#include <libyul/ControlFlowSideEffectsCollector.h>

#include <libyul/optimiser/FunctionDefinitionCollector.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Algorithms.h>

#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/algorithm/find_if.hpp>

using namespace std;
using namespace solidity::yul;


ControlFlowBuilder::ControlFlowBuilder(Block const& _ast)
{
	for (auto const& statement: _ast.statements)
		if (auto const* function = get_if<FunctionDefinition>(&statement))
			(*this)(*function);
}

void ControlFlowBuilder::operator()(FunctionCall const& _functionCall)
{
	walkVector(_functionCall.arguments | ranges::views::reverse);
	newConnectedNode();
	m_currentNode->functionCall = _functionCall.functionName.name;
}

void ControlFlowBuilder::operator()(If const& _if)
{
	visit(*_if.condition);
	ControlFlowNode* node = m_currentNode;
	(*this)(_if.body);
	newConnectedNode();
	node->successors.emplace_back(m_currentNode);
}

void ControlFlowBuilder::operator()(Switch const& _switch)
{
	visit(*_switch.expression);
	ControlFlowNode* initialNode = m_currentNode;
	ControlFlowNode* finalNode = newNode();

	if (_switch.cases.back().value)
		initialNode->successors.emplace_back(finalNode);

	for (Case const& case_: _switch.cases)
	{
		m_currentNode = initialNode;
		(*this)(case_.body);
		newConnectedNode();
		m_currentNode->successors.emplace_back(finalNode);
	}
	m_currentNode = finalNode;
}

void ControlFlowBuilder::operator()(FunctionDefinition const& _function)
{
	ScopedSaveAndRestore currentNode(m_currentNode, nullptr);
	yulAssert(!m_leave && !m_break && !m_continue, "Function hoister has not been used.");

	FunctionFlow flow;
	flow.exit = newNode();
	m_currentNode = newNode();
	flow.entry = m_currentNode;
	m_leave = flow.exit;

	(*this)(_function.body);

	m_currentNode->successors.emplace_back(flow.exit);

	m_functionFlows[_function.name] = move(flow);

	m_leave = nullptr;
}

void ControlFlowBuilder::operator()(ForLoop const& _for)
{
	ScopedSaveAndRestore scopedBreakNode(m_break, nullptr);
	ScopedSaveAndRestore scopedContinueNode(m_continue, nullptr);

	(*this)(_for.pre);

	ControlFlowNode* breakNode = newNode();
	m_break = breakNode;
	ControlFlowNode* continueNode = newNode();
	m_continue = continueNode;

	newConnectedNode();
	ControlFlowNode* loopNode = m_currentNode;
	visit(*_for.condition);
	m_currentNode->successors.emplace_back(m_break);
	newConnectedNode();

	(*this)(_for.body);

	m_currentNode->successors.emplace_back(m_continue);
	m_currentNode = continueNode;

	(*this)(_for.post);
	m_currentNode->successors.emplace_back(loopNode);

	m_currentNode = breakNode;
}

void ControlFlowBuilder::operator()(Break const&)
{
	yulAssert(m_break);
	m_currentNode->successors.emplace_back(m_break);
	m_currentNode = newNode();
}

void ControlFlowBuilder::operator()(Continue const&)
{
	yulAssert(m_continue);
	m_currentNode->successors.emplace_back(m_continue);
	m_currentNode = newNode();
}

void ControlFlowBuilder::operator()(Leave const&)
{
	yulAssert(m_leave);
	m_currentNode->successors.emplace_back(m_leave);
	m_currentNode = newNode();
}

void ControlFlowBuilder::newConnectedNode()
{
	ControlFlowNode* node = newNode();
	m_currentNode->successors.emplace_back(node);
	m_currentNode = node;
}

ControlFlowNode* ControlFlowBuilder::newNode()
{
	m_nodes.emplace_back(make_shared<ControlFlowNode>());
	return m_nodes.back().get();
}


ControlFlowSideEffectsCollector::ControlFlowSideEffectsCollector(
	Dialect const& _dialect,
	Block const& _ast
):
	m_dialect(_dialect),
	m_cfgBuilder(_ast)
{
	for (auto&& [name, flow]: m_cfgBuilder.functionFlows())
	{
		yulAssert(!flow.entry->functionCall);
		m_processedNodes[name] = {};
		m_pendingNodes[name].push_front(flow.entry);
		m_functionSideEffects[name] = {false, false, false};
	}

	// Process functions while we have progress. For now, we are only interested
	// in `canContinue`.
	bool progress = true;
	while (progress)
	{
		progress = false;
		for (auto const& functionName: m_pendingNodes | ranges::views::keys)
			if (processFunction(functionName))
				progress = true;
	}

	// No progress anymore: All remaining nodes are calls
	// to functions that always recurse.
	// If we have not set `canContinue` by now, the function's exit
	// is not reachable.

	for (auto&& [functionName, calls]: m_functionCalls)
	{
		ControlFlowSideEffects& sideEffects = m_functionSideEffects[functionName];
		auto _visit = [&, visited = std::set<YulString>{}](YulString _function, auto&& _recurse) mutable {
			if (sideEffects.canTerminate && sideEffects.canRevert)
				return;
			if (!visited.insert(_function).second)
				return;

			ControlFlowSideEffects const* calledSideEffects = nullptr;
			if (BuiltinFunction const* f = _dialect.builtin(_function))
				calledSideEffects = &f->controlFlowSideEffects;
			else
				calledSideEffects = &m_functionSideEffects.at(_function);

			if (calledSideEffects->canTerminate)
				sideEffects.canTerminate = true;
			if (calledSideEffects->canRevert)
				sideEffects.canRevert = true;

			for (YulString callee: util::valueOrDefault(m_functionCalls, _function))
				_recurse(callee, _recurse);
		};
		for (auto const& call: calls)
			_visit(call, _visit);
	}

}

bool ControlFlowSideEffectsCollector::processFunction(YulString _name)
{
	bool progress = false;
	while (ControlFlowNode const* node = nextProcessableNode(_name))
	{
		if (node == m_cfgBuilder.functionFlows().at(_name).exit)
		{
			m_functionSideEffects[_name].canContinue = true;
			return true;
		}
		for (ControlFlowNode const* s: node->successors)
			recordReachabilityAndQueue(_name, s);

		progress = true;
	}
	return progress;
}

ControlFlowNode const* ControlFlowSideEffectsCollector::nextProcessableNode(YulString _functionName)
{
	std::list<ControlFlowNode const*>& nodes = m_pendingNodes[_functionName];
	auto it = ranges::find_if(nodes, [this](ControlFlowNode const* _node) {
		return !_node->functionCall || sideEffects(*_node->functionCall).canContinue;
	});
	if (it == nodes.end())
		return nullptr;

	ControlFlowNode const* node = *it;
	nodes.erase(it);
	return node;
}

ControlFlowSideEffects const& ControlFlowSideEffectsCollector::sideEffects(YulString _functionName) const
{
	if (auto const* builtin = m_dialect.builtin(_functionName))
		return builtin->controlFlowSideEffects;
	else
		return m_functionSideEffects.at(_functionName);
}

void ControlFlowSideEffectsCollector::recordReachabilityAndQueue(
	YulString _functionName,
	ControlFlowNode const* _node
)
{
	if (_node->functionCall)
		m_functionCalls[_functionName].insert(*_node->functionCall);
	if (m_processedNodes[_functionName].insert(_node).second)
		m_pendingNodes.at(_functionName).push_front(_node);
}

