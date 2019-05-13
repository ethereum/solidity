#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <string>

enum
{
	GO_INSTRUMENT_CODE = 0,
	GO_INSTRUMENT_STACK = 1,
	GO_INSTRUMENT_HEAP = 2,
};

typedef std::pair<uint64_t, uint64_t> address_opcode_t;
typedef std::vector<address_opcode_t> vm_trace_t;

class TraceResult
{
public:
	static std::string Typename(void)
	{ return "Instruction trace"; }

	vm_trace_t result;
};

typedef std::vector<uint8_t> stack_item_t;
typedef std::vector<stack_item_t> stack_t;

class StackResult
{
public:
	static std::string Typename(void)
	{ return "Stack trace"; }

	stack_t result;
};

typedef uint64_t gas_item_t;
typedef std::vector<gas_item_t> gas_t;

class GasResult
{
public:
	static std::string Typename(void)
	{ return "Gas trace"; }

	gas_t result;
};

typedef uint64_t msize_item_t;
typedef std::vector<msize_item_t> msize_t;

class MSizeResult
{
public:
	static std::string Typename(void)
	{ return "Memory size trace"; }

	msize_t result;
};

class SuccessResult
{
public:
	static std::string Typename(void)
	{ return "Success"; }

	bool result;
};

extern vm_trace_t g_rust_vm_trace;
extern stack_t g_rust_stack;
extern gas_t g_rust_gas_trace;
extern msize_t g_rust_msize_trace;