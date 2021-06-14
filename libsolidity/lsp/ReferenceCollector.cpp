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
#include <libsolidity/lsp/ReferenceCollector.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/lsp/LanguageServer.h>

using namespace solidity::frontend;
using namespace std::string_literals;
using namespace std;

namespace solidity::lsp
{

ReferenceCollector::ReferenceCollector(
	frontend::Declaration const& _declaration,
	std::string const& _sourceIdentifierName
):
	m_declaration{_declaration},
	m_sourceIdentifierName{_sourceIdentifierName.empty() ? _declaration.name() : _sourceIdentifierName}
{
}

std::vector<DocumentHighlight> ReferenceCollector::collect(
	frontend::Declaration const* _declaration,
	frontend::ASTNode const& _ast,
	std::string const& _sourceIdentifierName
)
{
	if (!_declaration)
		return {};

	// TODO if vardecl, just use decl's scope (for lower overhead).
	ReferenceCollector collector(*_declaration, _sourceIdentifierName);
	_ast.accept(collector);
	return move(collector.m_result);
}

void ReferenceCollector::endVisit(frontend::ImportDirective const& _import)
{
	for (auto const& symbolAlias: _import.symbolAliases())
		if (m_sourceIdentifierName == *symbolAlias.alias)
		{
			m_result.emplace_back(DocumentHighlight{symbolAlias.location, DocumentHighlightKind::Text});
			break;
		}
}

bool ReferenceCollector::tryAddReference(frontend::Declaration const* _declaration, solidity::langutil::SourceLocation const& _location)
{
	if (&m_declaration != _declaration)
		return false;

	m_result.emplace_back(DocumentHighlight{_location, DocumentHighlightKind::Text});
	return true;
}

void ReferenceCollector::endVisit(frontend::Identifier const& _identifier)
{
	if (auto const* declaration = _identifier.annotation().referencedDeclaration)
		tryAddReference(declaration, _identifier.location());

	for (auto const* declaration: _identifier.annotation().candidateDeclarations + _identifier.annotation().overloadedDeclarations)
		tryAddReference(declaration, _identifier.location());
}

void ReferenceCollector::endVisit(frontend::IdentifierPath  const& _identifierPath)
{
	tryAddReference(_identifierPath.annotation().referencedDeclaration, _identifierPath.location());
}

void ReferenceCollector::endVisit(frontend::MemberAccess const& _memberAccess)
{
	if (_memberAccess.annotation().referencedDeclaration == &m_declaration)
		m_result.emplace_back(DocumentHighlight{_memberAccess.location(), DocumentHighlightKind::Text});
}

bool ReferenceCollector::visitNode(frontend::ASTNode const& _node)
{
	if (&_node == &m_declaration)
	{
		if (auto const* declaration = dynamic_cast<Declaration const*>(&_node))
			m_result.emplace_back(DocumentHighlight{declaration->nameLocation(), DocumentHighlightKind::Text});
		else
			m_result.emplace_back(DocumentHighlight{_node.location(), DocumentHighlightKind::Text});
	}

	return true;
}

}
