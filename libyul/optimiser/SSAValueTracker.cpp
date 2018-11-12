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
 * Component that collects variables that are never assigned to and their
 * initial values.
 */

#include <libyul/optimiser/SSAValueTracker.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

void SSAValueTracker::operator()(Assignment const& _assignment)
{
	for (auto const& var: _assignment.variableNames)
		m_values.erase(var.name);
}

void SSAValueTracker::operator()(VariableDeclaration const& _varDecl)
{
	if (_varDecl.variables.size() == 1)
		setValue(_varDecl.variables.front().name, _varDecl.value.get());
	else
		for (auto const& var: _varDecl.variables)
			setValue(var.name, nullptr);
}

void SSAValueTracker::setValue(YulString _name, Expression const* _value)
{
	assertThrow(
		m_values.count(_name) == 0,
		OptimizerException,
		"Source needs to be disambiguated."
	);
	m_values[_name] = _value;
}
