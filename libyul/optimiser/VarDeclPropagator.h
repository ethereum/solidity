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

#include <libyul/ASTDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmDataForward.h>
#include <vector>
#include <set>
#include <map>

namespace yul
{

/**
 * Rewrites Assignment statements into VariableDeclaration when the assignment's LHS
 * variables had no value yet.
 *
 * It recursively walks through the AST and moves each declaration of variables to
 * the first assignment within the same block (if possible)..
 */
class VarDeclPropagator: public ASTModifier
{
public:
	using ASTModifier::operator();
	void operator()(Block& _block) override;
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Assignment& _assignment) override;
	void operator()(Identifier& _ident) override;

private:
	bool allVarNamesUninitialized(std::vector<Identifier> const& _variableNames) const;
	bool isFullyLazyInitialized(std::vector<Identifier> const& _variableNames) const;
	VariableDeclaration recreateVariableDeclaration(Assignment& _assignment);

private:
	/// Holds a list of variables from current Block that have no value assigned yet.
	std::map<YulString, TypedName> m_emptyVarDecls;

	/// Holds a list variables (and their TypedName) within the current block.
	std::map<YulString, TypedName> m_lazyInitializedVarDecls;
};

}
