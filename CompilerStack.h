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
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#pragma once

#include <ostream>
#include <string>
#include <memory>
#include <libdevcore/Common.h>

namespace dev {
namespace solidity {

// forward declarations
class Scanner;
class ContractDefinition;
class Compiler;
class GlobalContext;

/**
 * Easy to use and self-contained Solidity compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack
{
public:
	CompilerStack() {}
	void reset() {  *this = CompilerStack(); }
	void setSource(std::string const& _sourceCode);
	void parse();
	void parse(std::string const& _sourceCode);
	/// Compiles the contract that was previously parsed.
	bytes const& compile(bool _optimize = false);
	/// Parses and compiles the given source code.
	bytes const& compile(std::string const& _sourceCode, bool _optimize = false);

	bytes const& getBytecode() const { return m_bytecode; }
	/// Streams a verbose version of the assembly to @a _outStream.
	/// Prerequisite: Successful compilation.
	void streamAssembly(std::ostream& _outStream);

	/// Returns a string representing the contract interface in JSON.
	/// Prerequisite: Successful call to parse or compile.
	std::string const& getInterface();

	/// Returns the previously used scanner, useful for counting lines during error reporting.
	Scanner const& getScanner() const { return *m_scanner; }
	ContractDefinition& getAST() const { return *m_contractASTNode; }

	/// Compile the given @a _sourceCode to bytecode. If a scanner is provided, it is used for
	/// scanning the source code - this is useful for printing exception information.
	static bytes staticCompile(std::string const& _sourceCode, bool _optimize = false);

private:
	std::shared_ptr<Scanner> m_scanner;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::shared_ptr<ContractDefinition> m_contractASTNode;
	bool m_parseSuccessful;
	std::string m_interface;
	std::shared_ptr<Compiler> m_compiler;
	bytes m_bytecode;
};

}
}
