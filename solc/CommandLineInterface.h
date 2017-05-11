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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#pragma once

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/inlineasm/AsmStack.h>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <memory>

namespace dev
{
namespace solidity
{

//forward declaration
enum class DocumentationType: uint8_t;

class CommandLineInterface
{
public:
	CommandLineInterface() {}

	/// Parse command line arguments and return false if we should not continue
	bool parseArguments(int _argc, char** _argv);
	/// Parse the files and create source code objects
	bool processInput();
	/// Perform actions on the input depending on provided compiler arguments
	/// @returns true on success.
	bool actOnInput();

private:
	bool link();
	void writeLinkedFiles();

	/// Parse assembly input.
	bool assemble();
	void outputAssembly();

	void outputCompilationResults();

	void handleCombinedJSON();
	void handleAst(std::string const& _argStr);
	void handleBinary(std::string const& _contract);
	void handleOpcode(std::string const& _contract);
	void handleBytecode(std::string const& _contract);
	void handleSignatureHashes(std::string const& _contract);
	void handleOnChainMetadata(std::string const& _contract);
	void handleABI(std::string const& _contract);
	void handleNatspec(DocumentationType _type, std::string const& _contract);
	void handleGasEstimation(std::string const& _contract);
	void handleFormal();

	/// Fills @a m_sourceCodes initially and @a m_redirects.
	void readInputFilesAndConfigureRemappings();
	/// Tries to read from the file @a _input or interprets _input literally if that fails.
	/// It then tries to parse the contents and appends to m_libraries.
	bool parseLibraryOption(std::string const& _input);

	/// Create a file in the given directory
	/// @arg _fileName the name of the file
	/// @arg _data to be written
	void createFile(std::string const& _fileName, std::string const& _data);

	bool m_error = false; ///< If true, some error occurred.

	bool m_onlyAssemble = false;
	bool m_onlyLink = false;

	/// Compiler arguments variable map
	boost::program_options::variables_map m_args;
	/// map of input files to source code strings
	std::map<std::string, std::string> m_sourceCodes;
	/// list of allowed directories to read files from
	std::vector<boost::filesystem::path> m_allowedDirectories;
	/// map of library names to addresses
	std::map<std::string, h160> m_libraries;
	/// Solidity compiler stack
	std::unique_ptr<dev::solidity::CompilerStack> m_compiler;
	/// Assembly stacks for assembly-only mode
	std::map<std::string, assembly::InlineAssemblyStack> m_assemblyStacks;
};

}
}
