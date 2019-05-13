#pragma once

#include "vmenv.h"
#include <stdint.h>
#include <vector>

class Writer
{
private:
	const VMEnv* vmEnv;

	bool vectorToBytes(std::vector<uint8_t>& out, const std::vector<uint8_t>& in) const;

	bool singleAccountToBytes(std::vector<uint8_t>& out, const Account* curAccount) const;

	bool accountsToBytes(std::vector<uint8_t>& out) const;

public:
	Writer(const VMEnv* vmEnv);

	bool ToBytes(std::vector<uint8_t>& out) const;
};