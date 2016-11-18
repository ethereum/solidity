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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Code generator for contracts.
 */

#pragma once

#include <ostream>
#include <functional>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libevmasm/Assembly.h>

namespace dev {
namespace solidity {

/**
 * Code generator at the contract level. Can be used to generate code for exactly one contract
 * either either in "runtime mode" or "creation mode".
 */
class ContractCompiler: private ASTConstVisitor
{
public:
	explicit ContractCompiler(ContractCompiler* _runtimeCompiler, CompilerContext& _context, bool _optimise):
		m_optimise(_optimise),
		m_runtimeCompiler(_runtimeCompiler),
		m_context(_context)
	{
		m_context = CompilerContext(_runtimeCompiler ? &_runtimeCompiler->m_context : nullptr);
	}

	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _contracts
	);
	/// Compiles the constructor part of the contract.
	/// @returns the identifier of the runtime sub-assembly.
	size_t compileConstructor(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _contracts
	);
	/// Compiles a contract that uses DELEGATECALL to call into a pre-deployed version of the given
	/// contract at runtime, but contains the full creation-time code.
	/// @returns the identifier of the runtime sub-assembly.
	size_t compileClone(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _contracts
	);

private:
	/// Registers the non-function objects inside the contract with the context and stores the basic
	/// information about the contract like the AST annotations.
	void initializeContext(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _compiledContracts
	);
	/// Adds the code that is run at creation time. Should be run after exchanging the run-time context
	/// with a new and initialized context. Adds the constructor code.
	/// @returns the identifier of the runtime sub assembly
	size_t packIntoContractCreator(ContractDefinition const& _contract);
	/// Appends state variable initialisation and constructor code.
	void appendInitAndConstructorCode(ContractDefinition const& _contract);
	void appendBaseConstructor(FunctionDefinition const& _constructor);
	void appendConstructor(FunctionDefinition const& _constructor);
	void appendFunctionSelector(ContractDefinition const& _contract);
	void appendCallValueCheck();
	/// Creates code that unpacks the arguments for the given function represented by a vector of TypePointers.
	/// From memory if @a _fromMemory is true, otherwise from call data.
	/// Expects source offset on the stack, which is removed.
	void appendCalldataUnpacker(TypePointers const& _typeParameters, bool _fromMemory = false);
	void appendReturnValuePacker(TypePointers const& _typeParameters, bool _isLibrary);

	void registerStateVariables(ContractDefinition const& _contract);
	void initializeStateVariables(ContractDefinition const& _contract);

	virtual bool visit(VariableDeclaration const& _variableDeclaration) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(InlineAssembly const& _inlineAssembly) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual bool visit(Continue const& _continue) override;
	virtual bool visit(Break const& _break) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(Throw const& _throw) override;
	virtual bool visit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	virtual bool visit(ExpressionStatement const& _expressionStatement) override;
	virtual bool visit(PlaceholderStatement const&) override;

	/// Repeatedly visits all function which are referenced but which are not compiled yet.
	void appendMissingFunctions();

	/// Appends one layer of function modifier code of the current function, or the function
	/// body itself if the last modifier was reached.
	void appendModifierOrFunctionCode();

	void appendStackVariableInitialisation(VariableDeclaration const& _variable);
	void compileExpression(Expression const& _expression, TypePointer const& _targetType = TypePointer());

	/// @returns the runtime assembly for clone contracts.
	static eth::AssemblyPointer cloneRuntime();

	bool const m_optimise;
	/// Pointer to the runtime compiler in case this is a creation compiler.
	ContractCompiler* m_runtimeCompiler = nullptr;
	CompilerContext& m_context;
	std::vector<eth::AssemblyItem> m_breakTags; ///< tag to jump to for a "break" statement
	std::vector<eth::AssemblyItem> m_continueTags; ///< tag to jump to for a "continue" statement
	/// Tag to jump to for a "return" statement, needs to be stacked because of modifiers.
	std::vector<eth::AssemblyItem> m_returnTags;
	unsigned m_modifierDepth = 0;
	FunctionDefinition const* m_currentFunction = nullptr;
	unsigned m_stackCleanupForReturn = 0; ///< this number of stack elements need to be removed before jump to m_returnTag
	// arguments for base constructors, filled in derived-to-base order
	std::map<FunctionDefinition const*, std::vector<ASTPointer<Expression>> const*> m_baseArguments;
};

}
}
