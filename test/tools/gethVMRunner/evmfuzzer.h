#pragma once

#include <functional>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include "vmrunner.h"
#include "options.h"
#include "vmenv.h"
#include "externalfunctions.h"

class EVMFuzzer
{
private:
	static bool initialized_once;
	VMEnv* vmEnv;
	const uint8_t* data;
	size_t datasize;
	GethVMRunner* geth;
	std::vector<VMRunner*> VMs;

	bool createVMEnv(void);

	void prepareVMs(void);

	bool processInput(void);

	void preprocess(void) const;

	void runVMs(void) const;

	void postprocess(void);

	void compare(void) const;

	int returnValue(void);

	void crashOnMismatch(void) const;

	template<typename T>
	void genericCompare(std::function<void(const std::vector<std::string>& names, const std::vector<T>& results,
	                                       const size_t i)> onMismatch = nullptr) const;

	const Options& global_options;
	static ExternalFunctions* EF;

	static bool use_parity, use_geth, use_cpp;

	GethExternalFunctions ef_geth;

	static size_t numVMs;
	static bool useSymcovWriter;
public:
	static size_t unique_opcodes_max;

	EVMFuzzer(VMEnv* vmEnv, const Options& global_options);

	~EVMFuzzer();

	bool Initialize(const bool prepareVMs = true);

	int Run(void);

	static void EnableSymcovWriter(void);

	static void WriteSymcov(const std::string symcovPath);
};