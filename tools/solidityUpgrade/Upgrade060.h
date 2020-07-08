// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <tools/solidityUpgrade/UpgradeChange.h>
#include <tools/solidityUpgrade/UpgradeSuite.h>

#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::tools
{

/**
 * Module that performs analysis on the AST. Finds abstract contracts that are
 * not marked as such and adds the `abstract` keyword.
 */
class AbstractContract: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;

	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::ContractDefinition const& _contract) override;
};

/**
 * Module that performs analysis on the AST. Finds functions that need to be
 * marked `override` and adds the keyword to the function header.
 */
class OverridingFunction: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;

	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::ContractDefinition const& _contract) override;
};

/**
 * Module that performs analysis on the AST. Finds functions that need to be
 * marked `virtual` and adds the keyword to the function header.
 */
class VirtualFunction: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;

	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::ContractDefinition const& _function) override;
};

}
