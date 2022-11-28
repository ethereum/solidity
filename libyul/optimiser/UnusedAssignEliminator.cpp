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
#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>

#include <libsolutil/CommonData.h>

#include <range/v3/action/remove_if.hpp>

#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

// TODO this component does not handle reverting function calls specially. Is that OK?
// We should set m_activeStores to empty set for a reverting function call, like wo do with `leave`.

void UnusedAssignEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	UnusedAssignEliminator rae{_context.dialect};
	rae(_ast);

	rae.m_storesToRemove += rae.m_allStores - rae.m_usedStores;

	StatementRemover remover{std::set<Statement const*>{rae.m_storesToRemove.begin(), rae.m_storesToRemove.end()}};
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

void UnusedAssignEliminator::operator()(Leave const&)
{
	for (YulString name: m_returnVariables)
		markUsed(name);
}

void UnusedAssignEliminator::operator()(Block const& _block)
{
	UnusedStoreBase::operator()(_block);

	// TODO we could also move some statements from "potentially" to "toRemove".
	for (auto const& statement: _block.statements)
		if (auto const* varDecl = get_if<VariableDeclaration>(&statement))
			for (auto const& var: varDecl->variables)
				m_activeStores.erase(var.name);
}

void UnusedAssignEliminator::visit(Statement const& _statement)
{
	UnusedStoreBase::visit(_statement);

	if (auto const* assignment = get_if<Assignment>(&_statement))
	{
		// TODO this should also use user function side effects.
		// Then we have to modify the multi-assign test (or verify that it is fine after all
		// by adding a test where one var is used but not the other)
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

//	cerr << "After " << std::visit(AsmPrinter{}, _statement) << endl;
//	for (auto&& [var, assigns]: m_activeStores)
//	{
//		cerr << "  " << var.str() << ":" << endl;
//		for (auto const& assign: assigns)
//			cerr << "    " << std::visit(AsmPrinter{}, *assign) << endl;
//	}
}

void UnusedAssignEliminator::shortcutNestedLoop(ActiveStores const& _zeroRuns)
{
	// Shortcut to avoid horrible runtime:
	// Change all assignments that were newly introduced in the for loop to "used".
	// We do not have to do that with the "break" or "continue" paths, because
	// they will be joined later anyway.
	// TODO parallel traversal might be more efficient here.

	// TODO is this correct?

	for (auto& [variable, stores]: m_activeStores)
		for (auto& assignment: stores)
		{
			auto zeroIt = _zeroRuns.find(variable);
			if (zeroIt != _zeroRuns.end() && zeroIt->second.count(assignment))
				continue;
			m_usedStores.insert(assignment);
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
