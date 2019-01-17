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
#include <libdevcore/StringUtils.h>

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

	if (
		dynamic_cast<FunctionDefinition const*>(&_declaration) ||
		dynamic_cast<EventDefinition const*>(&_declaration) ||
		dynamic_cast<MagicVariableDeclaration const*>(&_declaration)
	)
	{
		// check that all other declarations are of the same kind (in which
		// case the type checker will ensure that the signatures are different)
		for (Declaration const* declaration: declarations)
		{
			if (
				dynamic_cast<FunctionDefinition const*>(&_declaration) &&
				!dynamic_cast<FunctionDefinition const*>(declaration)
			)
				return declaration;
			if (
				dynamic_cast<EventDefinition const*>(&_declaration) &&
				!dynamic_cast<EventDefinition const*>(declaration)
			)
				return declaration;
			if (
				dynamic_cast<MagicVariableDeclaration const*>(&_declaration) &&
				!dynamic_cast<MagicVariableDeclaration const*>(declaration)
			)
				return declaration;
			// Or, continue.
		}
	}
	else if (declarations.size() == 1 && declarations.front() == &_declaration)
		return nullptr;
	else if (!declarations.empty())
		return declarations.front();

	return nullptr;
}

void DeclarationContainer::activateVariable(ASTString const& _name)
{
	solAssert(
		m_invisibleDeclarations.count(_name) && m_invisibleDeclarations.at(_name).size() == 1,
		"Tried to activate a non-inactive variable or multiple inactive variables with the same name."
	);
	solAssert(m_declarations.count(_name) == 0 || m_declarations.at(_name).empty(), "");
	m_declarations[_name].emplace_back(m_invisibleDeclarations.at(_name).front());
	m_invisibleDeclarations.erase(_name);
}

bool DeclarationContainer::isInvisible(ASTString const& _name) const
{
	return m_invisibleDeclarations.count(_name);
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

vector<Declaration const*> DeclarationContainer::resolveName(ASTString const& _name, bool _recursive, bool _alsoInvisible) const
{
	solAssert(!_name.empty(), "Attempt to resolve empty name.");
	vector<Declaration const*> result;
	if (m_declarations.count(_name))
		result = m_declarations.at(_name);
	if (_alsoInvisible && m_invisibleDeclarations.count(_name))
		result += m_invisibleDeclarations.at(_name);
	if (result.empty() && _recursive && m_enclosingContainer)
		result = m_enclosingContainer->resolveName(_name, true, _alsoInvisible);
	return result;
}

vector<ASTString> DeclarationContainer::similarNames(ASTString const& _name) const
{

	// because the function below has quadratic runtime - it will not magically improve once a better algorithm is discovered ;)
	// since 80 is the suggested line length limit, we use 80^2 as length threshold
	static size_t const MAXIMUM_LENGTH_THRESHOLD = 80 * 80;

	vector<ASTString> similar;
	size_t maximumEditDistance = _name.size() > 3 ? 2 : _name.size() / 2;
	for (auto const& declaration: m_declarations)
	{
		string const& declarationName = declaration.first;
		if (stringWithinDistance(_name, declarationName, maximumEditDistance, MAXIMUM_LENGTH_THRESHOLD))
			similar.push_back(declarationName);
	}
	for (auto const& declaration: m_invisibleDeclarations)
	{
		string const& declarationName = declaration.first;
		if (stringWithinDistance(_name, declarationName, maximumEditDistance, MAXIMUM_LENGTH_THRESHOLD))
			similar.push_back(declarationName);
	}

	if (m_enclosingContainer)
		similar += m_enclosingContainer->similarNames(_name);

	return similar;
}
