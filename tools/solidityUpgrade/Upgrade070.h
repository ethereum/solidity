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
#pragma once

#include <tools/solidityUpgrade/UpgradeSuite.h>

#include <libsolidity/ast/AST.h>

namespace solidity::tools
{

class DotSyntax: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;
	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::FunctionCall const& _expression) override;
};

class NowKeyword: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;
	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::Identifier const& _expression) override;
};

class ConstructorVisibility: public AnalysisUpgrade
{
public:
	using AnalysisUpgrade::AnalysisUpgrade;
	void analyze(frontend::SourceUnit const& _sourceUnit) { _sourceUnit.accept(*this); }
private:
	void endVisit(frontend::ContractDefinition const& _contract) override;
};

}
