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

#include <libsolidity/DeclarationContainer.h>
#include <libsolidity/AST.h>
#include <libsolidity/Types.h>

namespace dev
{
namespace solidity
{

bool DeclarationContainer::registerDeclaration(Declaration const& _declaration, bool _invisible, bool _update)
{
	ASTString const& name(_declaration.getName());
	if (name.empty())
		return true;

	if (!_update)
	{
		if (dynamic_cast<FunctionDefinition const*>(&_declaration))
		{
			// other declarations must be FunctionDefinition, otherwise clash with other declarations.
			for (auto&& declaration: m_declarations[_declaration.getName()])
				if (dynamic_cast<FunctionDefinition const*>(declaration) == nullptr)
					return false;
		}
		else if (m_declarations.count(_declaration.getName()) != 0)
				return false;
	}
	else
	{
		// update declaration
		solAssert(dynamic_cast<FunctionDefinition const*>(&_declaration) == nullptr, "cannot be FunctionDefinition");

		m_declarations[_declaration.getName()].clear();
	}

	if (_invisible)
		m_invisibleDeclarations.insert(name);
	else
		m_declarations[_declaration.getName()].insert(&_declaration);

	return true;
}

std::set<Declaration const*> DeclarationContainer::resolveName(ASTString const& _name, bool _recursive) const
{
	solAssert(!_name.empty(), "Attempt to resolve empty name.");
	auto result = m_declarations.find(_name);
	if (result != m_declarations.end())
		return result->second;
	if (_recursive && m_enclosingContainer)
		return m_enclosingContainer->resolveName(_name, true);
	return std::set<Declaration const*>({});
}

}
}
