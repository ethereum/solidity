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

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

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
	/// @param _errorReporter provides the error logging functionality.
	PostTypeChecker(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool check(ASTNode const& _astRoot);

private:
	/// Adds a new error to the list of errors.
	void typeError(langutil::SourceLocation const& _location, std::string const& _description);

	bool visit(ContractDefinition const& _contract) override;
	void endVisit(ContractDefinition const& _contract) override;

	bool visit(VariableDeclaration const& _variable) override;
	void endVisit(VariableDeclaration const& _variable) override;

	bool visit(Identifier const& _identifier) override;

	VariableDeclaration const* findCycle(VariableDeclaration const& _startingFrom);

	langutil::ErrorReporter& m_errorReporter;

	VariableDeclaration const* m_currentConstVariable = nullptr;
	std::vector<VariableDeclaration const*> m_constVariables; ///< Required for determinism.
	std::map<VariableDeclaration const*, std::set<VariableDeclaration const*>> m_constVariableDependencies;
};

}
}
