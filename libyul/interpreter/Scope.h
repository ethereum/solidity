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

#include <libyul/interpreter/types.h>

#include <libyul/ASTForward.h>
#include <libyul/YulName.h>

#include <libsolutil/Numeric.h>

#include <map>
#include <vector>

namespace solidity::yul::interpreter
{

/**
 * Scope structure built and maintained during execution.
 */
class Scope
{
public:
	Scope(Scope* const _parent = nullptr):
		m_parent(_parent)
	{}

	Scope(Scope* const _parent, Block const& _block);

	Scope* parent() const
	{
		return m_parent;
	}

	Scope* createSubscope(Block const& _block);

	void addDeclaredVariable(YulName const& _name);

	/// Note: m_declaredVariables will be cleared after this function
	void cleanupVariables(VariableValuesMap& _variables);

	/// Throw an error if function with the given name is not found.
	FunctionDefinition const& getFunction(YulName const& _functionName) const;

private:
	std::map<YulName, FunctionDefinition const&> m_definedFunctions{};
	std::vector<YulName> m_declaredVariables{};
	std::map<Block const*, std::unique_ptr<Scope>> m_subScopes{};
	Scope* const m_parent{};
};

}
