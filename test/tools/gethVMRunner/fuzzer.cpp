#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <utility>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include "fuzzer.h"
#include "presets.h"
#include "prestate.h"
#include "evmfuzzer.h"
#include "solidityutil.h"

Prestate* g_prestate;
static Options g_options;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
	g_options.Parse(*argc, *argv, get_preset_names());
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	std::string sourceCode(reinterpret_cast<char const*>(data), size);
	VMEnv* vmenv = new VMEnv();

	/* Set environment variables */
	vmenv->SetGas(g_options.get_gas());
	vmenv->SetTimestamp(0xdeadbeef);
	vmenv->SetGasLimit(0xffffffff);
	vmenv->SetDifficulty(0);
	vmenv->SetGasPrice(2);

	vmenv->SetBlockNumber(g_options.get_blocknum());
	vmenv->SetPrintTrace(g_options.get_trace());


	// Manually create a pre-state with fixed values except runtime code and calldata
	Prestate* prestate = new Prestate();

	Account* curAccount = new Account();
	curAccount->SetAddress("0x0123456789012345678901234567890123456789");
	curAccount->SetBalance(0x10000);

	// Compile Solidity contract
	solidityutil::SolidityExecutionFramework sEF;
	dev::bytes bytecode = sEF.compileContract(
			sourceCode
	);

	curAccount->SetCode(bytecode);

	// Set input
	dev::bytes input = dev::FixedHash<4>(dev::keccak256("f()")).asBytes();
	prestate->SetInput(input);

	// Set this account as executable
	if (!prestate->hasAccount(curAccount))
		prestate->Add(curAccount, true);
	else {
		delete vmenv;
		delete prestate;
		delete curAccount;
		return 0;
	}

	vmenv->SetPrestate(prestate);

	EVMFuzzer* evmfuzzer = new EVMFuzzer(vmenv, g_options);
	int ret = 0;
	if (evmfuzzer->Initialize() == true) {
		ret = evmfuzzer->Run();
	}

	delete evmfuzzer;
	return 0;
}