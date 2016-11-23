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

#pragma once

#include <libevmasm/SourceLocation.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace dev
{
namespace solidity
{

class LocationFinder: private ASTConstVisitor
{
public:
	LocationFinder(SourceLocation const& _location, std::vector<ASTNode const*> _rootNodes):
		m_rootNodes(_rootNodes), m_location(_location)
	{
	}

	/// @returns the "closest" (in the sense of most-leafward) AST node which is a descendant of
	/// _node and whose source location contains _location.
	ASTNode const* leastUpperBound();

private:
	bool visitNode(ASTNode const& _node);

	std::vector<ASTNode const*> m_rootNodes;
	SourceLocation m_location;
	ASTNode const* m_bestMatch = nullptr;
};

}
}
