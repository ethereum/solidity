// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <liblangutil/ErrorReporter.h>

#include <memory>

namespace solidity::frontend
{

/**
 * Validates access and initialization of immutable variables:
 * must be directly initialized in their respective c'tor
 * can not be read by any function/modifier called by the c'tor (or the c'tor itself)
 * must be initialized outside loops (only one initialization)
 * must be initialized outside ifs (must be initialized unconditionally)
 * must be initialized exactly once (no multiple statements)
 * must be initialized exactly once (no early return to skip initialization)
*/
class ImmutableValidator: private ASTConstVisitor
{
	using CallableDeclarationSet = std::set<CallableDeclaration const*, ASTNode::CompareByID>;

public:
	ImmutableValidator(langutil::ErrorReporter& _errorReporter, ContractDefinition const& _contractDefinition):
		m_currentContract(_contractDefinition),
		m_errorReporter(_errorReporter)
	{ }

	void analyze();

private:
	bool visit(FunctionDefinition const& _functionDefinition);
	bool visit(ModifierDefinition const& _modifierDefinition);
	bool visit(MemberAccess const& _memberAccess);
	bool visit(IfStatement const& _ifStatement);
	bool visit(WhileStatement const& _whileStatement);
	void endVisit(Identifier const& _identifier);
	void endVisit(Return const& _return);

	bool analyseCallable(CallableDeclaration const& _callableDeclaration);
	void analyseVariableReference(VariableDeclaration const& _variableReference, Expression const& _expression);

	void checkAllVariablesInitialized(langutil::SourceLocation const& _location);

	void visitCallableIfNew(Declaration const& _declaration);

	ContractDefinition const& m_currentContract;

	CallableDeclarationSet m_visitedCallables;

	std::set<VariableDeclaration const*, ASTNode::CompareByID> m_initializedStateVariables;
	langutil::ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentConstructor = nullptr;
	bool m_inLoop = false;
	bool m_inBranch = false;
	bool m_inConstructionContext = false;
};

}
