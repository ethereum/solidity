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
#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

namespace solidity::frontend::experimental
{

class SyntaxRestrictor: public ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	explicit SyntaxRestrictor(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool check(ASTNode const& _astRoot);

private:
	/// Default visit will reject all AST nodes that are not explicitly allowed.
	bool visitNode(ASTNode const& _node) override;

	bool visit(SourceUnit const&) override { return true; }
	bool visit(PragmaDirective const&) override { return true; }
	bool visit(ImportDirective const&) override { return true; }
	bool visit(ContractDefinition const& _contractDefinition) override;
	bool visit(FunctionDefinition const& _functionDefinition) override;
	bool visit(ExpressionStatement const&) override { return true; }
	bool visit(Assignment const&) override { return true; }
	bool visit(Block const&) override { return true; }
	bool visit(InlineAssembly const&) override { return true; }
	bool visit(Identifier const&) override { return true; }
	bool visit(VariableDeclarationStatement const&) override;
	bool visit(VariableDeclaration const&) override;
	bool visit(ElementaryTypeName const&) override { return true; }
	bool visit(ParameterList const&) override { return true; }

	langutil::ErrorReporter& m_errorReporter;
};

}
