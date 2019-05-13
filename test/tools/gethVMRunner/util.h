#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "fuzzer.h"
#include "vmrunner.h"
#include "vmenv.h"

void print_stack(std::string name1, const stack_t* stack1, std::string name2, const stack_t* stack2);

void print_gas(std::string name1, const gas_t* gas1, std::string name2, const gas_t* gas2);

void print_msize(std::string name1, const msize_t* msize1, std::string name2, const msize_t* msize2);

void print_code(const uint8_t* data, size_t size);

bool validate_code(const uint8_t* data, size_t size);

void print_trace_header(const VMEnv* vmEnv);

std::string dataToHexString(std::vector<uint8_t> in);

std::vector<uint8_t> hexToData(std::string in);

size_t sizetFromDecString(const std::string& in);

std::string stringFromUint64(const uint64_t in);

uint64_t uint64FromHexString(std::string in);

std::string hexStringFromUint64(uint64_t in);
