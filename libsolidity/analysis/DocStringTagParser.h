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

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Parses the doc tags and does basic validity checks.
 * Stores the parsing results in the AST annotations and reports errors.
 */
class DocStringTagParser: private ASTConstVisitor
{
public:
	explicit DocStringTagParser(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}
	bool parseDocStrings(SourceUnit const& _sourceUnit);

private:
	bool visit(ContractDefinition const& _contract) override;
	bool visit(FunctionDefinition const& _function) override;
	bool visit(VariableDeclaration const& _variable) override;
	bool visit(ModifierDefinition const& _modifier) override;
	bool visit(EventDefinition const& _event) override;

	void checkParameters(
		CallableDeclaration const& _callable,
		StructurallyDocumented const& _node,
		StructurallyDocumentedAnnotation& _annotation
	);

	void handleConstructor(
		CallableDeclaration const& _callable,
		StructurallyDocumented const& _node,
		StructurallyDocumentedAnnotation& _annotation
	);

	void handleCallable(
		CallableDeclaration const& _callable,
		StructurallyDocumented const& _node,
		StructurallyDocumentedAnnotation& _annotation
	);

	void parseDocStrings(
		StructurallyDocumented const& _node,
		StructurallyDocumentedAnnotation& _annotation,
		std::set<std::string> const& _validTags,
		std::string const& _nodeName
	);

	langutil::ErrorReporter& m_errorReporter;
};

}
