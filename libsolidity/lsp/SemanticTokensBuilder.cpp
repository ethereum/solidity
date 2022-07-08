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
#include <libsolidity/lsp/SemanticTokensBuilder.h>
#include <libsolidity/lsp/Utils.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/SourceLocation.h>

#include <fmt/format.h>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::lsp
{

namespace
{

optional<SemanticTokenType> semanticTokenTypeForType(frontend::Type const* _type)
{
	if (!_type)
		return nullopt;

	switch (_type->category())
	{
	case frontend::Type::Category::Address: return SemanticTokenType::Class;
	case frontend::Type::Category::Bool: return SemanticTokenType::Number;
	case frontend::Type::Category::Enum: return SemanticTokenType::Enum;
	case frontend::Type::Category::Function: return SemanticTokenType::Function;
	case frontend::Type::Category::Integer: return SemanticTokenType::Number;
	case frontend::Type::Category::RationalNumber: return SemanticTokenType::Number;
	case frontend::Type::Category::StringLiteral: return SemanticTokenType::String;
	case frontend::Type::Category::Struct: return SemanticTokenType::Struct;
	case frontend::Type::Category::Contract: return SemanticTokenType::Class;
	default:
		lspDebug(fmt::format("semanticTokenTypeForType: unknown category: {}", static_cast<unsigned>(_type->category())));
		return SemanticTokenType::Type;
	}
}

SemanticTokenType semanticTokenTypeForExpression(frontend::Type const* _type)
{
	if (!_type)
		return SemanticTokenType::Variable;

	switch (_type->category())
	{
	case frontend::Type::Category::Enum:
		return SemanticTokenType::Enum;
	default:
		return SemanticTokenType::Variable;
	}
}

} // end namespace

Json::Value SemanticTokensBuilder::build(SourceUnit const& _sourceUnit, CharStream const& _charStream)
{
	reset(&_charStream);
	_sourceUnit.accept(*this);
	return m_encodedTokens;
}

void SemanticTokensBuilder::reset(CharStream const* _charStream)
{
	m_encodedTokens = Json::arrayValue;
	m_charStream = _charStream;
	m_lastLine = 0;
	m_lastStartChar = 0;
}

void SemanticTokensBuilder::encode(
	SourceLocation const& _sourceLocation,
	SemanticTokenType _tokenType,
	SemanticTokenModifiers _modifiers
)
{
	/*
		https://microsoft.github.io/language-server-protocol/specifications/specification-3-17/#textDocument_semanticTokens

		// Step-1: Absolute positions
		{ line: 2, startChar:  5, length: 3, tokenType: 0, tokenModifiers: 3 },
		{ line: 2, startChar: 10, length: 4, tokenType: 1, tokenModifiers: 0 },
		{ line: 5, startChar:  2, length: 7, tokenType: 2, tokenModifiers: 0 }

		// Step-2: Relative positions as intermediate step
		{ deltaLine: 2, deltaStartChar: 5, length: 3, tokenType: 0, tokenModifiers: 3 },
		{ deltaLine: 0, deltaStartChar: 5, length: 4, tokenType: 1, tokenModifiers: 0 },
		{ deltaLine: 3, deltaStartChar: 2, length: 7, tokenType: 2, tokenModifiers: 0 }

		// Step-3: final array result
		// 1st token,  2nd token,  3rd token
		[  2,5,3,0,3,  0,5,4,1,0,  3,2,7,2,0 ]

		So traverse through the AST and assign each leaf a token 5-tuple.
	*/

	// solAssert(_sourceLocation.isValid());
	if (!_sourceLocation.isValid())
		return;

	auto const [line, startChar] = m_charStream->translatePositionToLineColumn(_sourceLocation.start);
	auto const length = _sourceLocation.end - _sourceLocation.start;

	lspDebug(fmt::format("encode [{}:{}..{}] {}", line, startChar, length, _tokenType));

	m_encodedTokens.append(line - m_lastLine);
	if (line == m_lastLine)
		m_encodedTokens.append(startChar - m_lastStartChar);
	else
		m_encodedTokens.append(startChar);
	m_encodedTokens.append(length);
	m_encodedTokens.append(static_cast<int>(_tokenType));
	m_encodedTokens.append(static_cast<int>(_modifiers));

	m_lastLine = line;
	m_lastStartChar = startChar;
}

bool SemanticTokensBuilder::visit(frontend::ContractDefinition const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::Class);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::ElementaryTypeName const& _node)
{
	encode(_node.location(), SemanticTokenType::Type);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::ElementaryTypeNameExpression const& _node)
{
	if (auto const tokenType = semanticTokenTypeForType(_node.annotation().type); tokenType.has_value())
		encode(_node.location(), tokenType.value());
	return true;
}

bool SemanticTokensBuilder::visit(frontend::EnumDefinition const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::Enum);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::EnumValue const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::EnumMember);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::ErrorDefinition const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::Event);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::FunctionDefinition const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::Function);
	return true;
}

bool SemanticTokensBuilder::visit(frontend::ModifierDefinition const& _node)
{
	encode(_node.nameLocation(), SemanticTokenType::Modifier);
	return true;
}

void SemanticTokensBuilder::endVisit(frontend::Literal const& _literal)
{
	encode(_literal.location(), SemanticTokenType::Number);
}

void SemanticTokensBuilder::endVisit(frontend::StructuredDocumentation const& _documentation)
{
	encode(_documentation.location(), SemanticTokenType::Comment);
}

void SemanticTokensBuilder::endVisit(frontend::Identifier const& _identifier)
{
	//lspDebug(fmt::format("Identifier: {}, {}..{} cat={}", _identifier.name(), _identifier.location().start, _identifier.location().end, _identifier.annotation().type->category()));

	SemanticTokenModifiers modifiers = SemanticTokenModifiers::None;
	if (_identifier.annotation().isConstant.set() && *_identifier.annotation().isConstant)
	{
		lspDebug("OMG We've found a const!");
		modifiers = modifiers | SemanticTokenModifiers::Readonly;
	}

	encode(_identifier.location(), semanticTokenTypeForExpression(_identifier.annotation().type), modifiers);
}

void SemanticTokensBuilder::endVisit(frontend::IdentifierPath const& _node)
{
	lspDebug(fmt::format("IdentifierPath: identifier path [{}..{}]", _node.location().start, _node.location().end));
	for (size_t i = 0; i < _node.path().size(); ++i)
		lspDebug(fmt::format("  [{}]: {}", i, _node.path().at(i)));
	if (dynamic_cast<EnumDefinition const*>(_node.annotation().referencedDeclaration))
		encode(_node.location(), SemanticTokenType::EnumMember);
	else
		encode(_node.location(), SemanticTokenType::Variable);
}

bool SemanticTokensBuilder::visit(frontend::MemberAccess const& _node)
{
	lspDebug(fmt::format("[{}..{}] MemberAccess({}): {}", _node.location().start, _node.location().end, _node.annotation().referencedDeclaration ?  _node.annotation().referencedDeclaration->name() : "?", _node.memberName()));

	auto const memberNameLength = static_cast<int>(_node.memberName().size());
	auto const memberTokenType = semanticTokenTypeForExpression(_node.annotation().type);

	auto lhsLocation = _node.location();
	lhsLocation.end -= (memberNameLength + 1 /*exclude the dot*/);

	auto rhsLocation = _node.location();
	rhsLocation.start = rhsLocation.end - static_cast<int>(memberNameLength);

	if (memberTokenType == SemanticTokenType::Enum)
	{
		// Special handling for enumeration symbols.
		encode(lhsLocation, SemanticTokenType::Enum);
		encode(rhsLocation, SemanticTokenType::EnumMember);
	}
	else if (memberTokenType == SemanticTokenType::Function)
	{
		// Special handling for function symbols.
		encode(lhsLocation, SemanticTokenType::Variable);
		encode(rhsLocation, memberTokenType);
	}
	else
	{
		encode(rhsLocation, memberTokenType);
	}

	return false; // we handle LHS and RHS explicitly above.
}

void SemanticTokensBuilder::endVisit(PragmaDirective const& _pragma)
{
	encode(_pragma.location(), SemanticTokenType::Macro);
	// NOTE: It would be nice if we could highlight based on the symbols,
	//       such as (version) numerics be different than identifiers.
}

bool SemanticTokensBuilder::visit(frontend::UserDefinedTypeName const& _node)
{
	if (auto const token = semanticTokenTypeForType(_node.annotation().type); token.has_value())
		encode(_node.location(), *token);
	return false;
}

bool SemanticTokensBuilder::visit(frontend::VariableDeclaration const& _node)
{
	lspDebug(fmt::format("VariableDeclaration: {}", _node.name()));

	if (auto const token = semanticTokenTypeForType(_node.typeName().annotation().type); token.has_value())
		encode(_node.typeName().location(), *token);

	encode(_node.nameLocation(), SemanticTokenType::Variable);
	if (_node.overrides())
		_node.overrides()->accept(*this);
	if (_node.value())
		_node.value()->accept(*this);
	return false;
}

} // end namespace
