#pragma once

#include <test/EVMHost.h>

#include <libsolidity/interface/CompilerStack.h>

#include <libyul/AssemblyStack.h>

#include <libsolutil/Keccak256.h>

#include <evmone/evmone.h>

namespace solidity::test::abiv2fuzzer
{
class SolidityCompilationFramework
{
public:
	explicit SolidityCompilationFramework(langutil::EVMVersion _evmVersion = {});

	Json::Value getMethodIdentifiers()
	{
		return m_compiler.methodIdentifiers(m_compiler.lastContractName());
	}
	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName,
		std::map<std::string, solidity::util::h160> const& _libraryAddresses = {},
		frontend::OptimiserSettings _optimization = frontend::OptimiserSettings::minimal()
	);
protected:
	frontend::CompilerStack m_compiler;
	langutil::EVMVersion m_evmVersion;
};

struct AbiV2Utility
{
	/// Compares the contents of the memory address pointed to
	/// by `_result` of `_length` bytes to the expected output.
	/// Returns true if `_result` matches expected output, false
	/// otherwise.
	static bool isOutputExpected(
		uint8_t const* _result,
		size_t _length,
		std::vector<uint8_t> const& _expectedOutput
	);
	/// Accepts a reference to a user-specified input and returns an
	/// evmc_message with all of its fields zero initialized except
	/// gas and input fields.
	/// The gas field is set to the maximum permissible value so that we
	/// don't run into out of gas errors. The input field is copied from
	/// user input.
	static evmc_message initializeMessage(bytes const& _input);
	/// Accepts host context implementation, and keccak256 hash of the function
	/// to be called at a specified address in the simulated blockchain as
	/// input and returns the result of the execution of the called function.
	static evmc::result executeContract(
		EVMHost& _hostContext,
		bytes const& _functionHash,
		evmc_address _deployedAddress
	);
	/// Accepts a reference to host context implementation and byte code
	/// as input and deploys it on the simulated blockchain. Returns the
	/// result of deployment.
	static evmc::result deployContract(EVMHost& _hostContext, bytes const& _code);
};

}
