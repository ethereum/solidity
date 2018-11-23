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
 * Scopes for identifiers.
 */

#include <libyul/AsmScope.h>

using namespace std;
using namespace dev;
using namespace yul;

bool Scope::registerLabel(YulString _name)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Label();
	return true;
}

bool Scope::registerVariable(YulString _name, YulType const& _type)
{
	if (exists(_name))
		return false;
	Variable variable;
	variable.type = _type;
	identifiers[_name] = variable;
	return true;
}

bool Scope::registerFunction(YulString _name, std::vector<YulType> const& _arguments, std::vector<YulType> const& _returns)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Function{_arguments, _returns};
	return true;
}

Scope::Identifier* Scope::lookup(YulString _name)
{
	bool crossedFunctionBoundary = false;
	for (Scope* s = this; s; s = s->superScope)
	{
		auto id = s->identifiers.find(_name);
		if (id != s->identifiers.end())
		{
			if (crossedFunctionBoundary && id->second.type() == typeid(Scope::Variable))
				return nullptr;
			else
				return &id->second;
		}

		if (s->functionScope)
			crossedFunctionBoundary = true;
	}
	return nullptr;
}

bool Scope::exists(YulString _name) const
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
		if (identifier.second.type() == typeid(Scope::Variable))
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
