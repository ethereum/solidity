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
/**
 * Counts the number of references to a variable.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Scope.h>

namespace solidity::yul
{
struct AsmAnalysisInfo;

/**
 * Counts the number of references to a variable. This includes actual (read) references
 * but also assignments to the variable. It does not include the declaration itself or
 * function parameters, but it does include function return parameters.
 *
 * This component can handle multiple variables of the same name.
 *
 * Can only be applied to strict assembly.
 */
struct VariableReferenceCounter: public yul::ASTWalker
{
public:
	static std::map<Scope::Variable const*, unsigned> run(AsmAnalysisInfo const& _assemblyInfo, Block const& _block)
	{
		VariableReferenceCounter variableReferenceCounter(_assemblyInfo);
		variableReferenceCounter(_block);
		return std::move(variableReferenceCounter.m_variableReferences);
	}

protected:
	void operator()(Block const& _block) override;
	void operator()(Identifier const& _identifier) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;

private:
	explicit VariableReferenceCounter(
		AsmAnalysisInfo const& _assemblyInfo
	): m_info(_assemblyInfo)
	{}

	void increaseRefIfFound(YulString _variableName);

	AsmAnalysisInfo const& m_info;
	Scope* m_scope = nullptr;
	std::map<Scope::Variable const*, unsigned> m_variableReferences;
};

}
