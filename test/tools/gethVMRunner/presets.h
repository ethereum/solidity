#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

size_t preset_add(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_call(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_returndatacopy(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_varpush(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_mstore8(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_mulmod(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_addmod(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_sdiv(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_exp(const uint8_t* data, size_t size, uint8_t** output_ptr);

size_t preset_signextend(const uint8_t* data, size_t size, uint8_t** output_ptr);

typedef size_t (preset_function_t)(const uint8_t* data, size_t size, uint8_t** output_ptr);

struct preset_function_desc_t
{
	preset_function_t* fn;
	const char* name;
};

std::vector<std::string> get_preset_names(void);

preset_function_t* find_preset(const char* name);