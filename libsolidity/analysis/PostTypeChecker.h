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

#pragma once

#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::frontend
{

/**
 * This module performs analyses on the AST that are done after type checking and assignments of types:
 *  - whether there are circular references in constant state variables
 *  - whether override specifiers are actually contracts
 *  - whether a modifier is in a function header
 *  - whether an event is used outside of an emit statement
 *  - whether a variable is declared in a interface
 *
 *  When adding a new checker, make sure a visitor that forwards calls that your
 *  checker uses exists in PostTypeChecker. Add missing ones.
 *
 *  The return value for the visit function of a checker is ignored, all nodes
 *  will always be visited.
 */
class PostTypeChecker: private ASTConstVisitor
{
public:
	struct Checker: public ASTConstVisitor
	{
		Checker(langutil::ErrorReporter& _errorReporter):
			m_errorReporter(_errorReporter) {}
	protected:
		langutil::ErrorReporter& m_errorReporter;
	};

	/// @param _errorReporter provides the error logging functionality.
	PostTypeChecker(langutil::ErrorReporter& _errorReporter);

	bool check(ASTNode const& _astRoot);

private:
	bool visit(ContractDefinition const& _contract) override;
	void endVisit(ContractDefinition const& _contract) override;
	void endVisit(OverrideSpecifier const& _overrideSpecifier) override;

	bool visit(VariableDeclaration const& _variable) override;
	void endVisit(VariableDeclaration const& _variable) override;

	bool visit(EmitStatement const& _emit) override;
	void endVisit(EmitStatement const& _emit) override;

	bool visit(FunctionCall const& _functionCall) override;

	bool visit(Identifier const& _identifier) override;

	bool visit(StructDefinition const& _struct) override;
	void endVisit(StructDefinition const& _struct) override;

	bool visit(ModifierInvocation const& _modifierInvocation) override;
	void endVisit(ModifierInvocation const& _modifierInvocation) override;

	template <class T>
	bool callVisit(T const& _node)
	{
		for (auto& checker: m_checkers)
			checker->visit(_node);

		return true;
	}

	template <class T>
	void callEndVisit(T const& _node)
	{
		for (auto& checker: m_checkers)
			checker->endVisit(_node);
	}

	langutil::ErrorReporter& m_errorReporter;

	std::vector<std::shared_ptr<Checker>> m_checkers;
};

}
