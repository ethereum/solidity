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
 * programmers write cleaner code. For every warning generated here, it has to be possible to write
 * equivalent code that does not generate the warning.
 */
class StaticAnalyzer: private ASTConstVisitor
{
public:
	/// @param _errorReporter provides the error logging functionality.
	explicit StaticAnalyzer(ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	/// Performs static analysis on the given source unit and all of its sub-nodes.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool analyze(SourceUnit const& _sourceUnit);

private:

	bool visit(ContractDefinition const& _contract) override;
	void endVisit(ContractDefinition const& _contract) override;

	bool visit(FunctionDefinition const& _function) override;
	void endVisit(FunctionDefinition const& _function) override;

	bool visit(ExpressionStatement const& _statement) override;
	bool visit(VariableDeclaration const& _variable) override;
	bool visit(Identifier const& _identifier) override;
	bool visit(Return const& _return) override;
	bool visit(MemberAccess const& _memberAccess) override;
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(BinaryOperation const& _operation) override;
	bool visit(FunctionCall const& _functionCall) override;

	/// @returns the size of this type in storage, including all sub-types.
	static bigint structureSizeEstimate(Type const& _type, std::set<StructDefinition const*>& _structsSeen);

	ErrorReporter& m_errorReporter;

	/// Flag that indicates whether the current contract definition is a library.
	bool m_library = false;

	/// Number of uses of each (named) local variable in a function, counter is initialized with zero.
	/// Pairs of AST ids and pointers are used as keys to ensure a deterministic order
	/// when traversing.
	std::map<std::pair<size_t, VariableDeclaration const*>, int> m_localVarUseCount;

	FunctionDefinition const* m_currentFunction = nullptr;

	/// Flag that indicates a constructor.
	bool m_constructor = false;

	/// Current contract.
	ContractDefinition const* m_currentContract = nullptr;
};

}
}
