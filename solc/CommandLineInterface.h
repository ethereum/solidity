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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#pragma once

#include <solc/CommandLineParser.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/FileReader.h>
#include <libyul/AssemblyStack.h>

#include <iostream>
#include <memory>
#include <string>

namespace solidity::frontend
{

class CommandLineInterface
{
public:
	explicit CommandLineInterface(
		std::istream& _sin,
		std::ostream& _sout,
		std::ostream& _serr,
		CommandLineOptions const& _options = CommandLineOptions{}
	):
		m_sin(_sin),
		m_sout(_sout),
		m_serr(_serr),
		m_options(_options)
	{}

	/// Parse command line arguments and return false if we should not continue
	bool parseArguments(int _argc, char const* const* _argv);
	/// Read the content of all input files and initialize the file reader.
	bool readInputFiles();
	/// Parse the files and create source code objects
	bool processInput();
	/// Perform actions on the input depending on provided compiler arguments
	/// @returns true on success.
	bool actOnInput();

	CommandLineOptions const& options() const { return m_options; }
	FileReader const& fileReader() const { return m_fileReader; }
	std::optional<std::string> const& standardJsonInput() const { return m_standardJsonInput; }

private:
	bool compile();
	bool link();
	void writeLinkedFiles();
	/// @returns the ``// <identifier> -> name`` hint for library placeholders.
	static std::string libraryPlaceholderHint(std::string const& _libraryName);
	/// @returns the full object with library placeholder hints in hex.
	static std::string objectWithLinkRefsHex(evmasm::LinkerObject const& _obj);

	bool assemble(
		yul::AssemblyStack::Language _language,
		yul::AssemblyStack::Machine _targetMachine,
		bool _optimize,
		std::optional<unsigned int> _expectedExecutionsPerDeployment = std::nullopt,
		std::optional<std::string> _yulOptimiserSteps = std::nullopt
	);

	void outputCompilationResults();

	void handleCombinedJSON();
	void handleAst();
	void handleBinary(std::string const& _contract);
	void handleOpcode(std::string const& _contract);
	void handleIR(std::string const& _contract);
	void handleIROptimized(std::string const& _contract);
	void handleEwasm(std::string const& _contract);
	void handleBytecode(std::string const& _contract);
	void handleSignatureHashes(std::string const& _contract);
	void handleMetadata(std::string const& _contract);
	void handleABI(std::string const& _contract);
	void handleNatspec(bool _natspecDev, std::string const& _contract);
	void handleGasEstimation(std::string const& _contract);
	void handleStorageLayout(std::string const& _contract);

	/// Tries to read @ m_sourceCodes as a JSONs holding ASTs
	/// such that they can be imported into the compiler  (importASTs())
	/// (produced by --combined-json ast,compact-format <file.sol>
	/// or standard-json output
	std::map<std::string, Json::Value> parseAstFromInput();

	/// Create a file in the given directory
	/// @arg _fileName the name of the file
	/// @arg _data to be written
	void createFile(std::string const& _fileName, std::string const& _data);

	/// Create a json file in the given directory
	/// @arg _fileName the name of the file (the extension will be replaced with .json)
	/// @arg _json json string to be written
	void createJson(std::string const& _fileName, std::string const& _json);

	/// Returns the stream that should receive normal output. Sets m_hasOutput to true if the
	/// stream has ever been used unless @arg _markAsUsed is set to false.
	std::ostream& sout(bool _markAsUsed = true);

	/// Returns the stream that should receive error output. Sets m_hasOutput to true if the
	/// stream has ever been used unless @arg _markAsUsed is set to false.
	std::ostream& serr(bool _markAsUsed = true);

	std::istream& m_sin;
	std::ostream& m_sout;
	std::ostream& m_serr;
	bool m_hasOutput = false;
	bool m_error = false; ///< If true, some error occurred.
	FileReader m_fileReader;
	std::optional<std::string> m_standardJsonInput;
	std::unique_ptr<frontend::CompilerStack> m_compiler;
	CommandLineOptions m_options;
};

}
