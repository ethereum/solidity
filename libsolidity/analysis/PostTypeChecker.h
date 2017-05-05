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

namespace dev
{
namespace solidity
{

/**
 * This module performs analyses on the AST that are done after type checking and assignments of types:
 *  - whether there are circular references in constant state variables
 * @TODO factor out each use-case into an individual class (but do the traversal only once)
 */
class PostTypeChecker: private ASTConstVisitor
{
public:
	/// @param _errors the reference to the list of errors and warnings to add them found during type checking.
	PostTypeChecker(ErrorList& _errors): m_errors(_errors) {}

	bool check(ASTNode const& _astRoot);

private:
	/// Adds a new error to the list of errors.
	void typeError(SourceLocation const& _location, std::string const& _description);

	virtual bool visit(ContractDefinition const& _contract) override;
	virtual void endVisit(ContractDefinition const& _contract) override;

	virtual bool visit(VariableDeclaration const& _declaration) override;
	virtual void endVisit(VariableDeclaration const& _declaration) override;

	virtual bool visit(Identifier const& _identifier) override;

	VariableDeclaration const* findCycle(
		VariableDeclaration const* _startingFrom,
		std::set<VariableDeclaration const*> const& _seen = std::set<VariableDeclaration const*>{}
	);

	ErrorList& m_errors;

	VariableDeclaration const* m_currentConstVariable = nullptr;
	std::vector<VariableDeclaration const*> m_constVariables; ///< Required for determinism.
	std::map<VariableDeclaration const*, std::set<VariableDeclaration const*>> m_constVariableDependencies;
};

}
}
