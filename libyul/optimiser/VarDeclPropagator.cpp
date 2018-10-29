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

#include <libyul/optimiser/VarDeclPropagator.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libdevcore/CommonData.h>
#include <boost/range/algorithm_ext/erase.hpp>
#include <algorithm>
#include <map>

using namespace std;
using namespace dev;
using namespace dev::yul;

using dev::solidity::assembly::TypedName;
using dev::solidity::assembly::TypedNameList;

void VarDeclPropagator::operator()(Block& _block)
{
	map<YulString, TypedName> outerEmptyVarDecls;
	map<YulString, TypedName> outerLazyInitializedVarDecls;
	swap(m_emptyVarDecls, outerEmptyVarDecls);
	swap(m_lazyInitializedVarDecls, outerLazyInitializedVarDecls);

	ASTModifier::operator()(_block);

	iterateReplacing(
		_block.statements,
		[this](Statement& _stmt) -> boost::optional<vector<Statement>>
		{
			if (_stmt.type() == typeid(VariableDeclaration))
			{
				VariableDeclaration& varDecl = boost::get<VariableDeclaration>(_stmt);
				boost::remove_erase_if(
					varDecl.variables,
					[&](TypedName const& _typedName) { return m_lazyInitializedVarDecls.count(_typedName.name); }
				);
				if (varDecl.variables.empty())
					return vector<Statement>{};
				else
					return {};
			}
			else if (_stmt.type() == typeid(Assignment))
			{
				Assignment& assignment = boost::get<Assignment>(_stmt);
				if (isFullyLazyInitialized(assignment.variableNames))
					return vector<Statement>{recreateVariableDeclaration(assignment)};
				else
					return {};
			}
			else
				return {};
		}
	);

	swap(m_emptyVarDecls, outerEmptyVarDecls);
	swap(m_lazyInitializedVarDecls, outerLazyInitializedVarDecls);
}

void VarDeclPropagator::operator()(VariableDeclaration& _varDecl)
{
	if (_varDecl.value)
		visit(*_varDecl.value);
	else
		for (TypedName const& typedName: _varDecl.variables)
			m_emptyVarDecls[typedName.name] = typedName;
}

void VarDeclPropagator::operator()(Assignment& _assignment)
{
	visit(*_assignment.value);

	if (allVarNamesUninitialized(_assignment.variableNames))
		for (Identifier const& ident: _assignment.variableNames)
			m_lazyInitializedVarDecls[ident.name] = m_emptyVarDecls[ident.name];

	for (Identifier& name: _assignment.variableNames)
		(*this)(name);
}

void VarDeclPropagator::operator()(Identifier& _ident)
{
	m_emptyVarDecls.erase(_ident.name);
}

bool VarDeclPropagator::allVarNamesUninitialized(vector<Identifier> const& _variableNames) const
{
	return all_of(
		begin(_variableNames),
		end(_variableNames),
		[&](Identifier const& _ident) -> bool { return m_emptyVarDecls.count(_ident.name); }
	);
}

bool VarDeclPropagator::isFullyLazyInitialized(vector<Identifier> const& _variableNames) const
{
	return all_of(
		begin(_variableNames),
		end(_variableNames),
		[&](Identifier const& ident) -> bool { return m_lazyInitializedVarDecls.count(ident.name); }
	);
}

VariableDeclaration VarDeclPropagator::recreateVariableDeclaration(Assignment& _assignment)
{
	TypedNameList variables;

	for (Identifier const& varName: _assignment.variableNames)
	{
		variables.emplace_back(move(m_lazyInitializedVarDecls.at(varName.name)));
		m_lazyInitializedVarDecls.erase(varName.name);
	}

	return VariableDeclaration{move(_assignment.location), move(variables), std::move(_assignment.value)};
}
