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
#include <set>
#include <boost/noncopyable.hpp>

#include <libsolidity/ASTForward.h>

namespace dev
{
namespace solidity
{

/**
 * Container that stores mappings between names and declarations. It also contains a link to the
 * enclosing scope.
 */
class DeclarationContainer
{
public:
	explicit DeclarationContainer(Declaration const* _enclosingDeclaration = nullptr,
								  DeclarationContainer const* _enclosingContainer = nullptr):
		m_enclosingDeclaration(_enclosingDeclaration), m_enclosingContainer(_enclosingContainer) {}
	/// Registers the declaration in the scope unless its name is already declared or the name is empty.
	/// @param _invisible if true, registers the declaration, reports name clashes but does not return it in @a resolveName
	/// @param _update if true, replaces a potential declaration that is already present
	/// @returns false if the name was already declared.
	bool registerDeclaration(Declaration const& _declaration, bool _invisible = false, bool _update = false);
	std::vector<Declaration const*> resolveName(ASTString const& _name, bool _recursive = false) const;
	Declaration const* getEnclosingDeclaration() const { return m_enclosingDeclaration; }
	std::map<ASTString, std::vector<Declaration const*>> const& getDeclarations() const { return m_declarations; }
	/// @returns whether declaration is valid, and if not also returns previous declaration.
	Declaration const* conflictingDeclaration(Declaration const& _declaration) const;

private:
	Declaration const* m_enclosingDeclaration;
	DeclarationContainer const* m_enclosingContainer;
	std::map<ASTString, std::vector<Declaration const*>> m_declarations;
	std::map<ASTString, std::vector<Declaration const*>> m_invisibleDeclarations;
};

}
}
