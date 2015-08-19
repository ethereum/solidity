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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity AST to EVM bytecode compiler.
 */

#pragma once

#include <ostream>
#include <functional>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/CompilerContext.h>
#include <libevmasm/Assembly.h>

namespace dev {
namespace solidity {

class Compiler: private ASTConstVisitor
{
public:
	explicit Compiler(bool _optimize = false, unsigned _runs = 200):
		m_optimize(_optimize),
		m_optimizeRuns(_runs),
		m_context(),
		m_returnTag(m_context.newTag())
	{
	}

	void compileContract(ContractDefinition const& _contract,
						 std::map<ContractDefinition const*, bytes const*> const& _contracts);
	/// Compiles a contract that uses CALLCODE to call into a pre-deployed version of the given
	/// contract at runtime, but contains the full creation-time code.
	void compileClone(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, bytes const*> const& _contracts
	);
	bytes getAssembledBytecode() { return m_context.getAssembledBytecode(); }
	bytes getRuntimeBytecode() { return m_context.getAssembledRuntimeBytecode(m_runtimeSub); }
	/// @arg _sourceCodes is the map of input files to source code strings
	/// @arg _inJsonFromat shows whether the out should be in Json format
	Json::Value streamAssembly(std::ostream& _stream, StringMap const& _sourceCodes = StringMap(), bool _inJsonFormat = false) const
	{
		return m_context.streamAssembly(_stream, _sourceCodes, _inJsonFormat);
	}
	/// @returns Assembly items of the normal compiler context
	eth::AssemblyItems const& getAssemblyItems() const { return m_context.getAssembly().getItems(); }
	/// @returns Assembly items of the runtime compiler context
	eth::AssemblyItems const& getRuntimeAssemblyItems() const { return m_context.getAssembly().getSub(m_runtimeSub).getItems(); }

	/// @returns the entry label of the given function. Might return an AssemblyItem of type
	/// UndefinedItem if it does not exist yet.
	eth::AssemblyItem getFunctionEntryLabel(FunctionDefinition const& _function) const;

private:
	/// Registers the non-function objects inside the contract with the context.
	void initializeContext(ContractDefinition const& _contract,
						   std::map<ContractDefinition const*, bytes const*> const& _contracts);
	/// Adds the code that is run at creation time. Should be run after exchanging the run-time context
	/// with a new and initialized context. Adds the constructor code.
	void packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext);
	/// Appends state variable initialisation and constructor code.
	void appendInitAndConstructorCode(ContractDefinition const& _contract);
	void appendBaseConstructor(FunctionDefinition const& _constructor);
	void appendConstructor(FunctionDefinition const& _constructor);
	void appendFunctionSelector(ContractDefinition const& _contract);
	/// Creates code that unpacks the arguments for the given function represented by a vector of TypePointers.
	/// From memory if @a _fromMemory is true, otherwise from call data.
	/// Expects source offset on the stack.
	void appendCalldataUnpacker(
		TypePointers const& _typeParameters,
		bool _fromMemory = false,
		u256 _startOffset = u256(-1)
	);
	void appendReturnValuePacker(TypePointers const& _typeParameters);

	void registerStateVariables(ContractDefinition const& _contract);
	void initializeStateVariables(ContractDefinition const& _contract);

	/// Initialises all memory arrays in the local variables to point to an empty location.
	void initialiseMemoryArrays(std::vector<VariableDeclaration const*> _variables);
	/// Pushes the initialised value of the given type to the stack. If the type is a memory
	/// reference type, allocates memory and pushes the memory pointer.
	/// Not to be used for storage references.
	void initialiseInMemory(Type const& _type);

	virtual bool visit(VariableDeclaration const& _variableDeclaration) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual bool visit(Continue const& _continue) override;
	virtual bool visit(Break const& _break) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	virtual bool visit(ExpressionStatement const& _expressionStatement) override;
	virtual bool visit(PlaceholderStatement const&) override;

	/// Repeatedly visits all function which are referenced but which are not compiled yet.
	void appendFunctionsWithoutCode();

	/// Appends one layer of function modifier code of the current function, or the function
	/// body itself if the last modifier was reached.
	void appendModifierOrFunctionCode();

	void appendStackVariableInitialisation(VariableDeclaration const& _variable);
	void compileExpression(Expression const& _expression, TypePointer const& _targetType = TypePointer());

	/// @returns the runtime assembly for clone contracts.
	static eth::Assembly getCloneRuntime();

	bool const m_optimize;
	unsigned const m_optimizeRuns;
	CompilerContext m_context;
	size_t m_runtimeSub = size_t(-1); ///< Identifier of the runtime sub-assembly
	CompilerContext m_runtimeContext;
	std::vector<eth::AssemblyItem> m_breakTags; ///< tag to jump to for a "break" statement
	std::vector<eth::AssemblyItem> m_continueTags; ///< tag to jump to for a "continue" statement
	eth::AssemblyItem m_returnTag; ///< tag to jump to for a "return" statement
	unsigned m_modifierDepth = 0;
	FunctionDefinition const* m_currentFunction = nullptr;
	unsigned m_stackCleanupForReturn = 0; ///< this number of stack elements need to be removed before jump to m_returnTag
	// arguments for base constructors, filled in derived-to-base order
	std::map<FunctionDefinition const*, std::vector<ASTPointer<Expression>> const*> m_baseArguments;
};

}
}
