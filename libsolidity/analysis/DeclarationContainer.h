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

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>

#include <memory>

namespace solidity::frontend
{

/**
 * Settings for how the function DeclarationContainer::resolveName operates.
 */
struct ResolvingSettings
{
	/// if true and there are no matching declarations in the current container,
	/// recursively searches the enclosing containers as well.
	bool recursive = false;
	/// if true, include invisible declaration in the results.
	bool alsoInvisible = false;
	/// if true, do not include declarations which can never actually be referenced using their
	/// name alone (without being qualified with the name of scope in which they are declared).
	bool onlyVisibleAsUnqualifiedNames = false;
	/// If false, ignores symbols in the global scope that are part of the standard library
	/// (this is set to false with "pragma stdlib").
	bool autoPopulateStdlib = true;
};


/**
 * Container that stores mappings between names and declarations. It also contains a link to the
 * enclosing scope.
 */
class DeclarationContainer
{
public:
	using Homonyms = std::vector<std::pair<langutil::SourceLocation const*, std::vector<Declaration const*>>>;

	DeclarationContainer() = default;
	explicit DeclarationContainer(ASTNode const* _selfNode, ASTNode const* _enclosingNode, DeclarationContainer* _enclosingContainer):
		m_selfNode(_selfNode),
		m_enclosingNode(_enclosingNode),
		m_enclosingContainer(_enclosingContainer)
	{
		if (_enclosingContainer)
			_enclosingContainer->m_innerContainers.emplace_back(this);
	}
	void setStdlibIdentifiers(std::set<std::string> const& _stdlibIdentifiers)
	{
		m_stdlibIdentifiers = &_stdlibIdentifiers;
	}
	/// Registers the declaration in the scope unless its name is already declared or the name is empty.
	/// @param _name the name to register, if nullptr the intrinsic name of @a _declaration is used.
	/// @param _location alternative location, used to point at homonymous declarations.
	/// @param _invisible if true, registers the declaration, reports name clashes but does not return it in @a resolveName.
	/// @param _update if true, replaces a potential declaration that is already present.
	/// @returns false if the name was already declared.
	bool registerDeclaration(Declaration const& _declaration, ASTString const* _name, langutil::SourceLocation const* _location, bool _invisible, bool _update);
	bool registerDeclaration(Declaration const& _declaration, bool _invisible, bool _update);

	/// Finds all declarations that in the current scope can be referred to using specified name.
	/// @param _name the name to look for.
	/// @param _settings see ResolvingSettings
	std::vector<Declaration const*> resolveName(ASTString const& _name, ResolvingSettings _settings = ResolvingSettings{}) const;
	ASTNode const* enclosingNode() const { return m_enclosingNode; }
	DeclarationContainer const* enclosingContainer() const { return m_enclosingContainer; }
	std::map<ASTString, std::vector<Declaration const*>> const& declarations() const { return m_declarations; }
	/// @returns whether declaration is valid, and if not also returns previous declaration.
	Declaration const* conflictingDeclaration(Declaration const& _declaration, ASTString const* _name = nullptr) const;

	/// Activates a previously inactive (invisible) variable. To be used in C99 scoping for
	/// VariableDeclarationStatements.
	void activateVariable(ASTString const& _name);

	/// @returns true if declaration is currently invisible.
	bool isInvisible(ASTString const& _name) const;

	/// @returns existing declaration names similar to @a _name.
	/// Searches this and all parent containers.
	std::vector<ASTString> similarNames(ASTString const& _name) const;

	/// Populates a vector of (location, declaration) pairs, where location is a location of an inner-scope declaration,
	/// and declaration is the corresponding homonymous outer-scope declaration.
	void populateHomonyms(std::back_insert_iterator<Homonyms> _it) const;

private:
	/// Disables autoPopulateStdlib in _settinsg if "pragma stdlib" is active in m_selfNode.
	void updateSettingsBasedOnStdlibPragma(ResolvingSettings& _settings) const;

	ASTNode const* m_selfNode = nullptr;
	ASTNode const* m_enclosingNode = nullptr;
	DeclarationContainer const* m_enclosingContainer = nullptr;
	std::vector<DeclarationContainer const*> m_innerContainers;
	std::map<ASTString, std::vector<Declaration const*>> m_declarations;
	std::map<ASTString, std::vector<Declaration const*>> m_invisibleDeclarations;
	std::set<std::string> const* m_stdlibIdentifiers = nullptr;
	/// List of declarations (name and location) to check later for homonymity.
	std::vector<std::pair<std::string, langutil::SourceLocation const*>> m_homonymCandidates;
};

}
