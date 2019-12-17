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

#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/DebugSettings.h>
#include <liblangutil/EVMVersion.h>
#include <libevmasm/Assembly.h>
#include <functional>
#include <ostream>

namespace dev {
namespace solidity {

class Compiler
{
public:
	Compiler(langutil::EVMVersion _evmVersion, RevertStrings _revertStrings, OptimiserSettings _optimiserSettings):
		m_optimiserSettings(std::move(_optimiserSettings)),
		m_revertStrings(_revertStrings),
		m_runtimeContext(_evmVersion),
		m_context(_evmVersion, &m_runtimeContext)
	{ }

	/// Compiles a contract.
	/// @arg _metadata contains the to be injected metadata CBOR
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, std::shared_ptr<Compiler const>> const& _otherCompilers,
		bytes const& _metadata
	);
	/// @returns Entire assembly.
	eth::Assembly const& assembly() const { return m_context.assembly(); }
	/// @returns Entire assembly as a shared pointer to non-const.
	std::shared_ptr<eth::Assembly> assemblyPtr() const { return m_context.assemblyPtr(); }
	/// @returns Runtime assembly.
	std::shared_ptr<eth::Assembly> runtimeAssemblyPtr() const;
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
	RevertStrings const m_revertStrings;
	CompilerContext m_runtimeContext;
	size_t m_runtimeSub = size_t(-1); ///< Identifier of the runtime sub-assembly, if present.
	CompilerContext m_context;
};

}
}
