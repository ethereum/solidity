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

/**
 * Base class that assigns scope to Yul AST
 */
#include <libyul/optimiser/ASTWalker.h>

#include <libyul/AST.h>

namespace solidity::yul
{

class Scoper: public ASTModifier
{
public:
	using ASTModifier::operator();

	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(FunctionDefinition& _fun) override;
	void operator()(Block& _block) override;

protected:
	/// Returns true iff the variable is in scope.
	bool inScope(YulString _variableName) const;

	/// Creates a new inner scope.
	virtual void pushScope(bool _functionScope);
	/// Removes the innermost scope and clears all variables in it.
	virtual void popScope();

	struct Scope
	{
		explicit Scope(bool _isFunction): isFunction(_isFunction) {}
		std::set<YulString> variables;
		bool isFunction;
	};

	std::vector<Scope> m_variableScopes;
};

}
