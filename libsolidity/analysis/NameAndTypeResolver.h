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
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#pragma once

#include <libsolidity/analysis/DeclarationContainer.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/ReferencesResolver.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <liblangutil/EVMVersion.h>

#include <boost/noncopyable.hpp>

#include <list>
#include <map>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Resolves name references, typenames and sets the (explicitly given) types for all variable
 * declarations.
 */
class NameAndTypeResolver: private boost::noncopyable
{
public:
	/// Creates the resolver with the given declarations added to the global scope.
	/// @param _scopes mapping of scopes to be used (usually default constructed), these
	/// are filled during the lifetime of this object.
	NameAndTypeResolver(
		GlobalContext& _globalContext,
		langutil::EVMVersion _evmVersion,
		langutil::ErrorReporter& _errorReporter
	);
	/// Registers all declarations found in the AST node, usually a source unit.
	/// @returns false in case of error.
	/// @param _currentScope should be nullptr but can be used to inject new declarations into
	/// existing scopes, used by the snippets feature.
	bool registerDeclarations(SourceUnit& _sourceUnit, ASTNode const* _currentScope = nullptr);
	/// Applies the effect of import directives.
	bool performImports(SourceUnit& _sourceUnit, std::map<std::string, SourceUnit const*> const& _sourceUnits);
	/// Resolves all names and types referenced from the given Source Node.
	/// @returns false in case of error.
	bool resolveNamesAndTypes(SourceUnit& _source);
	/// Updates the given global declaration (used for "this"). Not to be used with declarations
	/// that create their own scope.
	/// @returns false in case of error.
	bool updateDeclaration(Declaration const& _declaration);
	/// Activates a previously inactive (invisible) variable. To be used in C99 scoping for
	/// VariableDeclarationStatements.
	void activateVariable(std::string const& _name);

	/// Resolves the given @a _name inside the scope @a _scope. If @a _scope is omitted,
	/// the global scope is used (i.e. the one containing only the pre-defined global variables).
	/// @returns a pointer to the declaration on success or nullptr on failure.
	/// SHOULD only be used for testing.
	std::vector<Declaration const*> resolveName(ASTString const& _name, ASTNode const* _scope = nullptr) const;

	/// Resolves a name in the "current" scope, but also searches parent scopes.
	/// Should only be called during the initial resolving phase.
	std::vector<Declaration const*> nameFromCurrentScope(ASTString const& _name, bool _includeInvisibles = false) const;

	/// Resolves a path starting from the "current" scope, but also searches parent scopes.
	/// Should only be called during the initial resolving phase.
	/// @note Returns a null pointer if any component in the path was not unique or not found.
	Declaration const* pathFromCurrentScope(std::vector<ASTString> const& _path) const;

	/// Generate and store warnings about declarations with the same name.
	void warnHomonymDeclarations() const;

	/// @returns a list of similar identifiers in the current and enclosing scopes. May return empty string if no suggestions.
	std::string similarNameSuggestions(ASTString const& _name) const;

	/// Sets the current scope.
	void setScope(ASTNode const* _node);

private:
	/// Internal version of @a resolveNamesAndTypes (called from there) throws exceptions on fatal errors.
	bool resolveNamesAndTypesInternal(ASTNode& _node, bool _resolveInsideCode = true);

	/// Imports all members declared directly in the given contract (i.e. does not import inherited members)
	/// into the current scope if they are not present already.
	void importInheritedScope(ContractDefinition const& _base);

	/// Computes "C3-Linearization" of base contracts and stores it inside the contract. Reports errors if any
	void linearizeBaseContracts(ContractDefinition& _contract);
	/// Computes the C3-merge of the given list of lists of bases.
	/// @returns the linearized vector or an empty vector if linearization is not possible.
	template <class T>
	static std::vector<T const*> cThreeMerge(std::list<std::list<T const*>>& _toMerge);

	/// Maps nodes declaring a scope to scopes, i.e. ContractDefinition and FunctionDeclaration,
	/// where nullptr denotes the global scope. Note that structs are not scope since they do
	/// not contain code.
	/// Aliases (for example `import "x" as y;`) create multiple pointers to the same scope.
	std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>> m_scopes;

	langutil::EVMVersion m_evmVersion;
	DeclarationContainer* m_currentScope = nullptr;
	langutil::ErrorReporter& m_errorReporter;
	GlobalContext& m_globalContext;
};

/**
 * Traverses the given AST upon construction and fills _scopes with all declarations inside the
 * AST.
 */
class DeclarationRegistrationHelper: private ASTVisitor
{
public:
	/// Registers declarations in their scopes and creates new scopes as a side-effect
	/// of construction.
	/// @param _currentScope should be nullptr if we start at SourceUnit, but can be different
	/// to inject new declarations into an existing scope, used by snippets.
	DeclarationRegistrationHelper(
		std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>>& _scopes,
		ASTNode& _astRoot,
		langutil::ErrorReporter& _errorReporter,
		GlobalContext& _globalContext,
		ASTNode const* _currentScope = nullptr
	);

	static bool registerDeclaration(
		DeclarationContainer& _container,
		Declaration const& _declaration,
		std::string const* _name,
		langutil::SourceLocation const* _errorLocation,
		bool _inactive,
		langutil::ErrorReporter& _errorReporter
	);

private:
	bool visit(SourceUnit& _sourceUnit) override;
	void endVisit(SourceUnit& _sourceUnit) override;
	bool visit(ImportDirective& _import) override;
	bool visit(ContractDefinition& _contract) override;
	void endVisit(ContractDefinition& _contract) override;
	void endVisit(VariableDeclarationStatement& _variableDeclarationStatement) override;

	bool visitNode(ASTNode& _node) override;
	void endVisitNode(ASTNode& _node) override;


	void enterNewSubScope(ASTNode& _subScope);
	void closeCurrentScope();
	void registerDeclaration(Declaration& _declaration);

	static bool isOverloadedFunction(Declaration const& _declaration1, Declaration const& _declaration2);

	/// @returns the canonical name of the current scope.
	std::string currentCanonicalName() const;

	std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>>& m_scopes;
	ASTNode const* m_currentScope = nullptr;
	VariableScope* m_currentFunction = nullptr;
	ContractDefinition const* m_currentContract = nullptr;
	langutil::ErrorReporter& m_errorReporter;
	GlobalContext& m_globalContext;
};

}
