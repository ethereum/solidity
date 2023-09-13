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
#include <libsolidity/lsp/RenameSymbol.h>
#include <libsolidity/lsp/Utils.h>

#include <libyul/AST.h>

#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::lsp;

namespace
{

CallableDeclaration const* extractCallableDeclaration(FunctionCall const& _functionCall)
{
	if (
		auto const* functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);
		functionType && functionType->hasDeclaration()
	)
		if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(&functionType->declaration()))
			return functionDefinition;

	return nullptr;
}

}

void RenameSymbol::operator()(MessageID _id, Json::Value const& _args)
{
	auto const&& [sourceUnitName, lineColumn] = extractSourceUnitNameAndLineColumn(_args);
	std::string const newName = _args["newName"].asString();
	std::string const uri = _args["textDocument"]["uri"].asString();

	ASTNode const* sourceNode = m_server.astNodeAtSourceLocation(sourceUnitName, lineColumn);

	m_symbolName = {};
	m_declarationToRename = nullptr;
	m_sourceUnits = { &m_server.compilerStack().ast(sourceUnitName) };
	m_locations.clear();

	std::optional<int> cursorBytePosition = charStreamProvider()
		.charStream(sourceUnitName)
		.translateLineColumnToPosition(lineColumn);
	solAssert(cursorBytePosition.has_value(), "Expected source pos");

	extractNameAndDeclaration(*sourceNode, *cursorBytePosition);

	// Find all source units using this symbol
	for (auto const& [name, content]: fileRepository().sourceUnits())
	{
		auto const& sourceUnit = m_server.compilerStack().ast(name);
		for (auto const* referencedSourceUnit: sourceUnit.referencedSourceUnits(true, util::convertContainer<std::set<SourceUnit const*>>(m_sourceUnits)))
			if (*referencedSourceUnit->location().sourceName == sourceUnitName)
			{
				m_sourceUnits.insert(&sourceUnit);
				break;
			}
	}

	// Origin source unit should always be checked
	m_sourceUnits.insert(&m_declarationToRename->sourceUnit());

	Visitor visitor(*this);

	for (auto const* sourceUnit: m_sourceUnits)
		sourceUnit->accept(visitor);

	// Apply changes in reverse order (will iterate in reverse)
	sort(m_locations.begin(), m_locations.end());

	Json::Value reply = Json::objectValue;
	reply["changes"] = Json::objectValue;

	Json::Value edits = Json::arrayValue;

	for (auto i = m_locations.rbegin(); i != m_locations.rend(); i++)
	{
		solAssert(i->isValid());

		// Replace in our file repository
		std::string const uri = fileRepository().sourceUnitNameToUri(*i->sourceName);
		std::string buffer = fileRepository().sourceUnits().at(*i->sourceName);
		buffer.replace((size_t)i->start, (size_t)(i->end - i->start), newName);
		fileRepository().setSourceByUri(uri, std::move(buffer));

		Json::Value edit = Json::objectValue;
		edit["range"] = toRange(*i);
		edit["newText"] = newName;

		// Record changes for the client
		edits.append(edit);
		if (i + 1 == m_locations.rend() || (i + 1)->sourceName != i->sourceName)
		{
			reply["changes"][uri] = edits;
			edits = Json::arrayValue;
		}
	}

	client().reply(_id, reply);
}

void RenameSymbol::extractNameAndDeclaration(ASTNode const& _node, int _cursorBytePosition)
{
	// Identify symbol name and node
	if (auto const* declaration = dynamic_cast<Declaration const*>(&_node))
	{
		if (declaration->nameLocation().containsOffset(_cursorBytePosition))
		{
			m_symbolName = declaration->name();
			m_declarationToRename = declaration;
		}
		else if (auto const* importDirective = dynamic_cast<ImportDirective const*>(declaration))
			extractNameAndDeclaration(*importDirective, _cursorBytePosition);
	}
	else if (auto const* identifier = dynamic_cast<Identifier const*>(&_node))
	{
		if (auto const* declReference = dynamic_cast<Declaration const*>(identifier->annotation().referencedDeclaration))
		{
			m_symbolName = identifier->name();
			m_declarationToRename = declReference;
		}
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(&_node))
		extractNameAndDeclaration(*identifierPath, _cursorBytePosition);
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_node))
	{
		m_symbolName = memberAccess->memberName();
		m_declarationToRename = memberAccess->annotation().referencedDeclaration;
	}
	else if (auto const* functionCall = dynamic_cast<FunctionCall const*>(&_node))
		extractNameAndDeclaration(*functionCall, _cursorBytePosition);
	else if (auto const* inlineAssembly = dynamic_cast<InlineAssembly const*>(&_node))
		extractNameAndDeclaration(*inlineAssembly, _cursorBytePosition);
	else
		solAssert(false, "Unexpected ASTNODE id: " + std::to_string(_node.id()));

	lspDebug(fmt::format("Goal: rename '{}', loc: {}-{}", m_symbolName, m_declarationToRename->nameLocation().start, m_declarationToRename->nameLocation().end));
}

void RenameSymbol::extractNameAndDeclaration(ImportDirective const& _importDirective, int _cursorBytePosition)
{
	for (ImportDirective::SymbolAlias const& symbolAlias: _importDirective.symbolAliases())
		if (symbolAlias.location.containsOffset(_cursorBytePosition))
		{
			solAssert(symbolAlias.alias);
			m_symbolName = *symbolAlias.alias;
			m_declarationToRename = symbolAlias.symbol->annotation().referencedDeclaration;
			break;
		}
}

void RenameSymbol::Visitor::endVisit(ImportDirective const& _node)
{
	// Handles SourceUnit aliases
	if (handleGenericDeclaration(_node))
		return;

	for (ImportDirective::SymbolAlias const& symbolAlias: _node.symbolAliases())
		if (
			symbolAlias.alias != nullptr &&
			*symbolAlias.alias == m_outer.m_symbolName &&
			symbolAlias.symbol->annotation().referencedDeclaration == m_outer.m_declarationToRename
		)
			m_outer.m_locations.emplace_back(symbolAlias.location);
}

void RenameSymbol::extractNameAndDeclaration(FunctionCall const& _functionCall, int _cursorBytePosition)
{
	if (auto const* functionDefinition = extractCallableDeclaration(_functionCall))
		for (size_t i = 0; i < _functionCall.names().size(); i++)
			if (_functionCall.nameLocations()[i].containsOffset(_cursorBytePosition))
			{
				m_symbolName = *_functionCall.names()[i];
				for (size_t j = 0; j < functionDefinition->parameters().size(); j++)
					if (
						functionDefinition->parameters()[j] &&
						functionDefinition->parameters()[j]->name() == m_symbolName
					)
						m_declarationToRename =  functionDefinition->parameters()[j].get();
				return;
			}
}

void RenameSymbol::Visitor::endVisit(FunctionCall const& _node)
{
	SourceLocation nameLocationInFunctionCall;

	for (size_t i = 0; i < _node.names().size(); i++)
		if (_node.names()[i] && *_node.names()[i] == m_outer.m_symbolName)
			nameLocationInFunctionCall = _node.nameLocations()[i];

	if (!nameLocationInFunctionCall.isValid())
		return;

	if (auto const* functionDefinition = extractCallableDeclaration(_node))
		for (size_t j = 0; j < functionDefinition->parameters().size(); j++)
			if (
				functionDefinition->parameters()[j] &&
				*functionDefinition->parameters()[j] == *m_outer.m_declarationToRename
			)
				m_outer.m_locations.emplace_back(nameLocationInFunctionCall);
}

void RenameSymbol::Visitor::endVisit(MemberAccess const& _node)
{
	if (
		m_outer.m_symbolName == _node.memberName() &&
		*m_outer.m_declarationToRename == *_node.annotation().referencedDeclaration
	)
		m_outer.m_locations.emplace_back(_node.memberLocation());
}

void RenameSymbol::Visitor::endVisit(Identifier const& _node)
{
	if (
		m_outer.m_symbolName == _node.name() &&
		*m_outer.m_declarationToRename == *_node.annotation().referencedDeclaration
	)
		m_outer.m_locations.emplace_back(_node.location());
}

void RenameSymbol::extractNameAndDeclaration(IdentifierPath const& _identifierPath, int _cursorBytePosition)
{
	// iterate through the elements of the path to find the one the cursor is on
	size_t numIdentifiers = _identifierPath.pathLocations().size();
	for (size_t i = 0; i < numIdentifiers; i++)
	{
		auto& location = _identifierPath.pathLocations()[i];

		if (location.containsOffset(_cursorBytePosition))
		{
			solAssert(_identifierPath.annotation().pathDeclarations.size() == numIdentifiers);
			solAssert(_identifierPath.path().size() == numIdentifiers);

			m_declarationToRename = _identifierPath.annotation().pathDeclarations[i];
			m_symbolName = _identifierPath.path()[i];
		}
	}
}

void RenameSymbol::Visitor::endVisit(IdentifierPath const& _node)
{
	std::vector<Declaration const*>& declarations = _node.annotation().pathDeclarations;
	solAssert(declarations.size() == _node.path().size());

	for (size_t i = 0; i < _node.path().size(); i++)
		if (
			_node.path()[i] == m_outer.m_symbolName &&
			declarations[i] == m_outer.m_declarationToRename
		)
			m_outer.m_locations.emplace_back(_node.pathLocations()[i]);
}

void RenameSymbol::extractNameAndDeclaration(InlineAssembly const& _inlineAssembly, int _cursorBytePosition)
{
	for (auto&& [identifier, externalReference]: _inlineAssembly.annotation().externalReferences)
	{
		SourceLocation location = yul::nativeLocationOf(*identifier);
		location.end -= static_cast<int>(externalReference.suffix.size() + 1);

		if (location.containsOffset(_cursorBytePosition))
		{
			m_declarationToRename = externalReference.declaration;
			m_symbolName = identifier->name.str();

			if (!externalReference.suffix.empty())
				m_symbolName = m_symbolName.substr(0, m_symbolName.length() - externalReference.suffix.size() - 1);
			break;
		}
	}
}

void RenameSymbol::Visitor::endVisit(InlineAssembly const& _node)
{
	for (auto&& [identifier, externalReference]: _node.annotation().externalReferences)
	{
		std::string identifierName = identifier->name.str();
		if (!externalReference.suffix.empty())
			identifierName = identifierName.substr(0, identifierName.length() - externalReference.suffix.size() - 1);

		if (
			externalReference.declaration == m_outer.m_declarationToRename &&
			identifierName == m_outer.m_symbolName
		)
		{
			SourceLocation location = yul::nativeLocationOf(*identifier);
			location.end -= static_cast<int>(externalReference.suffix.size() + 1);

			m_outer.m_locations.emplace_back(location);
		}
	}

}
