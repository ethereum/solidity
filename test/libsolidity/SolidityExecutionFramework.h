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
 * @date 2014
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#pragma once

#include <functional>

#include <test/ExecutionFramework.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/DebugSettings.h>

#include <libyul/AssemblyStack.h>

namespace solidity::frontend::test
{

class SolidityExecutionFramework: public solidity::test::ExecutionFramework
{

public:
	SolidityExecutionFramework(): m_showMetadata(solidity::test::CommonOptions::get().showMetadata) {}
	explicit SolidityExecutionFramework(langutil::EVMVersion _evmVersion, std::vector<boost::filesystem::path> const& _vmPaths):
		ExecutionFramework(_evmVersion, _vmPaths), m_showMetadata(solidity::test::CommonOptions::get().showMetadata)
	{}

	bytes const& compileAndRunWithoutCheck(
		std::map<std::string, std::string> const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = {},
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {}
	) override
	{
		bytes bytecode = multiSourceCompileContract(_sourceCode, _contractName, _libraryAddresses);
		sendMessage(bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName = "",
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {}
	);

	bytes multiSourceCompileContract(
		std::map<std::string, std::string> const& _sources,
		std::string const& _contractName = "",
		std::map<std::string, solidity::test::Address> const& _libraryAddresses = {}
	);

	/// Returns @param _sourceCode prefixed with the version pragma and the abi coder v1 pragma,
	/// the latter only if it is forced.
	static std::string addPreamble(std::string const& _sourceCode);
protected:

	solidity::frontend::CompilerStack m_compiler;
	bool m_compileViaYul = false;
	bool m_compileToEwasm = false;
	bool m_showMetadata = false;
	RevertStrings m_revertStrings = RevertStrings::Default;
};

} // end namespaces
