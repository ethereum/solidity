/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Container of the (implicit and explicit) global objects.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <boost/noncopyable.hpp>
#include <libsolidity/ASTForward.h>

namespace dev
{
namespace solidity
{

class Type; // forward

/**
 * Container for all global objects which look like AST nodes, but are not part of the AST
 * that is currently being compiled.
 * @note must not be destroyed or moved during compilation as its objects can be referenced from
 * other objects.
 */
class GlobalContext: private boost::noncopyable
{
public:
	GlobalContext();
	void setCurrentContract(ContractDefinition const& _contract);
	MagicVariableDeclaration* getCurrentThis() const;

	std::vector<MagicVariableDeclaration const*> getMagicVariables() const;
	/// Returns a vector of all magic variables, excluding "this".
	std::vector<Declaration*> getDeclarations() const;

private:
	std::vector<std::shared_ptr<MagicVariableDeclaration>> m_magicVariables;
	ContractDefinition const* m_currentContract;
	std::map<ContractDefinition const*, std::shared_ptr<MagicVariableDeclaration>> mutable m_thisPointer;
};

}
}
