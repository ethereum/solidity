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

#include <libsolutil/JSON.h>
#include <liblangutil/SourceLocation.h>
#include <libyul/ASTForward.h>
#include <libyul/YulName.h>

#include <utility>

namespace solidity::yul
{

/**
 * Component that imports an AST from json format to the internal format
 */
class AsmJsonImporter
{
public:
	explicit AsmJsonImporter(std::vector<std::shared_ptr<std::string const>> const& _sourceNames, Dialect const& _dialect):
		m_dialect(_dialect), m_sourceNames(_sourceNames)
	{}

	yul::AST createAST(Json const& node);
private:
	langutil::SourceLocation const createSourceLocation(Json const& _node);
	template <class T>
	T createAsmNode(Json const& _node);
	/// helper function to access member functions of the JSON
	/// and throw an error if it does not exist
	Json member(Json const& _node, std::string const& _name);

	yul::Block createBlock(Json const& _node, YulNameRepository& _nameRepository);
	yul::Statement createStatement(Json const& _node, YulNameRepository& _nameRepository);
	yul::Expression createExpression(Json const& _node, YulNameRepository& _nameRepository);
	std::vector<yul::Statement> createStatementVector(Json const& _array, YulNameRepository& _nameRepository);
	std::vector<yul::Expression> createExpressionVector(Json const& _array, YulNameRepository& _nameRepository);

	yul::TypedName createTypedName(Json const& _node, YulNameRepository& _nameRepository);
	yul::Literal createLiteral(Json const& _node, YulNameRepository& _nameRepository);
	yul::Leave createLeave(Json const& _node);
	yul::Identifier createIdentifier(Json const& _node, YulNameRepository& _nameRepository);
	yul::Assignment createAssignment(Json const& _node, YulNameRepository& _nameRepository);
	yul::FunctionCall createFunctionCall(Json const& _node, YulNameRepository& _nameRepository);
	yul::ExpressionStatement createExpressionStatement(Json const& _node, YulNameRepository& _nameRepository);
	yul::VariableDeclaration createVariableDeclaration(Json const& _node, YulNameRepository& _nameRepository);
	yul::FunctionDefinition createFunctionDefinition(Json const& _node, YulNameRepository& _nameRepository);
	yul::If createIf(Json const& _node, YulNameRepository& _nameRepository);
	yul::Case createCase(Json const& _node, YulNameRepository& _nameRepository);
	yul::Switch createSwitch(Json const& _node, YulNameRepository& _nameRepository);
	yul::ForLoop createForLoop(Json const& _node, YulNameRepository& _nameRepository);
	yul::Break createBreak(Json const& _node);
	yul::Continue createContinue(Json const& _node);

	Dialect const& m_dialect;
	std::vector<std::shared_ptr<std::string const>> const& m_sourceNames;
};

}
