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

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/NameDispenser.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace yul
{
struct Dialect;

/**
 * Creates a copy of a Yul AST replacing all identifiers by unique names.
 */
class Disambiguator: public ASTCopier
{
public:
	explicit Disambiguator(
		Dialect const& _dialect,
		AsmAnalysisInfo const& _analysisInfo,
		std::set<YulString> const& _externallyUsedIdentifiers = {}
	):
		m_info(_analysisInfo),
		m_dialect(_dialect),
		m_externallyUsedIdentifiers(_externallyUsedIdentifiers),
		m_nameDispenser(_dialect, m_externallyUsedIdentifiers)
	{
	}

protected:
	void enterScope(Block const& _block) override;
	void leaveScope(Block const& _block) override;
	void enterFunction(FunctionDefinition const& _function) override;
	void leaveFunction(FunctionDefinition const& _function) override;
	YulString translateIdentifier(YulString _name) override;

	void enterScopeInternal(Scope& _scope);
	void leaveScopeInternal(Scope& _scope);

	AsmAnalysisInfo const& m_info;
	Dialect const& m_dialect;
	std::set<YulString> const& m_externallyUsedIdentifiers;

	std::vector<Scope*> m_scopes;
	std::map<void const*, YulString> m_translations;
	NameDispenser m_nameDispenser;
};

}
