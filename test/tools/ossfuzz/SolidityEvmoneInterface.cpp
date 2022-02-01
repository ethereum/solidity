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

#include <test/tools/ossfuzz/SolidityEvmoneInterface.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/span.hpp>

using namespace solidity::test::fuzzer;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace std;

optional<CompilerOutput> SolidityCompilationFramework::compileContract()
{
	m_compiler.setSources(m_compilerInput.sourceCode);
	m_compiler.setLibraries(m_compilerInput.libraryAddresses);
	m_compiler.setEVMVersion(m_compilerInput.evmVersion);
	m_compiler.setOptimiserSettings(m_compilerInput.optimiserSettings);
	m_compiler.setViaIR(m_compilerInput.viaIR);
	if (!m_compiler.compile())
	{
		if (m_compilerInput.debugFailure)
		{
			cerr << "Compiling contract failed" << endl;
			for (auto const& error: m_compiler.errors())
				cerr << SourceReferenceFormatter::formatErrorInformation(
					*error,
					m_compiler
				);
		}
		return {};
	}
	else
	{
		string contractName;
		if (m_compilerInput.contractName.empty())
			contractName = m_compiler.lastContractName();
		else
			contractName = m_compilerInput.contractName;
		evmasm::LinkerObject obj = m_compiler.object(contractName);
		Json::Value methodIdentifiers = m_compiler.interfaceSymbols(contractName)["methods"];
		return CompilerOutput{obj.bytecode, methodIdentifiers};
	}
}

bool EvmoneUtility::zeroWord(uint8_t const* _result, size_t _length)
{
	return _length == 32 &&
		ranges::all_of(
			ranges::span(_result, static_cast<long>(_length)),
			[](uint8_t _v) { return _v == 0; });
}

evmc_message EvmoneUtility::initializeMessage(bytes const& _input)
{
	// Zero initialize all message fields
	evmc_message msg = {};
	// Gas available (value of type int64_t) is set to its maximum
	// value.
	msg.gas = std::numeric_limits<int64_t>::max();
	msg.input_data = _input.data();
	msg.input_size = _input.size();
	return msg;
}

evmc::result EvmoneUtility::executeContract(
	bytes const& _functionHash,
	evmc_address _deployedAddress
)
{
	evmc_message message = initializeMessage(_functionHash);
	message.destination = _deployedAddress;
	message.kind = EVMC_CALL;
	return m_evmHost.call(message);
}

evmc::result EvmoneUtility::deployContract(bytes const& _code)
{
	evmc_message message = initializeMessage(_code);
	message.kind = EVMC_CREATE;
	return m_evmHost.call(message);
}

evmc::result EvmoneUtility::deployAndExecute(
	bytes const& _byteCode,
	string const& _hexEncodedInput
)
{
	// Deploy contract and signal failure if deploy failed
	evmc::result createResult = deployContract(_byteCode);
	solAssert(
		createResult.status_code == EVMC_SUCCESS,
		"SolidityEvmoneInterface: Contract creation failed"
	);

	// Execute test function and signal failure if EVM reverted or
	// did not return expected output on successful execution.
	evmc::result callResult = executeContract(
		util::fromHex(_hexEncodedInput),
		createResult.create_address
	);

	// We don't care about EVM One failures other than EVMC_REVERT
	solAssert(
		callResult.status_code != EVMC_REVERT,
		"SolidityEvmoneInterface: EVM One reverted"
	);
	return callResult;
}

optional<evmc::result> EvmoneUtility::compileDeployAndExecute(string _fuzzIsabelle)
{
	map<string, h160> libraryAddressMap;
	// Stage 1: Compile and deploy library if present.
	if (!m_libraryName.empty())
	{
		m_compilationFramework.contractName(m_libraryName);
		auto compilationOutput = m_compilationFramework.compileContract();
		if (compilationOutput.has_value())
		{
			CompilerOutput cOutput = compilationOutput.value();
			// Deploy contract and signal failure if deploy failed
			evmc::result createResult = deployContract(cOutput.byteCode);
			solAssert(
				createResult.status_code == EVMC_SUCCESS,
				"SolidityEvmoneInterface: Library deployment failed"
			);
			libraryAddressMap[m_libraryName] = EVMHost::convertFromEVMC(createResult.create_address);
			m_compilationFramework.libraryAddresses(libraryAddressMap);
		}
		else
			return {};
	}

	// Stage 2: Compile, deploy, and execute contract, optionally using library
	// address map.
	m_compilationFramework.contractName(m_contractName);
	auto cOutput = m_compilationFramework.compileContract();
	if (cOutput.has_value())
	{
		solAssert(
			!cOutput->byteCode.empty() && !cOutput->methodIdentifiersInContract.empty(),
			"SolidityEvmoneInterface: Invalid compilation output."
		);

		string methodName;
		if (!_fuzzIsabelle.empty())
			// TODO: Remove this once a cleaner solution is found for querying
			// isabelle test entry point. At the moment, we are sure that the
			// entry point is the second method in the contract (hence the ++)
			// but not its name.
			methodName = (++cOutput->methodIdentifiersInContract.begin())->asString() +
				_fuzzIsabelle.substr(2, _fuzzIsabelle.size());
		else
			methodName = cOutput->methodIdentifiersInContract[m_methodName].asString();

		return deployAndExecute(
			cOutput->byteCode,
			methodName
		);
	}
	else
		return {};
}

optional<CompilerOutput> EvmoneUtility::compileContract()
{
	try
	{
		return m_compilationFramework.compileContract();
	}
	catch (evmasm::StackTooDeepException const&)
	{
		return {};
	}
}
