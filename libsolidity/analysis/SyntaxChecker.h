/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
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
 * The module that performs syntax analysis on the AST:
 *  - whether continue/break is in a for/while loop.
 */
class SyntaxChecker: private ASTConstVisitor
{
public:
	/// @param _errors the reference to the list of errors and warnings to add them found during type checking.
	SyntaxChecker(ErrorList& _errors): m_errors(_errors) {}

	bool checkSyntax(SourceUnit const& _sourceUnit);

private:
	/// Adds a new error to the list of errors.
	void syntaxError(SourceLocation const& _location, std::string const& _description);

	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual void endVisit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual void endVisit(ForStatement const& _forStatement) override;

	virtual bool visit(Continue const& _continueStatement) override;
	virtual bool visit(Break const& _breakStatement) override;

	ErrorList& m_errors;

	int m_inLoopDepth = 0;
};

}
}
