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
// SPDX-License-Identifier: GPL-3.0
/**
 * Class that contains contextual information during IR generation.
 */

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/ir/IRVariable.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/DebugSettings.h>

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>
#include <libsolidity/codegen/ir/Common.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/Common.h>

#include <set>
#include <string>
#include <memory>
#include <vector>

namespace solidity::frontend
{

class YulUtilFunctions;
class ABIFunctions;

using InternalDispatchMap = std::map<YulArity, std::set<FunctionDefinition const*>>;

/**
 * Class that contains contextual information during IR generation.
 */
class IRGenerationContext
{
public:
	IRGenerationContext(
		langutil::EVMVersion _evmVersion,
		RevertStrings _revertStrings,
		OptimiserSettings _optimiserSettings
	):
		m_evmVersion(_evmVersion),
		m_revertStrings(_revertStrings),
		m_optimiserSettings(std::move(_optimiserSettings))
	{}

	MultiUseYulFunctionCollector& functionCollector() { return m_functions; }

	/// Adds a Solidity function to the function generation queue and returns the name of the
	/// corresponding Yul function.
	std::string enqueueFunctionForCodeGeneration(FunctionDefinition const& _function);

	/// Pops one item from the function generation queue. Must not be called if the queue is empty.
	FunctionDefinition const* dequeueFunctionForCodeGeneration();

	bool functionGenerationQueueEmpty() { return m_functionGenerationQueue.empty(); }

	/// Sets the most derived contract (the one currently being compiled)>
	void setMostDerivedContract(ContractDefinition const& _mostDerivedContract)
	{
		m_mostDerivedContract = &_mostDerivedContract;
	}
	ContractDefinition const& mostDerivedContract() const;


	IRVariable const& addLocalVariable(VariableDeclaration const& _varDecl);
	bool isLocalVariable(VariableDeclaration const& _varDecl) const { return m_localVariables.count(&_varDecl); }
	IRVariable const& localVariable(VariableDeclaration const& _varDecl);

	/// Registers an immutable variable of the contract.
	/// Should only be called at construction time.
	void registerImmutableVariable(VariableDeclaration const& _varDecl);
	/// @returns the reserved memory for storing the value of the
	/// immutable @a _variable during contract creation.
	size_t immutableMemoryOffset(VariableDeclaration const& _variable) const;
	/// @returns the reserved memory and resets it to mark it as used.
	/// Intended to be used only once for initializing the free memory pointer
	/// to after the area used for immutables.
	size_t reservedMemory();

	void addStateVariable(VariableDeclaration const& _varDecl, u256 _storageOffset, unsigned _byteOffset);
	bool isStateVariable(VariableDeclaration const& _varDecl) const { return m_stateVariables.count(&_varDecl); }
	std::pair<u256, unsigned> storageLocationOfVariable(VariableDeclaration const& _varDecl) const
	{
		return m_stateVariables.at(&_varDecl);
	}

	std::string newYulVariable();

	void initializeInternalDispatch(InternalDispatchMap _internalDispatchMap);
	InternalDispatchMap consumeInternalDispatchMap();
	bool internalDispatchClean() const { return m_internalDispatchMap.empty() && m_directInternalFunctionCalls.empty(); }

	/// Notifies the context that a function call that needs to go through internal dispatch was
	/// encountered while visiting the AST. This ensures that the corresponding dispatch function
	/// gets added to the dispatch map even if there are no entries in it (which may happen if
	/// the code contains a call to an uninitialized function variable).
	void internalFunctionCalledThroughDispatch(YulArity const& _arity);

	/// Notifies the context that a direct function call (i.e. not through internal dispatch) was
	/// encountered while visiting the AST. This lets the context know that the function should
	/// not be added to the dispatch (unless there are also indirect calls to it elsewhere else).
	void internalFunctionCalledDirectly(Expression const& _expression);

	/// Notifies the context that a name representing an internal function has been found while
	/// visiting the AST. If the name has not been reported as a direct call using
	/// @a internalFunctionCalledDirectly(), it's assumed to represent function variable access
	/// and the function gets added to internal dispatch.
	void internalFunctionAccessed(Expression const& _expression, FunctionDefinition const& _function);

	/// @returns a new copy of the utility function generator (but using the same function set).
	YulUtilFunctions utils();

	langutil::EVMVersion evmVersion() const { return m_evmVersion; };

	ABIFunctions abiFunctions();

	/// @returns code that stores @param _message for revert reason
	/// if m_revertStrings is debug.
	std::string revertReasonIfDebug(std::string const& _message = "");

	RevertStrings revertStrings() const { return m_revertStrings; }

	std::set<ContractDefinition const*, ASTNode::CompareByID>& subObjectsCreated() { return m_subObjects; }

private:
	langutil::EVMVersion m_evmVersion;
	RevertStrings m_revertStrings;
	OptimiserSettings m_optimiserSettings;
	ContractDefinition const* m_mostDerivedContract = nullptr;
	std::map<VariableDeclaration const*, IRVariable> m_localVariables;
	/// Memory offsets reserved for the values of immutable variables during contract creation.
	/// This map is empty in the runtime context.
	std::map<VariableDeclaration const*, size_t> m_immutableVariables;
	/// Total amount of reserved memory. Reserved memory is used to store
	/// immutable variables during contract creation.
	std::optional<size_t> m_reservedMemory = {0};
	/// Storage offsets of state variables
	std::map<VariableDeclaration const*, std::pair<u256, unsigned>> m_stateVariables;
	MultiUseYulFunctionCollector m_functions;
	size_t m_varCounter = 0;

	/// Function definitions queued for code generation. They're the Solidity functions whose calls
	/// were discovered by the IR generator during AST traversal.
	/// Note that the queue gets filled in a lazy way - new definitions can be added while the
	/// collected ones get removed and traversed.
	/// The order and duplicates are irrelevant here (hence std::set rather than std::queue) as
	/// long as the order of Yul functions in the generated code is deterministic and the same on
	/// all platforms - which is a property guaranteed by MultiUseYulFunctionCollector.
	std::set<FunctionDefinition const*> m_functionGenerationQueue;

	/// Collection of functions that need to be callable via internal dispatch.
	/// Note that having a key with an empty set of functions is a valid situation. It means that
	/// the code contains a call via a pointer even though a specific function is never assigned to it.
	/// It will fail at runtime but the code must still compile.
	InternalDispatchMap m_internalDispatchMap;
	std::set<Expression const*> m_directInternalFunctionCalls;

	std::set<ContractDefinition const*, ASTNode::CompareByID> m_subObjects;
};

}
