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
 * Yul dialects for EVM.
 */

#pragma once

#include <libyul/Dialect.h>

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/ASTForward.h>
#include <liblangutil/EVMVersion.h>

#include <map>
#include <set>

namespace solidity::yul
{

struct FunctionCall;
class Object;

/**
 * Context used during code generation.
 */
struct BuiltinContext
{
	Object const* currentObject = nullptr;
	/// Mapping from named objects to abstract assembly sub IDs.
	std::map<std::string, AbstractAssembly::SubID> subIDs;
};

struct BuiltinFunctionForEVM: public BuiltinFunction
{
	std::optional<evmasm::Instruction> instruction;
	/// Function to generate code for the given function call and append it to the abstract
	/// assembly. Expects all non-literal arguments of the call to be on stack in reverse order
	/// (i.e. right-most argument pushed first).
	/// Expects the caller to set the source location.
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&)> generateCode;
};


/**
 * Yul dialect for EVM as a backend.
 * The main difference is that the builtin functions take an AbstractAssembly for the
 * code generation.
 */
class EVMDialect: public Dialect
{
public:
	/// Constructor, should only be used internally. Use the factory functions below.
	EVMDialect(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion, bool _objectAccess);

	std::optional<BuiltinHandle> findBuiltin(std::string_view _name) const override;

	BuiltinFunctionForEVM const& builtin(BuiltinHandle const& _handle) const override;

	bool reservedIdentifier(std::string_view _name) const override;

	std::optional<BuiltinHandle> discardFunctionHandle() const override { return m_discardFunction; }
	std::optional<BuiltinHandle> equalityFunctionHandle() const override { return m_equalityFunction; }
	std::optional<BuiltinHandle> booleanNegationFunctionHandle() const override { return m_booleanNegationFunction; }
	std::optional<BuiltinHandle> memoryStoreFunctionHandle() const override { return m_memoryStoreFunction; }
	std::optional<BuiltinHandle> memoryLoadFunctionHandle() const override { return m_memoryLoadFunction; }
	std::optional<BuiltinHandle> storageStoreFunctionHandle() const override { return m_storageStoreFunction; }
	std::optional<BuiltinHandle> storageLoadFunctionHandle() const override { return m_storageLoadFunction; }
	std::optional<BuiltinHandle> hashFunctionHandle() const override { return m_hashFunction; }

	static EVMDialect const& strictAssemblyForEVM(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion);
	static EVMDialect const& strictAssemblyForEVMObjects(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion);

	langutil::EVMVersion evmVersion() const { return m_evmVersion; }
	std::optional<uint8_t> eofVersion() const { return m_eofVersion; }

	bool providesObjectAccess() const { return m_objectAccess; }

	static SideEffects sideEffectsOfInstruction(evmasm::Instruction _instruction);

	static size_t constexpr verbatimMaxInputSlots = 100;
	static size_t constexpr verbatimMaxOutputSlots = 100;

protected:
	static bool constexpr isVerbatimHandle(BuiltinHandle const& _handle) { return _handle.id < verbatimIDOffset; }
	static BuiltinFunctionForEVM createVerbatimFunctionFromHandle(BuiltinHandle const& _handle);
	static BuiltinFunctionForEVM createVerbatimFunction(size_t _arguments, size_t _returnVariables);
	BuiltinHandle verbatimFunction(size_t _arguments, size_t _returnVariables) const;

	static size_t constexpr verbatimIDOffset = verbatimMaxInputSlots * verbatimMaxOutputSlots;

	bool const m_objectAccess;
	langutil::EVMVersion const m_evmVersion;
	std::optional<uint8_t> m_eofVersion;
	std::unordered_map<std::string_view, BuiltinHandle> m_builtinFunctionsByName;
	std::vector<std::optional<BuiltinFunctionForEVM>> m_functions;
	std::array<std::unique_ptr<BuiltinFunctionForEVM>, verbatimIDOffset> mutable m_verbatimFunctions{};
	std::set<std::string, std::less<>> m_reserved;

	std::optional<BuiltinHandle> m_discardFunction;
	std::optional<BuiltinHandle> m_equalityFunction;
	std::optional<BuiltinHandle> m_booleanNegationFunction;
	std::optional<BuiltinHandle> m_memoryStoreFunction;
	std::optional<BuiltinHandle> m_memoryLoadFunction;
	std::optional<BuiltinHandle> m_storageStoreFunction;
	std::optional<BuiltinHandle> m_storageLoadFunction;
	std::optional<BuiltinHandle> m_hashFunction;
};

}
