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

namespace dev {
namespace solidity {

class Scanner;

class Parser
{
public:
	ptr<ASTNode> parse(std::shared_ptr<Scanner> const& _scanner);

private:
	class ASTNodeFactory;

	/// Start position of the current token
	int getPosition() const;
	/// End position of the current token
	int getEndPosition() const;

	/// Parsing functions for the AST nodes
	/// @{
	ptr<ContractDefinition> parseContractDefinition();
	ptr<FunctionDefinition> parseFunctionDefinition(bool _isPublic);
	ptr<StructDefinition> parseStructDefinition();
	ptr<VariableDeclaration> parseVariableDeclaration();
	ptr<TypeName> parseTypeName();
	ptr<Mapping> parseMapping();
	ptr<ParameterList> parseParameterList();
	ptr<Block> parseBlock();
	ptr<Statement> parseStatement();
	ptr<IfStatement> parseIfStatement();
	ptr<WhileStatement> parseWhileStatement();
	ptr<VariableDefinition> parseVariableDefinition();
	ptr<Expression> parseExpression();
	ptr<Expression> parseBinaryExpression(int _minPrecedence = 4);
	ptr<Expression> parseUnaryExpression();
	ptr<Expression> parseLeftHandSideExpression();
	ptr<Expression> parsePrimaryExpression();
	vecptr<Expression> parseFunctionCallArguments();
	/// @}

	/// Helper functions
	/// @{
	/// If current token value is not _value, throw exception otherwise advance token.
	void expectToken(Token::Value _value);
	Token::Value expectAssignmentOperator();
	std::string expectIdentifier();
	void throwExpectationError(const std::string& _description);
	/// @}

	std::shared_ptr<Scanner> m_scanner;
};

} }
