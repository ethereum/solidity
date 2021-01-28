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

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

using namespace std;
using namespace ranges;
using namespace solidity::frontend;

FunctionCallGraphBuilder::FunctionCallGraphBuilder(ContractDefinition const& _contract):
	m_contract(&_contract),
	m_graph(make_unique<ContractCallGraph>(_contract))
{
	// Create graph for constructor, state vars, etc
	m_currentNode = SpecialNode::EntryCreation;
	m_currentDispatch = SpecialNode::InternalCreationDispatch;

	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts | views::reverse)
	{
		for (auto const* stateVar: contract->stateVariables())
			stateVar->accept(*this);

		for (auto arg: contract->baseContracts())
			arg->accept(*this);

		if (contract->constructor())
		{
			add(*m_currentNode, contract->constructor());
			contract->constructor()->accept(*this);
			m_currentNode = contract->constructor();
		}
	}

	auto getSecondElement = [](auto const& _tuple){ return get<1>(_tuple); };

	// Create graph for all publicly reachable functions
	m_currentNode = SpecialNode::Entry;
	m_currentDispatch = SpecialNode::InternalDispatch;

	for (FunctionTypePointer functionType: _contract.interfaceFunctionList() | views::transform(getSecondElement))
	{
		auto const* function = dynamic_cast<FunctionDefinition const*>(&functionType->declaration());
		auto const* variable = dynamic_cast<VariableDeclaration const*>(&functionType->declaration());

		if (function)
			processFunction(*function);
		else
			// If it's not a function, it must be a getter of a public variable; we ignore those
			solAssert(variable, "");
	}

	// Add all InternalCreationDispatch calls to the InternalDispatch as well
	add(SpecialNode::InternalDispatch, SpecialNode::InternalCreationDispatch);

	if (_contract.fallbackFunction())
		add(SpecialNode::Entry, _contract.fallbackFunction());

	if (_contract.receiveFunction())
		add(SpecialNode::Entry, _contract.receiveFunction());

	m_currentNode.reset();
}

bool FunctionCallGraphBuilder::CompareByID::operator()(Node const& _lhs, Node const& _rhs) const
{
	if (_lhs.index() != _rhs.index())
		return _lhs.index() < _rhs.index();

	if (holds_alternative<SpecialNode>(_lhs))
		return get<SpecialNode>(_lhs) < get<SpecialNode>(_rhs);
	return get<ASTNode const*>(_lhs)->id() < get<ASTNode const*>(_rhs)->id();
}

bool FunctionCallGraphBuilder::CompareByID::operator()(Node const& _lhs, int64_t _rhs) const
{
	solAssert(!holds_alternative<SpecialNode>(_lhs), "");

	return get<ASTNode const*>(_lhs)->id() < _rhs;
}

bool FunctionCallGraphBuilder::CompareByID::operator()(int64_t _lhs, Node const& _rhs) const
{
	solAssert(!holds_alternative<SpecialNode>(_rhs), "");

	return _lhs < get<ASTNode const*>(_rhs)->id();
}

unique_ptr<FunctionCallGraphBuilder::ContractCallGraph> FunctionCallGraphBuilder::create(ContractDefinition const& _contract)
{
	return FunctionCallGraphBuilder(_contract).m_graph;
}

bool FunctionCallGraphBuilder::visit(FunctionCall const& _functionCall)
{
	solAssert(m_currentNode.has_value(), "");
	solAssert(holds_alternative<SpecialNode>(*m_currentNode) || get<ASTNode const*>(*m_currentNode) != nullptr, "");

	FunctionType const* functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);
	if (
		functionType &&
		functionType->kind() == FunctionType::Kind::Internal &&
		!_functionCall.expression().annotation().calledDirectly
	)
		// If it's not a direct call, we don't really know which function will be called (it may even
		// change at runtime). All we can do is to add an edge to the dispatch which in turn has
		// edges to all functions could possibly be called.
		add(*m_currentNode, m_currentDispatch);
	else if (functionType && functionType->kind() == FunctionType::Kind::Event)
	{
		// NOTE: calledDirectly is always false for events, no matter whether it's an actual emit
		// or just event name used in an expression. Does not matter since this visit(FunctionCall)
		// won't get triggered in the latter case.
		solAssert(!_functionCall.expression().annotation().calledDirectly, "");

		EventDefinition const* event = nullptr;
		if (auto memberAccess = dynamic_cast<MemberAccess const*>(&_functionCall.expression()))
		{
			solAssert(*memberAccess->annotation().requiredLookup == VirtualLookup::Static, "");
			event = dynamic_cast<EventDefinition const*>(memberAccess->annotation().referencedDeclaration);
		}
		else if (auto identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
		{
			// TMP: Why is the lookup type virtual for when we refer to the event just by name
			// and static when we qualify it with the contract name?
			solAssert(*identifier->annotation().requiredLookup == VirtualLookup::Virtual, "");
			event = dynamic_cast<EventDefinition const*>(identifier->annotation().referencedDeclaration);
		}
		else
			solAssert(false, "");

		solAssert(event, "");
		add(*m_currentNode, event);
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(Identifier const& _identifier)
{
	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		solAssert(*_identifier.annotation().requiredLookup == VirtualLookup::Virtual, "");

		auto funType = dynamic_cast<FunctionType const*>(_identifier.annotation().type);

		// For events kind() == Event, so we have an extra check here
		if (funType && funType->kind() == FunctionType::Kind::Internal)
			processFunction(callable->resolveVirtual(*m_contract), _identifier.annotation().calledDirectly);
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
		if (auto const* typeType = dynamic_cast<TypeType const*>(_memberAccess.expression().annotation().type))
			if (auto const contractType = dynamic_cast<ContractType const*>(typeType->actualType()))
			{
				solAssert(contractType->isSuper(), "");
				functionDef =
					&functionDef->resolveVirtual(
						*m_contract,
						contractType->contractDefinition().superContract(*m_contract)
					);
			}
	}
	else
		solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");

	processFunction(*functionDef, _memberAccess.annotation().calledDirectly);
	return;
}

void FunctionCallGraphBuilder::endVisit(ModifierInvocation const& _modifierInvocation)
{
	VirtualLookup const& requiredLookup = *_modifierInvocation.name().annotation().requiredLookup;

	if (auto const* modifier = dynamic_cast<ModifierDefinition const*>(_modifierInvocation.name().annotation().referencedDeclaration))
	{
		if (requiredLookup == VirtualLookup::Virtual)
			modifier = &modifier->resolveVirtual(*m_contract);
		else
			solAssert(requiredLookup == VirtualLookup::Static, "");

		processFunction(*modifier);
	}
}

void FunctionCallGraphBuilder::visitCallable(CallableDeclaration const* _callable)
{
	solAssert(!m_graph->edges.count(_callable), "");

	optional<Node> previousNode = m_currentNode;
	m_currentNode = _callable;

	_callable->accept(*this);

	m_currentNode = previousNode;
}

bool FunctionCallGraphBuilder::add(Node _caller, Node _callee)
{
	return m_graph->edges[_caller].insert(_callee).second;
}

void FunctionCallGraphBuilder::processFunction(CallableDeclaration const& _callable, bool _calledDirectly)
{
	solAssert(m_currentNode.has_value(), "");

	if (_calledDirectly)
		add(*m_currentNode, &_callable);
	else
		add(m_currentDispatch, &_callable);

	if (!m_graph->edges.count(&_callable))
		visitCallable(&_callable);
}

ostream& solidity::frontend::operator<<(ostream& _out, FunctionCallGraphBuilder::Node const& _node)
{
	using SpecialNode = FunctionCallGraphBuilder::SpecialNode;

	if (holds_alternative<SpecialNode>(_node))
	{
		auto specialNode = get<SpecialNode>(_node);
		switch (specialNode)
		{
			case SpecialNode::EntryCreation:
				_out<< "EntryCreation";
				break;
			case SpecialNode::InternalCreationDispatch:
				_out<< "InternalCreationDispatch";
				break;
			case SpecialNode::InternalDispatch:
				_out<< "InternalDispatch";
				break;
			case SpecialNode::Entry:
				_out<< "Entry";
				break;
			default: solAssert(false, "Invalid SpecialNode type");
		}
	}
	else
	{
		solAssert(get<ASTNode const*>(_node) != nullptr, "");
		auto const* callableDeclaration = dynamic_cast<CallableDeclaration const *>(get<ASTNode const*>(_node));

		auto const* function = dynamic_cast<FunctionDefinition const *>(callableDeclaration);
		auto const* event = dynamic_cast<EventDefinition const *>(callableDeclaration);
		auto const* modifier = dynamic_cast<ModifierDefinition const *>(callableDeclaration);

		auto typeToString = [](auto const& _var) -> string { return _var->type()->toString(true); };
		vector<string> parameters = callableDeclaration->parameters() | views::transform(typeToString) | to<vector<string>>();
		string joinedParameters = parameters | views::join(',') | to<string>();

		string scopeName;
		if (!function || !function->isFree())
		{
			solAssert(callableDeclaration->annotation().scope, "");
			auto const* parentContract = dynamic_cast<ContractDefinition const*>(callableDeclaration->annotation().scope);
			solAssert(parentContract, "");
			scopeName = parentContract->name();
		}

		if (function && function->isFree())
			_out << "function " << function->name() << "(" << joinedParameters << ")";
		else if (function && function->isConstructor())
			_out << "constructor of " << scopeName;
		else if (function && function->isFallback())
			_out << "fallback of " << scopeName;
		else if (function && function->isReceive())
			_out << "receive of " << scopeName;
		else if (function)
			_out << "function " << scopeName << "." << function->name() << "(" << joinedParameters << ")";
		else if (event)
			_out<< "event " << scopeName << "." << event->name() << "(" << joinedParameters << ")";
		else if (modifier)
			_out<< "modifier " << scopeName << "." << modifier->name();
		else
			solAssert(false, "Unexpected AST node type in function call graph");
	}

	return _out;
}
