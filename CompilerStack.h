/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
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
#include <boost/noncopyable.hpp>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{

namespace eth
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
}

namespace solidity
{

// forward declarations
class Scanner;
class ContractDefinition;
class FunctionDefinition;
class SourceUnit;
class Compiler;
class GlobalContext;
class InterfaceHandler;

enum class DocumentationType: uint8_t
{
	NatspecUser = 1,
	NatspecDev,
	ABIInterface,
	ABISolidityInterface
};

/**
 * Easy to use and self-contained Solidity compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: boost::noncopyable
{
public:
	/// Creates a new compiler stack. Adds standard sources if @a _addStandardSources.
	explicit CompilerStack(bool _addStandardSources = true);

	/// Resets the compiler to a state where the sources are not parsed or even removed.
	void reset(bool _keepSources = false, bool _addStandardSources = true);

	/// Adds a source object (e.g. file) to the parser. After this, parse has to be called again.
	/// @returns true if a source object by the name already existed and was replaced.
	void addSources(StringMap const& _nameContents, bool _isLibrary = false) { for (auto const& i: _nameContents) addSource(i.first, i.second, _isLibrary); }
	bool addSource(std::string const& _name, std::string const& _content, bool _isLibrary = false);
	void setSource(std::string const& _sourceCode);
	/// Parses all source units that were added
	void parse();
	/// Sets the given source code as the only source unit apart from standard sources and parses it.
	void parse(std::string const& _sourceCode);
	/// Returns a list of the contract names in the sources.
	std::vector<std::string> getContractNames() const;
	std::string defaultContractName() const;

	/// Compiles the source units that were previously added and parsed.
	void compile(bool _optimize = false, unsigned _runs = 200);
	/// Parses and compiles the given source code.
	/// @returns the compiled bytecode
	bytes const& compile(std::string const& _sourceCode, bool _optimize = false);

	/// @returns the assembled bytecode for a contract.
	bytes const& getBytecode(std::string const& _contractName = "") const;
	/// @returns the runtime bytecode for the contract, i.e. the code that is returned by the constructor.
	bytes const& getRuntimeBytecode(std::string const& _contractName = "") const;
	/// @returns the bytecode of a contract that uses an already deployed contract via CALLCODE.
	/// The returned bytes will contain a sequence of 20 bytes of the format "XXX...XXX" which have to
	/// substituted by the actual address. Note that this sequence starts end ends in three X
	/// characters but can contain anything in between.
	bytes const& getCloneBytecode(std::string const& _contractName = "") const;
	/// @returns normal contract assembly items
	eth::AssemblyItems const* getAssemblyItems(std::string const& _contractName = "") const;
	/// @returns runtime contract assembly items
	eth::AssemblyItems const* getRuntimeAssemblyItems(std::string const& _contractName = "") const;
	/// @returns hash of the runtime bytecode for the contract, i.e. the code that is returned by the constructor.
	dev::h256 getContractCodeHash(std::string const& _contractName = "") const;

	/// Streams a verbose version of the assembly to @a _outStream.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// @arg _inJsonFromat shows whether the out should be in Json format
	/// Prerequisite: Successful compilation.
	Json::Value streamAssembly(std::ostream& _outStream, std::string const& _contractName = "", StringMap _sourceCodes = StringMap(), bool _inJsonFormat = false) const;

	/// Returns a string representing the contract interface in JSON.
	/// Prerequisite: Successful call to parse or compile.
	std::string const& getInterface(std::string const& _contractName = "") const;
	/// Returns a string representing the contract interface in Solidity.
	/// Prerequisite: Successful call to parse or compile.
	std::string const& getSolidityInterface(std::string const& _contractName = "") const;
	/// Returns a string representing the contract's documentation in JSON.
	/// Prerequisite: Successful call to parse or compile.
	/// @param type The type of the documentation to get.
	/// Can be one of 4 types defined at @c DocumentationType
	std::string const& getMetadata(std::string const& _contractName, DocumentationType _type) const;

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	Scanner const& getScanner(std::string const& _sourceName = "") const;
	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& getAST(std::string const& _sourceName = "") const;
	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& getContractDefinition(std::string const& _contractName) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t getFunctionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	/// Compile the given @a _sourceCode to bytecode. If a scanner is provided, it is used for
	/// scanning the source code - this is useful for printing exception information.
	static bytes staticCompile(std::string const& _sourceCode, bool _optimize = false);

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(SourceLocation const& _sourceLocation) const;

private:
	/**
	 * Information pertaining to one source unit, filled gradually during parsing and compilation.
	 */
	struct Source
	{
		std::shared_ptr<Scanner> scanner;
		std::shared_ptr<SourceUnit> ast;
		std::string interface;
		bool isLibrary = false;
		void reset() { scanner.reset(); ast.reset(); interface.clear(); }
	};

	struct Contract
	{
		ContractDefinition const* contract = nullptr;
		std::shared_ptr<Compiler> compiler;
		bytes bytecode;
		bytes runtimeBytecode;
		bytes cloneBytecode;
		std::shared_ptr<InterfaceHandler> interfaceHandler;
		mutable std::unique_ptr<std::string const> interface;
		mutable std::unique_ptr<std::string const> solidityInterface;
		mutable std::unique_ptr<std::string const> userDocumentation;
		mutable std::unique_ptr<std::string const> devDocumentation;

		Contract();
	};

	void resolveImports();

	Contract const& getContract(std::string const& _contractName = "") const;
	Source const& getSource(std::string const& _sourceName = "") const;

	bool m_parseSuccessful;
	std::map<std::string const, Source> m_sources;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::vector<Source const*> m_sourceOrder;
	std::map<std::string const, Contract> m_contracts;
};

}
}
