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
/**
 * Optimiser component that removes assignments to variables that are not used
 * until they go out of scope or are re-assigned.
 */

#include <libyul/optimiser/RedundantAssignEliminator.h>

#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace dev;
using namespace yul;

void RedundantAssignEliminator::operator()(Identifier const& _identifier)
{
	changeUndecidedTo(_identifier.name, State::Used);
}

void RedundantAssignEliminator::operator()(VariableDeclaration const& _variableDeclaration)
{
	ASTWalker::operator()(_variableDeclaration);

	for (auto const& var: _variableDeclaration.variables)
		m_declaredVariables.emplace(var.name);
}

void RedundantAssignEliminator::operator()(Assignment const& _assignment)
{
	visit(*_assignment.value);
	for (auto const& var: _assignment.variableNames)
		changeUndecidedTo(var.name, State::Unused);

	if (_assignment.variableNames.size() == 1)
		// Default-construct it in "Undecided" state if it does not yet exist.
		m_assignments[_assignment.variableNames.front().name][&_assignment];
}

void RedundantAssignEliminator::operator()(If const& _if)
{
	visit(*_if.condition);

	TrackedAssignments skipBranch{m_assignments};
	(*this)(_if.body);

	merge(m_assignments, move(skipBranch));
}

void RedundantAssignEliminator::operator()(Switch const& _switch)
{
	visit(*_switch.expression);

	TrackedAssignments const preState{m_assignments};

	bool hasDefault = false;
	vector<TrackedAssignments> branches;
	for (auto const& c: _switch.cases)
	{
		if (!c.value)
			hasDefault = true;
		(*this)(c.body);
		branches.emplace_back(move(m_assignments));
		m_assignments = preState;
	}

	if (hasDefault)
	{
		m_assignments = move(branches.back());
		branches.pop_back();
	}
	for (auto& branch: branches)
		merge(m_assignments, move(branch));
}

void RedundantAssignEliminator::operator()(FunctionDefinition const& _functionDefinition)
{
	std::set<YulString> outerDeclaredVariables;
	TrackedAssignments outerAssignments;
	ForLoopInfo forLoopInfo;
	swap(m_declaredVariables, outerDeclaredVariables);
	swap(m_assignments, outerAssignments);
	swap(m_forLoopInfo, forLoopInfo);

	(*this)(_functionDefinition.body);

	for (auto const& param: _functionDefinition.parameters)
		finalize(param.name, State::Unused);
	for (auto const& retParam: _functionDefinition.returnVariables)
		finalize(retParam.name, State::Used);

	swap(m_declaredVariables, outerDeclaredVariables);
	swap(m_assignments, outerAssignments);
	swap(m_forLoopInfo, forLoopInfo);
}

void RedundantAssignEliminator::operator()(ForLoop const& _forLoop)
{
	ForLoopInfo outerForLoopInfo;
	swap(outerForLoopInfo, m_forLoopInfo);

	set<YulString> outerDeclaredVariables;
	swap(m_declaredVariables, outerDeclaredVariables);

	// We need to visit the statements directly because of the
	// scoping rules.
	walkVector(_forLoop.pre.statements);

	// We just run the loop twice to account for the
	// back edge.
	// There need not be more runs because we only have three different states.

	visit(*_forLoop.condition);

	TrackedAssignments zeroRuns{m_assignments};

	(*this)(_forLoop.body);
	merge(m_assignments, move(m_forLoopInfo.pendingContinueStmts));
	m_forLoopInfo.pendingContinueStmts = {};
	(*this)(_forLoop.post);

	visit(*_forLoop.condition);

	TrackedAssignments oneRun{m_assignments};

	(*this)(_forLoop.body);

	merge(m_assignments, move(m_forLoopInfo.pendingContinueStmts));
	m_forLoopInfo.pendingContinueStmts.clear();
	(*this)(_forLoop.post);

	visit(*_forLoop.condition);

	// Order does not matter because "max" is commutative and associative.
	merge(m_assignments, move(oneRun));
	merge(m_assignments, move(zeroRuns));
	merge(m_assignments, move(m_forLoopInfo.pendingBreakStmts));
	m_forLoopInfo.pendingBreakStmts.clear();

	for (auto const& var: m_declaredVariables)
		finalize(var, State::Unused);
	swap(m_declaredVariables, outerDeclaredVariables);

	// Restore potential outer for-loop states.
	swap(m_forLoopInfo, outerForLoopInfo);
}

void RedundantAssignEliminator::operator()(Break const&)
{
	m_forLoopInfo.pendingBreakStmts.emplace_back(move(m_assignments));
	m_assignments.clear();
}

void RedundantAssignEliminator::operator()(Continue const&)
{
	m_forLoopInfo.pendingContinueStmts.emplace_back(move(m_assignments));
	m_assignments.clear();
}

void RedundantAssignEliminator::operator()(Block const& _block)
{
	set<YulString> outerDeclaredVariables;
	swap(m_declaredVariables, outerDeclaredVariables);

	ASTWalker::operator()(_block);

	for (auto const& var: m_declaredVariables)
		finalize(var, State::Unused);

	swap(m_declaredVariables, outerDeclaredVariables);
}

void RedundantAssignEliminator::run(Dialect const& _dialect, Block& _ast)
{
	RedundantAssignEliminator rae{_dialect};
	rae(_ast);

	AssignmentRemover remover{rae.m_pendingRemovals};
	remover(_ast);
}

template <class K, class V, class F>
void joinMap(std::map<K, V>& _a, std::map<K, V>&& _b, F _conflictSolver)
{
	// TODO Perhaps it is better to just create a sorted list
	// and then use insert(begin, end)

	auto ita = _a.begin();
	auto aend = _a.end();
	auto itb = _b.begin();
	auto bend = _b.end();

	for (; itb != bend; ++ita)
	{
		if (ita == aend)
			ita = _a.insert(ita, std::move(*itb++));
		else if (ita->first < itb->first)
			continue;
		else if (itb->first < ita->first)
			ita = _a.insert(ita, std::move(*itb++));
		else
		{
			_conflictSolver(ita->second, std::move(itb->second));
			++itb;
		}
	}
}

void RedundantAssignEliminator::merge(TrackedAssignments& _target, TrackedAssignments&& _other)
{
	joinMap(_target, move(_other), [](
		map<Assignment const*, State>& _assignmentHere,
		map<Assignment const*, State>&& _assignmentThere
	)
	{
		return joinMap(_assignmentHere, move(_assignmentThere), State::join);
	});
}

void RedundantAssignEliminator::merge(TrackedAssignments& _target, vector<TrackedAssignments>&& _source)
{
	for (TrackedAssignments& ts: _source)
		merge(_target, move(ts));
	_source.clear();
}

void RedundantAssignEliminator::changeUndecidedTo(YulString _variable, RedundantAssignEliminator::State _newState)
{
	for (auto& assignment: m_assignments[_variable])
		if (assignment.second == State::Undecided)
			assignment.second = _newState;
}

void RedundantAssignEliminator::finalize(YulString _variable, RedundantAssignEliminator::State _finalState)
{
	finalize(m_assignments, _variable, _finalState);
	for (auto& assignments: m_forLoopInfo.pendingBreakStmts)
		finalize(assignments, _variable, _finalState);
	for (auto& assignments: m_forLoopInfo.pendingContinueStmts)
		finalize(assignments, _variable, _finalState);
}

void RedundantAssignEliminator::finalize(
	TrackedAssignments& _assignments,
	YulString _variable,
	RedundantAssignEliminator::State _finalState
)
{
	for (auto const& assignment: _assignments[_variable])
	{
		State const state = assignment.second == State::Undecided ? _finalState : assignment.second;

		if (state == State::Unused && MovableChecker{*m_dialect, *assignment.first->value}.movable())
			// TODO the only point where we actually need this
			// to be a set is for the for loop
			m_pendingRemovals.insert(assignment.first);
	}
	_assignments.erase(_variable);
}

void AssignmentRemover::operator()(Block& _block)
{
	boost::range::remove_erase_if(_block.statements, [=](Statement const& _statement) -> bool {
		return _statement.type() == typeid(Assignment) && m_toRemove.count(&boost::get<Assignment>(_statement));
	});

	ASTModifier::operator()(_block);
}
