// SPDX-License-Identifier: GPL-3.0
/**
 * Scopes for identifiers.
 */

#include <libyul/AsmScope.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

bool Scope::registerVariable(YulString _name, YulType const& _type)
{
	if (exists(_name))
		return false;
	Variable variable;
	variable.type = _type;
	identifiers[_name] = variable;
	return true;
}

bool Scope::registerFunction(YulString _name, std::vector<YulType> _arguments, std::vector<YulType> _returns)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Function{std::move(_arguments), std::move(_returns)};
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
			if (crossedFunctionBoundary && holds_alternative<Scope::Variable>(id->second))
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
		if (holds_alternative<Scope::Variable>(identifier.second))
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
