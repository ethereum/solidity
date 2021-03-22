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

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
//	if (_size <= 600)
	{
		string input(reinterpret_cast<char const*>(_data), _size);
		// TODO: Cannot fuzz tests containing libraries yet.
		if (input.find("library") != string::npos)
			return 0;

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
				{},
				methodName
			);

			auto compilerOutput = evmoneUtil.compileContract();
			if (!compilerOutput.has_value())
				return 0;

			auto r = evmoneUtil.randomFunction();
			if (!r.has_value())
				return 0;

			auto x = ValueGenerator{r.value()["inputs"], 0}.type();
			bool encodeStatus;
			string encodedData;
			bool functionWithInputs = x.first != "()";
			auto sig = r.value()["name"].asString() + x.first;
			cout << sig << endl;
			if (functionWithInputs)
			{
				abicoder::ABICoder coder(abiCoderHeapSize);
				auto s = coder.encode(x.first, x.second);
				encodeStatus = s.first;
				encodedData = s.second;
				solAssert(encodeStatus, "Isabelle coder: failed.");
			}

			auto deployResult = evmoneUtil.deployContract(compilerOutput->byteCode);
			if (deployResult.status_code != EVMC_SUCCESS)
				return 0;

			auto methodSig = compilerOutput->methodIdentifiersInContract[sig].asString();
			if (functionWithInputs)
				methodSig += encodedData.substr(2, encodedData.size());
			auto callResult = evmoneUtil.executeContract(
				solidity::util::fromHex(methodSig),
				deployResult.create_address
			);

			if (callResult.status_code != EVMC_SUCCESS)
			{
				cout << "Old code gen call failed with status code: "
					<< callResult.status_code
					<< endl;
				return 0;
			}

			solidity::bytes result;
			for (size_t i = 0; i < callResult.output_size; i++)
				result.push_back(callResult.output_data[i]);

			cout << solidity::util::toHex(result) << endl;

			EVMHostPrinter p(hostContext, deployResult.create_address);
			ostringstream oldCodeGen;
			oldCodeGen << p.state();

			compilerSetting.runYulOptimiser = true;
			compilerSetting.optimizeStackAllocation = true;
			hostContext.reset();
			evmoneUtil.reset(true);
			evmoneUtil.optSetting(compilerSetting);
			evmoneUtil.viaIR(true);
			auto compilerOutputOpt = evmoneUtil.compileContract();
			solAssert(compilerOutputOpt.has_value(), "Contract could not be optimised.");

			auto deployResultOpt = evmoneUtil.deployContract(compilerOutputOpt->byteCode);
			solAssert(deployResultOpt.status_code == EVMC_SUCCESS, "Contract compiled via new code gen could not be deployed.");

			auto callResultOpt = evmoneUtil.executeContract(
				solidity::util::fromHex(methodSig),
				deployResultOpt.create_address
			);
			solAssert(callResultOpt.status_code == EVMC_SUCCESS, "New code gen contract call failed.");

			solidity::bytes resultOpt;
			for (size_t i = 0; i < callResultOpt.output_size; i++)
				resultOpt.push_back(callResultOpt.output_data[i]);

			cout << solidity::util::toHex(resultOpt) << endl;
			solAssert(result == resultOpt, "Old and new code gen call results do not match.");

			EVMHostPrinter pOpt(hostContext, deployResultOpt.create_address);
			ostringstream newCodeGen;
			newCodeGen << pOpt.state();

			cout << oldCodeGen.str() << endl;
			cout << newCodeGen.str() << endl;

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
}
