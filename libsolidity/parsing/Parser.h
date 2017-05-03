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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity parser.
 */

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/parsing/ParserBase.h>

namespace dev
{
namespace solidity
{

class Scanner;

class Parser: public ParserBase
{
public:
	Parser(ErrorList& _errors): ParserBase(_errors) {}

	ASTPointer<SourceUnit> parse(std::shared_ptr<Scanner> const& _scanner);

private:
	class ASTNodeFactory;

	struct VarDeclParserOptions
	{
		VarDeclParserOptions() {}
		bool allowVar = false;
		bool isStateVariable = false;
		bool allowIndexed = false;
		bool allowEmptyName = false;
		bool allowInitialValue = false;
		bool allowLocationSpecifier = false;
	};

	/// This struct is shared for parsing a function header and a function type.
	struct FunctionHeaderParserResult
	{
		ASTPointer<ASTString> name;
		ASTPointer<ParameterList> parameters;
		ASTPointer<ParameterList> returnParameters;
		Declaration::Visibility visibility = Declaration::Visibility::Default;
		bool isDeclaredConst = false;
		bool isPayable = false;
		std::vector<ASTPointer<ModifierInvocation>> modifiers;
	};

	///@{
	///@name Parsing functions for the AST nodes
	ASTPointer<PragmaDirective> parsePragmaDirective();
	ASTPointer<ImportDirective> parseImportDirective();
	ContractDefinition::ContractKind tokenToContractKind(Token::Value _token);
	ASTPointer<ContractDefinition> parseContractDefinition(Token::Value _expectedKind);
	ASTPointer<InheritanceSpecifier> parseInheritanceSpecifier();
	Declaration::Visibility parseVisibilitySpecifier(Token::Value _token);
	FunctionHeaderParserResult parseFunctionHeader(bool _forceEmptyName, bool _allowModifiers);
	ASTPointer<ASTNode> parseFunctionDefinitionOrFunctionTypeStateVariable(ASTString const* _contractName);
	ASTPointer<FunctionDefinition> parseFunctionDefinition(ASTString const* _contractName);
	ASTPointer<StructDefinition> parseStructDefinition();
	ASTPointer<EnumDefinition> parseEnumDefinition();
	ASTPointer<EnumValue> parseEnumValue();
	ASTPointer<VariableDeclaration> parseVariableDeclaration(
		VarDeclParserOptions const& _options = VarDeclParserOptions(),
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>()
	);
	ASTPointer<ModifierDefinition> parseModifierDefinition();
	ASTPointer<EventDefinition> parseEventDefinition();
	ASTPointer<UsingForDirective> parseUsingDirective();
	ASTPointer<ModifierInvocation> parseModifierInvocation();
	ASTPointer<Identifier> parseIdentifier();
	ASTPointer<UserDefinedTypeName> parseUserDefinedTypeName();
	ASTPointer<TypeName> parseTypeNameSuffix(ASTPointer<TypeName> type, ASTNodeFactory& nodeFactory);
	ASTPointer<TypeName> parseTypeName(bool _allowVar);
	ASTPointer<FunctionTypeName> parseFunctionType();
	ASTPointer<Mapping> parseMapping();
	ASTPointer<ParameterList> parseParameterList(
		VarDeclParserOptions const& _options,
		bool _allowEmpty = true
	);
	ASTPointer<Block> parseBlock(ASTPointer<ASTString> const& _docString = {});
	ASTPointer<Statement> parseStatement();
	ASTPointer<InlineAssembly> parseInlineAssembly(ASTPointer<ASTString> const& _docString = {});
	ASTPointer<IfStatement> parseIfStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<WhileStatement> parseWhileStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<WhileStatement> parseDoWhileStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<ForStatement> parseForStatement(ASTPointer<ASTString> const& _docString);
	/// A "simple statement" can be a variable declaration statement or an expression statement.
	ASTPointer<Statement> parseSimpleStatement(ASTPointer<ASTString> const& _docString);
	ASTPointer<VariableDeclarationStatement> parseVariableDeclarationStatement(
		ASTPointer<ASTString> const& _docString,
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>()
	);
	ASTPointer<ExpressionStatement> parseExpressionStatement(
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseBinaryExpression(int _minPrecedence = 4,
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseUnaryExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parseLeftHandSideExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>()
	);
	ASTPointer<Expression> parsePrimaryExpression();
	std::vector<ASTPointer<Expression>> parseFunctionCallListArguments();
	std::pair<std::vector<ASTPointer<Expression>>, std::vector<ASTPointer<ASTString>>> parseFunctionCallArguments();
	///@}

	///@{
	///@name Helper functions

	/// Used as return value of @see peekStatementType.
	enum class LookAheadInfo
	{
		IndexAccessStructure, VariableDeclarationStatement, ExpressionStatement
	};

	/// Performs limited look-ahead to distinguish between variable declaration and expression statement.
	/// For source code of the form "a[][8]" ("IndexAccessStructure"), this is not possible to
	/// decide with constant look-ahead.
	LookAheadInfo peekStatementType() const;
	/// @returns a typename parsed in look-ahead fashion from something like "a.b[8][2**70]".
	ASTPointer<TypeName> typeNameIndexAccessStructure(
		std::vector<ASTPointer<PrimaryExpression>> const& _path,
		std::vector<std::pair<ASTPointer<Expression>, SourceLocation>> const& _indices
	);
	/// @returns an expression parsed in look-ahead fashion from something like "a.b[8][2**70]".
	ASTPointer<Expression> expressionFromIndexAccessStructure(
		std::vector<ASTPointer<PrimaryExpression>> const& _path,
		std::vector<std::pair<ASTPointer<Expression>, SourceLocation>> const& _indices
	);
	///@}

	/// Creates an empty ParameterList at the current location (used if parameters can be omitted).
	ASTPointer<ParameterList> createEmptyParameterList();

	/// Flag that signifies whether '_' is parsed as a PlaceholderStatement or a regular identifier.
	bool m_insideModifier = false;
};

}
}
