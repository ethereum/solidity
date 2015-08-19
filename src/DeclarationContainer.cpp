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

using namespace std;
using namespace dev;
using namespace dev::solidity;

Declaration const* DeclarationContainer::conflictingDeclaration(Declaration const& _declaration) const
{
	ASTString const& name(_declaration.getName());
	solAssert(!name.empty(), "");
	vector<Declaration const*> declarations;
	if (m_declarations.count(name))
		declarations += m_declarations.at(name);
	if (m_invisibleDeclarations.count(name))
		declarations += m_invisibleDeclarations.at(name);

	if (dynamic_cast<FunctionDefinition const*>(&_declaration))
	{
		// check that all other declarations with the same name are functions
		for (Declaration const* declaration: declarations)
			if (!dynamic_cast<FunctionDefinition const*>(declaration))
				return declaration;
	}
	else if (!declarations.empty())
		return declarations.front();

	return nullptr;
}

bool DeclarationContainer::registerDeclaration(Declaration const& _declaration, bool _invisible, bool _update)
{
	ASTString const& name(_declaration.getName());
	if (name.empty())
		return true;

	if (_update)
	{
		solAssert(!dynamic_cast<FunctionDefinition const*>(&_declaration), "Attempt to update function definition.");
		m_declarations.erase(name);
		m_invisibleDeclarations.erase(name);
	}
	else if (conflictingDeclaration(_declaration))
		return false;

	if (_invisible)
		m_invisibleDeclarations[name].push_back(&_declaration);
	else
		m_declarations[name].push_back(&_declaration);
	return true;
}

std::vector<Declaration const*> DeclarationContainer::resolveName(ASTString const& _name, bool _recursive) const
{
	solAssert(!_name.empty(), "Attempt to resolve empty name.");
	auto result = m_declarations.find(_name);
	if (result != m_declarations.end())
		return result->second;
	if (_recursive && m_enclosingContainer)
		return m_enclosingContainer->resolveName(_name, true);
	return vector<Declaration const*>({});
}
