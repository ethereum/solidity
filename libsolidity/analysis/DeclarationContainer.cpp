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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Scope - object that holds declaration of names.
 */

#include <libsolidity/analysis/DeclarationContainer.h>

#include <libsolidity/ast/AST.h>
#include <libsolutil/StringUtils.h>

#include <range/v3/view/filter.hpp>
#include <range/v3/range/conversion.hpp>

using namespace solidity;
using namespace solidity::frontend;

Declaration const* DeclarationContainer::conflictingDeclaration(
	Declaration const& _declaration,
	ASTString const* _name
) const
{
	if (!_name)
		_name = &_declaration.name();
	solAssert(!_name->empty(), "");
	std::vector<Declaration const*> declarations;
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
	langutil::SourceLocation const* _location,
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
	else
	{
		if (conflictingDeclaration(_declaration, _name))
			return false;

		if (m_enclosingContainer && _declaration.isVisibleAsUnqualifiedName())
			m_homonymCandidates.emplace_back(*_name, _location ? _location : &_declaration.location());
	}

	std::vector<Declaration const*>& decls = _invisible ? m_invisibleDeclarations[*_name] : m_declarations[*_name];
	if (!util::contains(decls, &_declaration))
		decls.push_back(&_declaration);
	return true;
}

bool DeclarationContainer::registerDeclaration(
	Declaration const& _declaration,
	bool _invisible,
	bool _update
)
{
	return registerDeclaration(_declaration, nullptr, nullptr, _invisible, _update);
}

std::vector<Declaration const*> DeclarationContainer::resolveName(
	ASTString const& _name,
	ResolvingSettings _settings
) const
{
	solAssert(!_name.empty(), "Attempt to resolve empty name.");
	std::vector<Declaration const*> result;

	if (m_declarations.count(_name))
	{
		if (_settings.onlyVisibleAsUnqualifiedNames)
			result += m_declarations.at(_name) | ranges::views::filter(&Declaration::isVisibleAsUnqualifiedName) | ranges::to_vector;
		else
			result += m_declarations.at(_name);
	}

	if (_settings.alsoInvisible && m_invisibleDeclarations.count(_name))
	{
		if (_settings.onlyVisibleAsUnqualifiedNames)
			result += m_invisibleDeclarations.at(_name) | ranges::views::filter(&Declaration::isVisibleAsUnqualifiedName) | ranges::to_vector;
		else
			result += m_invisibleDeclarations.at(_name);
	}

	if (result.empty() && _settings.recursive && m_enclosingContainer)
		result = m_enclosingContainer->resolveName(_name, _settings);

	return result;
}

std::vector<ASTString> DeclarationContainer::similarNames(ASTString const& _name) const
{

	// because the function below has quadratic runtime - it will not magically improve once a better algorithm is discovered ;)
	// since 80 is the suggested line length limit, we use 80^2 as length threshold
	static size_t const MAXIMUM_LENGTH_THRESHOLD = 80 * 80;

	std::vector<ASTString> similar;
	size_t maximumEditDistance = _name.size() > 3 ? 2 : _name.size() / 2;
	for (auto const& declaration: m_declarations)
	{
		std::string const& declarationName = declaration.first;
		if (util::stringWithinDistance(_name, declarationName, maximumEditDistance, MAXIMUM_LENGTH_THRESHOLD))
			similar.push_back(declarationName);
	}
	for (auto const& declaration: m_invisibleDeclarations)
	{
		std::string const& declarationName = declaration.first;
		if (util::stringWithinDistance(_name, declarationName, maximumEditDistance, MAXIMUM_LENGTH_THRESHOLD))
			similar.push_back(declarationName);
	}

	if (m_enclosingContainer)
		similar += m_enclosingContainer->similarNames(_name);

	return similar;
}

void DeclarationContainer::populateHomonyms(std::back_insert_iterator<Homonyms> _it) const
{
	for (DeclarationContainer const* innerContainer: m_innerContainers)
		innerContainer->populateHomonyms(_it);

	for (auto [name, location]: m_homonymCandidates)
	{
		ResolvingSettings settings;
		settings.recursive = true;
		settings.alsoInvisible = true;
		std::vector<Declaration const*> const& declarations = m_enclosingContainer->resolveName(name, std::move(settings));
		if (!declarations.empty())
			_it = make_pair(location, declarations);
	}
}
