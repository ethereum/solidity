// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Parses and analyses the doc strings.
 * Stores the parsing results in the AST annotations and reports errors.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Parses and analyses the doc strings.
 * Stores the parsing results in the AST annotations and reports errors.
 */
class DocStringAnalyser: private ASTConstVisitor
{
public:
	DocStringAnalyser(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}
	bool analyseDocStrings(SourceUnit const& _sourceUnit);

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

	void handleDeclaration(
		Declaration const& _declaration,
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
