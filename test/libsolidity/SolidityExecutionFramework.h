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

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

namespace solidity::frontend::test
{

class SolidityExecutionFramework: public solidity::test::ExecutionFramework
{

public:
	SolidityExecutionFramework(): m_showMetadata(solidity::test::CommonOptions::get().showMetadata) {}
	explicit SolidityExecutionFramework(langutil::EVMVersion _evmVersion):
		ExecutionFramework(_evmVersion), m_showMetadata(solidity::test::CommonOptions::get().showMetadata)
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

	/// Returns @param _sourceCode prefixed with the version pragma and the ABIEncoderV2 pragma,
	/// the latter only if it is required.
	static std::string addPreamble(std::string const& _sourceCode);
protected:
	solidity::frontend::CompilerStack m_compiler;
	bool m_compileViaYul = false;
	bool m_showMetadata = false;
	RevertStrings m_revertStrings = RevertStrings::Default;
};

} // end namespaces
