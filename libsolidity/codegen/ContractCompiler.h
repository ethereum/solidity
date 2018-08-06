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
		m_context = CompilerContext(_context.evmVersion(), _runtimeCompiler ? &_runtimeCompiler->m_context : nullptr);
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
	/// Appends code that deploys the given contract as a library.
	/// Will also add code that modifies the contract in memory by injecting the current address
	/// for the call protector.
	size_t deployLibrary(ContractDefinition const& _contract);
	/// Appends state variable initialisation and constructor code.
	void appendInitAndConstructorCode(ContractDefinition const& _contract);
	void appendBaseConstructor(FunctionDefinition const& _constructor);
	void appendConstructor(FunctionDefinition const& _constructor);
	/// Appends code that returns a boolean flag on the stack that tells whether
	/// the contract has been called via delegatecall (false) or regular call (true).
	/// This is done by inserting a specific push constant as the first instruction
	/// whose data will be modified in memory at deploy time.
	void appendDelegatecallCheck();
	void appendFunctionSelector(ContractDefinition const& _contract);
	void appendCallValueCheck();
	void appendReturnValuePacker(TypePointers const& _typeParameters, bool _isLibrary);

	void registerStateVariables(ContractDefinition const& _contract);
	void initializeStateVariables(ContractDefinition const& _contract);

	virtual bool visit(VariableDeclaration const& _variableDeclaration) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(InlineAssembly const& _inlineAssembly) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual bool visit(Continue const& _continueStatement) override;
	virtual bool visit(Break const& _breakStatement) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(Throw const& _throw) override;
	virtual bool visit(EmitStatement const& _emit) override;
	virtual bool visit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	virtual bool visit(ExpressionStatement const& _expressionStatement) override;
	virtual bool visit(PlaceholderStatement const&) override;
	virtual bool visit(Block const& _block) override;
	virtual void endVisit(Block const& _block) override;

	/// Repeatedly visits all function which are referenced but which are not compiled yet.
	void appendMissingFunctions();

	/// Appends one layer of function modifier code of the current function, or the function
	/// body itself if the last modifier was reached.
	void appendModifierOrFunctionCode();

	void appendStackVariableInitialisation(VariableDeclaration const& _variable);
	void compileExpression(Expression const& _expression, TypePointer const& _targetType = TypePointer());

	/// Frees the variables of a certain scope (to be used when leaving).
	void popScopedVariables(ASTNode const* _node);

	/// Sets the stack height for the visited loop.
	void storeStackHeight(ASTNode const* _node);

	bool const m_optimise;
	/// Pointer to the runtime compiler in case this is a creation compiler.
	ContractCompiler* m_runtimeCompiler = nullptr;
	CompilerContext& m_context;
	/// Tag to jump to for a "break" statement and the stack height after freeing the local loop variables.
	std::vector<std::pair<eth::AssemblyItem, unsigned>> m_breakTags;
	/// Tag to jump to for a "continue" statement and the stack height after freeing the local loop variables.
	std::vector<std::pair<eth::AssemblyItem, unsigned>> m_continueTags;
	/// Tag to jump to for a "return" statement and the stack height after freeing the local function or modifier variables.
	/// Needs to be stacked because of modifiers.
	std::vector<std::pair<eth::AssemblyItem, unsigned>> m_returnTags;
	unsigned m_modifierDepth = 0;
	FunctionDefinition const* m_currentFunction = nullptr;

	// arguments for base constructors, filled in derived-to-base order
	std::map<FunctionDefinition const*, ASTNode const*> const* m_baseArguments;

	/// Stores the variables that were declared inside a specific scope, for each modifier depth.
	std::map<unsigned, std::map<ASTNode const*, unsigned>> m_scopeStackHeight;
};

}
}
