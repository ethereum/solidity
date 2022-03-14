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
#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::lsp
{

enum class DocumentHighlightKind
{
	Unspecified = 0,    //!< could be for example a highlight found in a comment
	Text = 1,           //!< a textual occurrence
	Read = 2,           //!< read access to a variable
	Write = 3,          //!< write access to a variable
};

// Represents a symbol / AST node that is to be highlighted, with some context associated.
using Reference = std::tuple<langutil::SourceLocation, DocumentHighlightKind>;

class ReferenceCollector: public frontend::ASTConstVisitor
{
public:
	ReferenceCollector(frontend::Declaration const& _declaration, std::string const& _sourceIdentifierName);

	static std::vector<Reference> collect(
		frontend::Declaration const* _declaration,
		frontend::ASTNode const& _ast,
		std::string const& _sourceIdentifierName
	);

	static std::vector<Reference> collect(frontend::ASTNode const* _sourceNode, frontend::SourceUnit const& _sourceUnit);

	bool visit(frontend::ImportDirective const& _import) override;
	void endVisit(frontend::ImportDirective const& _import) override;
	void endVisit(frontend::Identifier const& _identifier) override;
	void endVisit(frontend::IdentifierPath  const& _identifierPath) override;
	void endVisit(frontend::MemberAccess const& _memberAccess) override;
	bool visit(frontend::Assignment const& _node) override;
	bool visit(frontend::VariableDeclaration const& _node) override;
	bool visitNode(frontend::ASTNode const& _node) override;

private:
	bool tryAddReference(frontend::Declaration const* _declaration, solidity::langutil::SourceLocation const& _location);

private:
	frontend::Declaration const& m_declaration;
	std::string const& m_sourceIdentifierName;
	std::vector<Reference> m_resultingReferences;
	DocumentHighlightKind m_kind = DocumentHighlightKind::Read;
};

} // end namespace
