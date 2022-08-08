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

#include <fstream>

using namespace solidity::frontend;
using namespace solidity::test::fuzzer;
using namespace solidity::test::abiv2fuzzer;
using namespace solidity::test;
using namespace solidity::util;
using namespace solidity;
using namespace std;

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

DEFINE_PROTO_FUZZER(Contract const& _input)
{
	string contract_source = ProtoConverter{_input.seed()}.contractToString(_input);

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		// With libFuzzer binary run this to generate the solidity source file x.sol from a proto input:
		// PROTO_FUZZER_DUMP_PATH=x.sol ./a.out proto-input
		ofstream of(dump_path);
		of << contract_source;
	}

	// We target the default EVM which is the latest
	langutil::EVMVersion version;
	EVMHost hostContext(version, evmone);
	string contractName = "C";
	string methodName = "test()";
	StringMap source({{"test.sol", contract_source}});
	CompilerInput cInput(version, source, contractName, OptimiserSettings::minimal(), {});
	EvmoneUtility evmoneUtil(
		hostContext,
		cInput,
		contractName,
		{},
		methodName
	);
	// Invoke test function
	auto result = evmoneUtil.compileDeployAndExecute();
	// We don't care about EVM One failures other than EVMC_REVERT
	solAssert(result.status_code != EVMC_REVERT, "Proto ABIv2 fuzzer: EVM One reverted");
	if (result.status_code == EVMC_SUCCESS)
		if (!EvmoneUtility::zeroWord(result.output_data, result.output_size))
		{
			solidity::bytes res;
			for (size_t i = 0; i < result.output_size; i++)
				res.push_back(result.output_data[i]);
			cout << solidity::util::toHex(res) << endl;
			solAssert(
				false,
				"Proto ABIv2 fuzzer: ABIv2 coding failure found"
			);
		}
}
