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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#pragma once

#include <libsolidity/CompilerStack.h>
#include <memory>
#include <boost/program_options.hpp>

namespace dev
{
namespace solidity
{

//forward declaration
enum class DocumentationType: uint8_t;

enum class OutputType: uint8_t
{
	STDOUT,
	FILE,
	BOTH
};

class CommandLineInterface
{
public:
	CommandLineInterface() {}

	/// Parse command line arguments and return false if we should not continue
	bool parseArguments(int argc, char** argv);
	/// Parse the files and create source code objects
	bool processInput();
	/// Perform actions on the input depending on provided compiler arguments
	void actOnInput();

private:
	void handleAst(std::string const& _argStr);
	void handleBinary(std::string const& _contract);
	void handleOpcode(std::string const& _contract);
	void handleBytecode(std::string const& _contract);
	void handleMeta(DocumentationType _type,
					std::string const& _contract);

	/// Compiler arguments variable map
	boost::program_options::variables_map m_args;
	/// map of input files to source code strings
	std::map<std::string, std::string> m_sourceCodes;
	/// Solidity compiler stack
	std::unique_ptr<dev::solidity::CompilerStack> m_compiler;
};

}
}
