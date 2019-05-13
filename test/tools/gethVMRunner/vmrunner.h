#pragma once

#include "fuzzer.h"
#include "go-boilerplate.h"
#include "vmenv.h"
#include <string>

class VMExternalFunctions
{
public:
};

typedef void (* GoResetCoverage_type)(void);

typedef void (* MergeMode_type)(void);

typedef void (* getTrace_type)(GoInt*, GoUint64*, GoUint64*);

typedef void (* getGasTrace_type)(GoInt*, GoUint64*);

typedef void (* getMSizeTrace_type)(GoInt*, GoUint64*);

typedef void (* getStack_type)(GoInt*, GoSlice);

typedef void (* addAccount_type)(GoSlice, GoUint64, GoSlice);

typedef void (* runVM_type)(GoSlice, GoSlice, GoSlice, GoInt*, GoInt, GoUint64, GoUint64, GoUint64, GoUint64, GoUint64,
                            GoUint64, GoInt64);

class GethExternalFunctions : public VMExternalFunctions
{
private:
	GoResetCoverage_type GoResetCoverage;
	MergeMode_type MergeMode;
	getTrace_type getTrace;
	getGasTrace_type getGasTrace;
	getMSizeTrace_type getMSizeTrace;
	getStack_type getStack;
	addAccount_type addAccount;
	runVM_type runVM;
public:
	void Set_GoResetCoverage(GoResetCoverage_type _GoResetCoverage)
	{ this->GoResetCoverage = _GoResetCoverage; }

	void Set_MergeMode(MergeMode_type _MergeMode)
	{ this->MergeMode = _MergeMode; }

	void Set_getTrace(getTrace_type _getTrace)
	{ this->getTrace = _getTrace; }

	void Set_getGasTrace(getGasTrace_type _getGasTrace)
	{ this->getGasTrace = _getGasTrace; }

	void Set_getMSizeTrace(getMSizeTrace_type _getMSizeTrace)
	{ this->getMSizeTrace = _getMSizeTrace; }

	void Set_getStack(getStack_type _getStack)
	{ this->getStack = _getStack; }

	void Set_addAccount(addAccount_type _addAccount)
	{ this->addAccount = _addAccount; }

	void Set_runVM(runVM_type _runVM)
	{ this->runVM = _runVM; }

	GoResetCoverage_type Get_GoResetCoverage(void) const
	{ return this->GoResetCoverage; }

	MergeMode_type Get_MergeMode(void) const
	{ return this->MergeMode; }

	getTrace_type Get_getTrace(void) const
	{ return this->getTrace; }

	getGasTrace_type Get_getGasTrace(void) const
	{ return this->getGasTrace; }

	getMSizeTrace_type Get_getMSizeTrace(void) const
	{ return this->getMSizeTrace; }

	getStack_type Get_getStack(void) const
	{ return this->getStack; }

	addAccount_type Get_addAccount(void) const
	{ return this->addAccount; }

	runVM_type Get_runVM(void) const
	{ return this->runVM; }
};

template<typename T>
class CachedTrace
{
private:
	T trace;
public:
	void Get(T& _trace, const bool move)
	{
		if (move == false) {
			_trace = trace;
		} else {
			_trace = std::move(trace);
			is_cached = false;
		}
	}

	void Set(T& _trace, const bool move = true)
	{
		if (move == false) {
			trace = _trace;
		} else {
			trace = std::move(_trace);
		}
		is_cached = true;
	}

	bool is_cached;
};


class VMRunner
{
protected:
	const VMEnv* vmEnv;
	bool success;

	CachedTrace<vm_trace_t> cached_vm_trace;
	CachedTrace<stack_t> cached_stack_trace;
	CachedTrace<gas_t> cached_gas_trace;
	CachedTrace<msize_t> cached_msize_trace;

	void resetCache(void)
	{
		this->cached_vm_trace.is_cached = false;
		this->cached_stack_trace.is_cached = false;
		this->cached_gas_trace.is_cached = false;
		this->cached_msize_trace.is_cached = false;
	}

public:
	VMRunner(const VMEnv* vmEnv, const VMExternalFunctions& externalFunctions) : vmEnv(vmEnv),
	                                                                             ExternalFunctions(externalFunctions)
	{
		resetCache();
	}

	virtual ~VMRunner()
	{}

	bool Success(void)
	{
		return success;
	}

	void GetResult(SuccessResult& trace)
	{
		trace.result = Success();
	}

	virtual std::string GetName(void) const = 0;

	virtual void GetResult(TraceResult& trace, const bool move = true) = 0;

	virtual void GetResult(StackResult& trace, const bool move = true) = 0;

	virtual void GetResult(GasResult& trace, const bool move = true) = 0;

	virtual void GetResult(MSizeResult& trace, const bool move = true) = 0;

	virtual void Run(void) = 0;

	const VMExternalFunctions& ExternalFunctions;
};

class GethVMRunner : public VMRunner
{
private:
	getTrace_type getTrace;
	getGasTrace_type getGasTrace;
	getMSizeTrace_type getMSizeTrace;
	getStack_type getStack;
	addAccount_type addAccount;
	runVM_type runVM;

public:
	GethVMRunner(const VMEnv* vmEnv, const GethExternalFunctions& externalFunctions) : VMRunner(vmEnv,
	                                                                                            externalFunctions)
	{
		this->getTrace = externalFunctions.Get_getTrace();
		this->getGasTrace = externalFunctions.Get_getGasTrace();
		this->getMSizeTrace = externalFunctions.Get_getMSizeTrace();
		this->getStack = externalFunctions.Get_getStack();
		this->addAccount = externalFunctions.Get_addAccount();
		this->runVM = externalFunctions.Get_runVM();
	}

	std::string GetName(void) const override;

	void GetResult(TraceResult& trace, const bool move = true) override;

	void GetResult(StackResult& trace, const bool move = true) override;

	void GetResult(GasResult& trace, const bool move = true) override;

	void GetResult(MSizeResult& trace, const bool move = true) override;

	void Run(void) override;
};