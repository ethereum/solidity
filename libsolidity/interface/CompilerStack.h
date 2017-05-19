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

#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/LinkerObject.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ReadFile.h>

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

enum class DocumentationType: uint8_t
{
	NatspecUser = 1,
	NatspecDev
};

/**
 * Easy to use and self-contained Solidity compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: boost::noncopyable
{
public:
	/// Creates a new compiler stack.
	/// @param _readFile callback to used to read files for import statements. Must return
	/// and must not emit exceptions.
	explicit CompilerStack(ReadFile::Callback const& _readFile = ReadFile::Callback());

	/// Sets path remappings in the format "context:prefix=target"
	void setRemappings(std::vector<std::string> const& _remappings);

	/// Resets the compiler to a state where the sources are not parsed or even removed.
	void reset(bool _keepSources = false);

	/// Adds a source object (e.g. file) to the parser. After this, parse has to be called again.
	/// @returns true if a source object by the name already existed and was replaced.
	void addSources(StringMap const& _nameContents, bool _isLibrary = false)
	{
		for (auto const& i: _nameContents) addSource(i.first, i.second, _isLibrary);
	}
	bool addSource(std::string const& _name, std::string const& _content, bool _isLibrary = false);
	void setSource(std::string const& _sourceCode);
	/// Parses all source units that were added
	/// @returns false on error.
	bool parse();
	/// Sets the given source code as the only source unit apart from standard sources and parses it.
	/// @returns false on error.
	bool parse(std::string const& _sourceCode);
	/// performs the analyisis steps (imports, scopesetting, syntaxCheck, referenceResolving,
	///  typechecking, staticAnalysis) on previously set sources
	/// @returns false on error.
	bool analyze();
	/// Parses and analyzes all source units that were added
	/// @returns false on error.
	bool parseAndAnalyze();
	/// Sets the given source code as the only source unit apart from standard sources and parses and analyzes it.
	/// @returns false on error.
	bool parseAndAnalyze(std::string const& _sourceCode);
	/// @returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const;
	std::string defaultContractName() const;

	/// Compiles the source units that were previously added and parsed.
	/// @returns false on error.
	bool compile(
		bool _optimize = false,
		unsigned _runs = 200,
		std::map<std::string, h160> const& _libraries = std::map<std::string, h160>{}
	);
	/// Parses and compiles the given source code.
	/// @returns false on error.
	bool compile(std::string const& _sourceCode, bool _optimize = false, unsigned _runs = 200);

	/// Tries to translate all source files into a language suitable for formal analysis.
	/// @param _errors list to store errors - defaults to the internal error list.
	/// @returns false on error.
	bool prepareFormalAnalysis(ErrorList* _errors = nullptr);
	std::string const& formalTranslation() const { return m_formalTranslation; }

	/// @returns the assembled object for a contract.
	eth::LinkerObject const& object(std::string const& _contractName = "") const;
	/// @returns the runtime object for the contract.
	eth::LinkerObject const& runtimeObject(std::string const& _contractName = "") const;
	/// @returns the bytecode of a contract that uses an already deployed contract via DELEGATECALL.
	/// The returned bytes will contain a sequence of 20 bytes of the format "XXX...XXX" which have to
	/// substituted by the actual address. Note that this sequence starts end ends in three X
	/// characters but can contain anything in between.
	eth::LinkerObject const& cloneObject(std::string const& _contractName = "") const;
	/// @returns normal contract assembly items
	eth::AssemblyItems const* assemblyItems(std::string const& _contractName = "") const;
	/// @returns runtime contract assembly items
	eth::AssemblyItems const* runtimeAssemblyItems(std::string const& _contractName = "") const;
	/// @returns the string that provides a mapping between bytecode and sourcecode or a nullptr
	/// if the contract does not (yet) have bytecode.
	std::string const* sourceMapping(std::string const& _contractName = "") const;
	/// @returns the string that provides a mapping between runtime bytecode and sourcecode.
	/// if the contract does not (yet) have bytecode.
	std::string const* runtimeSourceMapping(std::string const& _contractName = "") const;

	/// @returns either the contract's name or a mixture of its name and source file, sanitized for filesystem use
	std::string const filesystemFriendlyName(std::string const& _contractName) const;

	/// @returns hash of the runtime bytecode for the contract, i.e. the code that is
	/// returned by the constructor or the zero-h256 if the contract still needs to be linked or
	/// does not have runtime code.
	dev::h256 contractCodeHash(std::string const& _contractName = "") const;

	/// Streams a verbose version of the assembly to @a _outStream.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// @arg _inJsonFromat shows whether the out should be in Json format
	/// Prerequisite: Successful compilation.
	Json::Value streamAssembly(std::ostream& _outStream, std::string const& _contractName = "", StringMap _sourceCodes = StringMap(), bool _inJsonFormat = false) const;

	/// @returns the list of sources (paths) used
	std::vector<std::string> sourceNames() const;
	/// @returns a mapping assigning each source name its index inside the vector returned
	/// by sourceNames().
	std::map<std::string, unsigned> sourceIndices() const;
	/// @returns a JSON representing the contract ABI.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& contractABI(std::string const& _contractName = "") const;
	/// @returns a JSON representing the contract's documentation.
	/// Prerequisite: Successful call to parse or compile.
	/// @param type The type of the documentation to get.
	/// Can be one of 4 types defined at @c DocumentationType
	Json::Value const& natspec(std::string const& _contractName, DocumentationType _type) const;
	std::string const& onChainMetadata(std::string const& _contractName) const;
	void useMetadataLiteralSources(bool _metadataLiteralSources) { m_metadataLiteralSources = _metadataLiteralSources; }

	/// @returns a JSON representing the estimated gas usage for contract creation, internal and external functions
	Json::Value gasEstimates(std::string const& _contractName) const;

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	Scanner const& scanner(std::string const& _sourceName = "") const;
	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& ast(std::string const& _sourceName = "") const;
	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& contractDefinition(std::string const& _contractName) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t functionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(SourceLocation const& _sourceLocation) const;

	/// @returns the list of errors that occured during parsing and type checking.
	ErrorList const& errors() const { return m_errors; }

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
		std::string onChainMetadata; ///< The metadata json that will be hashed into the chain.
		mutable std::unique_ptr<Json::Value const> abi;
		mutable std::unique_ptr<Json::Value const> userDocumentation;
		mutable std::unique_ptr<Json::Value const> devDocumentation;
		mutable std::unique_ptr<std::string const> sourceMapping;
		mutable std::unique_ptr<std::string const> runtimeSourceMapping;
	};
	enum State {
		Empty,
		SourcesSet,
		ParsingSuccessful,
		AnalysisSuccessful,
		CompilationSuccessful
	};

	/// Loads the missing sources from @a _ast (named @a _path) using the callback
	/// @a m_readFile and stores the absolute paths of all imports in the AST annotations.
	/// @returns the newly loaded sources.
	StringMap loadMissingSources(SourceUnit const& _ast, std::string const& _path);
	std::string applyRemapping(std::string const& _path, std::string const& _context);
	void resolveImports();
	/// @returns the absolute path corresponding to @a _path relative to @a _reference.
	std::string absolutePath(std::string const& _path, std::string const& _reference) const;
	/// Helper function to return path converted strings.
	std::string sanitizePath(std::string const& _path) const { return boost::filesystem::path(_path).generic_string(); }

	/// Compile a single contract and put the result in @a _compiledContracts.
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, eth::Assembly const*>& _compiledContracts
	);
	void link();

	Contract const& contract(std::string const& _contractName = "") const;
	Source const& source(std::string const& _sourceName = "") const;

	std::string createOnChainMetadata(Contract const& _contract) const;
	std::string computeSourceMapping(eth::AssemblyItems const& _items) const;
	Json::Value const& contractABI(Contract const&) const;
	Json::Value const& natspec(Contract const&, DocumentationType _type) const;

	struct Remapping
	{
		std::string context;
		std::string prefix;
		std::string target;
	};

	ReadFile::Callback m_readFile;
	bool m_optimize = false;
	unsigned m_optimizeRuns = 200;
	std::map<std::string, h160> m_libraries;
	/// list of path prefix remappings, e.g. mylibrary: github.com/ethereum = /usr/local/ethereum
	/// "context:prefix=target"
	std::vector<Remapping> m_remappings;
	std::map<std::string const, Source> m_sources;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>> m_scopes;
	std::vector<Source const*> m_sourceOrder;
	std::map<std::string const, Contract> m_contracts;
	std::string m_formalTranslation;
	ErrorList m_errors;
	bool m_metadataLiteralSources = false;
	State m_stackState = Empty;
};

}
}
