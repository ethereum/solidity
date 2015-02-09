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
 * Scope - object that holds declaration of names.
 */

#pragma once

#include <map>
#include <boost/noncopyable.hpp>

#include <libsolidity/ASTForward.h>

namespace dev
{
namespace solidity
{

/**
 * Container that stores mappings betwee names and declarations. It also contains a link to the
 * enclosing scope.
 */
class DeclarationContainer
{
public:
	explicit DeclarationContainer(Declaration const* _enclosingDeclaration = nullptr,
								  DeclarationContainer const* _enclosingContainer = nullptr):
		m_enclosingDeclaration(_enclosingDeclaration), m_enclosingContainer(_enclosingContainer) {}
	/// Registers the declaration in the scope unless its name is already declared or the name is empty.
	/// @returns false if the name was already declared.
	bool registerDeclaration(Declaration const& _declaration, bool _update = false);
	Declaration const* resolveName(ASTString const& _name, bool _recursive = false) const;
	Declaration const* getEnclosingDeclaration() const { return m_enclosingDeclaration; }
	std::map<ASTString, Declaration const*> const& getDeclarations() const { return m_declarations; }

private:
	Declaration const* m_enclosingDeclaration;
	DeclarationContainer const* m_enclosingContainer;
	std::map<ASTString, Declaration const*> m_declarations;
};

}
}
