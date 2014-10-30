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
	ASTPointer<ContractDefinition> parse(std::shared_ptr<Scanner> const& _scanner);

private:
	class ASTNodeFactory;

	/// Start position of the current token
	int getPosition() const;
	/// End position of the current token
	int getEndPosition() const;

	///@{
	///@name Parsing functions for the AST nodes
	ASTPointer<ContractDefinition> parseContractDefinition();
	ASTPointer<FunctionDefinition> parseFunctionDefinition(bool _isPublic);
	ASTPointer<StructDefinition> parseStructDefinition();
	ASTPointer<VariableDeclaration> parseVariableDeclaration(bool _allowVar);
	ASTPointer<TypeName> parseTypeName(bool _allowVar);
	ASTPointer<Mapping> parseMapping();
	ASTPointer<ParameterList> parseParameterList(bool _allowEmpty = true);
	ASTPointer<Block> parseBlock();
	ASTPointer<Statement> parseStatement();
	ASTPointer<IfStatement> parseIfStatement();
	ASTPointer<WhileStatement> parseWhileStatement();
	ASTPointer<VariableDefinition> parseVariableDefinition();
	ASTPointer<ExpressionStatement> parseExpressionStatement();
	ASTPointer<Expression> parseExpression();
	ASTPointer<Expression> parseBinaryExpression(int _minPrecedence = 4);
	ASTPointer<Expression> parseUnaryExpression();
	ASTPointer<Expression> parseLeftHandSideExpression();
	ASTPointer<Expression> parsePrimaryExpression();
	std::vector<ASTPointer<Expression>> parseFunctionCallArguments();
	///@}

	///@{
	///@name Helper functions

	/// If current token value is not _value, throw exception otherwise advance token.
	void expectToken(Token::Value _value);
	Token::Value expectAssignmentOperator();
	ASTPointer<ASTString> expectIdentifierToken();
	ASTPointer<ASTString> getLiteralAndAdvance();
	///@}

	/// Creates a @ref ParserError exception and annotates it with the current position and the
	/// given @a _description.
	ParserError createParserError(std::string const& _description) const;

	std::shared_ptr<Scanner> m_scanner;
};

}
}
