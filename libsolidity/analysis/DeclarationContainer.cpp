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
 * @date 2014
 * Scope - object that holds declaration of names.
 */

#include <libsolidity/analysis/DeclarationContainer.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Declaration const* DeclarationContainer::conflictingDeclaration(
	Declaration const& _declaration,
	ASTString const* _name
) const
{
	if (!_name)
		_name = &_declaration.name();
	solAssert(!_name->empty(), "");
	vector<Declaration const*> declarations;
	if (m_declarations.count(*_name))
		declarations += m_declarations.at(*_name);
	if (m_invisibleDeclarations.count(*_name))
		declarations += m_invisibleDeclarations.at(*_name);

	if (dynamic_cast<FunctionDefinition const*>(&_declaration))
	{
		// check that all other declarations with the same name are functions or a public state variable
		for (Declaration const* declaration: declarations)
		{
			if (dynamic_cast<FunctionDefinition const*>(declaration))
				continue;
			if (auto variableDeclaration = dynamic_cast<VariableDeclaration const*>(declaration))
			{
				if (variableDeclaration->isStateVariable() && !variableDeclaration->isConstant() && variableDeclaration->isPublic())
					continue;
				return declaration;
			}
			return declaration;
		}
	}
	else if (declarations.size() == 1 && declarations.front() == &_declaration)
		return nullptr;
	else if (!declarations.empty())
		return declarations.front();

	return nullptr;
}

bool DeclarationContainer::registerDeclaration(
	Declaration const& _declaration,
	ASTString const* _name,
	bool _invisible,
	bool _update
)
{
	if (!_name)
		_name = &_declaration.name();
	if (_name->empty())
		return true;

	if (_update)
	{
		solAssert(!dynamic_cast<FunctionDefinition const*>(&_declaration), "Attempt to update function definition.");
		m_declarations.erase(*_name);
		m_invisibleDeclarations.erase(*_name);
	}
	else if (conflictingDeclaration(_declaration, _name))
		return false;

	vector<Declaration const*>& decls = _invisible ? m_invisibleDeclarations[*_name] : m_declarations[*_name];
	if (!contains(decls, &_declaration))
		decls.push_back(&_declaration);
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
