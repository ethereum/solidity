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
	using Contracts = std::set<frontend::ContractDefinition const*, frontend::OverrideChecker::CompareByID>;

	void endVisit(frontend::ContractDefinition const& _contract) override;

	std::string appendOverride(
		frontend::FunctionDefinition const& _function,
		Contracts const& _expectedContracts
	);
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

	std::string appendVirtual(frontend::FunctionDefinition const& _function) const;
};

}
