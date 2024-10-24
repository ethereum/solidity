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

#include <libyul/interpreter/Scope.h>

#include <liblangutil/Exceptions.h>

#include <libyul/AST.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::interpreter;

Scope::Scope(Scope* const _parent, Block const& _block)
	: m_parent(_parent)
{
	for (auto const& statement: _block.statements)
		if (auto const* functionDefinition = std::get_if<FunctionDefinition>(&statement))
			m_definedFunctions.emplace(functionDefinition->name, *functionDefinition);
}

Scope* Scope::createSubscope(Block const& _block)
{
	auto [it, isNew] = m_subScopes.try_emplace(&_block, nullptr);
	if (!isNew)
		return it->second.get();

	auto& subScope = it->second;
	subScope = std::make_unique<Scope>(this, _block);
	return subScope.get();
}

void Scope::addDeclaredVariable(YulName const& _name)
{
	m_declaredVariables.push_back(_name);
}

void Scope::cleanupVariables(VariableValuesMap& _variables)
{
	for (YulName const& varName: m_declaredVariables)
		_variables.erase(varName);

	m_declaredVariables.clear();
}

FunctionDefinition const& Scope::getFunction(YulName const& _functionName) const
{
	for (Scope const* scope = this; scope != nullptr; scope = scope->m_parent)
	{
		auto it = scope->m_definedFunctions.find(_functionName);
		if (it != scope->m_definedFunctions.end())
			return it->second;
	}
	solAssert(false);
}
