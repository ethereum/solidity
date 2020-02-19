#pragma once

#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/interface/Version.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SourceLocation.h>

#include <libevmasm/LinkerObject.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/AssemblyItem.h>

#include <liblangutil/Scanner.h>
#include <libsolidity/ast/AST.h>
#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <json/json.h>

#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace solidity::langutil
{
class Scanner;
}

namespace solidity::evmasm
{
using AssemblyItems = std::vector<AssemblyItem>;
} // namespace solidity::evmasm

namespace solidity::test
{
// forward declarations
class ASTNode;
class ContractDefinition;
class FunctionDefinition;
class Compiler;
class GlobalContext;
class Natspec;
class DeclarationContainer;

class FakeCompilerStack : boost::noncopyable
{
  public:
	enum State
	{
		Empty,
		SourcesSet,
		ParsingPerformed,
		AnalysisPerformed,
		CompilationSuccessful
	};

	enum class MetadataHash
	{
		IPFS,
		Bzzr1,
		None
	};

	struct Remapping
	{
		std::string context;
		std::string prefix;
		std::string target;
	};

	/// @returns the list of errors that occurred during parsing and type checking.
	langutil::ErrorList const &errors() const
	{
		static langutil::ErrorList result;
		return result;
	}

	/// @returns the current state.
	State state() const { return State(); }

	bool hasError() const { return true; }

	bool compilationSuccessful() const { return false; }

	/// Resets the compiler to an empty state. Unless @a _keepSettings is set to true,
	/// all settings are reset as well.
	void reset(bool _keepSettings = false) { (void) _keepSettings; }

	// Parses a remapping of the format "context:prefix=target".
	static std::optional<Remapping> parseRemapping(std::string const &_remapping)
	{
		(void) _remapping;
		static std::optional<Remapping> result;
		return result;
	}

	/// Sets path remappings.
	/// Must be set before parsing.
	void setRemappings(std::vector<Remapping> const &_remappings) { (void) _remappings; }

	/// Sets library addresses. Addresses are cleared iff @a _libraries is missing.
	/// Must be set before parsing.
	void setLibraries(std::map<std::string, util::h160> const &_libraries = {}) { (void) _libraries; }

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(bool _optimize, unsigned _runs = 200)
	{
		(void) _optimize;
		(void) _runs;
	}

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(frontend::OptimiserSettings _settings) { (void) _settings; }

	/// Sets whether to strip revert strings, add additional strings or do nothing at all.
	void setRevertStringBehaviour(frontend::RevertStrings _revertStrings) { (void) _revertStrings; }

	/// Set whether or not parser error is desired.
	/// When called without an argument it will revert to the default.
	/// Must be set before parsing.
	void setParserErrorRecovery(bool _wantErrorRecovery = false) { (void) _wantErrorRecovery; }

	/// Set the EVM version used before running compile.
	/// When called without an argument it will revert to the default version.
	/// Must be set before parsing.
	void setEVMVersion(langutil::EVMVersion _version = langutil::EVMVersion{}) { (void) _version; }

	/// Set which SMT solvers should be enabled.
	void setSMTSolverChoice(frontend::smt::SMTSolverChoice _enabledSolvers) { (void) _enabledSolvers; }

	/// Sets the requested contract names by source.
	/// If empty, no filtering is performed and every contract
	/// found in the supplied sources is compiled.
	/// Names are cleared iff @a _contractNames is missing.
	void setRequestedContractNames(std::map<std::string, std::set<std::string>> const &_contractNames
	                               = std::map<std::string, std::set<std::string>>{})
	{
		(void) _contractNames;
	}

	/// Enable experimental generation of Yul IR code.
	void enableIRGeneration(bool _enable = true) { (void) _enable; }

	/// Enable experimental generation of Ewasm code. If enabled, IR is also generated.
	void enableEwasmGeneration(bool _enable = true) { (void) _enable; }

	/// @arg _metadataLiteralSources When true, store sources as literals in the contract metadata.
	/// Must be set before parsing.
	void useMetadataLiteralSources(bool _metadataLiteralSources) { (void) _metadataLiteralSources; }

	/// Sets whether and which hash should be used
	/// to store the metadata in the bytecode.
	/// @param _metadataHash can be IPFS, Bzzr1, None
	void setMetadataHash(MetadataHash _metadataHash) { (void) _metadataHash; }

	/// Sets the sources. Must be set before parsing.
	void setSources(StringMap _sources) { (void) _sources; }

	/// Adds a response to an SMTLib2 query (identified by the hash of the query input).
	/// Must be set before parsing.
	void addSMTLib2Response(util::h256 const &_hash, std::string const &_response)
	{
		(void) _hash;
		(void) _response;
	}

	/// Parses all source units that were added
	/// @returns false on error.
	bool parse() { return false; }

	/// Imports given SourceUnits so they can be analyzed. Leads to the same internal state as parse().
	/// Will throw errors if the import fails
	void importASTs(std::map<std::string, Json::Value> const &_sources) { (void) _sources; }

	/// Performs the analysis steps (imports, scopesetting, syntaxCheck, referenceResolving,
	///  typechecking, staticAnalysis) on previously parsed sources.
	/// @returns false on error.
	bool analyze() { return false; }

	/// Parses and analyzes all source units that were added
	/// @returns false on error.
	bool parseAndAnalyze() { return false; }

	/// Compiles the source units that were previously added and parsed.
	/// @returns false on error.
	bool compile() { return false; }

	/// @returns the list of sources (paths) used
	std::vector<std::string> sourceNames() const { return std::vector<std::string>(); }

	/// @returns a mapping assigning each source name its index inside the vector returned
	/// by sourceNames().
	std::map<std::string, unsigned> sourceIndices() const { return std::map<std::string, unsigned>(); }

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	langutil::Scanner const &scanner(std::string const &_sourceName) const
	{
		(void) _sourceName;
		static langutil::Scanner result;
		return result;
	}

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(langutil::SourceLocation const &_sourceLocation) const
	{
		(void) _sourceLocation;
		return std::tuple<int, int, int, int>();
	}

	/// @returns a list of unhandled queries to the SMT solver (has to be supplied in a second run
	/// by calling @a addSMTLib2Response).
	std::vector<std::string> const &unhandledSMTLib2Queries() const
	{
		static std::vector<std::string> result;
		return result;
	}

	/// @returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const { return std::vector<std::string>(); }

	/// @returns the name of the last contract.
	std::string const lastContractName() const { return ""; }

	/// @returns either the contract's name or a mixture of its name and source file, sanitized for filesystem use
	std::string const filesystemFriendlyName(std::string const &_contractName) const
	{
		(void) _contractName;
		return "";
	}

	/// @returns the IR representation of a contract.
	std::string const &yulIR(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return result;
	}

	/// @returns the optimized IR representation of a contract.
	std::string const &yulIROptimized(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return result;
	}

	/// @returns the Ewasm text representation of a contract.
	std::string const &ewasm(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return result;
	}

	/// @returns the Ewasm representation of a contract.
	evmasm::LinkerObject const &ewasmObject(std::string const &_contractName) const
	{
		(void) _contractName;
		static evmasm::LinkerObject result;
		return result;
	}

	/// @returns the assembled object for a contract.
	evmasm::LinkerObject const &object(std::string const &_contractName) const
	{
		(void) _contractName;
		static evmasm::LinkerObject result;
		return result;
	}

	/// @returns the runtime object for the contract.
	evmasm::LinkerObject const &runtimeObject(std::string const &_contractName) const
	{
		(void) _contractName;
		static evmasm::LinkerObject result;
		return result;
	}

	/// @returns normal contract assembly items
	evmasm::AssemblyItems const *assemblyItems(std::string const &_contractName) const
	{
		(void) _contractName;
		static evmasm::AssemblyItems result;
		return &result;
	}

	/// @returns runtime contract assembly items
	evmasm::AssemblyItems const *runtimeAssemblyItems(std::string const &_contractName) const
	{
		(void) _contractName;
		static evmasm::AssemblyItems result;
		return &result;
	}

	/// @returns the string that provides a mapping between bytecode and sourcecode or a nullptr
	/// if the contract does not (yet) have bytecode.
	std::string const *sourceMapping(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return &result;
	}

	/// @returns the string that provides a mapping between runtime bytecode and sourcecode.
	/// if the contract does not (yet) have bytecode.
	std::string const *runtimeSourceMapping(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return &result;
	}

	/// @return a verbose text representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	std::string assemblyString(std::string const &_contractName, StringMap _sourceCodes = StringMap()) const
	{
		(void) _contractName;
		(void) _sourceCodes;
		return "";
	}

	/// @returns a JSON representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	Json::Value assemblyJSON(std::string const &_contractName, StringMap const &_sourceCodes = StringMap()) const
	{
		(void) _contractName;
		(void) _sourceCodes;
		return Json::Value();
	}

	/// @returns a JSON representing the contract ABI.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const &contractABI(std::string const &_contractName) const
	{
		(void) _contractName;
		static Json::Value result;
		return result;
	}


	/// @returns a JSON representing the storage layout of the contract.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const &storageLayout(std::string const &_contractName) const
	{
		(void) _contractName;
		static Json::Value result;
		return result;
	}

	/// @returns a JSON representing the contract's user documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const &natspecUser(std::string const &_contractName) const
	{
		(void) _contractName;
		static Json::Value result;
		return result;
	}

	/// @returns a JSON representing the contract's developer documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const &natspecDev(std::string const &_contractName) const
	{
		(void) _contractName;
		static Json::Value result;
		return result;
	}

	/// @returns a JSON representing a map of method identifiers (hashes) to function names.
	Json::Value methodIdentifiers(std::string const &_contractName) const
	{
		(void) _contractName;
		return Json::Value();
	}

	/// @returns the Contract Metadata
	std::string const &metadata(std::string const &_contractName) const
	{
		(void) _contractName;
		static std::string result;
		return result;
	}

	/// @returns a JSON representing the estimated gas usage for contract creation, internal and external functions
	Json::Value gasEstimates(std::string const &_contractName) const
	{
		(void) _contractName;
		return Json::Value();
	}

	/// Overwrites the release/prerelease flag. Should only be used for testing.
	void overwriteReleaseFlag(bool release) { (void) release; }
};

} // namespace solidity::test
