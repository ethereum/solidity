/*(
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
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 */

#include <libyul/optimiser/DataFlowAnalyzer.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

void DataFlowAnalyzer::operator()(Assignment& _assignment)
{
	set<YulString> names;
	for (auto const& var: _assignment.variableNames)
		names.emplace(var.name);
	assertThrow(_assignment.value, OptimizerException, "");
	visit(*_assignment.value);
	handleAssignment(names, _assignment.value.get());
}

void DataFlowAnalyzer::operator()(VariableDeclaration& _varDecl)
{
	set<YulString> names;
	for (auto const& var: _varDecl.variables)
		names.emplace(var.name);
	m_variableScopes.back().variables += names;

	if (_varDecl.value)
		visit(*_varDecl.value);

	handleAssignment(names, _varDecl.value.get());
}

void DataFlowAnalyzer::operator()(If& _if)
{
	ASTModifier::operator()(_if);

	Assignments assignments;
	assignments(_if.body);
	clearValues(assignments.names());
}

void DataFlowAnalyzer::operator()(Switch& _switch)
{
	visit(*_switch.expression);
	set<YulString> assignedVariables;
	for (auto& _case: _switch.cases)
	{
		(*this)(_case.body);
		Assignments assignments;
		assignments(_case.body);
		assignedVariables += assignments.names();
		// This is a little too destructive, we could retain the old values.
		clearValues(assignments.names());
	}
	clearValues(assignedVariables);
}

void DataFlowAnalyzer::operator()(FunctionDefinition& _fun)
{
	// Save all information. We might rather reinstantiate this class,
	// but this could be difficult if it is subclassed.
	map<YulString, Expression const*> value;
	map<YulString, set<YulString>> references;
	map<YulString, set<YulString>> referencedBy;
	m_value.swap(value);
	m_references.swap(references);
	m_referencedBy.swap(referencedBy);
	pushScope(true);

	for (auto const& parameter: _fun.parameters)
		m_variableScopes.back().variables.emplace(parameter.name);
	for (auto const& var: _fun.returnVariables)
	{
		m_variableScopes.back().variables.emplace(var.name);
		handleAssignment({var.name}, nullptr);
	}
	ASTModifier::operator()(_fun);

	popScope();
	m_value.swap(value);
	m_references.swap(references);
	m_referencedBy.swap(referencedBy);
}

void DataFlowAnalyzer::operator()(ForLoop& _for)
{
	// Special scope handling of the pre block.
	pushScope(false);
	for (auto& statement: _for.pre.statements)
		visit(statement);

	Assignments assignments;
	assignments(_for.body);
	assignments(_for.post);
	clearValues(assignments.names());

	visit(*_for.condition);
	(*this)(_for.body);
	(*this)(_for.post);

	clearValues(assignments.names());
	popScope();
}

void DataFlowAnalyzer::operator()(Block& _block)
{
	size_t numScopes = m_variableScopes.size();
	pushScope(false);
	ASTModifier::operator()(_block);
	popScope();
	assertThrow(numScopes == m_variableScopes.size(), OptimizerException, "");
}

void DataFlowAnalyzer::handleAssignment(set<YulString> const& _variables, Expression* _value)
{
	static Expression const zero{Literal{{}, LiteralKind::Number, YulString{"0"}, {}}};
	clearValues(_variables);

	MovableChecker movableChecker{m_dialect};
	if (_value)
		movableChecker.visit(*_value);
	else
		for (auto const& var: _variables)
			m_value[var] = &zero;

	if (_value && _variables.size() == 1)
	{
		YulString name = *_variables.begin();
		// Expression has to be movable and cannot contain a reference
		// to the variable that will be assigned to.
		if (movableChecker.movable() && !movableChecker.referencedVariables().count(name))
			m_value[name] = _value;
	}

	auto const& referencedVariables = movableChecker.referencedVariables();
	for (auto const& name: _variables)
	{
		m_references[name] = referencedVariables;
		for (auto const& ref: referencedVariables)
			m_referencedBy[ref].emplace(name);
	}
}

void DataFlowAnalyzer::pushScope(bool _functionScope)
{
	m_variableScopes.emplace_back(_functionScope);
}

void DataFlowAnalyzer::popScope()
{
	clearValues(std::move(m_variableScopes.back().variables));
	m_variableScopes.pop_back();
}

void DataFlowAnalyzer::clearValues(set<YulString> _variables)
{
	// All variables that reference variables to be cleared also have to be
	// cleared, but not recursively, since only the value of the original
	// variables changes. Example:
	// let a := 1
	// let b := a
	// let c := b
	// let a := 2
	// add(b, c)
	// In the last line, we can replace c by b, but not b by a.
	//
	// This cannot be easily tested since the substitutions will be done
	// one by one on the fly, and the last line will just be add(1, 1)

	// Clear variables that reference variables to be cleared.
	for (auto const& name: _variables)
		for (auto const& ref: m_referencedBy[name])
			_variables.emplace(ref);

	// Clear the value and update the reference relation.
	for (auto const& name: _variables)
		m_value.erase(name);
	for (auto const& name: _variables)
	{
		for (auto const& ref: m_references[name])
			m_referencedBy[ref].erase(name);
		m_references[name].clear();
	}
}

bool DataFlowAnalyzer::inScope(YulString _variableName) const
{
	for (auto const& scope: m_variableScopes | boost::adaptors::reversed)
	{
		if (scope.variables.count(_variableName))
			return true;
		if (scope.isFunction)
			return false;
	}
	return false;
}
