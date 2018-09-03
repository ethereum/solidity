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
 * The module that performs syntax analysis on the AST:
 *  - whether continue/break is in a for/while loop.
 *  - whether a modifier contains at least one '_'
 *  - issues deprecation warnings for unary '+'
 *  - issues deprecation warning for throw
 */
class SyntaxChecker: private ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	SyntaxChecker(ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool checkSyntax(ASTNode const& _astRoot);

private:

	virtual bool visit(SourceUnit const& _sourceUnit) override;
	virtual void endVisit(SourceUnit const& _sourceUnit) override;
	virtual bool visit(PragmaDirective const& _pragma) override;

	virtual bool visit(ModifierDefinition const& _modifier) override;
	virtual void endVisit(ModifierDefinition const& _modifier) override;

	// Reports an error if _statement is a VariableDeclarationStatement.
	// Used by if/while/for to check for single statement variable declarations
	// without a block.
	void checkSingleStatementVariableDeclaration(ASTNode const* _statement);

	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual void endVisit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual void endVisit(ForStatement const& _forStatement) override;

	virtual bool visit(Continue const& _continueStatement) override;
	virtual bool visit(Break const& _breakStatement) override;

	virtual bool visit(Throw const& _throwStatement) override;

	virtual bool visit(UnaryOperation const& _operation) override;

	virtual bool visit(PlaceholderStatement const& _placeholderStatement) override;

	virtual bool visit(ContractDefinition const& _contract) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(FunctionTypeName const& _node) override;

	virtual bool visit(VariableDeclarationStatement const& _statement) override;

	virtual bool visit(StructDefinition const& _struct) override;
	virtual bool visit(Literal const& _literal) override;

	ErrorReporter& m_errorReporter;

	/// Flag that indicates whether a function modifier actually contains '_'.
	bool m_placeholderFound = false;

	/// Flag that indicates whether some version pragma was present.
	bool m_versionPragmaFound = false;

	int m_inLoopDepth = 0;
	bool m_isInterface = false;

	SourceUnit const* m_sourceUnit = nullptr;
};

}
}
