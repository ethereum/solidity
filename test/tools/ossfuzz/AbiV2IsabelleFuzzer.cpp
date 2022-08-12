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

#include <test/tools/ossfuzz/protoToAbiV2.h>

#include <src/libfuzzer/libfuzzer_macro.h>
#include <abicoder.hpp>

using namespace solidity::frontend;
using namespace solidity::test::fuzzer;
using namespace solidity::test::abiv2fuzzer;
using namespace solidity::test;
using namespace solidity::util;
using namespace solidity;
using namespace std;

static constexpr size_t abiCoderHeapSize = 1024 * 512;
static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

DEFINE_PROTO_FUZZER(Contract const& _contract)
{
	ProtoConverter converter(_contract.seed());
	string contractSource = converter.contractToString(_contract);

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		// With libFuzzer binary run this to generate the solidity source file x.sol from a proto input:
		// PROTO_FUZZER_DUMP_PATH=x.sol ./a.out proto-input
		ofstream of(dump_path);
		of << contractSource;
	}

	string typeString = converter.isabelleTypeString();
	string valueString = converter.isabelleValueString();
	abicoder::ABICoder coder(abiCoderHeapSize);
	if (!typeString.empty() && converter.coderFunction())
	{
		auto [encodeStatus, encodedData] = coder.encode(typeString, valueString);
		solAssert(encodeStatus, "Isabelle abicoder fuzzer: Encoding failed");

		// We target the default EVM which is the latest
		langutil::EVMVersion version;
		EVMHost hostContext(version, evmone);
		string contractName = "C";
		StringMap source({{"test.sol", contractSource}});
		CompilerInput cInput(version, source, contractName, OptimiserSettings::minimal(), {});
		EvmoneUtility evmoneUtil(
			hostContext,
			cInput,
			contractName,
			{},
			{}
		);
		auto result = evmoneUtil.compileDeployAndExecute(encodedData);
		solAssert(result.status_code != EVMC_REVERT, "Proto ABIv2 fuzzer: EVM One reverted.");
		if (result.status_code == EVMC_SUCCESS)
			solAssert(
				EvmoneUtility::zeroWord(result.output_data, result.output_size),
				"Proto ABIv2 fuzzer: ABIv2 coding failure found."
			);
	}
}
