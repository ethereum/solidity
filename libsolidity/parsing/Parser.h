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

#pragma once

#include <libsolidity/ast/AST.h>
#include <liblangutil/ParserBase.h>
#include <liblangutil/EVMVersion.h>

namespace solidity::langutil
{
class CharStream;
}

namespace solidity::frontend
{

class Parser: public langutil::ParserBase
{
public:
	explicit Parser(
		langutil::ErrorReporter& _errorReporter,
		langutil::EVMVersion _evmVersion,
		bool _errorRecovery = false
	):
		ParserBase(_errorReporter, _errorRecovery),
		m_evmVersion(_evmVersion)
	{}

	ASTPointer<SourceUnit> parse(langutil::CharStream& _charStream);

private:
	class ASTNodeFactory;

	enum class VarDeclKind { FileLevel, State, Other };
	struct VarDeclParserOptions
	{
		// This is actually not needed, but due to a defect in the C++ standard, we have to.
		// https://stackoverflow.com/questions/17430377
		VarDeclParserOptions() {}
		VarDeclKind kind = VarDeclKind::Other;
		bool allowIndexed = false;
		bool allowEmptyName = false;
		bool allowInitialValue = false;
		bool allowLocationSpecifier = false;
	};

	/// This struct is shared for parsing a function header and a function type.
	struct FunctionHeaderParserResult
	{
		bool isVirtual = false;
		ASTPointer<OverrideSpecifier> overrides;
		ASTPointer<ParameterList> parameters;
		ASTPointer<ParameterList> returnParameters;
		Visibility visibility = Visibility::Default;
		StateMutability stateMutability = StateMutability::NonPayable;
		std::vector<ASTPointer<ModifierInvocation>> modifiers;
	};

	/// Struct to share parsed function call arguments.
	struct FunctionCallArguments
	{
		std::vector<ASTPointer<Expression>> arguments;
		std::vector<ASTPointer<ASTString>> parameterNames;
		std::vector<langutil::SourceLocation> parameterNameLocations;
	};

	///@{
	///@name Parsing functions for the AST nodes
	void parsePragmaVersion(langutil::SourceLocation const& _location, std::vector<Token> const& _tokens, std::vector<std::string> const& _literals);
	ASTPointer<StructuredDocumentation> parseStructuredDocumentation();
	ASTPointer<PragmaDirective> parsePragmaDirective();
	ASTPointer<ImportDirective> parseImportDirective();
	/// @returns an std::pair<ContractKind, bool>, where
	/// result.second is set to true, if an abstract contract was parsed, false otherwise.
	std::pair<ContractKind, bool> parseContractKind();
	ASTPointer<ContractDefinition> parseContractDefinition();
	ASTPointer<InheritanceSpecifier> parseInheritanceSpecifier();
	Visibility parseVisibilitySpecifier();
	ASTPointer<OverrideSpecifier> parseOverrideSpecifier();
	StateMutability parseStateMutability();
	FunctionHeaderParserResult parseFunctionHeader(bool _isStateVariable);
	ASTPointer<ASTNode> parseFunctionDefinition(bool _freeFunction = false);
	ASTPointer<StructDefinition> parseStructDefinition();
	ASTPointer<EnumDefinition> parseEnumDefinition();
	ASTPointer<UserDefinedValueTypeDefinition> parseUserDefinedValueTypeDefinition();
	ASTPointer<EnumValue> parseEnumValue();
	ASTPointer<VariableDeclaration> parseVariableDeclaration(
		VarDeclParserOptions const& _options = {},
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>()
	);
	ASTPointer<ModifierDefinition> parseModifierDefinition();
	ASTPointer<EventDefinition> parseEventDefinition();
	ASTPointer<ErrorDefinition> parseErrorDefinition();
	ASTPointer<UsingForDirective> parseUsingDirective();
	ASTPointer<ModifierInvocation> parseModifierInvocation();
	ASTPointer<Identifier> parseIdentifier();
	ASTPointer<Identifier> parseIdentifierOrAddress();
	ASTPointer<UserDefinedTypeName> parseUserDefinedTypeName();
	ASTPointer<IdentifierPath> parseIdentifierPath();
	ASTPointer<TypeName> parseTypeNameSuffix(ASTPointer<TypeName> type, ASTNodeFactory& nodeFactory);
	ASTPointer<TypeName> parseTypeName();
	ASTPointer<FunctionTypeName> parseFunctionType();
	ASTPointer<Mapping> parseMapping();
	ASTPointer<ParameterList> parseParameterList(
		VarDeclParserOptions const& _options = {},
		bool _allowEmpty = true
	);
	ASTPointer<Block> parseBlock(bool _allowUncheckedBlock = false, ASTPointer<ASTString> const& _docString = {});
	ASTPointer<Statement> parseStatement(bool _allowUncheckedBlock = false);
	ASTPointer<InlineAssembly> parseInlineAssembly(ASTPointer<ASTString> const& _docString = {});
	ASTPointer<IfStatement> parseIfStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<TryStatement> parseTryStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<TryCatchClause> parseCatchClause();
	ASTPointer<WhileStatement> parseWhileStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<WhileStatement> parseDoWhileStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<ForStatement> parseForStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<EmitStatement> parseEmitStatement(ASTPointer<ASTString> const& docString);
	ASTPointer<RevertStatement> parseRevertStatement(ASTPointer<ASTString> const& docString);
	/// A "simple statement" can be a variable declaration statement or an expression statement.
	ASTPointer<Statement> parseSimpleStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<VariableDeclarationStatement> parseVariableDeclarationStatement(
		ASTPointer<ASTString> const& _docString,
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>()
	);
	ASTPointer<ExpressionStatement> parseExpressionStatement(
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> const& _partiallyParsedExpression = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseExpression(
		ASTPointer<Expression> const& _partiallyParsedExpression = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseBinaryExpression(int _minPrecedence = 4,
		ASTPointer<Expression> const& _partiallyParsedExpression = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseUnaryExpression(
		ASTPointer<Expression> const& _partiallyParsedExpression = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseLeftHandSideExpression(
		ASTPointer<Expression> const& _partiallyParsedExpression = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parsePrimaryExpression();
	std::vector<ASTPointer<Expression>> parseFunctionCallListArguments();

	FunctionCallArguments parseFunctionCallArguments();
	FunctionCallArguments parseNamedArguments();
	std::pair<ASTPointer<ASTString>, langutil::SourceLocation> expectIdentifierWithLocation();
	///@}

	///@{
	///@name Helper functions

	/// @return true if we are at the start of a variable declaration.
	bool variableDeclarationStart();

	/// Used as return value of @see peekStatementType.
	enum class LookAheadInfo
	{
		IndexAccessStructure, VariableDeclaration, Expression
	};
	/// Structure that represents a.b.c[x][y][z]. Can be converted either to an expression
	/// or to a type name. For this to be valid, path cannot be empty, but indices can be empty.
	struct IndexAccessedPath
	{
		struct Index
		{
			ASTPointer<Expression> start;
			std::optional<ASTPointer<Expression>> end;
			langutil::SourceLocation location;
		};
		std::vector<ASTPointer<PrimaryExpression>> path;
		std::vector<Index> indices;
		bool empty() const;
	};

	std::optional<std::string> findLicenseString(std::vector<ASTPointer<ASTNode>> const& _nodes);

	/// Returns the next AST node ID
	int64_t nextID() { return ++m_currentNodeID; }

	std::pair<LookAheadInfo, IndexAccessedPath> tryParseIndexAccessedPath();
	/// Performs limited look-ahead to distinguish between variable declaration and expression statement.
	/// For source code of the form "a[][8]" ("IndexAccessStructure"), this is not possible to
	/// decide with constant look-ahead.
	LookAheadInfo peekStatementType() const;
	/// @returns an IndexAccessedPath as a prestage to parsing a variable declaration (type name)
	/// or an expression;
	IndexAccessedPath parseIndexAccessedPath();
	/// @returns a typename parsed in look-ahead fashion from something like "a.b[8][2**70]",
	/// or an empty pointer if an empty @a _pathAndIncides has been supplied.
	ASTPointer<TypeName> typeNameFromIndexAccessStructure(IndexAccessedPath const& _pathAndIndices);
	/// @returns an expression parsed in look-ahead fashion from something like "a.b[8][2**70]",
	/// or an empty pointer if an empty @a _pathAndIncides has been supplied.
	ASTPointer<Expression> expressionFromIndexAccessStructure(IndexAccessedPath const& _pathAndIndices);

	ASTPointer<ASTString> expectIdentifierToken();
	ASTPointer<ASTString> expectIdentifierTokenOrAddress();
	ASTPointer<ASTString> getLiteralAndAdvance();
	///@}

	/// Creates an empty ParameterList at the current location (used if parameters can be omitted).
	ASTPointer<ParameterList> createEmptyParameterList();

	/// Flag that signifies whether '_' is parsed as a PlaceholderStatement or a regular identifier.
	bool m_insideModifier = false;
	langutil::EVMVersion m_evmVersion;
	/// Counter for the next AST node ID
	int64_t m_currentNodeID = 0;
};

}
