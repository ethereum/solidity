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

#include <libyul/optimiser/RedundantAssignEliminator.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <range/v3/action/remove_if.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void RedundantAssignEliminator::run(OptimiserStepContext& _context, Block& _ast)
{
	RedundantAssignEliminator rae{_context.dialect};
	rae(_ast);

	StatementRemover remover{rae.m_pendingRemovals};
	remover(_ast);
}

void RedundantAssignEliminator::operator()(Identifier const& _identifier)
{
	changeUndecidedTo(_identifier.name, State::Used);
}

void RedundantAssignEliminator::operator()(VariableDeclaration const& _variableDeclaration)
{
	RedundantStoreBase::operator()(_variableDeclaration);

	for (auto const& var: _variableDeclaration.variables)
		m_declaredVariables.emplace(var.name);
}

void RedundantAssignEliminator::operator()(Assignment const& _assignment)
{
	visit(*_assignment.value);
	for (auto const& var: _assignment.variableNames)
		changeUndecidedTo(var.name, State::Unused);
}

void RedundantAssignEliminator::operator()(FunctionDefinition const& _functionDefinition)
{
	ScopedSaveAndRestore outerDeclaredVariables(m_declaredVariables, {});
	ScopedSaveAndRestore outerReturnVariables(m_returnVariables, {});

	for (auto const& retParam: _functionDefinition.returnVariables)
		m_returnVariables.insert(retParam.name);

	RedundantStoreBase::operator()(_functionDefinition);
}

void RedundantAssignEliminator::operator()(Leave const&)
{
	for (YulString name: m_returnVariables)
		changeUndecidedTo(name, State::Used);
}

void RedundantAssignEliminator::operator()(Block const& _block)
{
	ScopedSaveAndRestore outerDeclaredVariables(m_declaredVariables, {});

	RedundantStoreBase::operator()(_block);

	for (auto const& var: m_declaredVariables)
		finalize(var, State::Unused);
}

void RedundantAssignEliminator::visit(Statement const& _statement)
{
	RedundantStoreBase::visit(_statement);

	if (auto const* assignment = get_if<Assignment>(&_statement))
		if (assignment->variableNames.size() == 1)
			// Default-construct it in "Undecided" state if it does not yet exist.
			m_stores[assignment->variableNames.front().name][&_statement];
}

void RedundantAssignEliminator::shortcutNestedLoop(TrackedStores const& _zeroRuns)
{
	// Shortcut to avoid horrible runtime:
	// Change all assignments that were newly introduced in the for loop to "used".
	// We do not have to do that with the "break" or "continue" paths, because
	// they will be joined later anyway.
	// TODO parallel traversal might be more efficient here.
	for (auto& [variable, stores]: m_stores)
		for (auto& assignment: stores)
		{
			auto zeroIt = _zeroRuns.find(variable);
			if (zeroIt != _zeroRuns.end() && zeroIt->second.count(assignment.first))
				continue;
			assignment.second = State::Value::Used;
		}
}

void RedundantAssignEliminator::finalizeFunctionDefinition(FunctionDefinition const& _functionDefinition)
{
	for (auto const& param: _functionDefinition.parameters)
		finalize(param.name, State::Unused);
	for (auto const& retParam: _functionDefinition.returnVariables)
		finalize(retParam.name, State::Used);
}

void RedundantAssignEliminator::changeUndecidedTo(YulString _variable, RedundantAssignEliminator::State _newState)
{
	for (auto& assignment: m_stores[_variable])
		if (assignment.second == State::Undecided)
			assignment.second = _newState;
}

void RedundantAssignEliminator::finalize(YulString _variable, RedundantAssignEliminator::State _finalState)
{
	std::map<Statement const*, State> stores = std::move(m_stores[_variable]);
	m_stores.erase(_variable);

	for (auto& breakAssignments: m_forLoopInfo.pendingBreakStmts)
	{
		util::joinMap(stores, std::move(breakAssignments[_variable]), State::join);
		breakAssignments.erase(_variable);
	}
	for (auto& continueAssignments: m_forLoopInfo.pendingContinueStmts)
	{
		util::joinMap(stores, std::move(continueAssignments[_variable]), State::join);
		continueAssignments.erase(_variable);
	}

	for (auto&& [statement, state]: stores)
		if (
			(state == State::Unused || (state == State::Undecided && _finalState == State::Unused)) &&
			SideEffectsCollector{m_dialect, *std::get<Assignment>(*statement).value}.movable()
		)
			m_pendingRemovals.insert(statement);
}
