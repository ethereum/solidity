// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <tools/solidityUpgrade/UpgradeChange.h>
#include <tools/solidityUpgrade/UpgradeSuite.h>

#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::tools
{

/**
 * Module that performs analysis on the AST. It visits all contract
 * definitions and its defined functions and reports a source upgrade,
 * if one of the declared functions is the constructor but does not
 * use the `constructor` keyword.
 */
class ConstructorKeyword: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;

	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::ContractDefinition const& _contract) override;
};

/**
 * Module that performs analysis on the AST. It visits function definitions
 * and reports a source upgrade, if this function's visibility is `public`,
 * but not marked explicitly as such.
 */
class VisibilitySpecifier: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;

	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::FunctionDefinition const& _function) override;
};

}
