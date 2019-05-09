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

#include <libsolidity/ast/ASTVisitor.h>

#include <vector>
#include <set>

namespace dev
{
namespace solidity
{

/**
 * This class computes information about which variables are modified in a certain subtree.
 */
class VariableUsage: private ASTConstVisitor
{
public:
	/// @param _outerCallstack the current callstack in the callers context.
	std::set<VariableDeclaration const*> touchedVariables(ASTNode const& _node, std::vector<CallableDeclaration const*> const& _outerCallstack);

private:
	void endVisit(Identifier const& _node) override;
	void endVisit(IndexAccess const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	void endVisit(ModifierInvocation const& _node) override;
	void endVisit(PlaceholderStatement const& _node) override;

	/// Checks whether an identifier should be added to touchedVariables.
	void checkIdentifier(Identifier const& _identifier);

	std::set<VariableDeclaration const*> m_touchedVariables;
	std::vector<CallableDeclaration const*> m_callStack;
	CallableDeclaration const* m_lastCall = nullptr;
};

}
}
