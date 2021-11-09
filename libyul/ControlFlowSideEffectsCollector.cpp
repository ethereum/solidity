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

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/FunctionReferenceResolver.h>

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
	m_currentNode = newNode();
	(*this)(_ast);
}

void ControlFlowBuilder::operator()(FunctionCall const& _functionCall)
{
	walkVector(_functionCall.arguments | ranges::views::reverse);
	newConnectedNode();
	m_currentNode->functionCall = &_functionCall;
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
	ScopedSaveAndRestore leave(m_leave, nullptr);
	ScopedSaveAndRestore _break(m_break, nullptr);
	ScopedSaveAndRestore _continue(m_continue, nullptr);

	FunctionFlow flow;
	flow.exit = newNode();
	m_currentNode = newNode();
	flow.entry = m_currentNode;
	m_leave = flow.exit;

	(*this)(_function.body);

	m_currentNode->successors.emplace_back(flow.exit);

	m_functionFlows[&_function] = move(flow);

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
	m_cfgBuilder(_ast),
	m_functionReferences(FunctionReferenceResolver{_ast}.references())
{
	for (auto&& [function, flow]: m_cfgBuilder.functionFlows())
	{
		yulAssert(!flow.entry->functionCall);
		yulAssert(function);
		m_processedNodes[function] = {};
		m_pendingNodes[function].push_front(flow.entry);
		m_functionSideEffects[function] = {false, false, false};
		m_functionCalls[function] = {};
	}

	// Process functions while we have progress. For now, we are only interested
	// in `canContinue`.
	bool progress = true;
	while (progress)
	{
		progress = false;
		for (FunctionDefinition const* function: m_pendingNodes | ranges::views::keys)
			if (processFunction(*function))
				progress = true;
	}

	// No progress anymore: All remaining nodes are calls
	// to functions that always recurse.
	// If we have not set `canContinue` by now, the function's exit
	// is not reachable.

	// Now it is sufficient to handle the reachable function calls (`m_functionCalls`),
	// we do not have to consider the control-flow graph anymore.
	for (auto&& [function, calls]: m_functionCalls)
	{
		yulAssert(function);
		ControlFlowSideEffects& functionSideEffects = m_functionSideEffects[function];
		auto _visit = [&, visited = std::set<FunctionDefinition const*>{}](FunctionDefinition const& _function, auto&& _recurse) mutable {
			// Worst side-effects already, stop searching.
			if (functionSideEffects.canTerminate && functionSideEffects.canRevert)
				return;
			if (!visited.insert(&_function).second)
				return;

			for (FunctionCall const* call: m_functionCalls.at(&_function))
			{
				ControlFlowSideEffects const& calledSideEffects = sideEffects(*call);
				if (calledSideEffects.canTerminate)
					functionSideEffects.canTerminate = true;
				if (calledSideEffects.canRevert)
					functionSideEffects.canRevert = true;

				if (m_functionReferences.count(call))
					_recurse(*m_functionReferences.at(call), _recurse);
			}
		};
		_visit(*function, _visit);
	}
}

map<YulString, ControlFlowSideEffects> ControlFlowSideEffectsCollector::functionSideEffectsNamed() const
{
	map<YulString, ControlFlowSideEffects> result;
	for (auto&& [function, sideEffects]: m_functionSideEffects)
		yulAssert(result.insert({function->name, sideEffects}).second);
	return result;
}

bool ControlFlowSideEffectsCollector::processFunction(FunctionDefinition const& _function)
{
	bool progress = false;
	while (ControlFlowNode const* node = nextProcessableNode(_function))
	{
		if (node == m_cfgBuilder.functionFlows().at(&_function).exit)
		{
			m_functionSideEffects[&_function].canContinue = true;
			return true;
		}
		for (ControlFlowNode const* s: node->successors)
			recordReachabilityAndQueue(_function, s);

		progress = true;
	}
	return progress;
}

ControlFlowNode const* ControlFlowSideEffectsCollector::nextProcessableNode(FunctionDefinition const& _function)
{
	std::list<ControlFlowNode const*>& nodes = m_pendingNodes[&_function];
	auto it = ranges::find_if(nodes, [this](ControlFlowNode const* _node) {
		return !_node->functionCall || sideEffects(*_node->functionCall).canContinue;
	});
	if (it == nodes.end())
		return nullptr;

	ControlFlowNode const* node = *it;
	nodes.erase(it);
	return node;
}

ControlFlowSideEffects const& ControlFlowSideEffectsCollector::sideEffects(FunctionCall const& _call) const
{
	if (auto const* builtin = m_dialect.builtin(_call.functionName.name))
		return builtin->controlFlowSideEffects;
	else
		return m_functionSideEffects.at(m_functionReferences.at(&_call));
}

void ControlFlowSideEffectsCollector::recordReachabilityAndQueue(
	FunctionDefinition const& _function,
	ControlFlowNode const* _node
)
{
	if (_node->functionCall)
		m_functionCalls[&_function].insert(_node->functionCall);
	if (m_processedNodes[&_function].insert(_node).second)
		m_pendingNodes.at(&_function).push_front(_node);
}

