#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "fuzzer.h"
#include "vmrunner.h"
#include "go-boilerplate.h"

std::string GethVMRunner::GetName(void) const
{
	return std::string("Geth");
}

void GethVMRunner::GetResult(TraceResult& trace, const bool move)
{
	if (cached_vm_trace.is_cached == false) {
		vm_trace_t _trace;
		GoInt go_finished = 0;
		while (1) {
			GoUint64 address, opcode;
			getTrace(&go_finished, &address, &opcode);
			if (go_finished) {
				break;
			}
			_trace.push_back(address_opcode_t(address, opcode));
		}
		cached_vm_trace.Set(_trace);
	}

	cached_vm_trace.Get(trace.result, move);
}

void GethVMRunner::GetResult(StackResult& trace, const bool move)
{
	if (cached_stack_trace.is_cached == false) {
		stack_t stack;
		GoInt go_finished = 0;
		uint8_t stackitem[32];
		GoSlice dest = {stackitem, 32, 32};
		vm_trace_t trace;
		while (1) {
			memset(stackitem, 0, 32);

			/* Call getStack in Go; it stores the stack item in 'dest' */
			getStack(&go_finished, dest);

			/* Break if getStack signals that all items have been retrieved */
			if (go_finished) {
				break;
			}

			stack_item_t item(stackitem, stackitem + 32);
			stack.push_back(item);
		}
		cached_stack_trace.Set(stack);
	}

	cached_stack_trace.Get(trace.result, move);
}

void GethVMRunner::GetResult(GasResult& trace, const bool move)
{
	if (cached_gas_trace.is_cached == false) {
		GoInt go_finished = 0;
		gas_t _trace;
		while (1) {
			GoUint64 gas;
			getGasTrace(&go_finished, &gas);
			if (go_finished) {
				break;
			}
			_trace.push_back(gas);
		}
		cached_gas_trace.Set(_trace);
	}

	cached_gas_trace.Get(trace.result, move);
}

void GethVMRunner::GetResult(MSizeResult& trace, const bool move)
{
	if (cached_msize_trace.is_cached == false) {
		GoInt go_finished = 0;
		msize_t _trace;
		while (1) {
			GoUint64 msize;
			getMSizeTrace(&go_finished, &msize);
			if (go_finished) {
				break;
			}
			_trace.push_back(msize);
		}
		cached_msize_trace.Set(_trace);
	}
	cached_msize_trace.Get(trace.result, move);
}

void GethVMRunner::Run(void)
{
	resetCache();
	const auto prestate = vmEnv->GetPrestate();
	const auto execAccount = prestate->GetExecAccount();

	prestate->ForEach([&](Account* curAccount) {
		GoSlice addressSlice = {(void*) curAccount->GetAddressPtr(), (long long) 20, (long long) 20};
		GoUint64 balance = curAccount->GetBalance();
		GoSlice codeSlice = {(void*) curAccount->GetCodePtr(), (long long) curAccount->GetCodeSize(),
		                     (long long) curAccount->GetCodeSize()};
		addAccount(addressSlice, balance, codeSlice);
	});

	GoSlice addressSlice = {(void*) execAccount->GetAddressPtr(), (long long) 20, (long long) 20};
	GoSlice codeslice = {(void*) execAccount->GetCodePtr(), (long long) execAccount->GetCodeSize(),
	                     (long long) execAccount->GetCodeSize()};
	GoSlice inputslice = {(void*) prestate->GetInputPtr(), (long long) prestate->GetInputSize(),
	                      (long long) prestate->GetInputSize()};
	GoInt i_success = 0;

	if (vmEnv->GetPrintTrace()) {
		printf("Tracing Go VM:\n");
	}

	runVM(
			addressSlice,
			codeslice,
			inputslice,
			&i_success,
			vmEnv->GetPrintTrace() ? 1 : 0,
			vmEnv->GetGas(),
			vmEnv->GetBlockNumber(),
			vmEnv->GetTimestamp(),
			vmEnv->GetGasLimit(),
			vmEnv->GetDifficulty(),
			vmEnv->GetGasPrice(),
			execAccount->GetBalance());

	success = i_success ? true : false;

	if (vmEnv->GetPrintTrace()) {
		printf("\n\n");
	}
}