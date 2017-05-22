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
/**
 * @author Federico Bond <federicobond@gmail.com>
 * @date 2016
 * Static analyzer and checker.
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
 * The module that performs static analysis on the AST.
 * In this context, static analysis is anything that can produce warnings which can help
 * programmers write cleaner code. For every warning generated eher, it has to be possible to write
 * equivalent code that does generate the warning.
 */
class StaticAnalyzer: private ASTConstVisitor
{
public:
	/// @param _errors the reference to the list of errors and warnings to add them found during static analysis.
	explicit StaticAnalyzer(ErrorList& _errors): m_errors(_errors) {}

	/// Performs static analysis on the given source unit and all of its sub-nodes.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool analyze(SourceUnit const& _sourceUnit);

private:
	/// Adds a new warning to the list of errors.
	void warning(SourceLocation const& _location, std::string const& _description);

	virtual bool visit(ContractDefinition const& _contract) override;
	virtual void endVisit(ContractDefinition const& _contract) override;

	virtual bool visit(FunctionDefinition const& _function) override;
	virtual void endVisit(FunctionDefinition const& _function) override;

	virtual bool visit(ExpressionStatement const& _statement) override;
	virtual bool visit(VariableDeclaration const& _variable) override;
	virtual bool visit(Identifier const& _identifier) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(MemberAccess const& _memberAccess) override;
	virtual bool visit(InlineAssembly const& _inlineAssembly) override;

	ErrorList& m_errors;

	/// Flag that indicates whether the current contract definition is a library.
	bool m_library = false;

	/// Flag that indicates whether a public function does not contain the "payable" modifier.
	bool m_nonPayablePublic = false;

	/// Number of uses of each (named) local variable in a function, counter is initialized with zero.
	std::map<VariableDeclaration const*, int> m_localVarUseCount;

	FunctionDefinition const* m_currentFunction = nullptr;
};

}
}
