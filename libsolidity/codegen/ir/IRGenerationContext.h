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

struct AscendingFunctionIDCompare
{
	bool operator()(FunctionDefinition const* _f1, FunctionDefinition const* _f2) const
	{
		// NULLs always first.
		if (_f1 != nullptr && _f2 != nullptr)
			return _f1->id() < _f2->id();
		else
			return _f1 == nullptr;
	}
};

using DispatchSet = std::set<FunctionDefinition const*, AscendingFunctionIDCompare>;
using InternalDispatchMap = std::map<YulArity, DispatchSet>;

/**
 * Class that contains contextual information during IR generation.
 */
class IRGenerationContext
{
public:
	enum class ExecutionContext { Creation, Deployed };

	IRGenerationContext(
		langutil::EVMVersion _evmVersion,
		ExecutionContext _executionContext,
		RevertStrings _revertStrings,
		OptimiserSettings _optimiserSettings,
		std::map<std::string, unsigned> _sourceIndices
	):
		m_evmVersion(_evmVersion),
		m_executionContext(_executionContext),
		m_revertStrings(_revertStrings),
		m_optimiserSettings(std::move(_optimiserSettings)),
		m_sourceIndices(std::move(_sourceIndices))
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
	void resetLocalVariables();

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
	std::pair<u256, unsigned> storageLocationOfStateVariable(VariableDeclaration const& _varDecl) const
	{
		solAssert(isStateVariable(_varDecl), "");
		return m_stateVariables.at(&_varDecl);
	}

	std::string newYulVariable();

	void initializeInternalDispatch(InternalDispatchMap _internalDispatchMap);
	InternalDispatchMap consumeInternalDispatchMap();
	bool internalDispatchClean() const { return m_internalDispatchMap.empty(); }

	/// Notifies the context that a function call that needs to go through internal dispatch was
	/// encountered while visiting the AST. This ensures that the corresponding dispatch function
	/// gets added to the dispatch map even if there are no entries in it (which may happen if
	/// the code contains a call to an uninitialized function variable).
	void internalFunctionCalledThroughDispatch(YulArity const& _arity);

	/// Adds a function to the internal dispatch.
	void addToInternalDispatch(FunctionDefinition const& _function);

	/// @returns a new copy of the utility function generator (but using the same function set).
	YulUtilFunctions utils();

	langutil::EVMVersion evmVersion() const { return m_evmVersion; }
	ExecutionContext executionContext() const { return m_executionContext; }

	void setArithmetic(Arithmetic _value) { m_arithmetic = _value; }
	Arithmetic arithmetic() const { return m_arithmetic; }

	ABIFunctions abiFunctions();

	RevertStrings revertStrings() const { return m_revertStrings; }

	std::set<ContractDefinition const*, ASTNode::CompareByID>& subObjectsCreated() { return m_subObjects; }

	bool inlineAssemblySeen() const { return m_inlineAssemblySeen; }
	void setInlineAssemblySeen() { m_inlineAssemblySeen = true; }

	/// @returns the runtime ID to be used for the function in the dispatch routine
	/// and for internal function pointers.
	/// @param _requirePresent if false, generates a new ID if not yet done.
	uint64_t internalFunctionID(FunctionDefinition const& _function, bool _requirePresent);
	/// Copies the internal function IDs from the @a _other. For use in transferring
	/// function IDs from constructor code to deployed code.
	void copyFunctionIDsFrom(IRGenerationContext const& _other);

	std::map<std::string, unsigned> const& sourceIndices() const { return m_sourceIndices; }

	bool immutableRegistered(VariableDeclaration const& _varDecl) const { return m_immutableVariables.count(&_varDecl); }

private:
	langutil::EVMVersion m_evmVersion;
	ExecutionContext m_executionContext;
	RevertStrings m_revertStrings;
	OptimiserSettings m_optimiserSettings;
	std::map<std::string, unsigned> m_sourceIndices;
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
	/// Whether to use checked or wrapping arithmetic.
	Arithmetic m_arithmetic = Arithmetic::Checked;

	/// Flag indicating whether any inline assembly block was seen.
	bool m_inlineAssemblySeen = false;

	/// Function definitions queued for code generation. They're the Solidity functions whose calls
	/// were discovered by the IR generator during AST traversal.
	/// Note that the queue gets filled in a lazy way - new definitions can be added while the
	/// collected ones get removed and traversed.
	/// The order and duplicates are irrelevant here (hence std::set rather than std::queue) as
	/// long as the order of Yul functions in the generated code is deterministic and the same on
	/// all platforms - which is a property guaranteed by MultiUseYulFunctionCollector.
	DispatchSet m_functionGenerationQueue;

	/// Collection of functions that need to be callable via internal dispatch.
	/// Note that having a key with an empty set of functions is a valid situation. It means that
	/// the code contains a call via a pointer even though a specific function is never assigned to it.
	/// It will fail at runtime but the code must still compile.
	InternalDispatchMap m_internalDispatchMap;
	/// Map used by @a internalFunctionID.
	std::map<int64_t, uint64_t> m_functionIDs;

	std::set<ContractDefinition const*, ASTNode::CompareByID> m_subObjects;
};

}
