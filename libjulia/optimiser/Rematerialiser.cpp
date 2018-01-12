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
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 */

#include <libjulia/optimiser/Rematerialiser.h>

#include <libjulia/optimiser/ASTCopier.h>
#include <libjulia/optimiser/NameCollector.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libjulia/optimiser/Semantics.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;

void Rematerialiser::operator()(Assignment& _assignment)
{
	set<string> names;
	for (auto const& var: _assignment.variableNames)
		names.insert(var.name);
	handleAssignment(names, _assignment.value.get());
}

void Rematerialiser::operator()(VariableDeclaration& _varDecl)
{
	set<string> names;
	for (auto const& var: _varDecl.variables)
		names.insert(var.name);
	m_variableScopes.back().first += names;
	handleAssignment(names, _varDecl.value.get());
}

void Rematerialiser::operator()(If& _if)
{
	ASTModifier::operator()(_if);

	Assignments ass;
	ass(_if.body);
	handleAssignment(ass.names(), nullptr);
}

void Rematerialiser::operator()(Switch& _switch)
{
	boost::apply_visitor(*this, *_switch.expression);
	set<string> assignedVariables;
	for (auto& _case: _switch.cases)
	{
		(*this)(_case.body);
		Assignments ass;
		ass(_case.body);
		assignedVariables += ass.names();
		// This is a little too destructive, we could retain the old replacements.
		handleAssignment(ass.names(), nullptr);
	}
	handleAssignment(assignedVariables, nullptr);
}

void Rematerialiser::operator()(FunctionDefinition& _fun)
{
	m_variableScopes.push_back(make_pair(set<string>(), true));
	for (auto const& parameter: _fun.parameters)
		m_variableScopes.back().first.insert(parameter.name);
	for (auto const& var: _fun.returnVariables)
		m_variableScopes.back().first.insert(var.name);
	ASTModifier::operator()(_fun);
	m_variableScopes.pop_back();
}

void Rematerialiser::operator()(ForLoop& _for)
{
	// Special scope handling of the pre block.
	m_variableScopes.push_back(make_pair(set<string>(), false));
	for (auto& statement: _for.pre.statements)
		visit(statement);

	Assignments ass;
	ass(_for.body);
	ass(_for.post);
	handleAssignment(ass.names(), nullptr);

	visit(*_for.condition);
	(*this)(_for.body);
	(*this)(_for.post);

	handleAssignment(ass.names(), nullptr);

	m_variableScopes.pop_back();
}

void Rematerialiser::operator()(Block& _block)
{
	size_t numScopes = m_variableScopes.size();
	m_variableScopes.push_back(make_pair(set<string>(), false));
	ASTModifier::operator()(_block);
	m_variableScopes.pop_back();
	solAssert(numScopes == m_variableScopes.size(), "");
}

void Rematerialiser::handleAssignment(set<string> const& _variables, Expression* _value)
{
	MovableChecker movableChecker;
	if (_value)
	{
		visit(*_value);
		movableChecker.visit(*_value);
	}
	if (_variables.size() == 1)
	{
		string const& name = *_variables.begin();
		if (movableChecker.movable() && _value)
			// TODO Plus heuristic about size of value
			// TODO If _value is null, we could use zero.
			m_substitutions[name] = _value;
		else
			m_substitutions.erase(name);
	}
	// Disallow substitutions that use a variable that will be reassigned by this assignment.
	for (auto const& name: _variables)
		for (auto const& ref: m_referencedBy[name])
			m_substitutions.erase(ref);
	// Update the fact which variables are referenced by the newly assigned variables
	for (auto const& name: _variables)
	{
		for (auto const& ref: m_references[name])
			m_referencedBy[ref].erase(name);
		m_references[name].clear();
	}
	auto const& referencedVariables = movableChecker.referencedVariables();
	for (auto const& name: _variables)
	{
		m_references[name] = referencedVariables;
		for (auto const& ref: referencedVariables)
			m_referencedBy[ref].insert(name);
	}
}

bool Rematerialiser::inScope(string const& _variableName) const
{
	for (auto const& scope: m_variableScopes | boost::adaptors::reversed)
	{
		if (scope.first.count(_variableName))
			return true;
		if (scope.second)
			return false;
	}
	return false;
}

void Rematerialiser::visit(Expression& _e)
{
	if (_e.type() == typeid(Identifier))
	{
		Identifier& identifier = boost::get<Identifier>(_e);
		if (m_substitutions.count(identifier.name))
		{
			string name = identifier.name;
			bool doSubstitute = true;
			for (auto const& ref: m_references[name])
				if (!inScope(ref))
				{
					doSubstitute = false;
					break;
				}
			if (doSubstitute)
				_e = (ASTCopier{}).translate(*m_substitutions.at(name));
		}
	}
	ASTModifier::visit(_e);
}
