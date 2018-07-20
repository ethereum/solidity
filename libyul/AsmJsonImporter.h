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
 * @author julius <djudju@protonmail.com>
 * @date 2019
 * Converts an inlineAssembly AST from JSON format to AsmData
 */

#pragma once

#include <liblangutil/SourceLocation.h>
#include <libsolutil/JSON.h>
#include <libyul/ASTForward.h>

#include <utility>

namespace solidity::yul
{

/**
 * Component that imports an AST from json format to the internal format
 */
class AsmJsonImporter
{
public:
	explicit AsmJsonImporter(std::vector<std::shared_ptr<std::string const>> const& _sourceNames):
		m_sourceNames(_sourceNames)
	{}
	yul::Block createBlock(Json const& _node);

private:
	langutil::SourceLocation const createSourceLocation(Json const& _node);
	template <class T>
	T createAsmNode(Json const& _node);
	/// helper function to access member functions of the JSON
	/// and throw an error if it does not exist
	Json member(Json const& _node, std::string const& _name);

	yul::Statement createStatement(Json const& _node);
	yul::Expression createExpression(Json const& _node);
	std::vector<yul::Statement> createStatementVector(Json const& _array);
	std::vector<yul::Expression> createExpressionVector(Json const& _array);

	yul::TypedName createTypedName(Json const& _node);
	yul::Literal createLiteral(Json const& _node);
	yul::Leave createLeave(Json const& _node);
	yul::Identifier createIdentifier(Json const& _node);
	yul::Assignment createAssignment(Json const& _node);
	yul::FunctionCall createFunctionCall(Json const& _node);
	yul::ExpressionStatement createExpressionStatement(Json const& _node);
	yul::VariableDeclaration createVariableDeclaration(Json const& _node);
	yul::FunctionDefinition createFunctionDefinition(Json const& _node);
	yul::If createIf(Json const& _node);
	yul::Case createCase(Json const& _node);
	yul::Switch createSwitch(Json const& _node);
	yul::ForLoop createForLoop(Json const& _node);
	yul::Break createBreak(Json const& _node);
	yul::Continue createContinue(Json const& _node);

	std::vector<std::shared_ptr<std::string const>> const& m_sourceNames;
};

}
