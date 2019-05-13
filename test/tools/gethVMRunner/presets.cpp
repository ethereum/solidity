#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "presets.h"

preset_function_desc_t preset_descriptors[] = {
		{preset_add,            "add"},
		{preset_call,           "call"},
		{preset_returndatacopy, "returndatacopy"},
		{preset_varpush,        "varpush"},
		{preset_mstore8,        "mstore8"},
		{preset_mulmod,         "mulmod"},
		{preset_addmod,         "addmod"},
		{preset_sdiv,           "sdiv"},
		{preset_exp,            "exp"},
		{preset_signextend,     "signextend"},
		{NULL, NULL},
};

std::vector<std::string> get_preset_names(void)
{
	std::vector<std::string> ret;
	preset_function_desc_t* desc = preset_descriptors;

	while (desc->name) {
		ret.push_back(std::string(desc->name));
		desc++;
	}

	return ret;
}

preset_function_t* find_preset(const char* name)
{
	preset_function_desc_t* desc = preset_descriptors;

	while (desc->name) {
		if (strcmp(name, desc->name) == 0) {
			return desc->fn;
		}
		desc++;
	}
	return NULL;
}

size_t preset_add(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
	if (size < (2 * 32)) { return 0; }
	const size_t outsize = (1 + 32) * 2 /* 2 PUSHes*/
	                       + 1 /* ADD */;
	*output_ptr = (uint8_t*) malloc(outsize);
	uint8_t* output = *output_ptr;

	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x01;
	output += 1; /* ADD */

	return outsize;
}

size_t preset_call(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
	if (size < (8 * 2)) { return 0; }
	const size_t outsize = (1 + 2) * 8 /* 8 PUSHes */
	                       + 1 /* CALL */
	                       + 1 /* POP */
	                       + 1 /* RETURNDATASIZE */
	                       + 1 /* MLOAD */;
	*output_ptr = (uint8_t*) malloc(outsize);
	uint8_t* output = *output_ptr;

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0xF1;
	output += 1; /* CALL */

	output[0] = 0x50;
	output += 1; /* POP */

	output[0] = 0x3D;
	output += 1; /* RETURNDATASIZE */

	output[0] = 0x61;
	output += 1; /* PUSH2 */
	memcpy(output, data, 2);
	output += 2;
	data += 2;
	size -= 2; /* 2 bytes */

	output[0] = 0x51;
	output += 1; /* MLOAD */


	return outsize;
}

size_t preset_returndatacopy(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
	if (size < (3 * 32)) { return 0; }
	const size_t outsize = (1 + 32) * 3 /* 3 PUSHes*/
	                       + 1 /* RETURNDATACOPY */
	                       + 1 /* STOP */;
	*output_ptr = (uint8_t*) malloc(outsize);
	uint8_t* output = *output_ptr;

	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x3E;
	output += 1; /* RETURNDATACOPY */
	output[0] = 0x00;
	output += 1; /* STOP */

	return outsize;
}

size_t preset_varpush(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
	uint8_t num_pushes;

	/* Get num_pushes */
	if (size < 1) { return 0; }
	num_pushes = data[0];
	data += 1;
	size -= 1;
	if (num_pushes == 0 || num_pushes > 10) { return 0; }

	if (size < (num_pushes * 1) + 1) { return 0; }
	const size_t outsize = (1 + 1) * num_pushes /* n PUSHes*/
	                       + 1 /* variable opcode */
	                       + 1; /* STOP */
	*output_ptr = (uint8_t*) malloc(outsize);
	uint8_t* output = *output_ptr;

	while (num_pushes) {
		output[0] = 0x60;
		output += 1; /* PUSH1 */
		memcpy(output, data, 1);
		output += 1;
		data += 1;
		size -= 1; /* 1 byte */
		num_pushes--;
		if (size == 0) {
			abort();
		}
	}
	output[0] = data[0];
	data += 1;
	size -= 1;
	output += 1; /* random opcode */
	output[0] = 0x00;
	output += 1; /* STOP */

	return outsize;
}

size_t preset_mstore8(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
	if (size < (10 * 32) + 2) { return 0; }
	const size_t outsize = (1 + 32) * 10 /* 2 PUSHes*/
	                       + 1 /* MSTORE8 */
	                       + 1 + 1 /* STOP */;
	*output_ptr = (uint8_t*) malloc(outsize);
	uint8_t* output = *output_ptr;

	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = 0x7F;
	output += 1; /* PUSH32 */
	memcpy(output, data, 32);
	output += 32;
	data += 32;
	size -= 32; /* 32 bytes */
	output[0] = data[0];
	data += 1;
	size -= 1;
	output += 1; /* MSTORE8 */
	output[0] = data[0];
	data += 1;
	size -= 1;
	output += 1; /* MSTORE8 */
	output[0] = 0x00;
	output += 1; /* STOP */

	return outsize;
}

size_t preset_mulmod(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
#define MULMOD_CODE_SIZE (3+(3*32)+1+1)
	if (size < MULMOD_CODE_SIZE) {
		return 0;
	}
	*output_ptr = (uint8_t*) malloc(MULMOD_CODE_SIZE);
	memcpy(*output_ptr, data, MULMOD_CODE_SIZE);
	*(*output_ptr + 0 * 33) = 0x7F;
	*(*output_ptr + 1 * 33) = 0x7F;
	*(*output_ptr + 2 * 33) = 0x7F;
	*(*output_ptr + 3 * 33 + 0) = 0x09;
	*(*output_ptr + 3 * 33 + 1) = 0x00;
	return MULMOD_CODE_SIZE;
#undef MULMOD_CODE_SIZE
}

size_t preset_addmod(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
#define ADDMOD_CODE_SIZE (3+(3*32)+1+1)
	if (size < ADDMOD_CODE_SIZE) {
		return 0;
	}
	*output_ptr = (uint8_t*) malloc(ADDMOD_CODE_SIZE);
	memcpy(*output_ptr, data, ADDMOD_CODE_SIZE);
	*(*output_ptr + 0 * 33) = 0x7F;
	*(*output_ptr + 1 * 33) = 0x7F;
	*(*output_ptr + 2 * 33) = 0x7F;
	*(*output_ptr + 3 * 33 + 0) = 0x08;
	*(*output_ptr + 3 * 33 + 1) = 0x00;
	return ADDMOD_CODE_SIZE;
#undef ADDMOD_CODE_SIZE
}

size_t preset_sdiv(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
#define SDIV_CODE_SIZE (2+(2*32)+1+1)
	if (size < SDIV_CODE_SIZE) {
		return 0;
	}
	*output_ptr = (uint8_t*) malloc(SDIV_CODE_SIZE);
	memcpy(*output_ptr, data, SDIV_CODE_SIZE);
	*(*output_ptr + 0 * 33) = 0x7F;
	*(*output_ptr + 1 * 33) = 0x7F;
	*(*output_ptr + 2 * 33 + 0) = 0x05;
	*(*output_ptr + 2 * 33 + 1) = 0x00;
	return SDIV_CODE_SIZE;
#undef SDIV_CODE_SIZE
}

size_t preset_exp(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
#define EXP_CODE_SIZE (2+(2*32)+1+1)
	if (size < EXP_CODE_SIZE) {
		return 0;
	}
	*output_ptr = (uint8_t*) malloc(EXP_CODE_SIZE);
	memcpy(*output_ptr, data, EXP_CODE_SIZE);
	*(*output_ptr + 0 * 33) = 0x7F;
	*(*output_ptr + 1 * 33) = 0x7F;
	*(*output_ptr + 2 * 33 + 0) = 0x0A;
	*(*output_ptr + 2 * 33 + 1) = 0x00;
	return EXP_CODE_SIZE;
#undef EXP_CODE_SIZE
}

size_t preset_signextend(const uint8_t* data, size_t size, uint8_t** output_ptr)
{
#define SIGNEXTEND_CODE_SIZE (2+(2*32)+1+1)
	if (size < SIGNEXTEND_CODE_SIZE) {
		return 0;
	}
	*output_ptr = (uint8_t*) malloc(SIGNEXTEND_CODE_SIZE);
	memcpy(*output_ptr, data, SIGNEXTEND_CODE_SIZE);
	*(*output_ptr + 0 * 33) = 0x7F;
	*(*output_ptr + 1 * 33) = 0x7F;
	*(*output_ptr + 2 * 33 + 0) = 0x0B;
	*(*output_ptr + 2 * 33 + 1) = 0x00;
	return SIGNEXTEND_CODE_SIZE;
#undef SIGNEXTEND_CODE_SIZE
}
