// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <tools/solidityUpgrade/UpgradeChange.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/analysis/OverrideChecker.h>

#include <regex>

namespace solidity::tools
{

/**
 * The base upgrade module that can be inherited from. Doing so
 * creates a basic upgrade module that facilitates access to
 * change reporting.
 */
class Upgrade
{
public:
	Upgrade(std::vector<UpgradeChange>& _changes): m_changes(_changes) {}

protected:
	/// A reference to a suite-specific set of changes.
	/// It is passed to all upgrade modules and meant to collect
	/// reported changes.
	std::vector<UpgradeChange>& m_changes;
};

/**
 * A specific upgrade module meant to be run after the analysis phase
 * of the compiler.
 */
class AnalysisUpgrade: public Upgrade, public frontend::ASTConstVisitor
{
public:
	AnalysisUpgrade(std::vector<UpgradeChange>& _changes):
		Upgrade(_changes),
		m_errorReporter(m_errors),
		m_overrideChecker(m_errorReporter)
	{}
	/// Interface function for all upgrade modules that are meant
	/// be run after the analysis phase of the compiler.
	void analyze(frontend::SourceUnit const&) {}
protected:
	langutil::ErrorList m_errors;
	langutil::ErrorReporter m_errorReporter;
	frontend::OverrideChecker m_overrideChecker;
};

/**
 * The generic upgrade suite. Should be inherited from for each set of
 * desired upgrade modules.
 */
class UpgradeSuite
{
public:
	/// The base interface function that needs to be implemented for each
	/// suite. It should create suite-specific upgrade modules and trigger
	/// their analysis.
	void analyze(frontend::SourceUnit const& _sourceUnit);
	/// Resets all changes collected so far.
	void reset() { m_changes.clear(); }

	std::vector<UpgradeChange>& changes() { return m_changes; }
	std::vector<UpgradeChange> const& changes() const { return m_changes; }

protected:
	std::vector<UpgradeChange> m_changes;
};

}
