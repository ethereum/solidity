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

#include <libsolidity/inlineasm/AsmData.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;

void DataFlowAnalyzer::operator()(Assignment& _assignment)
{
	set<string> names;
	for (auto const& var: _assignment.variableNames)
		names.insert(var.name);
	assertThrow(_assignment.value, OptimizerException, "");
	visit(*_assignment.value);
	handleAssignment(names, _assignment.value.get());
}

void DataFlowAnalyzer::operator()(VariableDeclaration& _varDecl)
{
	set<string> names;
	for (auto const& var: _varDecl.variables)
		names.insert(var.name);
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
	set<string> assignedVariables;
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
	m_variableScopes.emplace_back(true);
	for (auto const& parameter: _fun.parameters)
		m_variableScopes.back().variables.insert(parameter.name);
	for (auto const& var: _fun.returnVariables)
		m_variableScopes.back().variables.insert(var.name);
	ASTModifier::operator()(_fun);
	m_variableScopes.pop_back();
}

void DataFlowAnalyzer::operator()(ForLoop& _for)
{
	// Special scope handling of the pre block.
	m_variableScopes.emplace_back(false);
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

	m_variableScopes.pop_back();
}

void DataFlowAnalyzer::operator()(Block& _block)
{
	size_t numScopes = m_variableScopes.size();
	m_variableScopes.emplace_back(false);
	ASTModifier::operator()(_block);
	m_variableScopes.pop_back();
	assertThrow(numScopes == m_variableScopes.size(), OptimizerException, "");
}

void DataFlowAnalyzer::handleAssignment(set<string> const& _variables, Expression* _value)
{
	clearValues(_variables);

	MovableChecker movableChecker;
	if (_value)
		movableChecker.visit(*_value);
	if (_variables.size() == 1)
	{
		string const& name = *_variables.begin();
		// Expression has to be movable and cannot contain a reference
		// to the variable that will be assigned to.
		if (_value && movableChecker.movable() && !movableChecker.referencedVariables().count(name))
			m_value[name] = _value;
	}

	auto const& referencedVariables = movableChecker.referencedVariables();
	for (auto const& name: _variables)
	{
		m_references[name] = referencedVariables;
		for (auto const& ref: referencedVariables)
			m_referencedBy[ref].insert(name);
	}
}

void DataFlowAnalyzer::clearValues(set<string> const& _variables)
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

	set<string> variables = _variables;
	// Clear variables that reference variables to be cleared.
	for (auto const& name: variables)
		for (auto const& ref: m_referencedBy[name])
			variables.insert(ref);

	// Clear the value and update the reference relation.
	for (auto const& name: variables)
		m_value.erase(name);
	for (auto const& name: variables)
	{
		for (auto const& ref: m_references[name])
			m_referencedBy[ref].erase(name);
		m_references[name].clear();
	}
}

bool DataFlowAnalyzer::inScope(string const& _variableName) const
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
