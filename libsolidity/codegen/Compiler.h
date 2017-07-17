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
 * Solidity AST to EVM bytecode compiler.
 */

#pragma once

#include <ostream>
#include <functional>
#include <libsolidity/codegen/CompilerContext.h>
#include <libevmasm/Assembly.h>

namespace dev {
namespace solidity {

class Compiler
{
public:
	struct OptimiserSettings
	{
		bool runOrderLiterals = false;
		bool runPeephole = false;
		bool runDeduplicate = false;
		bool runCSE = false;
		bool runConstantOptimiser = false;
		/// This specifies an estimate on how often each opcode in this assembly will be executed,
		/// i.e. use a small value to optimise for size and a large value to optimise for runtime gas usage.
		size_t expectedExecutionsPerDeployment = 200;
	};

	explicit Compiler(Compiler::OptimiserSettings const _settings):
		m_optimiserSettings(_settings),
		m_runtimeContext(),
		m_context(&m_runtimeContext)
	{ }

	/// Compiles a contract.
	/// @arg _metadata contains the to be injected metadata CBOR
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _contracts,
		bytes const& _metadata
	);
	/// Compiles a contract that uses DELEGATECALL to call into a pre-deployed version of the given
	/// contract at runtime, but contains the full creation-time code.
	void compileClone(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*> const& _contracts
	);
	/// @returns Entire assembly.
	eth::Assembly const& assembly() const { return m_context.assembly(); }
	/// @returns The entire assembled object (with constructor).
	eth::LinkerObject assembledObject() const { return m_context.assembledObject(); }
	/// @returns Only the runtime object (without constructor).
	eth::LinkerObject runtimeObject() const { return m_context.assembledRuntimeObject(m_runtimeSub); }
	/// @arg _sourceCodes is the map of input files to source code strings
	std::string assemblyString(StringMap const& _sourceCodes = StringMap()) const
	{
		return m_context.assemblyString(_sourceCodes);
	}
	/// @arg _sourceCodes is the map of input files to source code strings
	Json::Value assemblyJSON(StringMap const& _sourceCodes = StringMap()) const
	{
		return m_context.assemblyJSON(_sourceCodes);
	}
	/// @returns Assembly items of the normal compiler context
	eth::AssemblyItems const& assemblyItems() const { return m_context.assembly().items(); }
	/// @returns Assembly items of the runtime compiler context
	eth::AssemblyItems const& runtimeAssemblyItems() const { return m_context.assembly().sub(m_runtimeSub).items(); }

	/// @returns the entry label of the given function. Might return an AssemblyItem of type
	/// UndefinedItem if it does not exist yet.
	eth::AssemblyItem functionEntryLabel(FunctionDefinition const& _function) const;

private:
	OptimiserSettings const m_optimiserSettings;
	CompilerContext m_runtimeContext;
	size_t m_runtimeSub = size_t(-1); ///< Identifier of the runtime sub-assembly, if present.
	CompilerContext m_context;
};

}
}
