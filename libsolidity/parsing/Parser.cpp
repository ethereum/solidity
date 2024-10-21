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
 * Solidity parser.
 */

#include <libsolidity/parsing/Parser.h>

#include <libsolidity/ast/UserDefinableOperators.h>
#include <libsolidity/interface/Version.h>
#include <libyul/AST.h>
#include <libyul/AsmParser.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SemVerHandler.h>
#include <liblangutil/SourceLocation.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <cctype>
#include <memory>
#include <regex>
#include <tuple>
#include <vector>

using namespace solidity::langutil;
using namespace std::string_literals;

namespace solidity::frontend
{

/// AST node factory that also tracks the begin and end position of an AST node
/// while it is being parsed
class Parser::ASTNodeFactory
{
public:
	explicit ASTNodeFactory(Parser& _parser):
		m_parser(_parser), m_location{
			_parser.currentLocation().start,
			-1,
			_parser.currentLocation().sourceName
		}
	{}
	ASTNodeFactory(Parser& _parser, ASTPointer<ASTNode> const& _childNode):
		m_parser(_parser), m_location{_childNode->location()} {}

	void markEndPosition() { m_location.end = m_parser.currentLocation().end; }
	void setLocation(SourceLocation const& _location) { m_location = _location; }
	void setLocationEmpty() { m_location.end = m_location.start; }
	/// Set the end position to the one of the given node.
	void setEndPositionFromNode(ASTPointer<ASTNode> const& _node) { m_location.end = _node->location().end; }

	template <class NodeType, typename... Args>
	ASTPointer<NodeType> createNode(Args&& ... _args)
	{
		solAssert(m_location.sourceName, "");
		if (m_location.end < 0)
			markEndPosition();
		return std::make_shared<NodeType>(m_parser.nextID(), m_location, std::forward<Args>(_args)...);
	}

	SourceLocation const& location() const noexcept { return m_location; }

private:
	Parser& m_parser;
	SourceLocation m_location;
};

ASTPointer<SourceUnit> Parser::parse(CharStream& _charStream)
{
	solAssert(!m_insideModifier, "");
	try
	{
		m_recursionDepth = 0;
		m_scanner = std::make_shared<Scanner>(_charStream);
		ASTNodeFactory nodeFactory(*this);
		m_experimentalSolidityEnabledInCurrentSourceUnit = false;

		std::vector<ASTPointer<ASTNode>> nodes;
		while (m_scanner->currentToken() == Token::Pragma)
			nodes.push_back(parsePragmaDirective(false));

		if (m_experimentalSolidityEnabledInCurrentSourceUnit)
			m_scanner->setScannerMode(ScannerKind::ExperimentalSolidity);

		while (m_scanner->currentToken() != Token::EOS)
		{
			switch (m_scanner->currentToken())
			{
			case Token::Pragma:
				nodes.push_back(parsePragmaDirective(true));
				break;
			case Token::Import:
				nodes.push_back(parseImportDirective());
				break;
			case Token::Abstract:
			case Token::Interface:
			case Token::Contract:
			case Token::Library:
				nodes.push_back(parseContractDefinition());
				break;
			case Token::Struct:
				nodes.push_back(parseStructDefinition());
				break;
			case Token::Enum:
				nodes.push_back(parseEnumDefinition());
				break;
			case Token::Type:
				if (m_experimentalSolidityEnabledInCurrentSourceUnit)
					nodes.push_back(parseTypeDefinition());
				else
					nodes.push_back(parseUserDefinedValueTypeDefinition());
				break;
			case Token::Using:
				nodes.push_back(parseUsingDirective());
				break;
			case Token::Function:
				nodes.push_back(parseFunctionDefinition(true));
				break;
			case Token::ForAll:
				nodes.push_back(parseQuantifiedFunctionDefinition());
				break;
			case Token::Event:
				nodes.push_back(parseEventDefinition());
				break;
			case Token::Class:
				solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
				nodes.push_back(parseTypeClassDefinition());
				break;
			case Token::Instantiation:
				solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
				nodes.push_back(parseTypeClassInstantiation());
				break;
			default:
				if (
					// Workaround because `error` is not a keyword.
					m_scanner->currentToken() == Token::Identifier &&
					currentLiteral() == "error" &&
					m_scanner->peekNextToken() == Token::Identifier &&
					m_scanner->peekNextNextToken() == Token::LParen
				)
					nodes.push_back(parseErrorDefinition());
				// Constant variable.
				else if (variableDeclarationStart() && m_scanner->peekNextToken() != Token::EOS)
				{
					VarDeclParserOptions options;
					options.kind = VarDeclKind::FileLevel;
					options.allowInitialValue = true;
					nodes.push_back(parseVariableDeclaration(options));
					expectToken(Token::Semicolon);
				}
				else
					fatalParserError(7858_error, "Expected pragma, import directive or contract/interface/library/struct/enum/constant/function/error definition.");
			}
		}
		solAssert(m_recursionDepth == 0, "");
		return nodeFactory.createNode<SourceUnit>(findLicenseString(nodes), nodes, m_experimentalSolidityEnabledInCurrentSourceUnit);
	}
	catch (FatalError const& error)
	{
		solAssert(m_errorReporter.hasErrors(), "Unreported fatal error: "s + error.what());
		return nullptr;
	}
}

void Parser::parsePragmaVersion(SourceLocation const& _location, std::vector<Token> const& _tokens, std::vector<std::string> const& _literals)
{
	SemVerMatchExpressionParser parser(_tokens, _literals);
	try
	{
		SemVerMatchExpression matchExpression = parser.parse();
		static SemVerVersion const currentVersion{std::string(VersionString)};
		// FIXME: only match for major version incompatibility
		if (!matchExpression.matches(currentVersion))
			m_errorReporter.fatalParserError(
				5333_error,
				_location,
				"Source file requires different compiler version (current compiler is " +
				std::string(VersionString) + ") - note that nightly builds are considered to be "
				"strictly less than the released version"
			);
	}
	catch (SemVerError const& matchError)
	{
		m_errorReporter.fatalParserError(
			1684_error,
			_location,
			"Invalid version pragma. "s + matchError.what()
		);
	}
}

ASTPointer<StructuredDocumentation> Parser::parseStructuredDocumentation()
{
	if (m_scanner->currentCommentLiteral() != "")
	{
		ASTNodeFactory nodeFactory{*this};
		nodeFactory.setLocation(m_scanner->currentCommentLocation());
		return nodeFactory.createNode<StructuredDocumentation>(
			std::make_shared<ASTString>(m_scanner->currentCommentLiteral())
		);
	}
	return nullptr;
}

ASTPointer<PragmaDirective> Parser::parsePragmaDirective(bool const _finishedParsingTopLevelPragmas)
{
	RecursionGuard recursionGuard(*this);
	// pragma anything* ;
	// Currently supported:
	// pragma solidity ^0.4.0 || ^0.3.0;
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Pragma);
	std::vector<std::string> literals;
	std::vector<Token> tokens;

	do
	{
		Token token = m_scanner->currentToken();
		if (token == Token::Illegal)
			parserError(6281_error, "Token incompatible with Solidity parser as part of pragma directive.");
		else
		{
			std::string literal = m_scanner->currentLiteral();
			if (literal.empty() && TokenTraits::toString(token))
				literal = TokenTraits::toString(token);
			literals.push_back(literal);
			tokens.push_back(token);
		}
		advance();
	}
	while (m_scanner->currentToken() != Token::Semicolon && m_scanner->currentToken() != Token::EOS);
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);

	if (literals.size() >= 1 && literals[0] == "solidity")
	{
		parsePragmaVersion(
			nodeFactory.location(),
			std::vector<Token>(tokens.begin() + 1, tokens.end()),
			std::vector<std::string>(literals.begin() + 1, literals.end())
		);
	}

	if (literals.size() >= 2 && literals[0] == "experimental" && literals[1] == "solidity")
	{
		if (m_evmVersion < EVMVersion::constantinople())
			fatalParserError(7637_error, "Experimental solidity requires Constantinople EVM version at the minimum.");
		if (_finishedParsingTopLevelPragmas)
			fatalParserError(8185_error, "Experimental pragma \"solidity\" can only be set at the beginning of the source unit.");
		m_experimentalSolidityEnabledInCurrentSourceUnit = true;
	}

	return nodeFactory.createNode<PragmaDirective>(tokens, literals);
}

ASTPointer<ImportDirective> Parser::parseImportDirective()
{
	RecursionGuard recursionGuard(*this);
	// import "abc" [as x];
	// import * as x from "abc";
	// import {a as b, c} from "abc";
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Import);
	ASTPointer<ASTString> path;
	ASTPointer<ASTString> unitAlias = std::make_shared<std::string>();
	SourceLocation unitAliasLocation{};
	ImportDirective::SymbolAliasList symbolAliases;

	if (isQuotedPath() || isStdlibPath())
	{
		path = isQuotedPath() ? getLiteralAndAdvance() : getStdlibImportPathAndAdvance();
		if (m_scanner->currentToken() == Token::As)
		{
			advance();
			tie(unitAlias, unitAliasLocation) = expectIdentifierWithLocation();
		}
	}
	else
	{
		if (m_scanner->currentToken() == Token::LBrace)
		{
			advance();
			while (true)
			{
				ASTPointer<ASTString> alias;
				SourceLocation aliasLocation = currentLocation();
				ASTPointer<Identifier> id = parseIdentifier();
				if (m_scanner->currentToken() == Token::As)
				{
					expectToken(Token::As);
					tie(alias, aliasLocation) = expectIdentifierWithLocation();
				}
				symbolAliases.emplace_back(ImportDirective::SymbolAlias{std::move(id), std::move(alias), aliasLocation});
				if (m_scanner->currentToken() != Token::Comma)
					break;
				advance();
			}
			expectToken(Token::RBrace);
		}
		else if (m_scanner->currentToken() == Token::Mul)
		{
			advance();
			expectToken(Token::As);
			tie(unitAlias, unitAliasLocation) = expectIdentifierWithLocation();
		}
		else
			fatalParserError(9478_error, "Expected string literal (path), \"*\" or alias list.");
		// "from" is not a keyword but parsed as an identifier because of backwards
		// compatibility and because it is a really common word.
		if (m_scanner->currentToken() != Token::Identifier || m_scanner->currentLiteral() != "from")
			fatalParserError(8208_error, "Expected \"from\".");
		advance();
		if (!isQuotedPath() && !isStdlibPath())
			fatalParserError(6845_error, "Expected import path.");
		path = isQuotedPath() ? getLiteralAndAdvance() : getStdlibImportPathAndAdvance();
	}
	if (path->empty())
		fatalParserError(6326_error, "Import path cannot be empty.");
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<ImportDirective>(path, unitAlias, unitAliasLocation, std::move(symbolAliases));
}

std::pair<ContractKind, bool> Parser::parseContractKind()
{
	ContractKind kind;
	bool abstract = false;
	if (m_scanner->currentToken() == Token::Abstract)
	{
		abstract = true;
		advance();
	}
	switch (m_scanner->currentToken())
	{
	case Token::Interface:
		kind = ContractKind::Interface;
		break;
	case Token::Contract:
		kind = ContractKind::Contract;
		break;
	case Token::Library:
		kind = ContractKind::Library;
		break;
	default:
		parserError(3515_error, "Expected keyword \"contract\", \"interface\" or \"library\".");
		return std::make_pair(ContractKind::Contract, abstract);
	}
	advance();
	return std::make_pair(kind, abstract);
}

ASTPointer<Expression> Parser::parseContractStorageBaseLocationExpression()
{
	solAssert(m_scanner->currentLiteral() == "layout");
	expectToken(Token::Identifier);
	if (
		m_scanner->currentToken() != Token::Identifier ||
		m_scanner->currentLiteral() != "at"
	)
		m_errorReporter.parserError(
			1994_error,
			m_scanner->currentLocation(),
			"\'layout\' should be followed by \'at <storage-base-expression>\'."
		);

	advance();
	return parseExpression();
}

ASTPointer<ContractDefinition> Parser::parseContractDefinition()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<ASTString> name =  nullptr;
	SourceLocation nameLocation{};
	ASTPointer<StructuredDocumentation> documentation;
	std::vector<ASTPointer<InheritanceSpecifier>> baseContracts;
	std::vector<ASTPointer<ASTNode>> subNodes;
	std::pair<ContractKind, bool> contractKind{};
	ASTPointer<Expression> storageBaseLocationExpression;
	documentation = parseStructuredDocumentation();
	contractKind = parseContractKind();
	std::tie(name, nameLocation) = expectIdentifierWithLocation();
	while (true)
	{
		if (m_scanner->currentToken() == Token::Is)
		{
			if (baseContracts.size())
				m_errorReporter.parserError(
					6668_error,
					m_scanner->currentLocation(),
					SecondarySourceLocation().append("First inheritance definition is here", baseContracts[0]->location()),
					"Base contracts were already defined previously."
				);
			do
			{
				advance();
				baseContracts.push_back(parseInheritanceSpecifier());
			}
			while (m_scanner->currentToken() == Token::Comma);
		}
		else if (
			m_scanner->currentToken() == Token::Identifier &&
			m_scanner->currentLiteral() == "layout"
		)
		{
			if (storageBaseLocationExpression)
				m_errorReporter.parserError(
					8714_error,
					m_scanner->currentLocation(),
					SecondarySourceLocation().append("Another base location was defined here", storageBaseLocationExpression->location()),
					"Storage base location was already defined previously."
				);

			storageBaseLocationExpression = parseContractStorageBaseLocationExpression();
			subNodes.push_back(storageBaseLocationExpression);
		}
		else
			break;
	}
	expectToken(Token::LBrace);
	while (true)
	{
		Token currentTokenValue = m_scanner->currentToken();
		if (currentTokenValue == Token::RBrace)
			break;
		else if (
			(currentTokenValue == Token::Function && m_scanner->peekNextToken() != Token::LParen) ||
			currentTokenValue == Token::Constructor ||
			currentTokenValue == Token::Receive ||
			currentTokenValue == Token::Fallback
		)
			subNodes.push_back(parseFunctionDefinition());
		else if (currentTokenValue == Token::Struct)
			subNodes.push_back(parseStructDefinition());
		else if (currentTokenValue == Token::Enum)
			subNodes.push_back(parseEnumDefinition());
		else if (currentTokenValue == Token::Type)
			subNodes.push_back(parseUserDefinedValueTypeDefinition());
		else if (
			// Workaround because `error` is not a keyword.
			currentTokenValue == Token::Identifier &&
			currentLiteral() == "error" &&
			m_scanner->peekNextToken() == Token::Identifier &&
			m_scanner->peekNextNextToken() == Token::LParen
		)
			subNodes.push_back(parseErrorDefinition());
		else if (variableDeclarationStart())
		{
			VarDeclParserOptions options;
			options.kind = VarDeclKind::State;
			options.allowInitialValue = true;
			subNodes.push_back(parseVariableDeclaration(options));
			expectToken(Token::Semicolon);
		}
		else if (currentTokenValue == Token::Modifier)
			subNodes.push_back(parseModifierDefinition());
		else if (currentTokenValue == Token::Event)
			subNodes.push_back(parseEventDefinition());
		else if (currentTokenValue == Token::Using)
			subNodes.push_back(parseUsingDirective());
		else
			fatalParserError(9182_error, "Function, variable, struct or modifier declaration expected.");
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<ContractDefinition>(
		name,
		nameLocation,
		documentation,
		baseContracts,
		subNodes,
		contractKind.first,
		contractKind.second,
		storageBaseLocationExpression
	);
}

ASTPointer<InheritanceSpecifier> Parser::parseInheritanceSpecifier()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<IdentifierPath> name(parseIdentifierPath());
	std::unique_ptr<std::vector<ASTPointer<Expression>>> arguments;
	if (m_scanner->currentToken() == Token::LParen)
	{
		advance();
		arguments = std::make_unique<std::vector<ASTPointer<Expression>>>(parseFunctionCallListArguments());
		nodeFactory.markEndPosition();
		expectToken(Token::RParen);
	}
	else
		nodeFactory.setEndPositionFromNode(name);
	return nodeFactory.createNode<InheritanceSpecifier>(name, std::move(arguments));
}

Visibility Parser::parseVisibilitySpecifier()
{
	Visibility visibility(Visibility::Default);
	Token token = m_scanner->currentToken();
	switch (token)
	{
		case Token::Public:
			visibility = Visibility::Public;
			break;
		case Token::Internal:
			visibility = Visibility::Internal;
			break;
		case Token::Private:
			visibility = Visibility::Private;
			break;
		case Token::External:
			visibility = Visibility::External;
			break;
		default:
			solAssert(false, "Invalid visibility specifier.");
	}
	advance();
	return visibility;
}

ASTPointer<OverrideSpecifier> Parser::parseOverrideSpecifier()
{
	solAssert(m_scanner->currentToken() == Token::Override, "");

	ASTNodeFactory nodeFactory(*this);
	std::vector<ASTPointer<IdentifierPath>> overrides;

	nodeFactory.markEndPosition();
	advance();

	if (m_scanner->currentToken() == Token::LParen)
	{
		advance();
		while (true)
		{
			overrides.push_back(parseIdentifierPath());

			if (m_scanner->currentToken() == Token::RParen)
				break;

			expectToken(Token::Comma);
		}

		nodeFactory.markEndPosition();
		expectToken(Token::RParen);
	}

	return nodeFactory.createNode<OverrideSpecifier>(std::move(overrides));
}

StateMutability Parser::parseStateMutability()
{
	StateMutability stateMutability(StateMutability::NonPayable);
	Token token = m_scanner->currentToken();
	switch (token)
	{
		case Token::Payable:
			stateMutability = StateMutability::Payable;
			break;
		case Token::View:
			stateMutability = StateMutability::View;
			break;
		case Token::Pure:
			stateMutability = StateMutability::Pure;
			break;
		default:
			solAssert(false, "Invalid state mutability specifier.");
	}
	advance();
	return stateMutability;
}

Parser::FunctionHeaderParserResult Parser::parseFunctionHeader(bool _isStateVariable)
{
	RecursionGuard recursionGuard(*this);
	FunctionHeaderParserResult result;

	VarDeclParserOptions options;
	options.allowLocationSpecifier = true;
	result.parameters = parseParameterList(options);
	while (true)
	{
		Token token = m_scanner->currentToken();
		if (!_isStateVariable && token == Token::Identifier)
			result.modifiers.push_back(parseModifierInvocation());
		else if (TokenTraits::isVisibilitySpecifier(token))
		{
			if (result.visibility != Visibility::Default)
			{
				// There is the special case of a public state variable of function type.
				// Detect this and return early.
				if (_isStateVariable && (result.visibility == Visibility::External || result.visibility == Visibility::Internal))
					break;
				parserError(
					9439_error,
					"Visibility already specified as \"" +
					Declaration::visibilityToString(result.visibility) +
					"\"."
				);
				advance();
			}
			else
				result.visibility = parseVisibilitySpecifier();
		}
		else if (TokenTraits::isStateMutabilitySpecifier(token))
		{
			if (result.stateMutability != StateMutability::NonPayable)
			{
				parserError(
					9680_error,
					"State mutability already specified as \"" +
					stateMutabilityToString(result.stateMutability) +
					"\"."
				);
				advance();
			}
			else
				result.stateMutability = parseStateMutability();
		}
		else if (!_isStateVariable && token == Token::Override)
		{
			if (result.overrides)
				parserError(1827_error, "Override already specified.");

			result.overrides = parseOverrideSpecifier();
		}
		else if (!_isStateVariable && token == Token::Virtual)
		{
			if (result.isVirtual)
				parserError(6879_error, "Virtual already specified.");

			result.isVirtual = true;
			advance();
		}
		else
			break;
	}
	if (m_experimentalSolidityEnabledInCurrentSourceUnit)
	{
		if (m_scanner->currentToken() == Token::RightArrow)
		{
			advance();
			result.experimentalReturnExpression = parseBinaryExpression();
		}
	}
	else
	{
		if (m_scanner->currentToken() == Token::Returns)
		{
			bool const permitEmptyParameterList = m_experimentalSolidityEnabledInCurrentSourceUnit;
			advance();
			result.returnParameters = parseParameterList(options, permitEmptyParameterList);
		}
		else
			result.returnParameters = createEmptyParameterList();
	}
	return result;
}

ASTPointer<ForAllQuantifier> Parser::parseQuantifiedFunctionDefinition()
{
	solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::ForAll);
	ASTPointer<ParameterList> typeVariableDeclarations = parseParameterList();
	nodeFactory.markEndPosition();

	if (m_scanner->currentToken() != Token::Function)
		fatalParserError(5709_error, "Expected a function definition.");

	ASTPointer<FunctionDefinition> quantifiedFunction = parseFunctionDefinition(true /* _freeFunction */, true /* _allowBody */);

	return nodeFactory.createNode<ForAllQuantifier>(
		std::move(typeVariableDeclarations),
		std::move(quantifiedFunction)
	);
}

ASTPointer<FunctionDefinition> Parser::parseFunctionDefinition(bool _freeFunction, bool _allowBody)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();

	Token kind = m_scanner->currentToken();
	ASTPointer<ASTString> name;
	SourceLocation nameLocation;
	if (kind == Token::Function)
	{
		advance();
		if (
			m_scanner->currentToken() == Token::Constructor ||
			m_scanner->currentToken() == Token::Fallback ||
			m_scanner->currentToken() == Token::Receive
		)
		{
			std::string expected = std::map<Token, std::string>{
				{Token::Constructor, "constructor"},
				{Token::Fallback, "fallback function"},
				{Token::Receive, "receive function"},
			}.at(m_scanner->currentToken());
			nameLocation = currentLocation();
			name = std::make_shared<ASTString>(TokenTraits::toString(m_scanner->currentToken()));
			std::string message{
				"This function is named \"" + *name + "\" but is not the " + expected + " of the contract. "
				"If you intend this to be a " + expected + ", use \"" + *name + "(...) { ... }\" without "
				"the \"function\" keyword to define it."
			};
			if (m_scanner->currentToken() == Token::Constructor)
				parserError(3323_error, message);
			else
				parserWarning(3445_error, message);
			advance();
		}
		else
			tie(name, nameLocation) = expectIdentifierWithLocation();
	}
	else
	{
		solAssert(kind == Token::Constructor || kind == Token::Fallback || kind == Token::Receive, "");
		advance();
		name = std::make_shared<ASTString>();
	}

	FunctionHeaderParserResult header = parseFunctionHeader(false);

	if (m_experimentalSolidityEnabledInCurrentSourceUnit)
		solAssert(!header.returnParameters);
	else
		solAssert(!header.experimentalReturnExpression);

	ASTPointer<Block> block;
	nodeFactory.markEndPosition();
	if (!_allowBody)
		expectToken(Token::Semicolon);
	else if (m_scanner->currentToken() == Token::Semicolon)
		advance();
	else
	{
		block = parseBlock();
		nodeFactory.setEndPositionFromNode(block);
	}
	return nodeFactory.createNode<FunctionDefinition>(
		name,
		nameLocation,
		header.visibility,
		header.stateMutability,
		_freeFunction,
		kind,
		header.isVirtual,
		header.overrides,
		documentation,
		header.parameters,
		header.modifiers,
		header.returnParameters,
		block,
		header.experimentalReturnExpression
	);
}

ASTPointer<StructDefinition> Parser::parseStructDefinition()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();
	expectToken(Token::Struct);
	auto [name, nameLocation] = expectIdentifierWithLocation();
	std::vector<ASTPointer<VariableDeclaration>> members;
	expectToken(Token::LBrace);
	while (m_scanner->currentToken() != Token::RBrace)
	{
		members.push_back(parseVariableDeclaration());
		expectToken(Token::Semicolon);
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<StructDefinition>(std::move(name), std::move(nameLocation), std::move(members), std::move(documentation));
}

ASTPointer<EnumValue> Parser::parseEnumValue()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<EnumValue>(expectIdentifierToken());
}

ASTPointer<EnumDefinition> Parser::parseEnumDefinition()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();
	expectToken(Token::Enum);
	auto [name, nameLocation] = expectIdentifierWithLocation();
	std::vector<ASTPointer<EnumValue>> members;
	expectToken(Token::LBrace);

	while (m_scanner->currentToken() != Token::RBrace)
	{
		members.push_back(parseEnumValue());
		if (m_scanner->currentToken() == Token::RBrace)
			break;
		expectToken(Token::Comma);
		if (m_scanner->currentToken() != Token::Identifier)
			fatalParserError(1612_error, "Expected identifier after ','");
	}
	if (members.empty())
		parserError(3147_error, "Enum with no members is not allowed.");

	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<EnumDefinition>(name, nameLocation, members, documentation);
}

ASTPointer<VariableDeclaration> Parser::parseVariableDeclaration(
	VarDeclParserOptions const& _options,
	ASTPointer<TypeName> const& _lookAheadArrayType
)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory = _lookAheadArrayType ?
		ASTNodeFactory(*this, _lookAheadArrayType) : ASTNodeFactory(*this);

	ASTPointer<StructuredDocumentation> const documentation = parseStructuredDocumentation();
	ASTPointer<TypeName> type = _lookAheadArrayType ? _lookAheadArrayType : parseTypeName();
	nodeFactory.setEndPositionFromNode(type);

	if (dynamic_cast<FunctionTypeName*>(type.get()) && _options.kind == VarDeclKind::State && m_scanner->currentToken() == Token::LBrace)
		fatalParserError(
			2915_error,
			"Expected a state variable declaration. If you intended this as a fallback function "
			"or a function to handle plain ether transactions, use the \"fallback\" keyword "
			"or the \"receive\" keyword instead."
		);

	bool isIndexed = false;
	VariableDeclaration::Mutability mutability = VariableDeclaration::Mutability::Mutable;
	ASTPointer<OverrideSpecifier> overrides = nullptr;
	Visibility visibility(Visibility::Default);
	VariableDeclaration::Location location = VariableDeclaration::Location::Unspecified;
	ASTPointer<ASTString> identifier;
	SourceLocation nameLocation{};

	while (true)
	{
		Token token = m_scanner->currentToken();
		if (_options.kind == VarDeclKind::State && TokenTraits::isVariableVisibilitySpecifier(token))
		{
			nodeFactory.markEndPosition();
			if (visibility != Visibility::Default)
			{
				parserError(
					4110_error,
					"Visibility already specified as \"" +
					Declaration::visibilityToString(visibility) +
					"\"."
				);
				advance();
			}
			else
				visibility = parseVisibilitySpecifier();
		}
		else if (_options.kind == VarDeclKind::State && token == Token::Override)
		{
			if (overrides)
				parserError(9125_error, "Override already specified.");

			overrides = parseOverrideSpecifier();
		}
		else
		{
			if (_options.allowIndexed && token == Token::Indexed)
			{
				if (isIndexed)
					parserError(5399_error, "Indexed already specified.");

				isIndexed = true;
			}
			else if (token == Token::Constant || token == Token::Immutable)
			{
				if (mutability != VariableDeclaration::Mutability::Mutable)
					parserError(
						3109_error,
						std::string("Mutability already set to ") +
						(mutability == VariableDeclaration::Mutability::Constant ? "\"constant\"" : "\"immutable\"")
					);
				else if (token == Token::Constant)
					mutability = VariableDeclaration::Mutability::Constant;
				else if (token == Token::Immutable)
					mutability = VariableDeclaration::Mutability::Immutable;
			}
			else if (_options.allowLocationSpecifier && TokenTraits::isLocationSpecifier(token))
			{
				if (location != VariableDeclaration::Location::Unspecified)
					parserError(3548_error, "Location already specified.");
				else
				{
					switch (token)
					{
					case Token::Storage:
						location = VariableDeclaration::Location::Storage;
						break;
					case Token::Memory:
						location = VariableDeclaration::Location::Memory;
						break;
					case Token::CallData:
						location = VariableDeclaration::Location::CallData;
						break;
					default:
						solAssert(false, "Unknown data location.");
					}
				}
			}
			else if (
				_options.kind == VarDeclKind::State &&
				token == Token::Identifier &&
				m_scanner->currentLiteral() == "transient" &&
				m_scanner->peekNextToken() != Token::Assign &&
				m_scanner->peekNextToken() != Token::Semicolon
			)
			{
				if (location != VariableDeclaration::Location::Unspecified)
					parserError(ErrorId{3548}, "Location already specified.");
				else
					location = VariableDeclaration::Location::Transient;
			}
			else
				break;
			nodeFactory.markEndPosition();
			advance();
		}
	}

	if (_options.allowEmptyName && m_scanner->currentToken() != Token::Identifier)
		identifier = std::make_shared<ASTString>("");
	else
	{
		nodeFactory.markEndPosition();
		tie(identifier, nameLocation) = expectIdentifierWithLocation();
	}
	ASTPointer<Expression> value;
	if (_options.allowInitialValue)
	{
		if (m_scanner->currentToken() == Token::Assign)
		{
			advance();
			value = parseExpression();
			nodeFactory.setEndPositionFromNode(value);
		}
	}
	return nodeFactory.createNode<VariableDeclaration>(
		type,
		identifier,
		nameLocation,
		value,
		visibility,
		documentation,
		isIndexed,
		mutability,
		overrides,
		location
	);
}

ASTPointer<ModifierDefinition> Parser::parseModifierDefinition()
{
	RecursionGuard recursionGuard(*this);
	ScopeGuard resetModifierFlag([this]() { m_insideModifier = false; });
	m_insideModifier = true;

	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();

	expectToken(Token::Modifier);
	auto [name, nameLocation] = expectIdentifierWithLocation();
	ASTPointer<ParameterList> parameters;
	if (m_scanner->currentToken() == Token::LParen)
	{
		VarDeclParserOptions options;
		options.allowLocationSpecifier = true;
		parameters = parseParameterList(options);
	}
	else
		parameters = createEmptyParameterList();

	ASTPointer<OverrideSpecifier> overrides;
	bool isVirtual = false;

	while (true)
	{
		if (m_scanner->currentToken() == Token::Override)
		{
			if (overrides)
				parserError(9102_error, "Override already specified.");
			overrides = parseOverrideSpecifier();
		}
		else if (m_scanner->currentToken() == Token::Virtual)
		{
			if (isVirtual)
				parserError(2662_error, "Virtual already specified.");

			isVirtual = true;
			advance();
		}
		else
			break;
	}

	ASTPointer<Block> block;
	nodeFactory.markEndPosition();
	if (m_scanner->currentToken() != Token::Semicolon)
	{
		block = parseBlock();
		nodeFactory.setEndPositionFromNode(block);
	}
	else
		advance(); // just consume the ';'

	return nodeFactory.createNode<ModifierDefinition>(name, nameLocation, documentation, parameters, isVirtual, overrides, block);
}

std::pair<ASTPointer<ASTString>, SourceLocation> Parser::expectIdentifierWithLocation()
{
	SourceLocation nameLocation = currentLocation();
	ASTPointer<ASTString> name = expectIdentifierToken();

	return {std::move(name), std::move(nameLocation)};
}

ASTPointer<EventDefinition> Parser::parseEventDefinition()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();

	expectToken(Token::Event);
	auto [name, nameLocation] = expectIdentifierWithLocation();

	VarDeclParserOptions options;
	options.allowIndexed = true;
	ASTPointer<ParameterList> parameters = parseParameterList(options);

	bool anonymous = false;
	if (m_scanner->currentToken() == Token::Anonymous)
	{
		anonymous = true;
		advance();
	}
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<EventDefinition>(name, nameLocation, documentation, parameters, anonymous);
}

ASTPointer<ErrorDefinition> Parser::parseErrorDefinition()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<StructuredDocumentation> documentation = parseStructuredDocumentation();

	solAssert(*expectIdentifierToken() == "error", "");
	auto&& [name, nameLocation] = expectIdentifierWithLocation();

	ASTPointer<ParameterList> parameters = parseParameterList({});
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<ErrorDefinition>(name, std::move(nameLocation), documentation, parameters);
}

ASTPointer<UsingForDirective> Parser::parseUsingDirective()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::Using);

	std::vector<ASTPointer<IdentifierPath>> functions;
	std::vector<std::optional<Token>> operators;
	bool const usesBraces = m_scanner->currentToken() == Token::LBrace;
	if (usesBraces)
	{
		do
		{
			advance();
			functions.emplace_back(parseIdentifierPath());
			if (m_scanner->currentToken() == Token::As)
			{
				advance();
				Token operator_ = m_scanner->currentToken();
				if (!util::contains(userDefinableOperators, operator_))
				{
					std::string operatorName;
					if (!m_scanner->currentLiteral().empty())
						operatorName = m_scanner->currentLiteral();
					else if (char const* tokenString = TokenTraits::toString(operator_))
						operatorName = std::string(tokenString);

					parserError(
						4403_error,
						fmt::format(
							"Not a user-definable operator: {}. Only the following operators can be user-defined: {}",
							operatorName,
							util::joinHumanReadable(userDefinableOperators | ranges::views::transform([](Token _t) { return std::string{TokenTraits::toString(_t)}; }))
						)
					);
				}
				operators.emplace_back(operator_);
				advance();
			}
			else
				operators.emplace_back(std::nullopt);
		}
		while (m_scanner->currentToken() == Token::Comma);
		expectToken(Token::RBrace);
	}
	else
	{
		functions.emplace_back(parseIdentifierPath());
		operators.emplace_back(std::nullopt);
	}

	ASTPointer<TypeName> typeName;
	expectToken(Token::For);
	if (m_scanner->currentToken() == Token::Mul)
		advance();
	else
		typeName = parseTypeName();
	bool global = false;
	if (m_scanner->currentToken() == Token::Identifier && currentLiteral() == "global")
	{
		global = true;
		advance();
	}
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<UsingForDirective>(std::move(functions), std::move(operators), usesBraces, typeName, global);
}

ASTPointer<ModifierInvocation> Parser::parseModifierInvocation()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<IdentifierPath> name(parseIdentifierPath());
	std::unique_ptr<std::vector<ASTPointer<Expression>>> arguments;
	if (m_scanner->currentToken() == Token::LParen)
	{
		advance();
		arguments = std::make_unique<std::vector<ASTPointer<Expression>>>(parseFunctionCallListArguments());
		nodeFactory.markEndPosition();
		expectToken(Token::RParen);
	}
	else
		nodeFactory.setEndPositionFromNode(name);
	return nodeFactory.createNode<ModifierInvocation>(name, std::move(arguments));
}

ASTPointer<Identifier> Parser::parseIdentifier()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<Identifier>(expectIdentifierToken());
}

ASTPointer<Identifier> Parser::parseIdentifierOrAddress()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();
	return nodeFactory.createNode<Identifier>(expectIdentifierTokenOrAddress());
}

ASTPointer<UserDefinedTypeName> Parser::parseUserDefinedTypeName()
{
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<IdentifierPath> identifierPath = parseIdentifierPath();
	nodeFactory.setEndPositionFromNode(identifierPath);
	return nodeFactory.createNode<UserDefinedTypeName>(identifierPath);
}

ASTPointer<UserDefinedValueTypeDefinition> Parser::parseUserDefinedValueTypeDefinition()
{
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Type);
	auto&& [name, nameLocation] = expectIdentifierWithLocation();
	expectToken(Token::Is);
	ASTPointer<TypeName> typeName = parseTypeName();
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<UserDefinedValueTypeDefinition>(
		name,
		std::move(nameLocation),
		typeName
	);
}

ASTPointer<IdentifierPath> Parser::parseIdentifierPath()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.markEndPosition();

	auto [name, nameLocation] = expectIdentifierWithLocation();

	std::vector<ASTString> identifierPath{*name};
	std::vector<SourceLocation> identifierPathLocations{nameLocation};

	while (m_scanner->currentToken() == Token::Period)
	{
		advance();

		nodeFactory.markEndPosition();

		tie(name, nameLocation) = expectIdentifierWithLocation();

		identifierPath.push_back(*name);
		identifierPathLocations.push_back(nameLocation);
	}
	return nodeFactory.createNode<IdentifierPath>(identifierPath, identifierPathLocations);
}

ASTPointer<TypeName> Parser::parseTypeNameSuffix(ASTPointer<TypeName> type, ASTNodeFactory& nodeFactory)
{
	RecursionGuard recursionGuard(*this);
	while (m_scanner->currentToken() == Token::LBrack)
	{
		advance();
		ASTPointer<Expression> length;
		if (m_scanner->currentToken() != Token::RBrack)
			length = parseExpression();
		nodeFactory.markEndPosition();
		expectToken(Token::RBrack);
		type = nodeFactory.createNode<ArrayTypeName>(type, length);
	}
	return type;
}

ASTPointer<TypeName> Parser::parseTypeName()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<TypeName> type;
	Token token = m_scanner->currentToken();
	if (TokenTraits::isElementaryTypeName(token))
	{
		unsigned firstSize;
		unsigned secondSize;
		std::tie(firstSize, secondSize) = m_scanner->currentTokenInfo();
		ElementaryTypeNameToken elemTypeName(token, firstSize, secondSize);
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.markEndPosition();
		advance();
		auto stateMutability = elemTypeName.token() == Token::Address
			? std::optional<StateMutability>{StateMutability::NonPayable}
			: std::nullopt;
		if (TokenTraits::isStateMutabilitySpecifier(m_scanner->currentToken()))
		{
			if (elemTypeName.token() == Token::Address)
			{
				nodeFactory.markEndPosition();
				stateMutability = parseStateMutability();
			}
			else
			{
				parserError(9106_error, "State mutability can only be specified for address types.");
				advance();
			}
		}
		type = nodeFactory.createNode<ElementaryTypeName>(elemTypeName, stateMutability);
	}
	else if (token == Token::Function)
		type = parseFunctionType();
	else if (token == Token::Mapping)
		type = parseMapping();
	else if (token == Token::Identifier)
		type = parseUserDefinedTypeName();
	else
		fatalParserError(3546_error, "Expected type name");

	solAssert(type, "");
	// Parse "[...]" postfixes for arrays.
	type = parseTypeNameSuffix(type, nodeFactory);

	return type;
}

ASTPointer<FunctionTypeName> Parser::parseFunctionType()
{
	solAssert(!m_experimentalSolidityEnabledInCurrentSourceUnit);
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Function);
	FunctionHeaderParserResult header = parseFunctionHeader(true);
	solAssert(!header.experimentalReturnExpression);
	return nodeFactory.createNode<FunctionTypeName>(
		header.parameters,
		header.returnParameters,
		header.visibility,
		header.stateMutability
	);
}

ASTPointer<Mapping> Parser::parseMapping()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Mapping);
	expectToken(Token::LParen);
	ASTPointer<TypeName> keyType;
	Token token = m_scanner->currentToken();
	unsigned firstSize;
	unsigned secondSize;
	std::tie(firstSize, secondSize) = m_scanner->currentTokenInfo();
	if (token == Token::Identifier)
		keyType = parseUserDefinedTypeName();
	else if (TokenTraits::isElementaryTypeName(token))
	{
		keyType = ASTNodeFactory(*this).createNode<ElementaryTypeName>(
			ElementaryTypeNameToken{token, firstSize, secondSize}
		);
		advance();
	}
	else
		fatalParserError(1005_error, "Expected elementary type name or identifier for mapping key type");
	ASTPointer<ASTString> keyName = std::make_shared<ASTString>("");
	SourceLocation keyNameLocation{};
	if (m_scanner->currentToken() == Token::Identifier)
		tie(keyName, keyNameLocation) = expectIdentifierWithLocation();
	expectToken(Token::DoubleArrow);
	ASTPointer<TypeName> valueType = parseTypeName();
	ASTPointer<ASTString> valueName = std::make_shared<ASTString>("");
	SourceLocation valueNameLocation{};
	if (m_scanner->currentToken() == Token::Identifier)
		tie(valueName, valueNameLocation) = expectIdentifierWithLocation();
	nodeFactory.markEndPosition();
	expectToken(Token::RParen);
	return nodeFactory.createNode<Mapping>(keyType, keyName, keyNameLocation, valueType, valueName, valueNameLocation);
}

ASTPointer<ParameterList> Parser::parseParameterList(
	VarDeclParserOptions const& _options,
	bool _allowEmpty
)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	std::vector<ASTPointer<VariableDeclaration>> parameters;
	VarDeclParserOptions options(_options);
	options.allowEmptyName = true;
	if (m_experimentalSolidityEnabledInCurrentSourceUnit && m_scanner->currentToken() == Token::Identifier)
	{
		// Parses unary parameter lists without parentheses. TODO: is this a good idea in all cases? Including arguments?
		parameters = {parsePostfixVariableDeclaration()};
		nodeFactory.setEndPositionFromNode(parameters.front());
		return nodeFactory.createNode<ParameterList>(parameters);
	}
	expectToken(Token::LParen);
	auto parseSingleVariableDeclaration = [&]() {
		if (m_experimentalSolidityEnabledInCurrentSourceUnit)
			return parsePostfixVariableDeclaration();
		else
			return parseVariableDeclaration(options);
	};
	if (!_allowEmpty || m_scanner->currentToken() != Token::RParen)
	{
		parameters.push_back(parseSingleVariableDeclaration());
		while (m_scanner->currentToken() != Token::RParen)
		{
			if (m_scanner->currentToken() == Token::Comma && m_scanner->peekNextToken() == Token::RParen)
				fatalParserError(7591_error, "Unexpected trailing comma in parameter list.");
			expectToken(Token::Comma);
			parameters.push_back(parseSingleVariableDeclaration());
		}
	}
	nodeFactory.markEndPosition();
	advance();
	return nodeFactory.createNode<ParameterList>(parameters);
}

ASTPointer<Block> Parser::parseBlock(bool _allowUnchecked, ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	bool const unchecked = m_scanner->currentToken() == Token::Unchecked;
	if (unchecked)
	{
		if (!_allowUnchecked)
			parserError(5296_error, "\"unchecked\" blocks can only be used inside regular blocks.");
		advance();
	}
	expectToken(Token::LBrace);
	std::vector<ASTPointer<Statement>> statements;
	while (m_scanner->currentToken() != Token::RBrace)
		statements.push_back(parseStatement(true));
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);
	return nodeFactory.createNode<Block>(_docString, unchecked, statements);
}

ASTPointer<Statement> Parser::parseStatement(bool _allowUnchecked)
{
	RecursionGuard recursionGuard(*this);
	ASTPointer<ASTString> docString;
	ASTPointer<Statement> statement;
	if (m_scanner->currentCommentLiteral() != "")
		docString = std::make_shared<ASTString>(m_scanner->currentCommentLiteral());
	switch (m_scanner->currentToken())
	{
	case Token::If:
		return parseIfStatement(docString);
	case Token::While:
		return parseWhileStatement(docString);
	case Token::Do:
		return parseDoWhileStatement(docString);
	case Token::For:
		return parseForStatement(docString);
	case Token::Unchecked:
	case Token::LBrace:
		return parseBlock(_allowUnchecked, docString);
	case Token::Continue:
		statement = ASTNodeFactory(*this).createNode<Continue>(docString);
		advance();
		break;
	case Token::Break:
		statement = ASTNodeFactory(*this).createNode<Break>(docString);
		advance();
		break;
	case Token::Return:
	{
		ASTNodeFactory nodeFactory(*this);
		ASTPointer<Expression> expression;
		if (advance() != Token::Semicolon)
			{
				expression = parseExpression();
				nodeFactory.setEndPositionFromNode(expression);
			}
		statement = nodeFactory.createNode<Return>(docString, expression);
			break;
	}
	case Token::Throw:
	{
		statement = ASTNodeFactory(*this).createNode<Throw>(docString);
		advance();
		break;
	}
	case Token::Try:
		return parseTryStatement(docString);
	case Token::Assembly:
		return parseInlineAssembly(docString);
	case Token::Emit:
		statement = parseEmitStatement(docString);
		break;
	case Token::Identifier:
		if (m_scanner->currentLiteral() == "revert" && m_scanner->peekNextToken() == Token::Identifier)
			statement = parseRevertStatement(docString);
		else if (m_insideModifier && m_scanner->currentLiteral() == "_")
		{
			statement = ASTNodeFactory(*this).createNode<PlaceholderStatement>(docString);
			advance();
		}
		else
			statement = parseSimpleStatement(docString);
		break;
	default:
		statement = parseSimpleStatement(docString);
		break;
	}
	expectToken(Token::Semicolon);
	return statement;
}

ASTPointer<InlineAssembly> Parser::parseInlineAssembly(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	SourceLocation location = currentLocation();

	expectToken(Token::Assembly);
	yul::Dialect const& dialect = yul::EVMDialect::strictAssemblyForEVM(m_evmVersion, m_eofVersion);
	if (m_scanner->currentToken() == Token::StringLiteral)
	{
		if (m_scanner->currentLiteral() != "evmasm")
			fatalParserError(4531_error, "Only \"evmasm\" supported.");
		// This can be used in the future to set the dialect.
		advance();
	}

	ASTPointer<std::vector<ASTPointer<ASTString>>> flags;
	if (m_scanner->currentToken() == Token::LParen)
	{
		flags = std::make_shared<std::vector<ASTPointer<ASTString>>>();
		do
		{
			advance();
			expectToken(Token::StringLiteral, false);
			flags->emplace_back(std::make_shared<ASTString>(m_scanner->currentLiteral()));
			advance();
		}
		while (m_scanner->currentToken() == Token::Comma);
		expectToken(Token::RParen);
	}

	yul::Parser asmParser(m_errorReporter, dialect);
	std::shared_ptr<yul::AST> ast = asmParser.parseInline(m_scanner);
	if (ast == nullptr)
		BOOST_THROW_EXCEPTION(FatalError());

	location.end = nativeLocationOf(ast->root()).end;
	return std::make_shared<InlineAssembly>(nextID(), location, _docString, dialect, std::move(flags), ast);
}

ASTPointer<IfStatement> Parser::parseIfStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::If);
	expectToken(Token::LParen);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RParen);
	ASTPointer<Statement> trueBody = parseStatement();
	ASTPointer<Statement> falseBody;
	if (m_scanner->currentToken() == Token::Else)
	{
		advance();
		falseBody = parseStatement();
		nodeFactory.setEndPositionFromNode(falseBody);
	}
	else
		nodeFactory.setEndPositionFromNode(trueBody);
	return nodeFactory.createNode<IfStatement>(_docString, condition, trueBody, falseBody);
}

ASTPointer<TryStatement> Parser::parseTryStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Try);
	ASTPointer<Expression> externalCall = parseExpression();
	std::vector<ASTPointer<TryCatchClause>> clauses;

	ASTNodeFactory successClauseFactory(*this);
	ASTPointer<ParameterList> returnsParameters;
	if (m_scanner->currentToken() == Token::Returns)
	{
		advance();
		VarDeclParserOptions options;
		options.allowEmptyName = true;
		options.allowLocationSpecifier = true;
		returnsParameters = parseParameterList(options, false);
	}
	ASTPointer<Block> successBlock = parseBlock();
	successClauseFactory.setEndPositionFromNode(successBlock);
	clauses.emplace_back(successClauseFactory.createNode<TryCatchClause>(
		std::make_shared<ASTString>(), returnsParameters, successBlock
	));

	do
	{
		clauses.emplace_back(parseCatchClause());
	}
	while (m_scanner->currentToken() == Token::Catch);
	nodeFactory.setEndPositionFromNode(clauses.back());
	return nodeFactory.createNode<TryStatement>(
		_docString, externalCall, clauses
	);
}

ASTPointer<TryCatchClause> Parser::parseCatchClause()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Catch);
	ASTPointer<ASTString> errorName = std::make_shared<std::string>();
	ASTPointer<ParameterList> errorParameters;
	if (m_scanner->currentToken() != Token::LBrace)
	{
		if (m_scanner->currentToken() == Token::Identifier)
			errorName = expectIdentifierToken();
		VarDeclParserOptions options;
		options.allowEmptyName = true;
		options.allowLocationSpecifier = true;
		errorParameters = parseParameterList(options, !errorName->empty());
	}
	ASTPointer<Block> block = parseBlock();
	nodeFactory.setEndPositionFromNode(block);
	return nodeFactory.createNode<TryCatchClause>(errorName, errorParameters, block);
}

ASTPointer<WhileStatement> Parser::parseWhileStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::While);
	expectToken(Token::LParen);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RParen);
	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<WhileStatement>(_docString, condition, body, false);
}

ASTPointer<WhileStatement> Parser::parseDoWhileStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Do);
	ASTPointer<Statement> body = parseStatement();
	expectToken(Token::While);
	expectToken(Token::LParen);
	ASTPointer<Expression> condition = parseExpression();
	expectToken(Token::RParen);
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<WhileStatement>(_docString, condition, body, true);
}


ASTPointer<ForStatement> Parser::parseForStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	ASTPointer<Statement> initExpression;
	ASTPointer<Expression> conditionExpression;
	ASTPointer<ExpressionStatement> loopExpression;
	expectToken(Token::For);
	expectToken(Token::LParen);

	// TODO: Maybe here have some predicate like peekExpression() instead of checking for semicolon and RParen?
	if (m_scanner->currentToken() != Token::Semicolon)
		initExpression = parseSimpleStatement(ASTPointer<ASTString>());
	expectToken(Token::Semicolon);

	if (m_scanner->currentToken() != Token::Semicolon)
		conditionExpression = parseExpression();
	expectToken(Token::Semicolon);

	if (m_scanner->currentToken() != Token::RParen)
		loopExpression = parseExpressionStatement(ASTPointer<ASTString>());
	expectToken(Token::RParen);

	ASTPointer<Statement> body = parseStatement();
	nodeFactory.setEndPositionFromNode(body);
	return nodeFactory.createNode<ForStatement>(
		_docString,
		initExpression,
		conditionExpression,
		loopExpression,
		body
	);
}

ASTPointer<EmitStatement> Parser::parseEmitStatement(ASTPointer<ASTString> const& _docString)
{
	expectToken(Token::Emit, false);

	ASTNodeFactory nodeFactory(*this);
	advance();
	ASTNodeFactory eventCallNodeFactory(*this);

	if (m_scanner->currentToken() != Token::Identifier)
		fatalParserError(5620_error, "Expected event name or path.");

	IndexAccessedPath iap;
	while (true)
	{
		iap.path.push_back(parseIdentifier());
		if (m_scanner->currentToken() != Token::Period)
			break;
		advance();
	}

	auto eventName = expressionFromIndexAccessStructure(iap);
	expectToken(Token::LParen);

	auto functionCallArguments = parseFunctionCallArguments();
	eventCallNodeFactory.markEndPosition();
	nodeFactory.markEndPosition();
	expectToken(Token::RParen);
	auto eventCall = eventCallNodeFactory.createNode<FunctionCall>(
		eventName,
		functionCallArguments.arguments,
		functionCallArguments.parameterNames,
		functionCallArguments.parameterNameLocations
	);
	return nodeFactory.createNode<EmitStatement>(_docString, eventCall);
}

ASTPointer<RevertStatement> Parser::parseRevertStatement(ASTPointer<ASTString> const& _docString)
{
	ASTNodeFactory nodeFactory(*this);
	solAssert(*expectIdentifierToken() == "revert", "");

	ASTNodeFactory errorCallNodeFactory(*this);

	solAssert(m_scanner->currentToken() == Token::Identifier, "");

	IndexAccessedPath iap;
	while (true)
	{
		iap.path.push_back(parseIdentifier());
		if (m_scanner->currentToken() != Token::Period)
			break;
		advance();
	}

	auto errorName = expressionFromIndexAccessStructure(iap);
	expectToken(Token::LParen);

	auto functionCallArguments = parseFunctionCallArguments();
	errorCallNodeFactory.markEndPosition();
	nodeFactory.markEndPosition();
	expectToken(Token::RParen);
	auto errorCall = errorCallNodeFactory.createNode<FunctionCall>(
		errorName,
		functionCallArguments.arguments,
		functionCallArguments.parameterNames,
		functionCallArguments.parameterNameLocations
	);
	return nodeFactory.createNode<RevertStatement>(_docString, errorCall);
}

ASTPointer<VariableDeclarationStatement> Parser::parsePostfixVariableDeclarationStatement(
	ASTPointer<ASTString> const& _docString
)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	expectToken(Token::Let);

	std::vector<ASTPointer<VariableDeclaration>> variables;
	variables.emplace_back(parsePostfixVariableDeclaration());
	nodeFactory.setEndPositionFromNode(variables.back());

	ASTPointer<Expression> value;
	if (m_scanner->currentToken() == Token::Assign)
	{
		advance();
		value = parseExpression();
		nodeFactory.setEndPositionFromNode(value);
	}
	return nodeFactory.createNode<VariableDeclarationStatement>(_docString, variables, value);
}

ASTPointer<VariableDeclaration> Parser::parsePostfixVariableDeclaration()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	ASTPointer<StructuredDocumentation> const documentation = parseStructuredDocumentation();

	nodeFactory.markEndPosition();
	auto [identifier, nameLocation] = expectIdentifierWithLocation();

	ASTPointer<Expression> type;
	if (m_scanner->currentToken() == Token::Colon)
	{
		advance();
		type = parseBinaryExpression();
		nodeFactory.setEndPositionFromNode(type);
	}

	return nodeFactory.createNode<VariableDeclaration>(
		nullptr,
		identifier,
		nameLocation,
		nullptr,
		Visibility::Default,
		documentation,
		false,
		VariableDeclaration::Mutability::Mutable,
		nullptr,
		VariableDeclaration::Location::Unspecified,
		type
	);
}

ASTPointer<TypeClassDefinition> Parser::parseTypeClassDefinition()
{
	solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	std::vector<ASTPointer<ASTNode>> subNodes;

	ASTPointer<StructuredDocumentation> const documentation = parseStructuredDocumentation();

	expectToken(Token::Class);
	// TODO: parseTypeVariable()? parseTypeVariableDeclaration()?
	ASTPointer<VariableDeclaration> typeVariable;
	{
		ASTNodeFactory nodeFactory(*this);
		nodeFactory.markEndPosition();
		auto [identifier, nameLocation] = expectIdentifierWithLocation();
		typeVariable = nodeFactory.createNode<VariableDeclaration>(
			nullptr,
			identifier,
			nameLocation,
			nullptr,
			Visibility::Default,
			nullptr
		);
	}
	expectToken(Token::Colon);
	auto [name, nameLocation] = expectIdentifierWithLocation();
	expectToken(Token::LBrace);
	while (true)
	{
		Token currentTokenValue = m_scanner->currentToken();
		if (currentTokenValue == Token::RBrace)
			break;
		expectToken(Token::Function, false);
		subNodes.push_back(parseFunctionDefinition(false, false));
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);

	return nodeFactory.createNode<TypeClassDefinition>(
		typeVariable,
		name,
		nameLocation,
		documentation,
		subNodes
	);
}

ASTPointer<TypeClassName> Parser::parseTypeClassName()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	std::variant<Token, ASTPointer<IdentifierPath>> name;
	if (TokenTraits::isBuiltinTypeClassName(m_scanner->currentToken()))
	{
		nodeFactory.markEndPosition();
		name = m_scanner->currentToken();
		advance();
	}
	else
	{
		auto identifierPath = parseIdentifierPath();
		name = identifierPath;
		nodeFactory.setEndPositionFromNode(identifierPath);
	}
	return nodeFactory.createNode<TypeClassName>(name);
}

ASTPointer<TypeClassInstantiation> Parser::parseTypeClassInstantiation()
{
	solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);

	std::vector<ASTPointer<ASTNode>> subNodes;

	expectToken(Token::Instantiation);
	// TODO: parseTypeConstructor()
	ASTPointer<TypeName> typeConstructor = parseTypeName();
	ASTPointer<ParameterList> argumentSorts;
	if (m_scanner->currentToken() == Token::LParen)
	{
		argumentSorts = parseParameterList();
	}
	expectToken(Token::Colon);
	ASTPointer<TypeClassName> typeClassName = parseTypeClassName();
	expectToken(Token::LBrace);
	while (true)
	{
		Token currentTokenValue = m_scanner->currentToken();
		if (currentTokenValue == Token::RBrace)
			break;
		expectToken(Token::Function, false);
		// TODO: require body already during parsing?
		subNodes.push_back(parseFunctionDefinition(false, true));
	}
	nodeFactory.markEndPosition();
	expectToken(Token::RBrace);

	return nodeFactory.createNode<TypeClassInstantiation>(
		typeConstructor,
		argumentSorts,
		typeClassName,
		subNodes
	);
}

ASTPointer<TypeDefinition> Parser::parseTypeDefinition()
{
	solAssert(m_experimentalSolidityEnabledInCurrentSourceUnit);
	ASTNodeFactory nodeFactory(*this);
	expectToken(Token::Type);
	auto&& [name, nameLocation] = expectIdentifierWithLocation();

	ASTPointer<ParameterList> arguments;
	if (m_scanner->currentToken() == Token::LParen)
		arguments = parseParameterList();

	ASTPointer<Expression> expression;
	if (m_scanner->currentToken() == Token::Assign)
	{
		expectToken(Token::Assign);

		if (m_scanner->currentToken() != Token::Builtin)
			expression = parseExpression();
		else
		{
			expectToken(Token::Builtin);
			expectToken(Token::LParen);

			expression = nodeFactory.createNode<Builtin>(
				std::make_shared<std::string>(m_scanner->currentLiteral()),
				m_scanner->currentLocation()
			);

			expectToken(Token::StringLiteral);
			expectToken(Token::RParen);
		}
	}
	nodeFactory.markEndPosition();
	expectToken(Token::Semicolon);
	return nodeFactory.createNode<TypeDefinition>(
		std::move(name),
		std::move(nameLocation),
		std::move(arguments),
		std::move(expression)
	);
}

ASTPointer<Statement> Parser::parseSimpleStatement(ASTPointer<ASTString> const& _docString)
{
	RecursionGuard recursionGuard(*this);
	LookAheadInfo statementType;
	IndexAccessedPath iap;

	if (m_experimentalSolidityEnabledInCurrentSourceUnit && m_scanner->currentToken() == Token::Let)
		return parsePostfixVariableDeclarationStatement(_docString);

	if (m_scanner->currentToken() == Token::LParen)
	{
		ASTNodeFactory nodeFactory(*this);
		size_t emptyComponents = 0;
		// First consume all empty components.
		expectToken(Token::LParen);
		while (m_scanner->currentToken() == Token::Comma)
		{
			advance();
			emptyComponents++;
		}

		// Now see whether we have a variable declaration or an expression.
		std::tie(statementType, iap) = tryParseIndexAccessedPath();
		switch (statementType)
		{
		case LookAheadInfo::VariableDeclaration:
		{
			std::vector<ASTPointer<VariableDeclaration>> variables;
			ASTPointer<Expression> value;
			// We have already parsed something like `(,,,,a.b.c[2][3]`
			VarDeclParserOptions options;
			options.allowLocationSpecifier = true;
			variables = std::vector<ASTPointer<VariableDeclaration>>(emptyComponents, nullptr);
			variables.push_back(parseVariableDeclaration(options, typeNameFromIndexAccessStructure(iap)));

			while (m_scanner->currentToken() != Token::RParen)
			{
				expectToken(Token::Comma);
				if (m_scanner->currentToken() == Token::Comma || m_scanner->currentToken() == Token::RParen)
					variables.push_back(nullptr);
				else
					variables.push_back(parseVariableDeclaration(options));
			}
			expectToken(Token::RParen);
			expectToken(Token::Assign);
			value = parseExpression();
			nodeFactory.setEndPositionFromNode(value);
			return nodeFactory.createNode<VariableDeclarationStatement>(_docString, variables, value);
		}
		case LookAheadInfo::Expression:
		{
			// Complete parsing the expression in the current component.
			std::vector<ASTPointer<Expression>> components(emptyComponents, nullptr);
			components.push_back(parseExpression(expressionFromIndexAccessStructure(iap)));
			while (m_scanner->currentToken() != Token::RParen)
			{
				expectToken(Token::Comma);
				if (m_scanner->currentToken() == Token::Comma || m_scanner->currentToken() == Token::RParen)
					components.push_back(ASTPointer<Expression>());
				else
					components.push_back(parseExpression());
			}
			nodeFactory.markEndPosition();
			expectToken(Token::RParen);
			return parseExpressionStatement(_docString, nodeFactory.createNode<TupleExpression>(components, false));
		}
		default:
			solAssert(false);
		}
	}
	else
	{
		std::tie(statementType, iap) = tryParseIndexAccessedPath();
		switch (statementType)
		{
		case LookAheadInfo::VariableDeclaration:
			return parseVariableDeclarationStatement(_docString, typeNameFromIndexAccessStructure(iap));
		case LookAheadInfo::Expression:
			return parseExpressionStatement(_docString, expressionFromIndexAccessStructure(iap));
		default:
			solAssert(false);
		}
	}

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

bool Parser::IndexAccessedPath::empty() const
{
	if (!indices.empty())
		solAssert(!path.empty());

	return path.empty() && indices.empty();
}


std::pair<Parser::LookAheadInfo, Parser::IndexAccessedPath> Parser::tryParseIndexAccessedPath()
{
	// These two cases are very hard to distinguish:
	// x[7 * 20 + 3] a;     and     x[7 * 20 + 3] = 9;
	// In the first case, x is a type name, in the second it is the name of a variable.
	// As an extension, we can even have:
	// `x.y.z[1][2] a;` and `x.y.z[1][2] = 10;`
	// Where in the first, x.y.z leads to a type name where in the second, it accesses structs.

	auto statementType = peekStatementType();
	switch (statementType)
	{
	case LookAheadInfo::VariableDeclaration:
	case LookAheadInfo::Expression:
		return std::make_pair(
			m_experimentalSolidityEnabledInCurrentSourceUnit ? LookAheadInfo::Expression : statementType,
			IndexAccessedPath()
		);
	default:
		break;
	}

	// At this point, we have 'Identifier "["' or 'Identifier "." Identifier' or 'ElementoryTypeName "["'.
	// We parse '(Identifier ("." Identifier)* |ElementaryTypeName) ( "[" Expression "]" )*'
	// until we can decide whether to hand this over to ExpressionStatement or create a
	// VariableDeclarationStatement out of it.
	IndexAccessedPath iap = parseIndexAccessedPath();

	if (m_experimentalSolidityEnabledInCurrentSourceUnit)
		return std::make_pair(LookAheadInfo::Expression, std::move(iap));

	if (m_scanner->currentToken() == Token::Identifier || TokenTraits::isLocationSpecifier(m_scanner->currentToken()))
		return std::make_pair(LookAheadInfo::VariableDeclaration, std::move(iap));
	else
		return std::make_pair(LookAheadInfo::Expression, std::move(iap));
}

ASTPointer<VariableDeclarationStatement> Parser::parseVariableDeclarationStatement(
	ASTPointer<ASTString> const& _docString,
	ASTPointer<TypeName> const& _lookAheadArrayType
)
{
	// This does not parse multi variable declaration statements starting directly with
	// `(`, they are parsed in parseSimpleStatement, because they are hard to distinguish
	// from tuple expressions.
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	if (_lookAheadArrayType)
		nodeFactory.setLocation(_lookAheadArrayType->location());

	VarDeclParserOptions options;
	options.allowLocationSpecifier = true;
	std::vector<ASTPointer<VariableDeclaration>> variables;
	variables.emplace_back(parseVariableDeclaration(options, _lookAheadArrayType));
	nodeFactory.setEndPositionFromNode(variables.back());

	ASTPointer<Expression> value;
	if (m_scanner->currentToken() == Token::Assign)
	{
		advance();
		value = parseExpression();
		nodeFactory.setEndPositionFromNode(value);
	}
	return nodeFactory.createNode<VariableDeclarationStatement>(_docString, variables, value);
}

ASTPointer<ExpressionStatement> Parser::parseExpressionStatement(
	ASTPointer<ASTString> const& _docString,
	ASTPointer<Expression> const& _partialParserResult
)
{
	RecursionGuard recursionGuard(*this);
	ASTPointer<Expression> expression = parseExpression(_partialParserResult);
	return ASTNodeFactory(*this, expression).createNode<ExpressionStatement>(_docString, expression);
}

ASTPointer<Expression> Parser::parseExpression(
	ASTPointer<Expression> const& _partiallyParsedExpression
)
{
	RecursionGuard recursionGuard(*this);
	ASTPointer<Expression> expression = parseBinaryExpression(4, _partiallyParsedExpression);
	if (TokenTraits::isAssignmentOp(m_scanner->currentToken()))
	{
		Token assignmentOperator = m_scanner->currentToken();
		advance();
		ASTPointer<Expression> rightHandSide = parseExpression();
		ASTNodeFactory nodeFactory(*this, expression);
		nodeFactory.setEndPositionFromNode(rightHandSide);
		return nodeFactory.createNode<Assignment>(expression, assignmentOperator, rightHandSide);
	}
	else if (m_scanner->currentToken() == Token::Conditional)
	{
		advance();
		ASTPointer<Expression> trueExpression = parseExpression();
		expectToken(Token::Colon);
		ASTPointer<Expression> falseExpression = parseExpression();
		ASTNodeFactory nodeFactory(*this, expression);
		nodeFactory.setEndPositionFromNode(falseExpression);
		return nodeFactory.createNode<Conditional>(expression, trueExpression, falseExpression);
	}
	else
		return expression;
}

ASTPointer<Expression> Parser::parseBinaryExpression(
	int _minPrecedence,
	ASTPointer<Expression> const& _partiallyParsedExpression
)
{
	RecursionGuard recursionGuard(*this);
	ASTPointer<Expression> expression = parseUnaryExpression(_partiallyParsedExpression);
	ASTNodeFactory nodeFactory(*this, expression);
	int precedence = tokenPrecedence(m_scanner->currentToken());
	for (; precedence >= _minPrecedence; --precedence)
		while (tokenPrecedence(m_scanner->currentToken()) == precedence)
		{
			Token op = m_scanner->currentToken();
			advance();

			static_assert(TokenTraits::hasExpHighestPrecedence(), "Exp does not have the highest precedence");

			// Parse a**b**c as a**(b**c)
			ASTPointer<Expression> right = (op == Token::Exp) ?
				parseBinaryExpression(precedence) :
				parseBinaryExpression(precedence + 1);
			nodeFactory.setEndPositionFromNode(right);
			expression = nodeFactory.createNode<BinaryOperation>(expression, op, right);
		}
	return expression;
}

int Parser::tokenPrecedence(Token _token) const
{
	if (m_experimentalSolidityEnabledInCurrentSourceUnit)
	{
		switch (_token)
		{
		case Token::Colon:
			return 1000;
		case Token::RightArrow:
			return 999;
		default:
			break;
		}
	}
	return TokenTraits::precedence(m_scanner->currentToken());
}

ASTPointer<Expression> Parser::parseUnaryExpression(
	ASTPointer<Expression> const& _partiallyParsedExpression
)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory = _partiallyParsedExpression ?
		ASTNodeFactory(*this, _partiallyParsedExpression) : ASTNodeFactory(*this);
	Token token = m_scanner->currentToken();

	if (!_partiallyParsedExpression && token == Token::Add)
		fatalParserError(9636_error, "Use of unary + is disallowed.");

	if (!_partiallyParsedExpression && (TokenTraits::isUnaryOp(token) || TokenTraits::isCountOp(token)))
	{
		// prefix expression
		advance();
		ASTPointer<Expression> subExpression = parseUnaryExpression();
		nodeFactory.setEndPositionFromNode(subExpression);
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, true);
	}
	else
	{
		// potential postfix expression
		ASTPointer<Expression> subExpression = parseLeftHandSideExpression(_partiallyParsedExpression);
		token = m_scanner->currentToken();

		if (!TokenTraits::isCountOp(token))
			return subExpression;
		nodeFactory.markEndPosition();
		advance();
		return nodeFactory.createNode<UnaryOperation>(token, subExpression, false);
	}
}

ASTPointer<Expression> Parser::parseLeftHandSideExpression(
	ASTPointer<Expression> const& _partiallyParsedExpression
)
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory = _partiallyParsedExpression ?
		ASTNodeFactory(*this, _partiallyParsedExpression) : ASTNodeFactory(*this);

	ASTPointer<Expression> expression;
	if (_partiallyParsedExpression)
		expression = _partiallyParsedExpression;
	else if (m_scanner->currentToken() == Token::New)
	{
		expectToken(Token::New);
		ASTPointer<TypeName> typeName(parseTypeName());
		nodeFactory.setEndPositionFromNode(typeName);
		expression = nodeFactory.createNode<NewExpression>(typeName);
	}
	else if (m_scanner->currentToken() == Token::Payable)
	{
		expectToken(Token::Payable);
		nodeFactory.markEndPosition();
		auto expressionType = nodeFactory.createNode<ElementaryTypeName>(
			ElementaryTypeNameToken(Token::Address, 0, 0),
			std::make_optional(StateMutability::Payable)
		);
		expression = nodeFactory.createNode<ElementaryTypeNameExpression>(expressionType);
		expectToken(Token::LParen, false);
	}
	else
		expression = parsePrimaryExpression();

	while (true)
	{
		switch (m_scanner->currentToken())
		{
		case Token::LBrack:
		{
			advance();
			ASTPointer<Expression> index;
			ASTPointer<Expression> endIndex;
			if (m_scanner->currentToken() != Token::RBrack && m_scanner->currentToken() != Token::Colon)
				index = parseExpression();
			if (m_scanner->currentToken() == Token::Colon)
			{
				expectToken(Token::Colon);
				if (m_scanner->currentToken() != Token::RBrack)
					endIndex = parseExpression();
				nodeFactory.markEndPosition();
				expectToken(Token::RBrack);
				expression = nodeFactory.createNode<IndexRangeAccess>(expression, index, endIndex);
			}
			else
			{
				nodeFactory.markEndPosition();
				expectToken(Token::RBrack);
				expression = nodeFactory.createNode<IndexAccess>(expression, index);
			}
			break;
		}
		case Token::Period:
		{
			advance();
			nodeFactory.markEndPosition();
			SourceLocation memberLocation = currentLocation();
			ASTPointer<ASTString> memberName = expectIdentifierTokenOrAddress();
			expression = nodeFactory.createNode<MemberAccess>(expression, std::move(memberName), std::move(memberLocation));
			break;
		}
		case Token::LParen:
		{
			advance();
			auto functionCallArguments = parseFunctionCallArguments();
			nodeFactory.markEndPosition();
			expectToken(Token::RParen);
			expression = nodeFactory.createNode<FunctionCall>(
				expression,
				functionCallArguments.arguments,
				functionCallArguments.parameterNames,
				functionCallArguments.parameterNameLocations);
			break;
		}
		case Token::LBrace:
		{
			// See if this is followed by <identifier>, followed by ":". If not, it is not
			// a function call options but a Block (from a try statement).
			if (
				m_scanner->peekNextToken() != Token::Identifier ||
				m_scanner->peekNextNextToken() != Token::Colon
			)
				return expression;

			expectToken(Token::LBrace);
			auto optionList = parseNamedArguments();

			nodeFactory.markEndPosition();
			expectToken(Token::RBrace);

			expression = nodeFactory.createNode<FunctionCallOptions>(expression, optionList.arguments, optionList.parameterNames);
			break;
		}
		default:
			return expression;
		}
	}
}

ASTPointer<Expression> Parser::parseLiteral()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	Token initialToken = m_scanner->currentToken();
	ASTPointer<ASTString> value = std::make_shared<std::string>(m_scanner->currentLiteral());

	switch (initialToken)
	{
	case Token::TrueLiteral:
	case Token::FalseLiteral:
	case Token::Number:
	{
		nodeFactory.markEndPosition();
		advance();
		break;
	}
	case Token::StringLiteral:
	case Token::UnicodeStringLiteral:
	case Token::HexStringLiteral:
	{
		while (m_scanner->peekNextToken() == initialToken)
		{
			advance();
			*value += m_scanner->currentLiteral();
		}
		nodeFactory.markEndPosition();
		advance();
		if (m_scanner->currentToken() == Token::Illegal)
			fatalParserError(5428_error, to_string(m_scanner->currentError()));
		break;
	}
	default:
		solAssert(false);
	}

	if (initialToken == Token::Number && (
		TokenTraits::isEtherSubdenomination(m_scanner->currentToken()) ||
		TokenTraits::isTimeSubdenomination(m_scanner->currentToken())
	))
	{
		nodeFactory.markEndPosition();
		Literal::SubDenomination subDenomination = static_cast<Literal::SubDenomination>(m_scanner->currentToken());
		advance();
		return nodeFactory.createNode<Literal>(initialToken, std::move(value), subDenomination);
	}

	return nodeFactory.createNode<Literal>(initialToken, std::move(value), Literal::SubDenomination::None);
}

ASTPointer<Expression> Parser::parsePrimaryExpression()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	Token token = m_scanner->currentToken();
	ASTPointer<Expression> expression;

	switch (token)
	{
	case Token::TrueLiteral:
	case Token::FalseLiteral:
	case Token::Number:
	case Token::StringLiteral:
	case Token::UnicodeStringLiteral:
	case Token::HexStringLiteral:
		expression = parseLiteral();
		break;
	case Token::Identifier:
		nodeFactory.markEndPosition();
		expression = nodeFactory.createNode<Identifier>(getLiteralAndAdvance());
		break;
	case Token::Type:
		// Inside expressions "type" is the name of a special, globally-available function.
		nodeFactory.markEndPosition();
		advance();
		expression = nodeFactory.createNode<Identifier>(std::make_shared<ASTString>("type"));
		break;
	case Token::LParen:
	case Token::LBrack:
	{
		// Tuple/parenthesized expression or inline array/bracketed expression.
		// Special cases: ()/[] is empty tuple/array type, (x) is not a real tuple,
		// (x,) is one-dimensional tuple, elements in arrays cannot be left out, only in tuples.
		advance();
		std::vector<ASTPointer<Expression>> components;
		Token oppositeToken = (token == Token::LParen ? Token::RParen : Token::RBrack);
		bool isArray = (token == Token::LBrack);

		if (m_scanner->currentToken() != oppositeToken)
			while (true)
			{
				if (m_scanner->currentToken() != Token::Comma && m_scanner->currentToken() != oppositeToken)
					components.push_back(parseExpression());
				else if (isArray)
					parserError(4799_error, "Expected expression (inline array elements cannot be omitted).");
				else
					components.push_back(ASTPointer<Expression>());

				if (m_scanner->currentToken() == oppositeToken)
					break;

				expectToken(Token::Comma);
			}
		nodeFactory.markEndPosition();
		expectToken(oppositeToken);
		expression = nodeFactory.createNode<TupleExpression>(components, isArray);
		break;
	}
	case Token::Illegal:
		fatalParserError(8936_error, to_string(m_scanner->currentError()));
		break;
	default:
		if (TokenTraits::isElementaryTypeName(token))
		{
			//used for casts
			unsigned firstSize;
			unsigned secondSize;
			std::tie(firstSize, secondSize) = m_scanner->currentTokenInfo();
			auto expressionType = nodeFactory.createNode<ElementaryTypeName>(
				ElementaryTypeNameToken(m_scanner->currentToken(), firstSize, secondSize)
			);
			expression = nodeFactory.createNode<ElementaryTypeNameExpression>(expressionType);
			advance();
		}
		else
			fatalParserError(6933_error, "Expected primary expression.");
		break;
	}
	return expression;
}

std::vector<ASTPointer<Expression>> Parser::parseFunctionCallListArguments()
{
	RecursionGuard recursionGuard(*this);
	std::vector<ASTPointer<Expression>> arguments;
	if (m_scanner->currentToken() != Token::RParen)
	{
		arguments.push_back(parseExpression());
		while (m_scanner->currentToken() != Token::RParen)
		{
			expectToken(Token::Comma);
			arguments.push_back(parseExpression());
		}
	}
	return arguments;
}

Parser::FunctionCallArguments Parser::parseFunctionCallArguments()
{
	RecursionGuard recursionGuard(*this);
	FunctionCallArguments ret;

	Token token = m_scanner->currentToken();
	if (token == Token::LBrace)
	{
		// call({arg1 : 1, arg2 : 2 })
		expectToken(Token::LBrace);
		ret = parseNamedArguments();
		expectToken(Token::RBrace);
	}
	else
		ret.arguments = parseFunctionCallListArguments();
	return ret;
}

Parser::FunctionCallArguments Parser::parseNamedArguments()
{
	FunctionCallArguments ret;

	bool first = true;
	while (m_scanner->currentToken() != Token::RBrace)
	{
		if (!first)
			expectToken(Token::Comma);

		auto identifierWithLocation = expectIdentifierWithLocation();

		// Add name
		ret.parameterNames.emplace_back(std::move(identifierWithLocation.first));
		// Add location
		ret.parameterNameLocations.emplace_back(std::move(identifierWithLocation.second));

		expectToken(Token::Colon);
		ret.arguments.emplace_back(parseExpression());

		if (
			m_scanner->currentToken() == Token::Comma &&
			m_scanner->peekNextToken() == Token::RBrace
		)
		{
			parserError(2074_error, "Unexpected trailing comma.");
			advance();
		}

		first = false;
	}

	return ret;
}

bool Parser::variableDeclarationStart()
{
	Token currentToken = m_scanner->currentToken();
	return
		currentToken == Token::Identifier ||
		currentToken == Token::Mapping ||
		TokenTraits::isElementaryTypeName(currentToken) ||
		(currentToken == Token::Function && m_scanner->peekNextToken() == Token::LParen);
}

std::optional<std::string> Parser::findLicenseString(std::vector<ASTPointer<ASTNode>> const& _nodes)
{
	// We circumvent the scanner here, because it skips non-docstring comments.
	static std::regex const licenseNameRegex("([a-zA-Z0-9 ()+.-]+)");
	static std::regex const licenseDeclarationRegex("SPDX-License-Identifier:\\s*(.+?)([\n\r]|(\\*/))");

	// Search inside all parts of the source not covered by parsed nodes.
	// This will leave e.g. "global comments".
	using iter = std::string::const_iterator;
	std::vector<std::pair<iter, iter>> sequencesToSearch;
	std::string const& source = m_scanner->charStream().source();
	sequencesToSearch.emplace_back(source.begin(), source.end());
	for (ASTPointer<ASTNode> const& node: _nodes)
		if (node->location().hasText())
		{
			sequencesToSearch.back().second = source.begin() + node->location().start;
			sequencesToSearch.emplace_back(source.begin() + node->location().end, source.end());
		}

	std::vector<std::string> licenseNames;
	for (auto const& [start, end]: sequencesToSearch)
	{
		auto declarationsBegin = std::sregex_iterator(start, end, licenseDeclarationRegex);
		auto declarationsEnd = std::sregex_iterator();

		for (std::sregex_iterator declIt = declarationsBegin; declIt != declarationsEnd; ++declIt)
			if (!declIt->empty())
			{
				std::string license = boost::trim_copy(std::string((*declIt)[1]));
				licenseNames.emplace_back(std::move(license));
			}
	}

	if (licenseNames.size() == 1)
	{
std::string const& license = licenseNames.front();
		if (regex_match(license, licenseNameRegex))
			return license;
		else
			parserError(
				1114_error,
				{-1, -1, m_scanner->currentLocation().sourceName},
				"Invalid SPDX license identifier."
			);
	}
	else if (licenseNames.empty())
		parserWarning(
			1878_error,
			{-1, -1, m_scanner->currentLocation().sourceName},
			"SPDX license identifier not provided in source file. "
			"Before publishing, consider adding a comment containing "
			"\"SPDX-License-Identifier: <SPDX-License>\" to each source file. "
			"Use \"SPDX-License-Identifier: UNLICENSED\" for non-open-source code. "
			"Please see https://spdx.org for more information."
		);
	else
		parserError(
			3716_error,
			{-1, -1, m_scanner->currentLocation().sourceName},
			"Multiple SPDX license identifiers found in source file. "
			"Use \"AND\" or \"OR\" to combine multiple licenses. "
			"Please see https://spdx.org for more information."
		);

	return {};
}

Parser::LookAheadInfo Parser::peekStatementType() const
{
	// Distinguish between variable declaration (and potentially assignment) and expression statement
	// (which include assignments to other expressions and pre-declared variables).
	// We have a variable declaration if we get a keyword that specifies a type name.
	// If it is an identifier or an elementary type name followed by an identifier
	// or a mutability specifier, we also have a variable declaration.
	// If we get an identifier followed by a "[" or ".", it can be both ("lib.type[9] a;" or "variable.el[9] = 7;").
	// In all other cases, we have an expression statement.
	Token token(m_scanner->currentToken());
	bool mightBeTypeName = (TokenTraits::isElementaryTypeName(token) || token == Token::Identifier);

	if (token == Token::Mapping || token == Token::Function)
		return LookAheadInfo::VariableDeclaration;
	if (mightBeTypeName)
	{
		Token next = m_scanner->peekNextToken();
		// So far we only allow ``address payable`` in variable declaration statements and in no other
		// kind of statement. This means, for example, that we do not allow type expressions of the form
		// ``address payable;``.
		// If we want to change this in the future, we need to consider another scanner token here.
		if (TokenTraits::isElementaryTypeName(token) && TokenTraits::isStateMutabilitySpecifier(next))
			return LookAheadInfo::VariableDeclaration;
		if (next == Token::Identifier || TokenTraits::isLocationSpecifier(next))
			return LookAheadInfo::VariableDeclaration;
		if (next == Token::LBrack || next == Token::Period)
			return LookAheadInfo::IndexAccessStructure;
	}
	return LookAheadInfo::Expression;
}

Parser::IndexAccessedPath Parser::parseIndexAccessedPath()
{
	IndexAccessedPath iap;
	if (m_scanner->currentToken() == Token::Identifier)
	{
		iap.path.push_back(parseIdentifier());
		while (m_scanner->currentToken() == Token::Period)
		{
			advance();
			if (m_experimentalSolidityEnabledInCurrentSourceUnit && m_scanner->currentToken() == Token::Number)
			{
				ASTNodeFactory nodeFactory(*this);
				nodeFactory.markEndPosition();
				iap.path.push_back(nodeFactory.createNode<Identifier>(getLiteralAndAdvance()));
			}
			else
				iap.path.push_back(parseIdentifierOrAddress());
		}
	}
	else
	{
		unsigned firstNum;
		unsigned secondNum;
		std::tie(firstNum, secondNum) = m_scanner->currentTokenInfo();
		auto expressionType = ASTNodeFactory(*this).createNode<ElementaryTypeName>(
			ElementaryTypeNameToken(m_scanner->currentToken(), firstNum, secondNum)
		);
		iap.path.push_back(ASTNodeFactory(*this).createNode<ElementaryTypeNameExpression>(expressionType));
		advance();
	}
	while (m_scanner->currentToken() == Token::LBrack)
	{
		expectToken(Token::LBrack);
		ASTPointer<Expression> index;
		if (m_scanner->currentToken() != Token::RBrack && m_scanner->currentToken() != Token::Colon)
			index = parseExpression();
		SourceLocation indexLocation = iap.path.front()->location();
		if (m_scanner->currentToken() == Token::Colon)
		{
			expectToken(Token::Colon);
			ASTPointer<Expression> endIndex;
			if (m_scanner->currentToken() != Token::RBrack)
				endIndex = parseExpression();
			indexLocation.end = currentLocation().end;
			iap.indices.emplace_back(IndexAccessedPath::Index{index, {endIndex}, indexLocation});
			expectToken(Token::RBrack);
		}
		else
		{
			indexLocation.end = currentLocation().end;
			iap.indices.emplace_back(IndexAccessedPath::Index{index, {}, indexLocation});
			expectToken(Token::RBrack);
		}
	}

	return iap;
}

ASTPointer<TypeName> Parser::typeNameFromIndexAccessStructure(Parser::IndexAccessedPath const& _iap)
{
	if (_iap.empty())
		return {};

	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	SourceLocation location = _iap.path.front()->location();
	location.end = _iap.path.back()->location().end;
	nodeFactory.setLocation(location);

	ASTPointer<TypeName> type;
	if (auto typeName = dynamic_cast<ElementaryTypeNameExpression const*>(_iap.path.front().get()))
	{
		solAssert(_iap.path.size() == 1, "");
		type = nodeFactory.createNode<ElementaryTypeName>(typeName->type().typeName());
	}
	else
	{
		std::vector<ASTString> path;
		std::vector<SourceLocation> pathLocations;

		for (auto const& el: _iap.path)
		{
			auto& identifier = dynamic_cast<Identifier const&>(*el);
			path.push_back(identifier.name());
			pathLocations.push_back(identifier.location());
		}

		type = nodeFactory.createNode<UserDefinedTypeName>(nodeFactory.createNode<IdentifierPath>(path, pathLocations));
	}
	for (auto const& lengthExpression: _iap.indices)
	{
		if (lengthExpression.end)
			parserError(5464_error, lengthExpression.location, "Expected array length expression.");
		nodeFactory.setLocation(lengthExpression.location);
		type = nodeFactory.createNode<ArrayTypeName>(type, lengthExpression.start);
	}
	return type;
}

ASTPointer<Expression> Parser::expressionFromIndexAccessStructure(
	Parser::IndexAccessedPath const& _iap
)
{
	if (_iap.empty())
		return {};

	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this, _iap.path.front());
	ASTPointer<Expression> expression(_iap.path.front());
	for (size_t i = 1; i < _iap.path.size(); ++i)
	{
		SourceLocation location(_iap.path.front()->location());
		location.end = _iap.path[i]->location().end;
		nodeFactory.setLocation(location);
		Identifier const& identifier = dynamic_cast<Identifier const&>(*_iap.path[i]);
		expression = nodeFactory.createNode<MemberAccess>(
			expression,
			std::make_shared<ASTString>(identifier.name()),
			identifier.location()
		);
	}
	for (auto const& index: _iap.indices)
	{
		nodeFactory.setLocation(index.location);
		if (index.end)
			expression = nodeFactory.createNode<IndexRangeAccess>(expression, index.start, *index.end);
		else
			expression = nodeFactory.createNode<IndexAccess>(expression, index.start);
	}
	return expression;
}

ASTPointer<ParameterList> Parser::createEmptyParameterList()
{
	RecursionGuard recursionGuard(*this);
	ASTNodeFactory nodeFactory(*this);
	nodeFactory.setLocationEmpty();
	return nodeFactory.createNode<ParameterList>(std::vector<ASTPointer<VariableDeclaration>>());
}

ASTPointer<ASTString> Parser::expectIdentifierToken()
{
	expectToken(Token::Identifier, false /* do not advance */);
	return getLiteralAndAdvance();
}

ASTPointer<ASTString> Parser::expectIdentifierTokenOrAddress()
{
	ASTPointer<ASTString> result;
	if (m_scanner->currentToken() == Token::Address)
	{
		result = std::make_shared<ASTString>("address");
		advance();
	}
	else
	{
		expectToken(Token::Identifier, false /* do not advance */);
		result = getLiteralAndAdvance();
	}
	return result;
}

ASTPointer<ASTString> Parser::getLiteralAndAdvance()
{
	ASTPointer<ASTString> identifier = std::make_shared<ASTString>(m_scanner->currentLiteral());
	advance();
	return identifier;
}

bool Parser::isQuotedPath() const
{
	return m_scanner->currentToken() == Token::StringLiteral;
}

bool Parser::isStdlibPath() const
{
	return m_experimentalSolidityEnabledInCurrentSourceUnit
		&& m_scanner->currentToken() == Token::Identifier
		&& m_scanner->currentLiteral() == "std";
}

ASTPointer<ASTString> Parser::getStdlibImportPathAndAdvance()
{
	ASTPointer<ASTString> std = expectIdentifierToken();
	if (m_scanner->currentToken() == Token::Period)
		advance();
	ASTPointer<ASTString> library = expectIdentifierToken();
	return std::make_shared<ASTString>(*std + "." + *library);
}

}
