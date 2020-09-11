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

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::frontend
{

/**
 * AST visitor that assigns syntactic scopes.
 */
class Scoper: private ASTConstVisitor
{
public:
	static void assignScopes(ASTNode const& _astRoot);

private:
	bool visit(ContractDefinition const& _contract) override;
	void endVisit(ContractDefinition const& _contract) override;
	bool visitNode(ASTNode const& _node) override;
	void endVisitNode(ASTNode const& _node) override;

	ContractDefinition const* m_contract = nullptr;
	std::vector<ASTNode const*> m_scopes;
};

}
