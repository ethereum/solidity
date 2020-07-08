// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <liblangutil/EVMVersion.h>

#include <boost/noncopyable.hpp>
#include <list>
#include <map>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Assigns types to declarations.
 */
class DeclarationTypeChecker: private ASTConstVisitor
{
public:
	DeclarationTypeChecker(
		langutil::ErrorReporter& _errorReporter,
		langutil::EVMVersion _evmVersion
	):
		m_errorReporter(_errorReporter),
		m_evmVersion(_evmVersion)
	{}

	bool check(ASTNode const& _contract);

private:

	bool visit(ElementaryTypeName const& _typeName) override;
	void endVisit(UserDefinedTypeName const& _typeName) override;
	bool visit(FunctionTypeName const& _typeName) override;
	void endVisit(Mapping const& _mapping) override;
	void endVisit(ArrayTypeName const& _typeName) override;
	void endVisit(VariableDeclaration const& _variable) override;
	bool visit(StructDefinition const& _struct) override;
	void endVisit(UsingForDirective const& _usingForDirective) override;

	langutil::ErrorReporter& m_errorReporter;
	langutil::EVMVersion m_evmVersion;
	bool m_insideFunctionType = false;
	bool m_recursiveStructSeen = false;
	std::set<StructDefinition const*> m_currentStructsSeen;
};

}
