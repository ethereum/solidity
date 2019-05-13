#include "fuzzer.h"
#include "prestate.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern Prestate* g_prestate;
extern vm_trace_t g_rust_vm_trace;
extern stack_t g_rust_stack;
extern gas_t g_rust_gas_trace;
extern msize_t g_rust_msize_trace;

/* Called by Rust when it is ready to report a tuple (address, opcode)
 * when iterating over the backtrace.
 * It will keep calling this function until each stack item has been
 * reported, after which it will call once more with 'last' set to 1
 */
extern "C" void rust_report_trace(size_t address, size_t opcode, int last)
{
	if (last == 0) {
		g_rust_vm_trace.push_back(address_opcode_t(address, opcode));
	}
}

/* Called by Rust when it is ready to report the stack state.
 * It will keep calling this function until each stack item has been
 * reported, after which it will call once more with 'last' set to 1
 */
extern "C" void rust_report_stack(const uint8_t* stackitem, int last)
{
	if (last == 0) {
		uint8_t stackitem_fixed[32];
		size_t i, j;
		memset(stackitem_fixed, 0, 32);

		/* Find the first non-zero byte */
		for (i = 0; i < 32 && stackitem[i] == 0x00; i++);

		/* Copy everything from the first non-zero byte
		 * until the end to the beginning of stackitem_fixed */
		for (j = 0; i < 32; i++, j++) {
			stackitem_fixed[j] = stackitem[i];
		}

		/* The remainder of stackitem_fixed is already set to 0
		 * because the buffer was memset() earlier */

		/* Convert to vector (stack_item_t) */
		stack_item_t item(stackitem_fixed, stackitem_fixed + 32);

		g_rust_stack.push_back(item);
	} else if (last == 2) {
		/* If last == 2, this is a request from the Rust code that g_rust_stack
		 * must be cleared, because a more recent stack is pending to be
		 * communicated */
		g_rust_stack.clear();
	}
}

extern "C" void rust_report_gas(size_t gas, int last)
{
	if (last == 0) {
		g_rust_gas_trace.push_back(gas);
	}
}

extern "C" void rust_report_msize(size_t msize, int last)
{
	if (last == 0) {
		g_rust_msize_trace.push_back(msize);
	}
}

extern "C" void
rust_get_prestate(uint8_t** address, size_t* balance, uint8_t** code, size_t* code_size, size_t idx, size_t* end)
{
	if (g_prestate->Size() == 0 || idx > g_prestate->Size() - 1) {
		*end = 1;
		return;
	}

	auto curAccount = g_prestate->GetAccountByIdx(idx);

	*address = curAccount->GetAddressPtr();
	*balance = curAccount->GetBalance();
	*code = curAccount->GetCodePtr();
	*code_size = curAccount->GetCodeSize();

	*end = 0;

	return;
}
