// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that makes all identifiers unique.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/NameDispenser.h>

#include <optional>
#include <set>

namespace solidity::yul
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
