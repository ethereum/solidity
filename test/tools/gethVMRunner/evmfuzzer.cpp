#include <functional>
#include <iostream>
#include <fstream>
#include <tuple>
#include <set>
#include <stdexcept>
#include "reader.h"
#include "evmfuzzer.h"
#include "util.h"
#include "externalfunctions.h"

extern Prestate* g_prestate;
extern Options global_options;

/* Static class variable definition */
size_t EVMFuzzer::unique_opcodes_max = 0;
bool EVMFuzzer::initialized_once = false;
bool EVMFuzzer::use_geth = false;
size_t EVMFuzzer::numVMs = 0;
bool EVMFuzzer::useSymcovWriter = false;
ExternalFunctions* EVMFuzzer::EF = NULL;

EVMFuzzer::EVMFuzzer(VMEnv* vmEnv, const Options& global_options) :
		global_options(global_options)
{
	this->vmEnv = vmEnv;
	geth = NULL;
	vmEnv = NULL;

	if (EF == NULL) {
		EF = new ExternalFunctions();
	}

	if (initialized_once == false) {

		use_geth = global_options.get_no_geth() == false &&
		           EF->getGasTrace != NULL &&
		           EF->getMSizeTrace != NULL &&
		           EF->getStack != NULL &&
		           EF->addAccount != NULL &&
		           EF->runVM != NULL &&
		           EF->DisableTracer != NULL;

		numVMs = 0;
		if (EVMFuzzer::use_geth == true) numVMs++;

		printf("\nThe following clients are enabled:\n");
		if (EVMFuzzer::use_geth == true) printf("\tGeth\n");
		printf("\n");

		initialized_once = true;
	}

	if (use_geth && global_options.get_merge()) {
		EF->MergeMode();
	}
}

EVMFuzzer::~EVMFuzzer()
{
	delete geth;
	delete vmEnv;
}

void EVMFuzzer::prepareVMs(void)
{
	if (useSymcovWriter == true)
		EF->EnableSymcovWriter();


	/* Instantiate Geth runner */
	/* Set external functions */
	ef_geth.Set_getTrace(EF->getTrace);
	ef_geth.Set_getGasTrace(EF->getGasTrace);
	ef_geth.Set_getMSizeTrace(EF->getMSizeTrace);
	ef_geth.Set_getStack(EF->getStack);
	ef_geth.Set_addAccount(EF->addAccount);
	ef_geth.Set_runVM(EF->runVM);

	geth = new GethVMRunner(vmEnv, ef_geth);
	VMs.push_back(dynamic_cast<VMRunner*>(geth));
}

bool EVMFuzzer::processInput(void)
{
	vmEnv->SetPrintTrace(global_options.get_trace());

	vmEnv->SetGas(std::min(vmEnv->GetGas(), global_options.get_max_gas()));

	vmEnv->SetReportTrace(numVMs > 1 ? true : false); /* TODO and only if comparing is enabled */

	if (global_options.get_gas())
		vmEnv->SetGas(global_options.get_gas());

	if (global_options.get_balance())
		vmEnv->SetGas(global_options.get_balance());

	if (global_options.get_validate_code()) {
		auto execAccount = vmEnv->GetPrestate()->GetExecAccount();
		if (validate_code(execAccount->GetCodePtr(), execAccount->GetCodeSize()) == false)
			return false;
	}

	return true;
}

bool EVMFuzzer::Initialize(const bool prepareVMs)
{
	if (processInput() == false)
		return false;

	if (prepareVMs == true)
		this->prepareVMs();

	return true;
}

void EVMFuzzer::preprocess(void) const
{
	if (global_options.get_trace())
		print_trace_header(vmEnv);

	if (global_options.get_trace() == false && global_options.get_cmp() == false)
		/* If there is no need for tracing, disable it in Geth
		 * for more speed */
		EF->DisableTracer();
}

void EVMFuzzer::runVMs(void) const
{
	for (const auto& VM : VMs)
		VM->Run();
}

void EVMFuzzer::postprocess(void)
{
	if (global_options.get_diversity_guided()) {
		/* Count the number of unique opcodes
		 * (not in the input code, but in the executed trace */
		std::set<uint64_t> unique_opcodes;
		TraceResult trace;

		geth->GetResult(trace);

		for (auto tr : trace.result)
			unique_opcodes.insert(tr.second);

		std::vector<uint64_t> unique_opcodes_vec(unique_opcodes.begin(), unique_opcodes.end());
		if (unique_opcodes_vec.size() > unique_opcodes_max)
			unique_opcodes_max = unique_opcodes_vec.size();
	}
}

void EVMFuzzer::crashOnMismatch(void) const
{
	printf("For a full trace, run this input with --trace\n");
	abort();
}

template<typename T>
void EVMFuzzer::genericCompare(std::function<void(const std::vector<std::string>& names, const std::vector<T>& results,
                                                  const size_t i)> onMismatch) const
{
	std::vector<std::string> names;
	std::vector<T> results;

	for (const auto& VM : VMs) {
		T result;
		VM->GetResult(result);
		results.push_back(std::move(result));
	}

	for (size_t i = 1; i < results.size(); i++) {
		if (results[i - 1].result != results[i].result) {
			for (const auto& VM : VMs)
				names.push_back(VM->GetName());

			printf("%s mismatch between %s and %s\n\n", T::Typename().c_str(), names[i - 1].c_str(), names[i].c_str());
			if (onMismatch != nullptr)
				onMismatch(names, results, i);
			this->crashOnMismatch();
		}
	}
}

void EVMFuzzer::compare(void) const
{
	/* Refrain from doing any comparisons if only 1 VM is defined */
	if (global_options.get_num_vms() >= 2) {
		if (global_options.get_cmp_trace())
			genericCompare<TraceResult>();

		if (global_options.get_cmp_stack())
			genericCompare<StackResult>(
					[](const std::vector<std::string>& names, const std::vector<StackResult>& results, const size_t i) {
						/* Print both stacks */
						print_stack(names[i - 1], &results[i - 1].result, names[i], &results[i].result);
					}
			);

		if (global_options.get_cmp_gas())
			genericCompare<GasResult>(
					[](const std::vector<std::string>& names, const std::vector<GasResult>& results, const size_t i) {
						/* Print both gas traces */
						print_gas(names[i - 1], &results[i - 1].result, names[i], &results[i].result);
					}
			);

		if (global_options.get_cmp_msize())
			genericCompare<MSizeResult>(
					[](const std::vector<std::string>& names, const std::vector<MSizeResult>& results, const size_t i) {
						/* Print both msize traces */
						print_msize(names[i - 1], &results[i - 1].result, names[i], &results[i].result);
					}
			);
	}
}

int EVMFuzzer::returnValue()
{
	return 0;
}

int EVMFuzzer::Run(void)
{
	this->preprocess();

	this->runVMs();

	this->postprocess();

	this->compare();

	return this->returnValue();
}