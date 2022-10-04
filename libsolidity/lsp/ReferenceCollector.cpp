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
#include <libsolidity/lsp/Utils.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/lsp/LanguageServer.h>

#include <libsolutil/Common.h>

#include <typeinfo>

using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace std::string_literals;
using namespace std;

namespace solidity::lsp
{

ReferenceCollector::ReferenceCollector(
	Declaration const& _declaration,
	string const& _identifierAtCursorLocation
):
	m_declaration{_declaration},
	m_identifierAtCursorLocation{_identifierAtCursorLocation.empty() ? _declaration.name() : _identifierAtCursorLocation}
{
}

vector<Reference> ReferenceCollector::collect(
	Declaration const* _declaration,
	ASTNode const& _astSearchRoot,
	string const& _identifierAtCursorLocation
)
{
	if (!_declaration)
		return {};

	ReferenceCollector collector(*_declaration, _identifierAtCursorLocation);
	_astSearchRoot.accept(collector);
	return std::move(collector.m_collectedReferences);
}

vector<Reference> ReferenceCollector::collect(
	ASTNode const* _sourceNode,
	int _sourceOffset,
	SourceUnit const& _sourceUnit
)
{
	if (!_sourceNode)
		return {};

	solAssert(_sourceNode->location().containsOffset(_sourceOffset), "");

	if (auto const* identifier = dynamic_cast<Identifier const*>(_sourceNode))
		return collect(identifier->annotation().referencedDeclaration, _sourceUnit, identifier->name());
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(_sourceNode))
	{
		solAssert(identifierPath->path().size() >= 1, "");
		for (size_t i = 0; i < identifierPath->pathLocations().size(); ++i)
		{
			SourceLocation const location = identifierPath->pathLocations()[i];
			if (location.containsOffset(_sourceOffset))
			{
				Declaration const* declaration = identifierPath->annotation().pathDeclarations.at(i);
				ASTString const& name = identifierPath->path().at(i);
				return collect(declaration, _sourceUnit, name);
			}
		}
		return {};
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(_sourceNode))
		return collect(memberAccess->annotation().referencedDeclaration, _sourceUnit, memberAccess->memberName());
	else if (auto const* declaration = dynamic_cast<Declaration const*>(_sourceNode))
		return collect(declaration, _sourceUnit, declaration->name());
	else
		lspRequire(false, ErrorCode::InternalError, "Unhandled AST node "s + typeid(*_sourceNode).name());
}

bool ReferenceCollector::visit(ImportDirective const& _import)
{
	for (ImportDirective::SymbolAlias const& symbolAlias: _import.symbolAliases())
	{
		if (
			symbolAlias.alias && *symbolAlias.alias == m_identifierAtCursorLocation &&
			symbolAlias.symbol && symbolAlias.symbol->annotation().referencedDeclaration == &m_declaration
		)
			m_collectedReferences.emplace_back(symbolAlias.location, DocumentHighlightKind::Read);
	}
	return false;
}

void ReferenceCollector::endVisit(Identifier const& _identifier)
{
	if (_identifier.annotation().referencedDeclaration == &m_declaration)
		m_collectedReferences.emplace_back(_identifier.location(), m_kind);
}

void ReferenceCollector::endVisit(IdentifierPath const& _identifierPath)
{
	for (size_t i = 0; i < _identifierPath.path().size(); ++i)
	{
		ASTString const& name = _identifierPath.path()[i];
		Declaration const* declaration = i < _identifierPath.annotation().pathDeclarations.size() ?
			_identifierPath.annotation().pathDeclarations.at(i) :
			nullptr;

		if (declaration == &m_declaration && name == m_identifierAtCursorLocation)
			m_collectedReferences.emplace_back(_identifierPath.pathLocations().at(i), m_kind);
	}
}

void ReferenceCollector::endVisit(MemberAccess const& _memberAccess)
{
	if (_memberAccess.annotation().referencedDeclaration == &m_declaration)
		m_collectedReferences.emplace_back(_memberAccess.location(), m_kind);
}

bool ReferenceCollector::visit(Assignment const& _assignment)
{
	auto const getDocumentHighlightKind = [](Expression const& _expression) noexcept -> DocumentHighlightKind {
		if (_expression.annotation().willBeWrittenTo)
			return DocumentHighlightKind::Write;
		return DocumentHighlightKind::Read;
	};

	auto const restoreKind = ScopeGuard{[this, savedKind=m_kind]() { m_kind = savedKind; }};

	m_kind = getDocumentHighlightKind(_assignment.leftHandSide());
	_assignment.leftHandSide().accept(*this);

	m_kind = getDocumentHighlightKind(_assignment.rightHandSide());
	_assignment.rightHandSide().accept(*this);

	return false;
}

bool ReferenceCollector::visit(VariableDeclaration const& _variableDeclaration)
{
	if (&_variableDeclaration == &m_declaration && _variableDeclaration.name() == m_identifierAtCursorLocation)
		m_collectedReferences.emplace_back(_variableDeclaration.nameLocation(), DocumentHighlightKind::Write);
	return true;
}

bool ReferenceCollector::visitNode(ASTNode const& _genericNode)
{
	if (&_genericNode == &m_declaration)
	{
		Declaration const* declaration = dynamic_cast<Declaration const*>(&_genericNode);
		if (declaration->name() == m_identifierAtCursorLocation)
			m_collectedReferences.emplace_back(declaration->nameLocation(), m_kind);
	}

	return true;
}

}
