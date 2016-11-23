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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Utilities to work with the AST.
 */

#include <libsolidity/ast/ASTUtils.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;



ASTNode const* LocationFinder::leastUpperBound()
{
	m_bestMatch = nullptr;
	for (ASTNode const* rootNode: m_rootNodes)
		rootNode->accept(*this);

	return m_bestMatch;
}

bool LocationFinder::visitNode(const ASTNode& _node)
{
	if (_node.location().contains(m_location))
	{
		m_bestMatch = &_node;
		return true;
	}
	return false;
}
