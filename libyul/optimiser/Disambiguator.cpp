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
 * Optimiser component that makes all identifiers unique.
 */

#include <libyul/optimiser/Disambiguator.h>

#include <libyul/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScope.h>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;

using Scope = dev::solidity::assembly::Scope;

string Disambiguator::translateIdentifier(string const& _originalName)
{
	assertThrow(!m_scopes.empty() && m_scopes.back(), OptimizerException, "");
	Scope::Identifier const* id = m_scopes.back()->lookup(_originalName);
	assertThrow(id, OptimizerException, "");
	if (!m_translations.count(id))
		m_translations[id] = m_nameDispenser.newName(_originalName);
	return m_translations.at(id);
}

void Disambiguator::enterScope(Block const& _block)
{
	enterScopeInternal(*m_info.scopes.at(&_block));
}

void Disambiguator::leaveScope(Block const& _block)
{
	leaveScopeInternal(*m_info.scopes.at(&_block));
}

void Disambiguator::enterFunction(FunctionDefinition const& _function)
{
	enterScopeInternal(*m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()));
}

void Disambiguator::leaveFunction(FunctionDefinition const& _function)
{
	leaveScopeInternal(*m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()));
}

void Disambiguator::enterScopeInternal(Scope& _scope)
{
	m_scopes.push_back(&_scope);
}

void Disambiguator::leaveScopeInternal(Scope& _scope)
{
	assertThrow(!m_scopes.empty(), OptimizerException, "");
	assertThrow(m_scopes.back() == &_scope, OptimizerException, "");
	m_scopes.pop_back();
}
