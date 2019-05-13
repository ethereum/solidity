#pragma once

#include "vmenv.h"
#include "presets.h"

class Reader
{
private:
	const bool noPrestate;
	const uint8_t* data;
	size_t size;
	size_t blockNum;
	const size_t minCodeSize;
	preset_function_t* presetFunction;

	Prestate* extractPrestate(const bool onlyExecAccount);

	bool extractInput(std::vector<uint8_t>& input);

public:
	Reader(const uint8_t* data, size_t size, const size_t blockNum, const size_t minCodeSize,
	       preset_function_t* presetFunction = nullptr, bool noPrestate = false);

	VMEnv* Read(void);
};