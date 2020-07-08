// SPDX-License-Identifier: GPL-3.0
/**
 * Component that collects variables that are never assigned to and their
 * initial values.
 */

#include <libyul/optimiser/SSAValueTracker.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void SSAValueTracker::operator()(Assignment const& _assignment)
{
	for (auto const& var: _assignment.variableNames)
		m_values.erase(var.name);
}

void SSAValueTracker::operator()(FunctionDefinition const& _funDef)
{
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

set<YulString> SSAValueTracker::ssaVariables(Block const& _ast)
{
	SSAValueTracker t;
	t(_ast);
	set<YulString> ssaVars;
	for (auto const& value: t.values())
		ssaVars.insert(value.first);
	return ssaVars;
}

void SSAValueTracker::setValue(YulString _name, Expression const* _value)
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
