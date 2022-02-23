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

#include <memory>

namespace solidity::frontend
{

/**
 * Validates access and initialization of immutable variables:
 * must be directly initialized in their respective c'tor or inline
 * cannot be read before being initialized
 * cannot be read when initializing state variables inline
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
		m_mostDerivedContract(_contractDefinition),
		m_errorReporter(_errorReporter)
	{ }

	void analyze();

private:
	bool visit(Assignment const& _assignment);
	bool visit(FunctionDefinition const& _functionDefinition);
	bool visit(ModifierDefinition const& _modifierDefinition);
	bool visit(MemberAccess const& _memberAccess);
	bool visit(IfStatement const& _ifStatement);
	bool visit(WhileStatement const& _whileStatement);
	void endVisit(IdentifierPath const& _identifierPath);
	void endVisit(Identifier const& _identifier);
	void endVisit(Return const& _return);

	bool analyseCallable(CallableDeclaration const& _callableDeclaration);
	void analyseVariableReference(VariableDeclaration const& _variableReference, Expression const& _expression);

	void checkAllVariablesInitialized(langutil::SourceLocation const& _location);

	void visitCallableIfNew(Declaration const& _declaration);

	ContractDefinition const& m_mostDerivedContract;

	CallableDeclarationSet m_visitedCallables;

	std::set<VariableDeclaration const*, ASTNode::CompareByID> m_initializedStateVariables;
	langutil::ErrorReporter& m_errorReporter;

	FunctionDefinition const* m_currentConstructor = nullptr;
	ContractDefinition const* m_currentConstructorContract = nullptr;
	bool m_inLoop = false;
	bool m_inBranch = false;
	bool m_inCreationContext = true;
};

}
