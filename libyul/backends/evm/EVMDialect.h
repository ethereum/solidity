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

class YulString;
using Type = YulString;
struct FunctionCall;
struct Object;

/**
 * Context used during code generation.
 */
struct BuiltinContext
{
	Object const* currentObject = nullptr;
	/// Mapping from named objects to abstract assembly sub IDs.
	std::map<YulString, AbstractAssembly::SubID> subIDs;
};

struct BuiltinFunctionForEVM: public BuiltinFunction
{
	std::optional<evmasm::Instruction> instruction;
	/// Function to generate code for the given function call and append it to the abstract
	/// assembly. The fourth parameter is called to visit (and generate code for) the given
	/// argument.
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void(Expression const&)>)> generateCode;
};


/**
 * Yul dialect for EVM as a backend.
 * The main difference is that the builtin functions take an AbstractAssembly for the
 * code generation.
 */
struct EVMDialect: public Dialect
{
	/// Constructor, should only be used internally. Use the factory functions below.
	EVMDialect(langutil::EVMVersion _evmVersion, bool _objectAccess);

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	BuiltinFunctionForEVM const* builtin(YulString _name) const override;

	/// @returns true if the identifier is reserved. This includes the builtins too.
	bool reservedIdentifier(YulString _name) const override;

	BuiltinFunctionForEVM const* discardFunction(YulString /*_type*/) const override { return builtin("pop"_yulstring); }
	BuiltinFunctionForEVM const* equalityFunction(YulString /*_type*/) const override { return builtin("eq"_yulstring); }
	BuiltinFunctionForEVM const* booleanNegationFunction() const override { return builtin("iszero"_yulstring); }
	BuiltinFunctionForEVM const* memoryStoreFunction(YulString /*_type*/) const override { return builtin("mstore"_yulstring); }
	BuiltinFunctionForEVM const* memoryLoadFunction(YulString /*_type*/) const override { return builtin("mload"_yulstring); }
	BuiltinFunctionForEVM const* storageStoreFunction(YulString /*_type*/) const override { return builtin("sstore"_yulstring); }
	BuiltinFunctionForEVM const* storageLoadFunction(YulString /*_type*/) const override { return builtin("sload"_yulstring); }

	static EVMDialect const& strictAssemblyForEVM(langutil::EVMVersion _version);
	static EVMDialect const& strictAssemblyForEVMObjects(langutil::EVMVersion _version);

	langutil::EVMVersion evmVersion() const { return m_evmVersion; }

	bool providesObjectAccess() const { return m_objectAccess; }

	static SideEffects sideEffectsOfInstruction(evmasm::Instruction _instruction);

protected:
	bool const m_objectAccess;
	langutil::EVMVersion const m_evmVersion;
	std::map<YulString, BuiltinFunctionForEVM> m_functions;
	std::set<YulString> m_reserved;
};

/**
 * EVM dialect with types u256 (default) and bool.
 * Difference to EVMDialect:
 *  - All comparison functions return type bool
 *  - bitwise operations are called bitor, bitand, bitxor and bitnot
 *  - and, or, xor take bool and return bool
 *  - iszero is replaced by not, which takes bool and returns bool
 *  - there are conversion functions bool_to_u256 and u256_to_bool.
 *  - there is popbool
 */
struct EVMDialectTyped: public EVMDialect
{
	/// Constructor, should only be used internally. Use the factory function below.
	EVMDialectTyped(langutil::EVMVersion _evmVersion, bool _objectAccess);

	BuiltinFunctionForEVM const* discardFunction(YulString _type) const override;
	BuiltinFunctionForEVM const* equalityFunction(YulString _type) const override;
	BuiltinFunctionForEVM const* booleanNegationFunction() const override { return builtin("not"_yulstring); }

	static EVMDialectTyped const& instance(langutil::EVMVersion _version);
};

}
