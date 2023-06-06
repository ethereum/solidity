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
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#pragma once

#include <libsolidity/analysis/FunctionCallGraph.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/interface/ImportRemapper.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/Version.h>
#include <libsolidity/interface/DebugSettings.h>

#include <libsolidity/formal/ModelCheckerSettings.h>

#include <libsmtutil/SolverInterface.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceLocation.h>

#include <libevmasm/LinkerObject.h>

#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/LazyInit.h>

#include <json/json.h>

#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace solidity::langutil
{
class CharStream;
}


namespace solidity::evmasm
{
class Assembly;
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
}

namespace solidity::frontend
{

// forward declarations
class ASTNode;
class ContractDefinition;
class FunctionDefinition;
class SourceUnit;
class Compiler;
class GlobalContext;
class Natspec;
class DeclarationContainer;
namespace experimental
{
class Analysis;
}

/**
 * Easy to use and self-contained Solidity compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: public langutil::CharStreamProvider
{
public:
	/// Noncopyable.
	CompilerStack(CompilerStack const&) = delete;
	CompilerStack& operator=(CompilerStack const&) = delete;

	enum State {
		Empty,
		SourcesSet,
		Parsed,
		ParsedAndImported,
		AnalysisSuccessful,
		CompilationSuccessful
	};

	enum class MetadataFormat {
		WithReleaseVersionTag,
		WithPrereleaseVersionTag,
		NoMetadata
	};

	enum class MetadataHash {
		IPFS,
		Bzzr1,
		None
	};

	enum class CompilationSourceType {
		/// Regular compilation from Solidity source files.
		Solidity,
		/// Compilation from an imported Solidity AST.
		SolidityAST
	};

	/// Creates a new compiler stack.
	/// @param _readFile callback used to read files for import statements. Must return
	/// and must not emit exceptions.
	explicit CompilerStack(ReadCallback::Callback _readFile = ReadCallback::Callback());

	~CompilerStack() override;

	/// @returns the list of errors that occurred during parsing and type checking.
	langutil::ErrorList const& errors() const { return m_errorReporter.errors(); }

	/// @returns the current state.
	State state() const { return m_stackState; }

	/// Resets the compiler to an empty state. Unless @a _keepSettings is set to true,
	/// all settings are reset as well.
	void reset(bool _keepSettings = false);

	/// Sets path remappings.
	/// Must be set before parsing.
	void setRemappings(std::vector<ImportRemapper::Remapping> _remappings);

	/// Sets library addresses. Addresses are cleared iff @a _libraries is missing.
	/// Must be set before parsing.
	void setLibraries(std::map<std::string, util::h160> const& _libraries = {});

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(bool _optimize, size_t _runs = OptimiserSettings{}.expectedExecutionsPerDeployment);

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(OptimiserSettings _settings);

	/// Sets whether to strip revert strings, add additional strings or do nothing at all.
	void setRevertStringBehaviour(RevertStrings _revertStrings);

	/// Sets the pipeline to go through the Yul IR or not.
	/// Must be set before parsing.
	void setViaIR(bool _viaIR);

	/// Set the EVM version used before running compile.
	/// When called without an argument it will revert to the default version.
	/// Must be set before parsing.
	void setEVMVersion(langutil::EVMVersion _version = langutil::EVMVersion{});

	/// Set the EOF version used before running compile.
	/// If set to std::nullopt (the default), legacy non-EOF bytecode is generated.
	void setEOFVersion(std::optional<uint8_t> version);

	/// Set model checker settings.
	void setModelCheckerSettings(ModelCheckerSettings _settings);

	/// Sets the requested contract names by source.
	/// If empty, no filtering is performed and every contract
	/// found in the supplied sources is compiled.
	/// Names are cleared iff @a _contractNames is missing.
	void setRequestedContractNames(std::map<std::string, std::set<std::string>> const& _contractNames = std::map<std::string, std::set<std::string>>{})
	{
		m_requestedContractNames = _contractNames;
	}

	/// Enable EVM Bytecode generation. This is enabled by default.
	void enableEvmBytecodeGeneration(bool _enable = true) { m_generateEvmBytecode = _enable; }

	/// Enable generation of Yul IR code.
	void enableIRGeneration(bool _enable = true) { m_generateIR = _enable; }

	/// @arg _metadataLiteralSources When true, store sources as literals in the contract metadata.
	/// Must be set before parsing.
	void useMetadataLiteralSources(bool _metadataLiteralSources);

	/// Sets whether and which hash should be used
	/// to store the metadata in the bytecode.
	/// @param _metadataHash can be IPFS, Bzzr1, None
	void setMetadataHash(MetadataHash _metadataHash);

	/// Select components of debug info that should be included in comments in generated assembly.
	void selectDebugInfo(langutil::DebugInfoSelection _debugInfoSelection);

	/// Sets the sources. Must be set before parsing.
	void setSources(StringMap _sources);

	/// Adds a response to an SMTLib2 query (identified by the hash of the query input).
	/// Must be set before parsing.
	void addSMTLib2Response(util::h256 const& _hash, std::string const& _response);

	/// Parses all source units that were added
	/// @returns false on error.
	bool parse();

	/// Imports given SourceUnits so they can be analyzed. Leads to the same internal state as parse().
	/// Will throw errors if the import fails
	void importASTs(std::map<std::string, Json::Value> const& _sources);

	/// Performs the analysis steps (imports, scopesetting, syntaxCheck, referenceResolving,
	///  typechecking, staticAnalysis) on previously parsed sources.
	/// @returns false on error.
	bool analyze();

	/// Perform the analysis steps of legacy language mode.
	/// @returns false on error.
	bool analyzeLegacy(bool _noErrorsSoFar);
	/// Perform the analysis steps of experimental language mode.
	/// @returns false on error.
	bool analyzeExperimental();

	/// Parses and analyzes all source units that were added
	/// @returns false on error.
	bool parseAndAnalyze(State _stopAfter = State::CompilationSuccessful);

	/// Compiles the source units that were previously added and parsed.
	/// @returns false on error.
	bool compile(State _stopAfter = State::CompilationSuccessful);

	/// @returns the list of sources (paths) used
	std::vector<std::string> sourceNames() const;

	/// @returns a mapping assigning each source name its index inside the vector returned
	/// by sourceNames().
	std::map<std::string, unsigned> sourceIndices() const;

	/// @returns the previously used character stream, useful for counting lines during error reporting.
	langutil::CharStream const& charStream(std::string const& _sourceName) const override;

	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& ast(std::string const& _sourceName) const;

	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& contractDefinition(std::string const& _contractName) const;

	/// @returns a list of unhandled queries to the SMT solver (has to be supplied in a second run
	/// by calling @a addSMTLib2Response).
	std::vector<std::string> const& unhandledSMTLib2Queries() const { return m_unhandledSMTLib2Queries; }

	/// @returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const;

	/// @returns the name of the last contract. If _sourceName is defined the last contract of that source will be returned.
	std::string const lastContractName(std::optional<std::string> const& _sourceName = std::nullopt) const;

	/// @returns either the contract's name or a mixture of its name and source file, sanitized for filesystem use
	std::string const filesystemFriendlyName(std::string const& _contractName) const;

	/// @returns the IR representation of a contract.
	std::string const& yulIR(std::string const& _contractName) const;

	/// @returns the IR representation of a contract AST in format.
	Json::Value const& yulIRAst(std::string const& _contractName) const;

	/// @returns the optimized IR representation of a contract.
	std::string const& yulIROptimized(std::string const& _contractName) const;

	/// @returns the optimized IR representation of a contract AST in JSON format.
	Json::Value const& yulIROptimizedAst(std::string const& _contractName) const;

	/// @returns the assembled object for a contract.
	evmasm::LinkerObject const& object(std::string const& _contractName) const;

	/// @returns the runtime object for the contract.
	evmasm::LinkerObject const& runtimeObject(std::string const& _contractName) const;

	/// @returns normal contract assembly items
	evmasm::AssemblyItems const* assemblyItems(std::string const& _contractName) const;

	/// @returns runtime contract assembly items
	evmasm::AssemblyItems const* runtimeAssemblyItems(std::string const& _contractName) const;

	/// @returns an array containing all utility sources generated during compilation.
	/// Format: [ { name: string, id: number, language: "Yul", contents: string }, ... ]
	Json::Value generatedSources(std::string const& _contractName, bool _runtime = false) const;

	/// @returns the string that provides a mapping between bytecode and sourcecode or a nullptr
	/// if the contract does not (yet) have bytecode.
	std::string const* sourceMapping(std::string const& _contractName) const;

	/// @returns the string that provides a mapping between runtime bytecode and sourcecode.
	/// if the contract does not (yet) have bytecode.
	std::string const* runtimeSourceMapping(std::string const& _contractName) const;

	/// @return a verbose text representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	std::string assemblyString(std::string const& _contractName, StringMap const& _sourceCodes = StringMap()) const;

	/// @returns a JSON representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	Json::Value assemblyJSON(std::string const& _contractName) const;

	/// @returns a JSON representing the contract ABI.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& contractABI(std::string const& _contractName) const;

	/// @returns a JSON representing the storage layout of the contract.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& storageLayout(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's user documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecUser(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's developer documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecDev(std::string const& _contractName) const;

	/// @returns a JSON object with the three members ``methods``, ``events``, ``errors``. Each is a map, mapping identifiers (hashes) to function names.
	Json::Value interfaceSymbols(std::string const& _contractName) const;

	/// @returns the Contract Metadata matching the pipeline selected using the viaIR setting.
	std::string const& metadata(std::string const& _contractName) const { return metadata(contract(_contractName)); }

	/// @returns the CBOR-encoded metadata matching the pipeline selected using the viaIR setting.
	bytes cborMetadata(std::string const& _contractName) const { return cborMetadata(_contractName, m_viaIR); }

	/// @returns the CBOR-encoded metadata.
	/// @param _forIR If true, the metadata for the IR codegen is used. Otherwise it's the metadata
	///               for the EVM codegen
	bytes cborMetadata(std::string const& _contractName, bool _forIR) const;

	/// @returns a JSON representing the estimated gas usage for contract creation, internal and external functions
	Json::Value gasEstimates(std::string const& _contractName) const;

	/// Changes the format of the metadata appended at the end of the bytecode.
	void setMetadataFormat(MetadataFormat _metadataFormat) { m_metadataFormat = _metadataFormat; }

	static MetadataFormat defaultMetadataFormat()
	{
		return VersionIsRelease ? MetadataFormat::WithReleaseVersionTag : MetadataFormat::WithPrereleaseVersionTag;
	}

private:
	/// The state per source unit. Filled gradually during parsing.
	struct Source
	{
		std::shared_ptr<langutil::CharStream> charStream;
		std::shared_ptr<SourceUnit> ast;
		util::h256 mutable keccak256HashCached;
		util::h256 mutable swarmHashCached;
		std::string mutable ipfsUrlCached;
		void reset() { *this = Source(); }
		util::h256 const& keccak256() const;
		util::h256 const& swarmHash() const;
		std::string const& ipfsUrl() const;
	};

	/// The state per contract. Filled gradually during compilation.
	struct Contract
	{
		ContractDefinition const* contract = nullptr;
		std::shared_ptr<Compiler> compiler;
		std::shared_ptr<evmasm::Assembly> evmAssembly;
		std::shared_ptr<evmasm::Assembly> evmRuntimeAssembly;
		evmasm::LinkerObject object; ///< Deployment object (includes the runtime sub-object).
		evmasm::LinkerObject runtimeObject; ///< Runtime object.
		std::string yulIR; ///< Yul IR code.
		std::string yulIROptimized; ///< Optimized Yul IR code.
		Json::Value yulIRAst; ///< JSON AST of Yul IR code.
		Json::Value yulIROptimizedAst; ///< JSON AST of optimized Yul IR code.
		util::LazyInit<std::string const> metadata; ///< The metadata json that will be hashed into the chain.
		util::LazyInit<Json::Value const> abi;
		util::LazyInit<Json::Value const> storageLayout;
		util::LazyInit<Json::Value const> userDocumentation;
		util::LazyInit<Json::Value const> devDocumentation;
		util::LazyInit<Json::Value const> generatedSources;
		util::LazyInit<Json::Value const> runtimeGeneratedSources;
		mutable std::optional<std::string const> sourceMapping;
		mutable std::optional<std::string const> runtimeSourceMapping;
	};

	void createAndAssignCallGraphs();
	void findAndReportCyclicContractDependencies();

	/// Loads the missing sources from @a _ast (named @a _path) using the callback
	/// @a m_readFile
	/// @returns the newly loaded sources.
	StringMap loadMissingSources(SourceUnit const& _ast);
	std::string applyRemapping(std::string const& _path, std::string const& _context);
	bool resolveImports();

	/// Store the contract definitions in m_contracts.
	void storeContractDefinitions();

	/// Annotate internal dispatch function Ids
	void annotateInternalFunctionIDs();

	/// @returns true if the source is requested to be compiled.
	bool isRequestedSource(std::string const& _sourceName) const;

	/// @returns true if the contract is requested to be compiled.
	bool isRequestedContract(ContractDefinition const& _contract) const;

	/// Assembles the contract.
	/// This function should only be internally called by compileContract and generateEVMFromIR.
	void assembleYul(
		ContractDefinition const& _contract,
		std::shared_ptr<evmasm::Assembly> _assembly,
		std::shared_ptr<evmasm::Assembly> _runtimeAssembly
	);

	/// Compile a single contract.
	/// @param _otherCompilers provides access to compilers of other contracts, to get
	///                        their bytecode if needed. Only filled after they have been compiled.
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, std::shared_ptr<Compiler const>>& _otherCompilers
	);

	/// Generate Yul IR for a single contract.
	/// The IR is stored but otherwise unused.
	void generateIR(ContractDefinition const& _contract);

	/// Generate EVM representation for a single contract.
	/// Depends on output generated by generateIR.
	void generateEVMFromIR(ContractDefinition const& _contract);

	/// Links all the known library addresses in the available objects. Any unknown
	/// library will still be kept as an unlinked placeholder in the objects.
	void link();

	/// @returns the contract object for the given @a _contractName.
	/// Can only be called after state is CompilationSuccessful.
	Contract const& contract(std::string const& _contractName) const;

	/// @returns the source object for the given @a _sourceName.
	/// Can only be called after state is SourcesSet.
	Source const& source(std::string const& _sourceName) const;

	/// @param _forIR If true, include a flag that indicates that the bytecode comes from IR codegen.
	/// @returns the metadata JSON as a compact string for the given contract.
	std::string createMetadata(Contract const& _contract, bool _forIR) const;

	/// @returns the metadata CBOR for the given serialised metadata JSON.
	/// @param _forIR If true, use the metadata for the IR codegen. Otherwise the one for EVM codegen.
	bytes createCBORMetadata(Contract const& _contract, bool _forIR) const;

	/// @returns the contract ABI as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& contractABI(Contract const&) const;

	/// @returns the storage layout of the contract as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& storageLayout(Contract const&) const;

	/// @returns the Natspec User documentation as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& natspecUser(Contract const&) const;

	/// @returns the Natspec Developer documentation as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& natspecDev(Contract const&) const;

	/// @returns the Contract Metadata matching the pipeline selected using the viaIR setting.
	/// This will generate the metadata and store it in the Contract object if it is not present yet.
	std::string const& metadata(Contract const& _contract) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t functionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	ReadCallback::Callback m_readFile;
	OptimiserSettings m_optimiserSettings;
	RevertStrings m_revertStrings = RevertStrings::Default;
	State m_stopAfter = State::CompilationSuccessful;
	bool m_viaIR = false;
	langutil::EVMVersion m_evmVersion;
	std::optional<uint8_t> m_eofVersion;
	ModelCheckerSettings m_modelCheckerSettings;
	std::map<std::string, std::set<std::string>> m_requestedContractNames;
	bool m_generateEvmBytecode = true;
	bool m_generateIR = false;
	std::map<std::string, util::h160> m_libraries;
	ImportRemapper m_importRemapper;
	std::map<std::string const, Source> m_sources;
	std::vector<std::string> m_unhandledSMTLib2Queries;
	std::map<util::h256, std::string> m_smtlib2Responses;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::vector<Source const*> m_sourceOrder;
	std::map<std::string const, Contract> m_contracts;

	langutil::ErrorList m_errorList;
	langutil::ErrorReporter m_errorReporter;
	std::unique_ptr<experimental::Analysis> m_experimentalAnalysis;
	bool m_metadataLiteralSources = false;
	MetadataHash m_metadataHash = MetadataHash::IPFS;
	langutil::DebugInfoSelection m_debugInfoSelection = langutil::DebugInfoSelection::Default();
	State m_stackState = Empty;
	CompilationSourceType m_compilationSourceType = CompilationSourceType::Solidity;
	MetadataFormat m_metadataFormat = defaultMetadataFormat();
};

}
