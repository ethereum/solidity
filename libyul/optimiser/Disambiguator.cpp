// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that makes all identifiers unique.
 */

#include <libyul/optimiser/Disambiguator.h>

#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/AsmScope.h>
#include <libyul/Dialect.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

YulString Disambiguator::translateIdentifier(YulString _originalName)
{
	if (m_dialect.builtin(_originalName) || m_externallyUsedIdentifiers.count(_originalName))
		return _originalName;

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
