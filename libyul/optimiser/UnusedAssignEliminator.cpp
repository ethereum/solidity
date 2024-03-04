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
/**
 * Optimiser component that removes assignments to variables that are not used
 * until they go out of scope or are re-assigned.
 */

#include <libyul/optimiser/UnusedAssignEliminator.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>

#include <libsolutil/CommonData.h>

#include <range/v3/action/remove_if.hpp>

#include <iostream>

using namespace solidity;
using namespace solidity::yul;

void UnusedAssignEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	UnusedAssignEliminator uae{
		_context.dialect,
		ControlFlowSideEffectsCollector{_context.dialect, _ast}.functionSideEffectsNamed()
	};
	uae(_ast);

	uae.m_storesToRemove += uae.m_allStores - uae.m_usedStores;

	std::set<Statement const*> toRemove{uae.m_storesToRemove.begin(), uae.m_storesToRemove.end()};
	StatementRemover remover{toRemove};
	remover(_ast);
}

void UnusedAssignEliminator::operator()(Identifier const& _identifier)
{
	markUsed(_identifier.name);
}

void UnusedAssignEliminator::operator()(Assignment const& _assignment)
{
	visit(*_assignment.value);
	// Do not visit the variables because they are Identifiers
}


void UnusedAssignEliminator::operator()(FunctionDefinition const& _functionDefinition)
{
	ScopedSaveAndRestore outerReturnVariables(m_returnVariables, {});

	for (auto const& retParam: _functionDefinition.returnVariables)
		m_returnVariables.insert(retParam.name);

	UnusedStoreBase::operator()(_functionDefinition);
}

void UnusedAssignEliminator::operator()(FunctionCall const& _functionCall)
{
	UnusedStoreBase::operator()(_functionCall);

	ControlFlowSideEffects sideEffects;
	if (auto builtin = m_dialect.builtin(_functionCall.functionName.name))
		sideEffects = builtin->controlFlowSideEffects;
	else
		sideEffects = m_controlFlowSideEffects.at(_functionCall.functionName.name);

	if (!sideEffects.canContinue)
		// We do not return from the current function, so it is OK to also
		// clear the return variables.
		m_activeStores.clear();
}

void UnusedAssignEliminator::operator()(Leave const&)
{
	for (YulString name: m_returnVariables)
		markUsed(name);
	m_activeStores.clear();
}

void UnusedAssignEliminator::operator()(Block const& _block)
{
	UnusedStoreBase::operator()(_block);

	for (auto const& statement: _block.statements)
		if (auto const* varDecl = std::get_if<VariableDeclaration>(&statement))
			for (auto const& var: varDecl->variables)
				m_activeStores.erase(var.name);
}

void UnusedAssignEliminator::visit(Statement const& _statement)
{
	UnusedStoreBase::visit(_statement);

	if (auto const* assignment = std::get_if<Assignment>(&_statement))
	{
		// We do not remove assignments whose values might have side-effects,
		// but clear the active stores to the assigned variables in any case.
		if (SideEffectsCollector{m_dialect, *assignment->value}.movable())
		{
			m_allStores.insert(&_statement);
			for (auto const& var: assignment->variableNames)
				m_activeStores[var.name] = {&_statement};
		}
		else
			for (auto const& var: assignment->variableNames)
				m_activeStores[var.name].clear();
	}
}

void UnusedAssignEliminator::shortcutNestedLoop(ActiveStores const& _zeroRuns)
{
	// Shortcut to avoid horrible runtime:
	// Change all assignments that were newly introduced in the for loop to "used".
	// We do not have to do that with the "break" or "continue" paths, because
	// they will be joined later anyway.

	for (auto& [variable, stores]: m_activeStores)
	{
		auto zeroIt = _zeroRuns.find(variable);
		for (auto& assignment: stores)
		{
			if (zeroIt != _zeroRuns.end() && zeroIt->second.count(assignment))
				continue;
			m_usedStores.insert(assignment);
		}
	}
}

void UnusedAssignEliminator::finalizeFunctionDefinition(FunctionDefinition const& _functionDefinition)
{
	for (auto const& retParam: _functionDefinition.returnVariables)
		markUsed(retParam.name);
}

void UnusedAssignEliminator::markUsed(YulString _variable)
{
	for (auto& assignment: m_activeStores[_variable])
		m_usedStores.insert(assignment);
	m_activeStores.erase(_variable);
}
