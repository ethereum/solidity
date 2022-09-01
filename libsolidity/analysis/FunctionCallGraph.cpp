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

#include <libsolutil/StringUtils.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::util;

CallGraph FunctionCallGraphBuilder::buildCreationGraph(ContractDefinition const& _contract)
{
	FunctionCallGraphBuilder builder(_contract);
	solAssert(builder.m_currentNode == CallGraph::Node(CallGraph::SpecialNode::Entry), "");

	// Create graph for constructor, state vars, etc
	for (ContractDefinition const* base: _contract.annotation().linearizedBaseContracts | ranges::views::reverse)
	{
		// The constructor and functions called in state variable initial assignments should have
		// an edge from Entry
		builder.m_currentNode = CallGraph::SpecialNode::Entry;
		for (auto const* stateVar: base->stateVariables())
			if (!stateVar->isConstant())
				stateVar->accept(builder);

		if (base->constructor())
		{
			builder.functionReferenced(*base->constructor());

			// Constructors and functions called in state variable initializers have an edge either from
			// the previous class in linearized order or from Entry if there's no class before.
			builder.m_currentNode = base->constructor();
		}

		// Functions called from the inheritance specifier should have an edge from the constructor
		// for consistency with functions called from constructor modifiers.
		for (auto const& inheritanceSpecifier: base->baseContracts())
			inheritanceSpecifier->accept(builder);
	}

	builder.m_currentNode = CallGraph::SpecialNode::Entry;
	builder.processQueue();

	return std::move(builder.m_graph);
}

CallGraph FunctionCallGraphBuilder::buildDeployedGraph(
	ContractDefinition const& _contract,
	CallGraph const& _creationGraph
)
{
	FunctionCallGraphBuilder builder(_contract);
	solAssert(builder.m_currentNode == CallGraph::Node(CallGraph::SpecialNode::Entry), "");

	auto getSecondElement = [](auto const& _tuple){ return get<1>(_tuple); };

	// Create graph for all publicly reachable functions
	for (FunctionTypePointer functionType: _contract.interfaceFunctionList() | ranges::views::transform(getSecondElement))
	{
		auto const* function = dynamic_cast<FunctionDefinition const*>(&functionType->declaration());
		auto const* variable = dynamic_cast<VariableDeclaration const*>(&functionType->declaration());

		if (function)
			builder.functionReferenced(*function);
		else
			// If it's not a function, it must be a getter of a public variable; we ignore those
			solAssert(variable, "");
	}

	if (_contract.fallbackFunction())
		builder.functionReferenced(*_contract.fallbackFunction());

	if (_contract.receiveFunction())
		builder.functionReferenced(*_contract.receiveFunction());

	// All functions present in internal dispatch at creation time could potentially be pointers
	// assigned to state variables and as such may be reachable after deployment as well.
	builder.m_currentNode = CallGraph::SpecialNode::InternalDispatch;
	set<CallGraph::Node, CallGraph::CompareByID> defaultNode;
	for (CallGraph::Node const& dispatchTarget: util::valueOrDefault(_creationGraph.edges, CallGraph::SpecialNode::InternalDispatch, defaultNode))
	{
		solAssert(!holds_alternative<CallGraph::SpecialNode>(dispatchTarget), "");
		solAssert(get<CallableDeclaration const*>(dispatchTarget) != nullptr, "");

		// Visit the callable to add not only it but also everything it calls too
		builder.functionReferenced(*get<CallableDeclaration const*>(dispatchTarget), false);
	}

	builder.m_currentNode = CallGraph::SpecialNode::Entry;
	builder.processQueue();

	return std::move(builder.m_graph);
}

bool FunctionCallGraphBuilder::visit(FunctionCall const& _functionCall)
{
	if (*_functionCall.annotation().kind != FunctionCallKind::FunctionCall)
		return true;

	auto const* functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);
	solAssert(functionType, "");

	if (functionType->kind() == FunctionType::Kind::Internal && !_functionCall.expression().annotation().calledDirectly)
		// If it's not a direct call, we don't really know which function will be called (it may even
		// change at runtime). All we can do is to add an edge to the dispatch which in turn has
		// edges to all functions could possibly be called.
		add(m_currentNode, CallGraph::SpecialNode::InternalDispatch);
	else if (functionType->kind() == FunctionType::Kind::Error)
		m_graph.usedErrors.insert(&dynamic_cast<ErrorDefinition const&>(functionType->declaration()));

	return true;
}

bool FunctionCallGraphBuilder::visit(EmitStatement const& _emitStatement)
{
	auto const* functionType = dynamic_cast<FunctionType const*>(_emitStatement.eventCall().expression().annotation().type);
	solAssert(functionType, "");

	m_graph.emittedEvents.insert(&dynamic_cast<EventDefinition const&>(functionType->declaration()));

	return true;
}

bool FunctionCallGraphBuilder::visit(Identifier const& _identifier)
{
	if (auto const* variable = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		if (variable->isConstant())
		{
			solAssert(variable->isStateVariable() || variable->isFileLevelVariable(), "");
			variable->accept(*this);
		}
	}
	else if (auto const* callable = dynamic_cast<CallableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		solAssert(*_identifier.annotation().requiredLookup == VirtualLookup::Virtual, "");

		auto funType = dynamic_cast<FunctionType const*>(_identifier.annotation().type);

		// For events kind() == Event, so we have an extra check here
		if (funType && funType->kind() == FunctionType::Kind::Internal)
			functionReferenced(callable->resolveVirtual(m_contract), _identifier.annotation().calledDirectly);
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(MemberAccess const& _memberAccess)
{
	Type const* exprType = _memberAccess.expression().annotation().type;
	ASTString const& memberName = _memberAccess.memberName();

	if (auto magicType = dynamic_cast<MagicType const*>(exprType))
		if (magicType->kind() == MagicType::Kind::MetaType && (
			memberName == "creationCode" || memberName == "runtimeCode"
		))
		{
			ContractType const& accessedContractType = dynamic_cast<ContractType const&>(*magicType->typeArgument());
			m_graph.bytecodeDependency.emplace(&accessedContractType.contractDefinition(), &_memberAccess);
		}

	auto functionType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type);
	auto functionDef = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration);
	if (!functionType || !functionDef || functionType->kind() != FunctionType::Kind::Internal)
		return true;

	// Super functions
	if (*_memberAccess.annotation().requiredLookup == VirtualLookup::Super)
	{
		if (auto const* typeType = dynamic_cast<TypeType const*>(exprType))
			if (auto const contractType = dynamic_cast<ContractType const*>(typeType->actualType()))
			{
				solAssert(contractType->isSuper(), "");
				functionDef = &functionDef->resolveVirtual(
					m_contract,
					contractType->contractDefinition().superContract(m_contract)
				);
			}
	}
	else
		solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");

	functionReferenced(*functionDef, _memberAccess.annotation().calledDirectly);
	return true;
}

bool FunctionCallGraphBuilder::visit(BinaryOperation const& _binaryOperation)
{
	if (_binaryOperation.annotation().userDefinedFunction.set())
		functionReferenced(**_binaryOperation.annotation().userDefinedFunction, true /* called directly */);
	return true;
}

bool FunctionCallGraphBuilder::visit(UnaryOperation const& _unaryOperation)
{
	if (_unaryOperation.annotation().userDefinedFunction.set())
		functionReferenced(**_unaryOperation.annotation().userDefinedFunction, true /* called directly */);
	return true;
}

bool FunctionCallGraphBuilder::visit(ModifierInvocation const& _modifierInvocation)
{
	if (auto const* modifier = dynamic_cast<ModifierDefinition const*>(_modifierInvocation.name().annotation().referencedDeclaration))
	{
		VirtualLookup const& requiredLookup = *_modifierInvocation.name().annotation().requiredLookup;

		if (requiredLookup == VirtualLookup::Virtual)
			functionReferenced(modifier->resolveVirtual(m_contract));
		else
		{
			solAssert(requiredLookup == VirtualLookup::Static, "");
			functionReferenced(*modifier);
		}
	}

	return true;
}

bool FunctionCallGraphBuilder::visit(NewExpression const& _newExpression)
{
	if (ContractType const* contractType = dynamic_cast<ContractType const*>(_newExpression.typeName().annotation().type))
		m_graph.bytecodeDependency.emplace(&contractType->contractDefinition(), &_newExpression);

	return true;
}

void FunctionCallGraphBuilder::enqueueCallable(CallableDeclaration const& _callable)
{
	if (!m_graph.edges.count(&_callable))
	{
		m_visitQueue.push_back(&_callable);

		// Insert the callable to the graph (with no edges coming out of it) to mark it as visited.
		m_graph.edges.insert({CallGraph::Node(&_callable), {}});
	}
}

void FunctionCallGraphBuilder::processQueue()
{
	solAssert(m_currentNode == CallGraph::Node(CallGraph::SpecialNode::Entry), "Visit queue is already being processed.");

	while (!m_visitQueue.empty())
	{
		m_currentNode = m_visitQueue.front();
		solAssert(holds_alternative<CallableDeclaration const*>(m_currentNode), "");

		m_visitQueue.pop_front();
		get<CallableDeclaration const*>(m_currentNode)->accept(*this);
	}

	m_currentNode = CallGraph::SpecialNode::Entry;
}

void FunctionCallGraphBuilder::add(CallGraph::Node _caller, CallGraph::Node _callee)
{
	m_graph.edges[_caller].insert(_callee);
}

void FunctionCallGraphBuilder::functionReferenced(CallableDeclaration const& _callable, bool _calledDirectly)
{
	if (_calledDirectly)
	{
		solAssert(
			holds_alternative<CallGraph::SpecialNode>(m_currentNode) || m_graph.edges.count(m_currentNode) > 0,
			"Adding an edge from a node that has not been visited yet."
		);

		add(m_currentNode, &_callable);
	}
	else
		add(CallGraph::SpecialNode::InternalDispatch, &_callable);

	enqueueCallable(_callable);
}

ostream& solidity::frontend::operator<<(ostream& _out, CallGraph::Node const& _node)
{
	if (holds_alternative<CallGraph::SpecialNode>(_node))
		switch (get<CallGraph::SpecialNode>(_node))
		{
		case CallGraph::SpecialNode::InternalDispatch:
			_out << "InternalDispatch";
			break;
		case CallGraph::SpecialNode::Entry:
			_out << "Entry";
			break;
		default:
			solAssert(false, "Invalid SpecialNode type");
		}
	else
	{
		solAssert(holds_alternative<CallableDeclaration const*>(_node), "");

		auto const* callableDeclaration = get<CallableDeclaration const*>(_node);
		solAssert(callableDeclaration, "");

		auto const* function = dynamic_cast<FunctionDefinition const *>(callableDeclaration);
		auto const* event = dynamic_cast<EventDefinition const *>(callableDeclaration);
		auto const* modifier = dynamic_cast<ModifierDefinition const *>(callableDeclaration);

		auto typeToString = [](auto const& _var) -> string { return _var->type()->toString(true); };
		vector<string> parameters = callableDeclaration->parameters() | ranges::views::transform(typeToString) | ranges::to<vector<string>>();

		string scopeName;
		if (!function || !function->isFree())
		{
			solAssert(callableDeclaration->annotation().scope, "");
			auto const* parentContract = dynamic_cast<ContractDefinition const*>(callableDeclaration->annotation().scope);
			solAssert(parentContract, "");
			scopeName = parentContract->name();
		}

		if (function && function->isFree())
			_out << "function " << function->name() << "(" << joinHumanReadable(parameters, ",") << ")";
		else if (function && function->isConstructor())
			_out << "constructor of " << scopeName;
		else if (function && function->isFallback())
			_out << "fallback of " << scopeName;
		else if (function && function->isReceive())
			_out << "receive of " << scopeName;
		else if (function)
			_out << "function " << scopeName << "." << function->name() << "(" << joinHumanReadable(parameters, ",") << ")";
		else if (event)
			_out << "event " << scopeName << "." << event->name() << "(" << joinHumanReadable(parameters, ",") << ")";
		else if (modifier)
			_out << "modifier " << scopeName << "." << modifier->name();
		else
			solAssert(false, "Unexpected AST node type in function call graph");
	}

	return _out;
}
