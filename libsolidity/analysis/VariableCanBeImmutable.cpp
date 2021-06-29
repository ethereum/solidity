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

#include <libsolidity/analysis/VariableCanBeImmutable.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/CallGraph.h>
#include <libsolutil/Algorithms.h>

#include <liblangutil/ErrorReporter.h>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range/conversion.hpp>

#include <map>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;


bool VariableCanBeImmutable::visit(ContractDefinition const& _contractDefinition)
{
	m_contract = &_contractDefinition;
	auto const& edges = (*_contractDefinition.annotation().deployedCallGraph)->edges;
	frontend::CallGraph::Node _node = frontend::CallGraph::SpecialNode::Entry;
	// A contract with no additional functions
	if (!edges.count(_node))
		return true;
	auto functions =
		edges.at(_node) |
		ranges::views::transform(
			[](auto const& _node) -> optional<FunctionDefinition const*>
			{
				auto callable = get<CallableDeclaration const*>(_node);
				solAssert(callable, "");
				if (auto functionDefinition = dynamic_cast<FunctionDefinition const*>(callable))
					return functionDefinition;
				return std::nullopt;
			}
		) |
		ranges::views::filter([](auto const& _optional) { return _optional.has_value(); }) |
		ranges::views::transform([](auto const& _optional) { return *_optional; }) |
		ranges::to<list<FunctionDefinition const*>>
	;
	m_reachableFunctions = util::BreadthFirstSearch<FunctionDefinition const*>{functions}.run(
		[&](FunctionDefinition const* _function, auto&& _addChild)
		{
			for (CallGraph::Node const& node: edges.at(_function))
			{
				auto callable = get<CallableDeclaration const*>(node);
				solAssert(callable, "");
				if (auto function = dynamic_cast<FunctionDefinition const*>(callable))
					_addChild(function);
			}
		}
	).visited;

	return true;
}

void VariableCanBeImmutable::endVisit(ContractDefinition const& _contractDefinition)
{
	for (auto const& variable: _contractDefinition.stateVariables())
		if (
			!variable->immutable() &&
			!m_variablesWrittenTo.count(variable) &&
			!variable->isConstant()
		)
			m_errorReporter.warning(
				0000_error,
				variable->location(),
				"Variable declaration can be converted into an immutable."
			);

	m_variablesWrittenTo.clear();
	m_contract = nullptr;
}

bool VariableCanBeImmutable::visit(FunctionDefinition const& _functionDefinition)
{
	m_function = &_functionDefinition;

	return true;
}
void VariableCanBeImmutable::endVisit(FunctionDefinition const& )
{
	m_function = nullptr;
}

void VariableCanBeImmutable::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	solAssert(declaration, "");

	bool writes = _identifier.annotation().willBeWrittenTo;
	if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
		if (
			writes &&
			!varDecl->immutable() &&
			varDecl->isStateVariable() &&
			!varDecl->isConstant()
		)
		{
			solAssert(m_contract, "");
			if (m_function && m_reachableFunctions.count(m_function))
				m_variablesWrittenTo.insert(varDecl);
		}
}
