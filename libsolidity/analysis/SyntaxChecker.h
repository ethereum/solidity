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
}

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
 *  - whether the msize instruction is used and the Yul optimizer is enabled at the same time.
 */
class SyntaxChecker: private ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	SyntaxChecker(langutil::ErrorReporter& _errorReporter, bool _useYulOptimizer):
		m_errorReporter(_errorReporter),
		m_useYulOptimizer(_useYulOptimizer)
	{}

	bool checkSyntax(ASTNode const& _astRoot);

private:

	bool visit(SourceUnit const& _sourceUnit) override;
	void endVisit(SourceUnit const& _sourceUnit) override;
	bool visit(PragmaDirective const& _pragma) override;

	bool visit(ModifierDefinition const& _modifier) override;
	void endVisit(ModifierDefinition const& _modifier) override;

	/// Reports an error if _statement is a VariableDeclarationStatement.
	/// Used by if/while/for to check for single statement variable declarations
	/// without a block.
	void checkSingleStatementVariableDeclaration(ASTNode const& _statement);

	bool visit(IfStatement const& _ifStatement) override;
	bool visit(WhileStatement const& _whileStatement) override;
	void endVisit(WhileStatement const& _whileStatement) override;
	bool visit(ForStatement const& _forStatement) override;
	void endVisit(ForStatement const& _forStatement) override;

	bool visit(Continue const& _continueStatement) override;
	bool visit(Break const& _breakStatement) override;

	bool visit(Throw const& _throwStatement) override;

	bool visit(UnaryOperation const& _operation) override;

	bool visit(InlineAssembly const& _inlineAssembly) override;

	bool visit(PlaceholderStatement const& _placeholderStatement) override;

	bool visit(ContractDefinition const& _contract) override;
	bool visit(FunctionDefinition const& _function) override;
	bool visit(FunctionTypeName const& _node) override;

	bool visit(VariableDeclarationStatement const& _statement) override;

	bool visit(StructDefinition const& _struct) override;
	bool visit(Literal const& _literal) override;

	langutil::ErrorReporter& m_errorReporter;

	bool m_useYulOptimizer = false;

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
