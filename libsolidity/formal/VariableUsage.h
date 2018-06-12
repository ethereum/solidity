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

#include <map>
#include <set>
#include <vector>

namespace dev
{
namespace solidity
{

class ASTNode;
class VariableDeclaration;

/**
 * This class collects information about which local variables of value type
 * are modified in which parts of the AST.
 */
class VariableUsage
{
public:
	explicit VariableUsage(ASTNode const& _node);

	std::vector<VariableDeclaration const*> touchedVariables(ASTNode const& _node) const;

private:
	// Variable touched by a specific AST node.
	std::map<ASTNode const*, VariableDeclaration const*> m_touchedVariable;
	std::map<ASTNode const*, std::vector<ASTNode const*>> m_children;
};

}
}
