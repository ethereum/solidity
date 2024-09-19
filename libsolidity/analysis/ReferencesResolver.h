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
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <liblangutil/EVMVersion.h>
#include <libyul/optimiser/ASTWalker.h>

#include <list>
#include <map>
#include <set>
#include <stack>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::frontend
{

class NameAndTypeResolver;

/**
 * Resolves references to declarations (of variables and types) and also establishes the link
 * between a return statement and the return parameter list.
 */
class ReferencesResolver: private ASTConstVisitor, private yul::ASTWalker
{
public:
	ReferencesResolver(
		langutil::ErrorReporter& _errorReporter,
		NameAndTypeResolver& _resolver,
		langutil::EVMVersion _evmVersion,
		bool _resolveInsideCode = false
	):
		m_errorReporter(_errorReporter),
		m_resolver(_resolver),
		m_evmVersion(_evmVersion),
		m_resolveInsideCode(_resolveInsideCode)
	{}

	/// @returns true if no errors during resolving and throws exceptions on fatal errors.
	bool resolve(ASTNode const& _root);

private:
	using yul::ASTWalker::visit;
	using yul::ASTWalker::operator();

	bool visit(Block const& _block) override;
	void endVisit(Block const& _block) override;
	bool visit(TryCatchClause const& _tryCatchClause) override;
	void endVisit(TryCatchClause const& _tryCatchClause) override;
	bool visit(ForStatement const& _for) override;
	void endVisit(ForStatement const& _for) override;
	void endVisit(VariableDeclarationStatement const& _varDeclStatement) override;
	bool visit(VariableDeclaration const& _varDecl) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(FunctionDefinition const& _functionDefinition) override;
	void endVisit(FunctionDefinition const& _functionDefinition) override;
	bool visit(ModifierDefinition const& _modifierDefinition) override;
	void endVisit(ModifierDefinition const& _modifierDefinition) override;
	void endVisit(IdentifierPath const& _path) override;
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(Return const& _return) override;
	bool visit(UsingForDirective const& _usingFor) override;
	bool visit(BinaryOperation const& _binaryOperation) override;

	void operator()(yul::FunctionDefinition const& _function) override;
	void operator()(yul::Identifier const& _identifier) override;
	void operator()(yul::VariableDeclaration const& _varDecl) override;

	void resolveInheritDoc(StructuredDocumentation const& _documentation, StructurallyDocumentedAnnotation& _annotation);

	/// Checks if the name contains a '.'.
	void validateYulIdentifierName(yul::YulName _name, langutil::SourceLocation const& _location);

	langutil::ErrorReporter& m_errorReporter;
	NameAndTypeResolver& m_resolver;
	langutil::EVMVersion m_evmVersion;
	/// Stack of function definitions.
	std::vector<FunctionDefinition const*> m_functionDefinitions;
	// Stack of sets to track local assembly variables and their scopes
	std::stack<std::set<std::string>> m_localAssemblyVariables;
	bool const m_resolveInsideCode;

	InlineAssemblyAnnotation* m_yulAnnotation = nullptr;
	bool m_yulInsideFunction = false;
	bool m_typeContext = false;
};

}
