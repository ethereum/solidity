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

#include <test/tools/fuzzer_common.h>

#include <test/tools/ossfuzz/SolidityEvmoneInterface.h>

#include <test/tools/ossfuzz/ValueGenerator.h>

#include <test/TestCaseReader.h>

#include <libsolidity/util/ContractABIUtils.h>

#include <libyul/backends/evm/EVMCodeTransform.h>

#include <liblangutil/EVMVersion.h>

#include <evmone/evmone.h>

#include <abicoder.hpp>

#include <regex>
#include <sstream>

using namespace solidity::frontend::test;
using namespace solidity::frontend;
using namespace solidity::test::fuzzer;
using namespace solidity::langutil;
using namespace solidity::test;
using namespace std;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};
static constexpr size_t abiCoderHeapSize = 1024 * 512;

namespace
{
string abiEncoding(
	Json::Value const& _functionABI,
	map<h160, vector<string>> _addressSelectors,
	Json::Value const& _methodIdentifiers
)
{
	string abiTypeString;
	string abiValueString;
	tie(abiTypeString, abiValueString) = ValueGenerator{
		_functionABI["inputs"],
		0,
		_addressSelectors
		}.type();
	string encodedData;
	// A function with inputs must contain type names within
	// parentheses.
	bool functionWithInputs = abiTypeString != "()";
	string functionSignature = _functionABI["name"].asString() + abiTypeString;
	cout << functionSignature << endl;
	string encoding = _methodIdentifiers[functionSignature].asString();
	if (functionWithInputs)
	{
		abicoder::ABICoder coder(abiCoderHeapSize);
		auto [status, data] = coder.encode(abiTypeString, abiValueString);
		cout << abiTypeString << endl;
		cout << abiValueString << endl;
		solAssert(status, "Isabelle coder: failed.");
		encoding += data.substr(2, data.size());
	}
	return encoding;
}
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	string input(reinterpret_cast<char const*>(_data), _size);
	regex re = regex("library\\s*(\\w+)\\s*\\{");
	smatch matches;
	std::string libraryName;
	auto match = regex_search(input, matches, re);
	if (match && matches[1].matched)
		libraryName = matches[1].str();

	map<string, string> sourceCode;
	try
	{
		EVMVersion version;
		EVMHost hostContext(version, evmone);

		TestCaseReader t = TestCaseReader(std::istringstream(input));
		sourceCode = t.sources().sources;
		string contractName;
		string methodName;
		auto compilerSetting = OptimiserSettings::standard();
		CompilerInput cInput = {
			version,
			sourceCode,
			contractName,
			compilerSetting,
			{},
			true,
			false
		};
		EvmoneUtility evmoneUtil(
			hostContext,
			cInput,
			contractName,
			libraryName,
			methodName
		);

		if (!libraryName.empty()) {
			cout << "Deploying library" << endl;
			auto l = evmoneUtil.compileAndDeployLibrary();
			if (!l.has_value())
				return 0;
			cout << "Deployed" << endl;
		}

		map<h160, vector<string>> addressSelector;
		for (auto const& account: hostContext.accounts)
			addressSelector[EVMHost::convertFromEVMC(account.first)] = {};

		auto compilerOutput = evmoneUtil.compileContract();
		if (!compilerOutput.has_value())
			return 0;

		auto r = evmoneUtil.randomFunction(_size);
		if (!r.has_value())
			return 0;

		auto deployResult = evmoneUtil.deployContract(compilerOutput->byteCode);
		if (deployResult.status_code != EVMC_SUCCESS)
			return 0;

		// Record function selectors in contract
		vector<string> functionSelectors;
		for (auto const& item: compilerOutput->methodIdentifiersInContract)
			functionSelectors.push_back(item.asString());

		// Add deployed contract to list of address literals.
		addressSelector[EVMHost::convertFromEVMC(deployResult.create_address)] = functionSelectors;

		string encodedData = abiEncoding(r.value(), addressSelector,
		                                 compilerOutput->methodIdentifiersInContract);
		auto callResult = evmoneUtil.executeContract(
			solidity::util::fromHex(encodedData),
			deployResult.create_address
		);

		if (callResult.status_code != EVMC_SUCCESS) {
			cout << "Old code gen call failed with status code: "
			     << callResult.status_code
			     << endl;
			return 0;
		}

		solidity::bytes result;
		for (size_t i = 0; i < callResult.output_size; i++)
			result.push_back(callResult.output_data[i]);

		EVMHostPrinter p(hostContext, deployResult.create_address);
		ostringstream oldCodeGen;
		oldCodeGen << p.state();

		compilerSetting.runYulOptimiser = true;
		compilerSetting.optimizeStackAllocation = true;

		// Reset call state post old code gen run
		hostContext.reset();
		addressSelector.clear();
		evmoneUtil.optSetting(compilerSetting);
		evmoneUtil.viaIR(true);
		if (!libraryName.empty())
		{
			auto l = evmoneUtil.compileAndDeployLibrary();
			solAssert(l.has_value(), "Deploying library failed for the second time");
		}

		for (auto const& account: hostContext.accounts)
			addressSelector[EVMHost::convertFromEVMC(account.first)] = {};

		auto compilerOutputOpt = evmoneUtil.compileContract();
		solAssert(compilerOutputOpt.has_value(), "Contract could not be optimised.");

		auto deployResultOpt = evmoneUtil.deployContract(compilerOutputOpt->byteCode);
		solAssert(deployResultOpt.status_code == EVMC_SUCCESS,
		          "Contract compiled via new code gen could not be deployed.");

		// Add address literal of contract compiled via Yul IR.
		addressSelector[EVMHost::convertFromEVMC(deployResultOpt.create_address)] = functionSelectors;

		string encodedDataIR = abiEncoding(r.value(), addressSelector,
		                                   compilerOutput->methodIdentifiersInContract);

		auto callResultOpt = evmoneUtil.executeContract(
			solidity::util::fromHex(encodedDataIR),
			deployResultOpt.create_address
		);
		solAssert(callResultOpt.status_code == EVMC_SUCCESS, "New code gen contract call failed.");

		solidity::bytes resultOpt;
		for (size_t i = 0; i < callResultOpt.output_size; i++)
			resultOpt.push_back(callResultOpt.output_data[i]);

		if (result != resultOpt)
		{
			cout << solidity::util::toHex(result) << endl;
			cout << solidity::util::toHex(resultOpt) << endl;
		}
		solAssert(result == resultOpt, "Old and new code gen call results do not match.");

		EVMHostPrinter pOpt(hostContext, deployResultOpt.create_address);
		ostringstream newCodeGen;
		newCodeGen << pOpt.state();

		if (oldCodeGen.str() != newCodeGen.str())
		{
			cout << oldCodeGen.str() << endl;
			cout << newCodeGen.str() << endl;
		}
		solAssert(oldCodeGen.str() == newCodeGen.str(), "Old and new code gen state do not match.");
		return 0;
	}
	catch (runtime_error const&)
	{
		cout << "Runtime error!" << endl;
		return 0;
	}
	catch (solidity::langutil::UnimplementedFeatureError const&)
	{
		cout << "Unimplemented feature!" << endl;
		return 0;
	}
	catch (solidity::langutil::CompilerError const& _e)
	{
		cout << "Compiler error!" << endl;
		cout << _e.what() << endl;
		return 0;
	}
	catch (solidity::yul::StackTooDeepError const&)
	{
		cout << "Stack too deep" << endl;
		return 0;
	}
}
