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
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#pragma once

#include <libsolidity/interface/ErrorReporter.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/interface/EVMVersion.h>

#include <libevmasm/SourceLocation.h>
#include <libevmasm/LinkerObject.h>

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

#include <json/json.h>

#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace dev
{

namespace eth
{
class Assembly;
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
}

namespace solidity
{

// forward declarations
class Scanner;
class ASTNode;
class ContractDefinition;
class FunctionDefinition;
class SourceUnit;
class Compiler;
class GlobalContext;
class Natspec;
class Error;
class DeclarationContainer;

/**
 * Easy to use and self-contained Solidity compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: boost::noncopyable
{
public:
	enum State {
		Empty,
		SourcesSet,
		ParsingSuccessful,
		AnalysisSuccessful,
		CompilationSuccessful
	};

	/// Creates a new compiler stack.
	/// @param _readFile callback to used to read files for import statements. Must return
	/// and must not emit exceptions.
	explicit CompilerStack(ReadCallback::Callback const& _readFile = ReadCallback::Callback()):
		m_readFile(_readFile),
		m_errorList(),
		m_errorReporter(m_errorList) {}

	/// @returns the list of errors that occured during parsing and type checking.
	ErrorList const& errors() const { return m_errorReporter.errors(); }

	/// @returns the current state.
	State state() const { return m_stackState; }

	/// Resets the compiler to a state where the sources are not parsed or even removed.
	/// Sets the state to SourcesSet if @a _keepSources is true, otherwise to Empty.
	/// All settings, with the exception of remappings, are reset.
	void reset(bool _keepSources = false);

	/// Sets path remappings in the format "context:prefix=target"
	void setRemappings(std::vector<std::string> const& _remappings);

	/// Sets library addresses. Addresses are cleared iff @a _libraries is missing.
	/// Will not take effect before running compile.
	void setLibraries(std::map<std::string, h160> const& _libraries = std::map<std::string, h160>{})
	{
		m_libraries = _libraries;
	}

	/// Changes the optimiser settings.
	/// Will not take effect before running compile.
	void setOptimiserSettings(bool _optimize, unsigned _runs = 200)
	{
		m_optimize = _optimize;
		m_optimizeRuns = _runs;
	}

	void setEVMVersion(EVMVersion _version = EVMVersion{});

	/// Sets the list of requested contract names. If empty, no filtering is performed and every contract
	/// found in the supplied sources is compiled. Names are cleared iff @a _contractNames is missing.
	void setRequestedContractNames(std::set<std::string> const& _contractNames = std::set<std::string>{})
	{
		m_requestedContractNames = _contractNames;
	}

	/// @arg _metadataLiteralSources When true, store sources as literals in the contract metadata.
	void useMetadataLiteralSources(bool _metadataLiteralSources) { m_metadataLiteralSources = _metadataLiteralSources; }

	/// Adds a source object (e.g. file) to the parser. After this, parse has to be called again.
	/// @returns true if a source object by the name already existed and was replaced.
	bool addSource(std::string const& _name, std::string const& _content, bool _isLibrary = false);

	/// Parses all source units that were added
	/// @returns false on error.
	bool parse();

	/// Performs the analysis steps (imports, scopesetting, syntaxCheck, referenceResolving,
	///  typechecking, staticAnalysis) on previously parsed sources.
	/// @returns false on error.
	bool analyze();

	/// Parses and analyzes all source units that were added
	/// @returns false on error.
	bool parseAndAnalyze();

	/// Compiles the source units that were previously added and parsed.
	/// @returns false on error.
	bool compile();

	/// @returns the list of sources (paths) used
	std::vector<std::string> sourceNames() const;

	/// @returns a mapping assigning each source name its index inside the vector returned
	/// by sourceNames().
	std::map<std::string, unsigned> sourceIndices() const;

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	Scanner const& scanner(std::string const& _sourceName) const;

	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& ast(std::string const& _sourceName) const;

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(SourceLocation const& _sourceLocation) const;

	/// @returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const;

	/// @returns the name of the last contract.
	std::string const lastContractName() const;

	/// @returns either the contract's name or a mixture of its name and source file, sanitized for filesystem use
	std::string const filesystemFriendlyName(std::string const& _contractName) const;

	/// @returns the assembled object for a contract.
	eth::LinkerObject const& object(std::string const& _contractName) const;

	/// @returns the runtime object for the contract.
	eth::LinkerObject const& runtimeObject(std::string const& _contractName) const;

	/// @returns the bytecode of a contract that uses an already deployed contract via DELEGATECALL.
	/// The returned bytes will contain a sequence of 20 bytes of the format "XXX...XXX" which have to
	/// substituted by the actual address. Note that this sequence starts end ends in three X
	/// characters but can contain anything in between.
	eth::LinkerObject const& cloneObject(std::string const& _contractName) const;

	/// @returns normal contract assembly items
	eth::AssemblyItems const* assemblyItems(std::string const& _contractName) const;

	/// @returns runtime contract assembly items
	eth::AssemblyItems const* runtimeAssemblyItems(std::string const& _contractName) const;

	/// @returns the string that provides a mapping between bytecode and sourcecode or a nullptr
	/// if the contract does not (yet) have bytecode.
	std::string const* sourceMapping(std::string const& _contractName) const;

	/// @returns the string that provides a mapping between runtime bytecode and sourcecode.
	/// if the contract does not (yet) have bytecode.
	std::string const* runtimeSourceMapping(std::string const& _contractName) const;

	/// @return a verbose text representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	std::string assemblyString(std::string const& _contractName, StringMap _sourceCodes = StringMap()) const;

	/// @returns a JSON representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	Json::Value assemblyJSON(std::string const& _contractName, StringMap _sourceCodes = StringMap()) const;

	/// @returns a JSON representing the contract ABI.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& contractABI(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's user documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecUser(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's developer documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecDev(std::string const& _contractName) const;

	/// @returns a JSON representing a map of method identifiers (hashes) to function names.
	Json::Value methodIdentifiers(std::string const& _contractName) const;

	/// @returns the Contract Metadata
	std::string const& metadata(std::string const& _contractName) const;

	/// @returns a JSON representing the estimated gas usage for contract creation, internal and external functions
	Json::Value gasEstimates(std::string const& _contractName) const;

private:
	/**
	 * Information pertaining to one source unit, filled gradually during parsing and compilation.
	 */
	struct Source
	{
		std::shared_ptr<Scanner> scanner;
		std::shared_ptr<SourceUnit> ast;
		bool isLibrary = false;
		void reset() { scanner.reset(); ast.reset(); }
	};

	struct Contract
	{
		ContractDefinition const* contract = nullptr;
		std::shared_ptr<Compiler> compiler;
		eth::LinkerObject object;
		eth::LinkerObject runtimeObject;
		eth::LinkerObject cloneObject;
		std::string metadata; ///< The metadata json that will be hashed into the chain.
		mutable std::unique_ptr<Json::Value const> abi;
		mutable std::unique_ptr<Json::Value const> userDocumentation;
		mutable std::unique_ptr<Json::Value const> devDocumentation;
		mutable std::unique_ptr<std::string const> sourceMapping;
		mutable std::unique_ptr<std::string const> runtimeSourceMapping;
	};

	/// Loads the missing sources from @a _ast (named @a _path) using the callback
	/// @a m_readFile and stores the absolute paths of all imports in the AST annotations.
	/// @returns the newly loaded sources.
	StringMap loadMissingSources(SourceUnit const& _ast, std::string const& _path);
	std::string applyRemapping(std::string const& _path, std::string const& _context);
	void resolveImports();
	/// @returns the absolute path corresponding to @a _path relative to @a _reference.
	static std::string absolutePath(std::string const& _path, std::string const& _reference);
	/// Helper function to return path converted strings.
	static std::string sanitizePath(std::string const& _path) { return boost::filesystem::path(_path).generic_string(); }

	/// @returns true if the contract is requested to be compiled.
	bool isRequestedContract(ContractDefinition const& _contract) const;

	/// Compile a single contract and put the result in @a _compiledContracts.
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*>& _compiledContracts
	);
	void link();

	Contract const& contract(std::string const& _contractName) const;
	Source const& source(std::string const& _sourceName) const;

	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& contractDefinition(std::string const& _contractName) const;

	std::string createMetadata(Contract const& _contract) const;
	std::string computeSourceMapping(eth::AssemblyItems const& _items) const;
	Json::Value const& contractABI(Contract const&) const;
	Json::Value const& natspecUser(Contract const&) const;
	Json::Value const& natspecDev(Contract const&) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t functionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	struct Remapping
	{
		std::string context;
		std::string prefix;
		std::string target;
	};

	ReadCallback::Callback m_readFile;
	ReadCallback::Callback m_smtQuery;
	bool m_optimize = false;
	unsigned m_optimizeRuns = 200;
	EVMVersion m_evmVersion;
	std::set<std::string> m_requestedContractNames;
	std::map<std::string, h160> m_libraries;
	/// list of path prefix remappings, e.g. mylibrary: github.com/ethereum = /usr/local/ethereum
	/// "context:prefix=target"
	std::vector<Remapping> m_remappings;
	std::map<std::string const, Source> m_sources;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>> m_scopes;
	std::vector<Source const*> m_sourceOrder;
	std::map<std::string const, Contract> m_contracts;
	ErrorList m_errorList;
	ErrorReporter m_errorReporter;
	bool m_metadataLiteralSources = false;
	State m_stackState = Empty;
};

}
}
