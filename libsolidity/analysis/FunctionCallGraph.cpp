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

#include <libsolidity/analysis/FunctionCallGraph.h>

using namespace std;
using namespace solidity::frontend;

bool FunctionCallGraphBuilder::CompareByID::operator()(Node const& _lhs, Node const& _rhs) const
{
	if (_lhs.index() != _rhs.index())
		return _lhs.index() < _rhs.index();

	if (std::holds_alternative<SpecialNode>(_lhs))
		return std::get<SpecialNode>(_lhs) < std::get<SpecialNode>(_rhs);
	return std::get<ASTNode const*>(_lhs)->id() < std::get<ASTNode const*>(_rhs)->id();
}

bool FunctionCallGraphBuilder::CompareByID::operator()(Node const& _lhs, int64_t _rhs) const
{
	solAssert(!std::holds_alternative<SpecialNode>(_lhs), "");

	return std::get<ASTNode const*>(_lhs)->id() < _rhs;
}

bool FunctionCallGraphBuilder::CompareByID::operator()(int64_t _lhs, Node const& _rhs) const
{
	solAssert(!std::holds_alternative<SpecialNode>(_rhs), "");

	return _lhs < std::get<ASTNode const*>(_rhs)->id();
}

shared_ptr<FunctionCallGraphBuilder::ContractCallGraph> FunctionCallGraphBuilder::create(ContractDefinition const& _contract)
{
	m_contract = &_contract;

	m_graph = make_shared<ContractCallGraph>(_contract);

	// Create graph for constructor, state vars, etc
	m_currentNode = SpecialNode::EntryCreation;
	m_currentDispatch = SpecialNode::InternalCreationDispatch;
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
		visitConstructor(*contract);

	m_currentNode.reset();
	m_currentDispatch = SpecialNode::InternalDispatch;

	// Create graph for all publicly reachable functions
	for (auto& [hash, functionType]: _contract.interfaceFunctionList())
	{
		(void)hash;
		if (auto const* funcDef = dynamic_cast<FunctionDefinition const*>(&functionType->declaration()))
			if (!m_graph->edges.count(funcDef))
				visitCallable(funcDef);

		// Add all external functions to the RuntimeDispatch
		add(SpecialNode::Entry, &functionType->declaration());
	}

	// Add all InternalCreationDispatch calls to the RuntimeDispatch as well
	add(SpecialNode::InternalDispatch, SpecialNode::InternalCreationDispatch);

	if (_contract.fallbackFunction())
		add(SpecialNode::Entry, _contract.fallbackFunction());

	if (_contract.receiveFunction())
		add(SpecialNode::Entry, _contract.receiveFunction());

	m_contract = nullptr;
	solAssert(!m_currentNode.has_value(), "Current node not properly reset.");

	return m_graph;
}

bool FunctionCallGraphBuilder::visit(Identifier const& _identifier)
{
	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		solAssert(*_identifier.annotation().requiredLookup == VirtualLookup::Virtual, "");

		auto funType = dynamic_cast<FunctionType const*>(_identifier.annotation().type);

		// For events kind() == Event, so we have an extra check here
		if (funType && funType->kind() == FunctionType::Kind::Internal)
		{
			processFunction(callable->resolveVirtual(*m_contract), _identifier.annotation());

			solAssert(m_currentNode.has_value(), "");
		}
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(NewExpression const& _newExpression)
{
	if (ContractType const* contractType = dynamic_cast<ContractType const*>(_newExpression.typeName().annotation().type))
		m_graph->createdContracts.emplace(&contractType->contractDefinition());

	return true;
}

void FunctionCallGraphBuilder::endVisit(MemberAccess const& _memberAccess)
{
	auto functionType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type);
	auto functionDef = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration);
	if (!functionType || !functionDef || functionType->kind() != FunctionType::Kind::Internal)
		return;

	// Super functions
	if (*_memberAccess.annotation().requiredLookup == VirtualLookup::Super)
	{
		if (ContractType const* type = dynamic_cast<ContractType const*>(_memberAccess.expression().annotation().type))
		{
			solAssert(type->isSuper(), "");
			functionDef = &functionDef->resolveVirtual(*m_contract, type->contractDefinition().superContract(*m_contract));
		}
	}
	else
		solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");

	processFunction(*functionDef, _memberAccess.annotation());
	return;
}

void FunctionCallGraphBuilder::visitCallable(CallableDeclaration const* _callable, bool _directCall)
{
	solAssert(!m_graph->edges.count(_callable), "");

	std::optional<Node> previousNode = m_currentNode;
	m_currentNode = _callable;

	if (previousNode.has_value() && _directCall)
		add(*previousNode, _callable);
	if (!_directCall)
		add(*m_currentNode, m_currentDispatch);

	_callable->accept(*this);

	m_currentNode = previousNode;
}

void FunctionCallGraphBuilder::visitConstructor(ContractDefinition const& _contract)
{
	for (auto const* stateVar: _contract.stateVariables())
		stateVar->accept(*this);

	for (auto arg: _contract.baseContracts())
		arg->accept(*this);

	if (_contract.constructor())
	{
		add(*m_currentNode, _contract.constructor());
		_contract.constructor()->accept(*this);
	}
}

bool FunctionCallGraphBuilder::add(Node _caller, Node _callee)
{
	return m_graph->edges[_caller].insert(_callee).second;
}

void FunctionCallGraphBuilder::processFunction(CallableDeclaration const& _callable, ExpressionAnnotation const& _annotation)
{
	if (m_graph->edges.count(&_callable))
		return;

	// Create edge to creation dispatch
	if (!_annotation.calledDirectly)
		add(m_currentDispatch, &_callable);
	visitCallable(&_callable, _annotation.calledDirectly);
}
