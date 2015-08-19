/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity parser.
 */

#pragma once

#include "libsolidity/AST.h"

namespace dev
{
namespace solidity
{

class Scanner;

class Parser
{
public:
	Parser() {}

	ASTPointer<SourceUnit> parse(std::shared_ptr<Scanner> const& _scanner);
	std::shared_ptr<std::string const> const& getSourceName() const;

private:
	class ASTNodeFactory;

	/// Start position of the current token
	int getPosition() const;
	/// End position of the current token
	int getEndPosition() const;

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

	///@{
	///@name Parsing functions for the AST nodes
	ASTPointer<ImportDirective> parseImportDirective();
	ASTPointer<ContractDefinition> parseContractDefinition();
	ASTPointer<InheritanceSpecifier> parseInheritanceSpecifier();
	Declaration::Visibility parseVisibilitySpecifier(Token::Value _token);
	ASTPointer<FunctionDefinition> parseFunctionDefinition(ASTString const* _contractName);
	ASTPointer<StructDefinition> parseStructDefinition();
	ASTPointer<EnumDefinition> parseEnumDefinition();
	ASTPointer<EnumValue> parseEnumValue();
	ASTPointer<VariableDeclaration> parseVariableDeclaration(VarDeclParserOptions const& _options = VarDeclParserOptions(),
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>());
	ASTPointer<ModifierDefinition> parseModifierDefinition();
	ASTPointer<EventDefinition> parseEventDefinition();
	ASTPointer<ModifierInvocation> parseModifierInvocation();
	ASTPointer<Identifier> parseIdentifier();
	ASTPointer<TypeName> parseTypeName(bool _allowVar);
	ASTPointer<Mapping> parseMapping();
	ASTPointer<ParameterList> parseParameterList(
		VarDeclParserOptions const& _options,
		bool _allowEmpty = true
	);
	ASTPointer<Block> parseBlock();
	ASTPointer<Statement> parseStatement();
	ASTPointer<IfStatement> parseIfStatement();
	ASTPointer<WhileStatement> parseWhileStatement();
	ASTPointer<ForStatement> parseForStatement();
	/// A "simple statement" can be a variable declaration statement or an expression statement.
	ASTPointer<Statement> parseSimpleStatement();
	ASTPointer<VariableDeclarationStatement> parseVariableDeclarationStatement(
		ASTPointer<TypeName> const& _lookAheadArrayType = ASTPointer<TypeName>());
	ASTPointer<ExpressionStatement> parseExpressionStatement(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>());
	ASTPointer<Expression> parseExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>());
	ASTPointer<Expression> parseBinaryExpression(int _minPrecedence = 4,
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>());
	ASTPointer<Expression> parseUnaryExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>());
	ASTPointer<Expression> parseLeftHandSideExpression(
		ASTPointer<Expression> const& _lookAheadIndexAccessStructure = ASTPointer<Expression>());
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
	/// Returns a typename parsed in look-ahead fashion from something like "a[8][2**70]".
	ASTPointer<TypeName> typeNameIndexAccessStructure(
		ASTPointer<PrimaryExpression> const& _primary,
		std::vector<std::pair<ASTPointer<Expression>, SourceLocation>> const& _indices);
	/// Returns an expression parsed in look-ahead fashion from something like "a[8][2**70]".
	ASTPointer<Expression> expressionFromIndexAccessStructure(
		ASTPointer<PrimaryExpression> const& _primary,
		std::vector<std::pair<ASTPointer<Expression>, SourceLocation>> const& _indices);
	/// If current token value is not _value, throw exception otherwise advance token.
	void expectToken(Token::Value _value);
	Token::Value expectAssignmentOperator();
	ASTPointer<ASTString> expectIdentifierToken();
	ASTPointer<ASTString> getLiteralAndAdvance();
	///@}

	/// Creates an empty ParameterList at the current location (used if parameters can be omitted).
	ASTPointer<ParameterList> createEmptyParameterList();

	/// Creates a @ref ParserError exception and annotates it with the current position and the
	/// given @a _description.
	ParserError createParserError(std::string const& _description) const;

	std::shared_ptr<Scanner> m_scanner;
	/// Flag that signifies whether '_' is parsed as a PlaceholderStatement or a regular identifier.
	bool m_insideModifier = false;
};

}
}
