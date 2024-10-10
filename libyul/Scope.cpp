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
 * Scopes for identifiers.
 */

#include <libyul/Scope.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

bool Scope::registerVariable(YulName _name)
{
	if (exists(_name))
		return false;
	Variable variable;
	variable.name = _name;
	identifiers[_name] = variable;
	return true;
}

bool Scope::registerFunction(YulName _name, size_t _numArguments, size_t _numReturns)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Function{_numArguments, _numReturns, _name};
	return true;
}

Scope::Identifier* Scope::lookup(YulName _name)
{
	bool crossedFunctionBoundary = false;
	for (Scope* s = this; s; s = s->superScope)
	{
		auto id = s->identifiers.find(_name);
		if (id != s->identifiers.end())
		{
			if (crossedFunctionBoundary && std::holds_alternative<Scope::Variable>(id->second))
				return nullptr;
			else
				return &id->second;
		}

		if (s->functionScope)
			crossedFunctionBoundary = true;
	}
	return nullptr;
}

bool Scope::exists(YulName _name) const
{
	if (identifiers.count(_name))
		return true;
	else if (superScope)
		return superScope->exists(_name);
	else
		return false;
}

size_t Scope::numberOfVariables() const
{
	size_t count = 0;
	for (auto const& identifier: identifiers)
		if (std::holds_alternative<Scope::Variable>(identifier.second))
			count++;
	return count;
}

bool Scope::insideFunction() const
{
	for (Scope const* s = this; s; s = s->superScope)
		if (s->functionScope)
			return true;
	return false;
}
