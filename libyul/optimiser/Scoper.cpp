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

#include <libyul/optimiser/Scoper.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

#include <set>

using namespace solidity::yul;
using namespace solidity::util;
using namespace std;

void Scoper::operator()(VariableDeclaration& _varDecl)
{
	set<YulString> names;
	for (auto const& var: _varDecl.variables)
		names.emplace(var.name);
	m_variableScopes.back().variables += names;

	ASTModifier::operator()(_varDecl);
}

void Scoper::operator()(FunctionDefinition& _fun)
{
	pushScope(true);
	for (auto const& parameter: _fun.parameters)
		m_variableScopes.back().variables.emplace(parameter.name);
	for (auto const& var: _fun.returnVariables)
	{
		m_variableScopes.back().variables.emplace(var.name);
	}

	ASTModifier::operator()(_fun);

	popScope();
}

void Scoper::operator()(Block& _block)
{
	size_t numScopes = m_variableScopes.size();
	pushScope(false);
	ASTModifier::operator()(_block);
	popScope();
	yulAssert(numScopes == m_variableScopes.size(), "");
}

void Scoper::pushScope(bool _functionScope)
{
	m_variableScopes.emplace_back(_functionScope);
}

void Scoper::popScope()
{
	m_variableScopes.pop_back();
}

bool Scoper::inScope(YulString _variableName) const
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
