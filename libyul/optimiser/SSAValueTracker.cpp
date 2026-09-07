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
 * Component that collects variables that are never assigned to and their
 * initial values.
 */

#include <libyul/optimiser/SSAValueTracker.h>

#include <liblangutil/Exceptions.h>

#include <libyul/AST.h>

#include <libsolutil/Visitor.h>

#include <algorithm>

using namespace solidity;
using namespace solidity::yul;

void SSAValueTracker::operator()(Assignment const& _assignment)
{
	for (auto const& var: _assignment.variableNames)
		m_values.erase(var.name);
}

void SSAValueTracker::operator()(FunctionDefinition const& _funDef)
{
	for (auto const& param: _funDef.parameters)
	{
		solAssert(!m_values.contains(param.name), "Source needs to be disambiguated.");
		// `setValue` is not used here because it interprets `nullptr` as "known to be zero". Intention here is "unset".
		m_values[param.name] = nullptr;
	}

	for (auto const& var: _funDef.returnVariables)
		setValue(var.name, nullptr);
	ASTWalker::operator()(_funDef);
}

void SSAValueTracker::operator()(VariableDeclaration const& _varDecl)
{
	if (!_varDecl.value)
		for (auto const& var: _varDecl.variables)
			setValue(var.name, nullptr);
	else if (_varDecl.variables.size() == 1)
		setValue(_varDecl.variables.front().name, _varDecl.value.get());
}

bool SSAValueTracker::isSSAWithDependencies(Expression const* _expression) const
{
	if (_expression == nullptr)
		return true;

	// Check cache first
	if (
		auto const cacheIt = m_isSSACache.find(_expression);
		cacheIt != m_isSSACache.end()
	)
		return cacheIt->second;

	bool const result = std::visit(
		util::GenericVisitor{
			[&](FunctionCall const& _call) {
				return std::all_of(_call.arguments.begin(), _call.arguments.end(), [&](Expression const& _argument) {
					return isSSAWithDependencies(&_argument);
				});
			},
			[&](Identifier const& _identifier) {
				auto const it = m_values.find(_identifier.name);
				return it == m_values.end() ? false : isSSAWithDependencies(it->second);
			},
			[](Literal const&) { return true; }
		},
		*_expression
	);

	m_isSSACache[_expression] = result;
	return result;
}

std::set<YulName> SSAValueTracker::ssaVariables(Block const& _ast)
{
	SSAValueTracker t;
	t(_ast);
	std::set<YulName> ssaVars;
	for (auto const& value: t.values())
		ssaVars.insert(value.first);
	return ssaVars;
}

void SSAValueTracker::setValue(YulName _name, Expression const* _value)
{
	assertThrow(
		m_values.count(_name) == 0,
		OptimizerException,
		"Source needs to be disambiguated."
	);
	if (!_value)
		_value = &m_zero;
	m_values[_name] = _value;
}
