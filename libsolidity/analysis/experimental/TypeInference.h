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
#include <libsolidity/ast/experimental/TypeSystem.h>

#include <liblangutil/ErrorReporter.h>

#include <range/v3/span.hpp>

namespace solidity::frontend::experimental
{

class TypeInference: public ASTConstVisitor
{
public:
	TypeInference(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool analyze(SourceUnit const& _sourceUnit);
private:
	bool visit(Block const&) override { return true; }
	bool visit(VariableDeclarationStatement const&) override { return true; }
	bool visit(VariableDeclaration const& _variableDeclaration) override;

	bool visit(FunctionDefinition const& _functionDefinition) override;
	bool visit(ParameterList const& _parameterList) override;
	bool visit(SourceUnit const&) override { return true; }
	bool visit(ContractDefinition const&) override { return true; }
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(PragmaDirective const&) override { return false; }

	bool visit(ExpressionStatement const&) override { return true; }
	bool visit(Assignment const&) override;
	bool visit(Identifier const&) override;

	bool visitNode(ASTNode const& _node) override;

	Type fromTypeName(TypeName const& _typeName);
	TypeSystem m_typeSystem;
	langutil::ErrorReporter& m_errorReporter;
	TypeEnvironment m_env;
};

}