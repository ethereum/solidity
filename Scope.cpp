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

#include <libsolidity/Scope.h>
#include <libsolidity/AST.h>

namespace dev
{
namespace solidity
{

bool Scope::registerDeclaration(Declaration& _declaration)
{
	if (m_declarations.find(_declaration.getName()) != m_declarations.end())
		return false;
	m_declarations[_declaration.getName()] = &_declaration;
	return true;
}

Declaration* Scope::resolveName(ASTString const& _name, bool _recursive) const
{
	auto result = m_declarations.find(_name);
	if (result != m_declarations.end())
		return result->second;
	if (_recursive && m_outerScope)
		return m_outerScope->resolveName(_name, true);
	return nullptr;
}

}
}
