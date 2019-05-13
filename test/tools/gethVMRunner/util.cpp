#include "util.h"
#include <iostream>
#include <algorithm>
#include <set>

/* Print the stacks of two VMs in a tabulated formatting */
void print_stack(std::string name1, const stack_t* stack1, std::string name2, const stack_t* stack2)
{
	size_t longest = std::max(stack1->size(), stack2->size());

	printf("%s / %s stacks:\n", name1.c_str(), name2.c_str());

	for (size_t i = 0; i < longest; i++) {
		if (i < stack1->size()) {
			/* Print the stack item with index i */
			stack_item_t stackitem = (*stack1)[i];
			uint8_t* c = &stackitem[0];
			printf("%zu : ", i + 1);
			for (size_t j = 0; j < 32; j++) {
				printf("%02X", c[j]);
			}
		} else {
			/* If this stack index doesn't exist, print dots */
			printf("%zu : ", i + 1);
			for (size_t j = 0; j < 32; j++) {
				printf("..");
			}
		}

		printf("\t");

		/* The same procedure for the other VM */
		if (i < stack2->size()) {
			stack_item_t stackitem = (*stack2)[i];
			uint8_t* c = &stackitem[0];
			printf("%zu : ", i + 1);
			for (size_t j = 0; j < 32; j++) {
				printf("%02X", c[j]);
			}
		} else {
			printf("%zu : ", i + 1);
			for (size_t j = 0; j < 32; j++) {
				printf("..");
			}
		}

		/* To make it easier to discern which stack items differ between
		 * the two VMs, a marker is appended to the output when
		 * this is the case.
		 */
		if (i < stack1->size() && i < stack2->size()) {
			if (memcmp(&((*stack1)[i][0]), &((*stack2)[i][0]), 32)) {
				printf("\t(differ)");
			}
		}

		printf("\n");
	}
}

/* Print the gas traces of two VMs in a tabulated formatting */
void print_gas(std::string name1, const gas_t* gas1, std::string name2, const gas_t* gas2)
{
	size_t longest = std::max(gas1->size(), gas2->size());

	printf("%s / %s gas traces:\n", name1.c_str(), name2.c_str());

	for (size_t i = 0; i < longest; i++) {
		if (i < gas1->size()) {
			/* Print the gas item with index i */
			gas_item_t gasitem = (*gas1)[i];
			printf("%zu : %zu", i, gasitem);
		} else {
			printf("%zu : <none>", i);
		}

		printf("\t");

		/* The same procedure for the other VM */
		if (i < gas2->size()) {
			/* Print the gas item with index i */
			gas_item_t gasitem = (*gas2)[i];
			printf("%zu : %zu", i, gasitem);
		} else {
			printf("%zu : <none>", i);
		}

		if (i < gas1->size() && i < gas2->size()) {
			if ((*gas1)[i] != (*gas2)[i]) {
				printf("\t(differ)");
			}
		}

		printf("\n");
	}
}

/* Print the msize traces of two VMs in a tabulated formatting */
void print_msize(std::string name1, const msize_t* msize1, std::string name2, const msize_t* msize2)
{
	size_t longest = std::max(msize1->size(), msize2->size());

	printf("%s / %s msize traces:\n", name1.c_str(), name2.c_str());

	for (size_t i = 0; i < longest; i++) {
		if (i < msize1->size()) {
			/* Print the msize item with index i */
			msize_item_t msizeitem = (*msize1)[i];
			printf("%zu : %zu", i, msizeitem);
		} else {
			printf("%zu : <none>", i);
		}

		printf("\t");

		/* The same procedure for the other VM */
		if (i < msize2->size()) {
			/* Print the msize item with index i */
			msize_item_t msizeitem = (*msize2)[i];
			printf("%zu : %zu", i, msizeitem);
		} else {
			printf("%zu : <none>", i);
		}

		if (i < msize1->size() && i < msize2->size()) {
			if ((*msize1)[i] != (*msize2)[i]) {
				printf("\t(differ)");
			}
		}

		printf("\n");
	}
}

static std::string bufferToString(const std::vector<uint8_t> data, const int tabs)
{
	std::stringstream ss;

	size_t i;

	for (i = 0; i < data.size(); i++) {
		if (i % 16 == 0) {
			if (i) {
				ss << "\n";
			}
			for (int j = 0; j < tabs; j++) {
				ss << "\t";
			}
		}
		{
			std::stringstream ss2;
			ss2 << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int) (data[i]);
			ss << ss2.str() << " ";
		}
	}

	if (i % 16 == 0) {
		ss << "\n";
	}

	return ss.str();
}

void print_code(const uint8_t* data, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++) {
		if (i % 16 == 0) {
			printf("\n");
		}
		printf("%02X ", data[i]);
	}

	if (i % 16 == 0) {
		printf("\n");
	}
	printf("\n");
}

bool validate_code(const uint8_t* data, size_t size)
{
	/* Valid Byzanthium opcodes */
	static uint8_t opcodes[] = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x20,
			0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
			0x3C, 0x3D, 0x3E, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x50, 0x51, 0x52,
			0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x60, 0x61, 0x62,
			0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
			0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
			0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
			0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92,
			0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E,
			0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xFB,
			0xFD, 0xFA};
	static std::set<uint8_t> opcodes_set(opcodes, opcodes + sizeof(opcodes));

	while (size) {
		if (opcodes_set.count(data[0])) {
			if (data[0] >= 0x60 && data[0] <= 0x7F) {
				/* PUSH1..31 opcode */

				/* Calculate number of bytes it needs */
				const size_t num_bytes = data[0] - 0x5F;

				/* More than available? */
				if (num_bytes > size - 1) {
					return false;
				}

				/* Advance by the number of bytes PUSH needs */
				data += num_bytes;
				size -= num_bytes;
			}
		} else {
			/* Invalid opcode */
			return false;
		}

		/* Advance to the next opcode */
		data++;
		size--;
	}

	return true;
}

void print_trace_header(const VMEnv* vmEnv)
{
	const auto prestate = vmEnv->GetPrestate();
	const auto execAccount = prestate->GetExecAccount();

	std::cout << "VM configuration:" << std::endl;
	std::cout << "\t" << "gas: " << vmEnv->GetGas() << std::endl;
	std::cout << "\t" << "block number: " << vmEnv->GetBlockNumber() << std::endl;
	std::cout << "\t" << "timestamp: " << vmEnv->GetTimestamp() << std::endl;
	std::cout << "\t" << "gas limit: " << vmEnv->GetGasLimit() << std::endl;
	std::cout << "\t" << "difficulty: " << vmEnv->GetDifficulty() << std::endl;
	std::cout << "\t" << "gas price: " << vmEnv->GetGasPrice() << std::endl;
	std::cout << std::endl;

	std::cout << "\t" << "Execution account: " << std::endl;
	std::cout << "\t\t" << "Address: " << execAccount->GetAddressHexString() << std::endl;
	std::cout << "\t\t" << "Balance: " << execAccount->GetBalanceString() << std::endl;
	std::cout << "\t\t" << "Code: " << std::endl << bufferToString(execAccount->GetCode(), 3) << std::endl;
	std::cout << "\t\t" << "Input: " << std::endl << bufferToString(prestate->GetInput(), 3) << std::endl;

	std::cout << "\t" << "Other accounts: " << std::endl;

	prestate->ForEach([&](Account* curAccount) {
		std::cout << "\t\t" << "Address: " << curAccount->GetAddressHexString() << std::endl;
		std::cout << "\t\t" << "Balance: " << curAccount->GetBalanceString() << std::endl;
		std::cout << "\t\t" << "Code: " << std::endl << bufferToString(curAccount->GetCode(), 3) << std::endl;
		std::cout << std::endl;
	});
}

std::string dataToHexString(std::vector<uint8_t> in)
{
	std::stringstream sstream;
	if (in.size()) {
		sstream << "0x";
		for (size_t i = 0; i < in.size(); i++) {
			sstream << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int) (in[i]);
		}
	}
	return sstream.str();
}


std::vector<uint8_t> hexToData(std::string in)
{
	std::vector<uint8_t> ret;
	static int hexToVal[] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	static_assert(sizeof(hexToVal) == (256 * sizeof(int)), "hexToData() hexToVal[] is not 256 bytes large");

	if ((in.size() % 2) != 0) {
		throw std::runtime_error("Invalid input length in hexToData");
	}

	if (in.size() >= 2) {
		/* If present, chop off '0x' prefix */
		if (in[0] == '0' && in[1] == 'x') {
			in = in.substr(2);
		}
	}

	ret.resize(in.size() / 2);

	for (size_t i = 0, j = 0; i < in.size(); i += 2, j++) {
		int val;

		val = hexToVal[(uint8_t) (in[i])];
		if (val < 0) {
			throw std::runtime_error("Expected hex character in hexToData");
		}
		ret[j] = ((size_t) val) << 4;

		val = hexToVal[(uint8_t) (in[i + 1])];
		if (val < 0) {
			throw std::runtime_error("Expected hex character in hexToData");
		}
		ret[j] += val;
	}

	return ret;
}

size_t sizetFromDecString(const std::string& in)
{
	std::stringstream ss;
	size_t ret;
	ss << in;
	ss >> std::hex >> ret;

	return ret;
}

std::string stringFromUint64(const uint64_t in)
{
	std::stringstream ss;
	ss << in;
	return ss.str();
}

uint64_t uint64FromHexString(std::string in)
{
	if (in.size() >= 2) {
		if (in[0] == '0' && in[1] == 'x') {
			in = in.substr(2);
		}
	}

	/*
	if ( in.size() > 16 ) {
		throw std::runtime_error("Input too large in uint64FromHexString");
	}
	*/

	if ((in.size() % 2) != 0) {
		in = std::string(1, '0').append(in);
	}

	const auto data = hexToData(in);

	if (data.size() == 0) {
		return 0;
	}

	uint64_t multiplier = 1;
	int idx = data.size() - 1;
	uint64_t ret = 0;

	while (idx >= 0) {
		ret += data[idx] * multiplier;
		multiplier *= 256;
		idx--;
	}

	return ret;
}

std::string hexStringFromUint64(uint64_t in)
{
	std::string ret;

	static auto hexChars = std::string("0123456789ABCDEF");

	while (in) {
		ret += hexChars[(uint8_t) (in & 0xF)];
		in >>= 4;
		ret += hexChars[(uint8_t) (in & 0xF)];
		in >>= 4;
	}

	std::reverse(ret.begin(), ret.end());

	return ret;
}
